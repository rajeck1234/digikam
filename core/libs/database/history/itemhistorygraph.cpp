/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-23
 * Description : Graph data class for image history
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemhistorygraph.h"

// Local includes

#include "digikam_debug.h"
#include "dimagehistory.h"
#include "itemscanner.h"
#include "itemhistorygraphdata.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemHistoryGraphDataSharedNull : public QSharedDataPointer<ItemHistoryGraphData>
{
public:

    ItemHistoryGraphDataSharedNull()
        : QSharedDataPointer<ItemHistoryGraphData>(new ItemHistoryGraphData)
    {
    }
};

Q_GLOBAL_STATIC(ItemHistoryGraphDataSharedNull, imageHistoryGraphDataSharedNull)

// -----------------------------------------------------------------------------------------------

ItemInfo HistoryVertexProperties::firstItemInfo() const
{
    if (infos.isEmpty())
    {
        return ItemInfo();
    }

    return infos.first();
}

bool HistoryVertexProperties::markedAs(HistoryImageId::Type type) const
{
    if (referredImages.isEmpty())
    {
        return false;
    }

    Q_FOREACH (const HistoryImageId& ref, referredImages)
    {
        if (ref.m_type == type)
        {   // cppcheck-suppress useStlAlgorithm
            return true;
        }
    }

    return false;
}

bool HistoryVertexProperties::alwaysMarkedAs(HistoryImageId::Type type) const
{
    if (referredImages.isEmpty())
    {
        return false;
    }

    Q_FOREACH (const HistoryImageId& ref, referredImages)
    {
        if (ref.m_type != type)
        {   // cppcheck-suppress useStlAlgorithm
            return false;
        }
    }

    return true;
}

bool HistoryVertexProperties::operator==(const QString& id) const
{
    return uuid == id;
}

bool HistoryVertexProperties::operator==(const ItemInfo& info) const
{
    return infos.contains(info);
}

bool HistoryVertexProperties::operator==(qlonglong id) const
{
    Q_FOREACH (const ItemInfo& info, infos)
    {
        if (info.id() == id)
        {   // cppcheck-suppress useStlAlgorithm
            return true;
        }
    }

    return false;
}

bool HistoryVertexProperties::operator==(const HistoryImageId& other) const
{
    if (!uuid.isEmpty() && !other.m_uuid.isEmpty())
    {
        return (uuid == other.m_uuid);
    }

    Q_FOREACH (const HistoryImageId& id, referredImages)
    {
        if (ItemScanner::sameReferredImage(id, other))
        {   // cppcheck-suppress useStlAlgorithm
/*
            qCDebug(DIGIKAM_DATABASE_LOG) << id << "is the same as" << other;
*/
            return true;
        }
    }

    return false;
}

HistoryVertexProperties& HistoryVertexProperties::operator+=(const QString& id)
{
    if (!uuid.isNull() && id.isNull())
    {
        uuid = id;
    }

    return *this;
}

HistoryVertexProperties& HistoryVertexProperties::operator+=(const ItemInfo& info)
{
    if (!info.isNull() && !infos.contains(info))
    {
        infos << info;

        if (uuid.isNull())
        {
            uuid = info.uuid();
        }
    }

    return *this;
}

HistoryVertexProperties& HistoryVertexProperties::operator+=(const HistoryImageId& id)
{
    if (id.isValid() && !referredImages.contains(id))
    {
        referredImages << id;

        if (uuid.isNull() && !id.m_uuid.isEmpty())
        {
            uuid = id.m_uuid;
        }
    }

    return *this;
}

QDebug operator<<(QDebug dbg, const HistoryVertexProperties& props)
{
    Q_FOREACH (const ItemInfo& info, props.infos)
    {
        dbg.space() << info.id();
    }

    return dbg;
}

QDebug operator<<(QDebug dbg, const HistoryImageId& id)
{
    dbg.nospace() << " { ";
    dbg.nospace() << id.m_uuid;
    dbg.space()   << id.m_type;
    dbg.space()   << id.m_fileName;
    dbg.space()   << id.m_filePath;
    dbg.space()   << id.m_creationDate;
    dbg.space()   << id.m_uniqueHash;
    dbg.space()   << id.m_fileSize;
    dbg.space()   << id.m_originalUUID;
    dbg.nospace() << " } ";

    return dbg;
}

// -----------------------------------------------------------------------------------------------

HistoryEdgeProperties& HistoryEdgeProperties::operator+=(const FilterAction& action)
{
    actions << action;

    return *this;
}

// -----------------------------------------------------------------------------------------------

HistoryGraph::Vertex ItemHistoryGraphData::addVertex(const QList<HistoryImageId>& imageIds)
{
    if (imageIds.isEmpty())
    {
        return Vertex();
    }

    Vertex v = addVertex(imageIds.first());

    if (imageIds.size() > 1)
    {
        applyProperties(v, QList<ItemInfo>(), imageIds);
    }

    return v;
}

HistoryGraph::Vertex ItemHistoryGraphData::addVertex(const HistoryImageId& imageId)
{
    if (!imageId.isValid())
    {
        return Vertex();
    }

    Vertex v;
    QList<ItemInfo> infos;
/*
    qCDebug(DIGIKAM_DATABASE_LOG) << "Adding vertex" << imageId.m_uuid.left(6) << imageId.fileName();
*/
    // find by HistoryImageId (most notably, by UUID)

    v = findVertexByProperties(imageId);
/*
    qCDebug(DIGIKAM_DATABASE_LOG) << "Found by properties:" << (v.isNull() ? -1 : int(v));
*/
    if (v.isNull())
    {
        // Resolve HistoryImageId, find by ItemInfo

        Q_FOREACH (const qlonglong& id, ItemScanner::resolveHistoryImageId(imageId))
        {
            ItemInfo info(id);
/*
            qCDebug(DIGIKAM_DATABASE_LOG) << "Found info id:" << info.id();
*/
            infos << info;

            v = findVertexByProperties(info);
        }
    }

    applyProperties(v, infos, QList<HistoryImageId>() << imageId);
/*
    qCDebug(DIGIKAM_DATABASE_LOG) << "Returning vertex" << v;
*/
    return v;
}

HistoryGraph::Vertex ItemHistoryGraphData::addVertex(qlonglong id)
{
    return addVertex(ItemInfo(id));
}

HistoryGraph::Vertex ItemHistoryGraphData::addVertex(const ItemInfo& info)
{
    Vertex         v;
    QString        uuid;
    HistoryImageId id;

    // Simply find by image id

    v = findVertexByProperties(info);
/*
    qCDebug(DIGIKAM_DATABASE_LOG) << "Find by id" << info.id() << ": found" << v.isNull();
*/
    if (v.isNull())
    {
        // Find by contents

        uuid = info.uuid();

        if (!uuid.isNull())
        {
            v = findVertexByProperties(uuid);
        }
/*
        qCDebug(DIGIKAM_DATABASE_LOG) << "Find by uuid" << uuid << ": found" << v.isNull();
*/
        if (v.isNull())
        {
            id = info.historyImageId();
            v  = findVertexByProperties(id);
/*
            qCDebug(DIGIKAM_DATABASE_LOG) << "Find by h-i-m" << ": found" << v.isNull();
*/
        }

        // Need to add new vertex. Do this through the method which will resolve the history id

        if (v.isNull())
        {
            v = addVertex(id);
        }
    }

    applyProperties(v, QList<ItemInfo>() << info, QList<HistoryImageId>() << id);
/*
    qCDebug(DIGIKAM_DATABASE_LOG) << "Returning vertex" << v << properties(v).infos.size();
*/
    return v;
}

HistoryGraph::Vertex ItemHistoryGraphData::addVertexScanned(qlonglong id)
{
    // short version where we do not read information about id from an ItemInfo

    Vertex v = findVertexByProperties(id);

    applyProperties(v, QList<ItemInfo>() << ItemInfo(id), QList<HistoryImageId>());

    return v;
}

void ItemHistoryGraphData::applyProperties(Vertex& v,
                                           const QList<ItemInfo>& infos,
                                           const QList<HistoryImageId>& ids)
{
    // if needed, add a new vertex; or retrieve properties to add possibly new entries

    if (v.isNull())
    {
        v = HistoryGraph::addVertex();
    }

    HistoryVertexProperties& props = properties(v);

    // adjust properties

    Q_FOREACH (const ItemInfo& info, infos)
    {
        // cppcheck-suppress useStlAlgorithm
        props += info;
    }

    Q_FOREACH (const HistoryImageId& id, ids)
    {
        // cppcheck-suppress useStlAlgorithm
        props += id;
    }
}

int ItemHistoryGraphData::removeNextUnresolvedVertex(int index)
{
    QList<Vertex> vs = vertices();       // clazy:exclude=missing-typeinfo

    for ( ; index < vs.size() ; ++index)
    {
        Vertex& v = vs[index];
        const HistoryVertexProperties& props = properties(v);

        if (props.infos.isEmpty())
        {
            Q_FOREACH (const HistoryGraph::Edge& upperEdge, edges(v, HistoryGraph::EdgesToRoot))
            {
                Q_FOREACH (const HistoryGraph::Edge& lowerEdge, edges(v, HistoryGraph::EdgesToLeaf))
                {
                    HistoryEdgeProperties combinedProps;
                    combinedProps.actions        += properties(upperEdge).actions;
                    combinedProps.actions        += properties(lowerEdge).actions;
                    HistoryGraph::Edge connection = addEdge(source(lowerEdge), target(upperEdge));
                    setProperties(connection, combinedProps);
                }
            }

            remove(v);

            return index;
        }
    }

    return index;
}

QHash<HistoryGraph::Vertex, HistoryImageId::Types> ItemHistoryGraphData::categorize() const
{
    QHash<Vertex, HistoryImageId::Types> types;

    Q_FOREACH (const Vertex& v, vertices())
    {
        const HistoryVertexProperties& props = properties(v);

        HistoryImageId::Types type;

        if      (props.alwaysMarkedAs(HistoryImageId::Source))
        {
            type |= HistoryImageId::Source;
        }
        else if (isLeaf(v))
        {
            // Leaf: Assume current version

            type |= HistoryImageId::Current;
        }
        else if (isRoot(v))
        {
            // Root: Assume original if at least once marked as such

            if (props.markedAs(HistoryImageId::Original))
            {
                type |= HistoryImageId::Original;
            }
        }
        else
        {
            type = HistoryImageId::Intermediate;
        }

        /*
         * There is one situation which cannot be deduced from the graph structure:
         * When there is a derived image, but the parent image shall be kept as visible "Current".
         * In this case, the ExplicitBranch flag can be set on the next action.
         */
        if (!(type & HistoryImageId::Current) && hasEdges(v, EdgesToLeaf))
        {
            // We check if all immediate actions set the ExplicitBranch flag

            bool allBranches = true;

            Q_FOREACH (const Edge& e, edges(v, EdgesToLeaf))
            {
                const HistoryEdgeProperties& props2 = properties(e);

                if (props2.actions.isEmpty())
                {
                    continue; // unclear situation, ignore
                }

                if (!(props2.actions.first().flags() & FilterAction::ExplicitBranch))
                {
                    allBranches = false;
                    break;
                }
            }

            if (allBranches)
            {
                type |= HistoryImageId::Current;
            }
        }

        types[v] = type;
    }

    return types;
}

// -----------------------------------------------------------------------------------------------

ItemHistoryGraph::ItemHistoryGraph()
    : d(*imageHistoryGraphDataSharedNull)
{
}

ItemHistoryGraph::ItemHistoryGraph(const ItemHistoryGraph& other)
    : d(other.d)
{
}

ItemHistoryGraph::~ItemHistoryGraph()
{
}

ItemHistoryGraph& ItemHistoryGraph::operator=(const ItemHistoryGraph& other)
{
    d = other.d;

    return *this;
}

bool ItemHistoryGraph::isNull() const
{
    return (d == *imageHistoryGraphDataSharedNull);
}

bool ItemHistoryGraph::isEmpty() const
{
    return d->isEmpty();
}

bool ItemHistoryGraph::isSingleVertex() const
{
    return (d->vertexCount() == 1);
}

bool ItemHistoryGraph::hasEdges() const
{
    return d->hasEdges();
}

ItemHistoryGraphData& ItemHistoryGraph::data()
{
    return *d;
}

const ItemHistoryGraphData& ItemHistoryGraph::data() const
{
    return *d;
}

void ItemHistoryGraph::clear()
{
    *d = HistoryGraph();
}

ItemHistoryGraph ItemHistoryGraph::fromInfo(const ItemInfo& info,
                                            HistoryLoadingMode loadingMode,
                                            ProcessingMode processingMode)
{
    ItemHistoryGraph graph;

    if (loadingMode & LoadRelationCloud)
    {
        graph.addRelations(info.relationCloud());
    }

    if (loadingMode & LoadSubjectHistory)
    {
        graph.addHistory(info.imageHistory(), info);
    }

    if (loadingMode & LoadLeavesHistory)
    {
        Q_FOREACH (const ItemInfo& leaf, graph.leafImages())
        {
            if (leaf != info)
            {
                graph.addHistory(leaf.imageHistory(), leaf);
            }
        }
    }

    if (processingMode == PrepareForDisplay)
    {
        graph.prepareForDisplay(info);
    }

    return graph;
}

void ItemHistoryGraph::addHistory(const DImageHistory& givenHistory, const ItemInfo& historySubject)
{
    addHistory(givenHistory, historySubject.historyImageId());
}

void ItemHistoryGraph::addHistory(const DImageHistory& givenHistory, const HistoryImageId& subjectId)
{
    // append the subject to its history

    DImageHistory history = givenHistory;

    if (subjectId.isValid())
    {
        history << subjectId;
    }

    d->addHistory(history);
}

void ItemHistoryGraph::addScannedHistory(const DImageHistory& history, qlonglong historySubjectId)
{
    d->addHistory(history, historySubjectId);
}

void ItemHistoryGraphData::addHistory(const DImageHistory& history, qlonglong extraCurrent)
{
    if (history.isEmpty())
    {
        return;
    }

    HistoryGraph::Vertex  last;
    HistoryEdgeProperties edgeProps;

    Q_FOREACH (const DImageHistory::Entry& entry, history.entries())
    {
        if (!last.isNull())
        {
            edgeProps += entry.action;
        }

        HistoryGraph::Vertex v;

        if (!entry.referredImages.isEmpty())
        {
            v = addVertex(entry.referredImages);
        }

        if (!v.isNull())
        {
            if (!last.isNull())
            {
                if (v != last)
                {
                    HistoryGraph::Edge e = addEdge(v, last);
                    setProperties(e, edgeProps);
                    edgeProps            = HistoryEdgeProperties();
                }
                else
                {
                    qCWarning(DIGIKAM_DATABASE_LOG) << "Broken history: Same file referred by different entries. Refusing to add a loop.";
                }
            }

            last = v;
        }
    }

    if (extraCurrent)
    {
        HistoryGraph::Vertex v = addVertexScanned(extraCurrent);

        if (!v.isNull() && !last.isNull() && (v != last))
        {
            HistoryGraph::Edge e = addEdge(v, last);
            setProperties(e, edgeProps);
        }
    }
}

void ItemHistoryGraph::addRelations(const QList<QPair<qlonglong, qlonglong> >& pairs)
{
    HistoryGraph::Vertex v1, v2;
    typedef QPair<qlonglong, qlonglong> IdPair;

    Q_FOREACH (const IdPair& pair, pairs)
    {
        if ((pair.first < 1) || (pair.second < 1))
        {
            continue;
        }

        if (pair.first == pair.second)
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "Broken relations cloud: Refusing to add a loop.";
        }

        v1 = d->addVertex(pair.first);
        v2 = d->addVertex(pair.second);
/*
        qCDebug(DIGIKAM_DATABASE_LOG) << "Adding" << v1 << "->" << v2;
*/
        d->addEdge(v1, v2);
    }
}

void ItemHistoryGraph::reduceEdges()
{
    if (d->vertexCount() <= 1)
    {
        return;
    }

    QList<HistoryGraph::Edge> removedEgdes;
    HistoryGraph reduction = d->transitiveReduction(&removedEgdes);

    if (reduction.isEmpty())
    {
        return;    // reduction failed, not a DAG
    }

    Q_FOREACH (const HistoryGraph::Edge& e, removedEgdes)
    {
        if (!d->properties(e).actions.isEmpty())
        {
            // TODO: conflict resolution

            qCDebug(DIGIKAM_DATABASE_LOG) << "Conflicting history information: Edge removed by transitiveReduction is not empty.";
        }
    }

    *d = reduction;
}

bool ItemHistoryGraph::hasUnresolvedEntries() const
{
    Q_FOREACH (const HistoryGraph::Vertex& v, d->vertices())
    {
        if (d->properties(v).infos.isEmpty())
        {
            return true;
        }
    }

    return false;
}

void ItemHistoryGraph::dropUnresolvedEntries()
{
    // Remove nodes which could not be resolved into image infos

    // The problem is that with each removable, the vertex list is invalidated,
    // so we cannot do one loop over d->vertices

    for (int i = 0 ; i < d->vertexCount() ; )
    {
        i = d->removeNextUnresolvedVertex(i);
    }
}

void ItemHistoryGraph::sortForInfo(const ItemInfo& subject)
{
    // Remove nodes which could not be resolved into image infos

    QList<HistoryGraph::Vertex> toRemove;   // clazy:exclude=missing-typeinfo

    Q_FOREACH (const HistoryGraph::Vertex& v, d->vertices())
    {
        HistoryVertexProperties& props = d->properties(v);
        ItemScanner::sortByProximity(props.infos, subject);
    }
}

void ItemHistoryGraph::prepareForDisplay(const ItemInfo& subject)
{
    reduceEdges();
    dropUnresolvedEntries();
    sortForInfo(subject);
}

QList<QPair<qlonglong, qlonglong> > ItemHistoryGraph::relationCloud() const
{
    QList<QPair<qlonglong, qlonglong> > pairs;
    ItemHistoryGraphData closure          = ItemHistoryGraphData(d->transitiveClosure());
    QList<HistoryGraph::VertexPair> edges = closure.edgePairs();

    Q_FOREACH (const HistoryGraph::VertexPair& edge, edges)
    {
        Q_FOREACH (const ItemInfo& source, closure.properties(edge.first).infos)
        {
            Q_FOREACH (const ItemInfo& target, closure.properties(edge.second).infos)
            {
                pairs << QPair<qlonglong, qlonglong>(source.id(), target.id());
            }
        }
    }

    return pairs;
}

QPair<QList<qlonglong>, QList<qlonglong> > ItemHistoryGraph::relationCloudParallel() const
{
    QList<qlonglong> subjects, objects;
    ItemHistoryGraphData closure          = ItemHistoryGraphData(d->transitiveClosure());
    QList<HistoryGraph::VertexPair> edges = closure.edgePairs();

    Q_FOREACH (const HistoryGraph::VertexPair& edge, edges)
    {
        Q_FOREACH (const ItemInfo& source, closure.properties(edge.first).infos)
        {
            Q_FOREACH (const ItemInfo& target, closure.properties(edge.second).infos)
            {
                subjects << source.id();
                objects  << target.id();
            }
        }
    }

    return qMakePair(subjects, objects);
}

QList<ItemInfo> ItemHistoryGraph::allImages() const
{
    return d->toInfoList(d->vertices());
}

QList<qlonglong> ItemHistoryGraph::allImageIds() const
{
    QList<qlonglong> ids;

    Q_FOREACH (const HistoryGraph::Vertex& v, d->vertices())
    {
        Q_FOREACH (const ItemInfo& info, d->properties(v).infos)
        {
            ids << info.id();
        }
    }

    return ids;
}

QList<ItemInfo> ItemHistoryGraph::rootImages() const
{
    return d->toInfoList(d->roots());
}

QList<ItemInfo> ItemHistoryGraph::leafImages() const
{
    return d->toInfoList(d->leaves());
}

QHash<ItemInfo, HistoryImageId::Types> ItemHistoryGraph::categorize() const
{
    QHash<HistoryGraph::Vertex, HistoryImageId::Types> vertexType = d->categorize();

    QHash<ItemInfo, HistoryImageId::Types> types;

    Q_FOREACH (const HistoryGraph::Vertex& v, d->vertices())
    {
        const HistoryVertexProperties& props = d->properties(v);

        if (props.infos.isEmpty())
        {
            continue;
        }

        HistoryImageId::Types type = vertexType.value(v);

        Q_FOREACH (const ItemInfo& info, props.infos)
        {
            types[info] = type;
        }
    }

    return types;
}

static QString toString(const HistoryVertexProperties& props)
{
    QString s = QLatin1String("Ids: ");
    QStringList ids;

    Q_FOREACH (const ItemInfo& info, props.infos)
    {
        ids << QString::number(info.id());
    }

    if (props.uuid.isEmpty())
    {
        if (ids.size() == 1)
        {
            return (QLatin1String("Id: ") + ids.first());
        }
        else
        {
            return (QLatin1String("Ids: (") + ids.join(QLatin1String(",")) + QLatin1Char(')'));
        }
    }
    else
    {
        if (ids.size() == 1)
        {
            return (QLatin1String("Id: ") + ids.first() + QLatin1String(" UUID: ") + props.uuid.left(6) + QLatin1String("..."));
        }
        else
        {
            return (QLatin1String("Ids: (") + ids.join(QLatin1String(",")) + QLatin1String(") UUID: ") + props.uuid.left(6) + QLatin1String("..."));
        }
    }
}

QDebug operator<<(QDebug dbg, const ItemHistoryGraph& g)
{
    if (g.data().isEmpty())
    {
        dbg << "(Empty graph)";

        return dbg;
    }

    QList<HistoryGraph::Vertex> vertices = g.data().topologicalSort();  // clazy:exclude=missing-typeinfo

    if (vertices.isEmpty())
    {
        vertices = g.data().vertices();
        dbg << "Not-a-DAG-Graph with" << vertices.size() << "vertices:" << QT_ENDL;;
    }
    else
    {
        dbg << "Graph with" << vertices.size() << "vertices:" << QT_ENDL;;
    }

    Q_FOREACH (const HistoryGraph::Vertex& target, vertices)
    {
        QString targetString = toString(g.data().properties(target));

        QStringList sourceVertexTexts;

        Q_FOREACH (const HistoryGraph::Vertex& source, g.data().adjacentVertices(target, HistoryGraph::InboundEdges))
        {
            sourceVertexTexts << toString(g.data().properties(source));
        }

        if      (!sourceVertexTexts.isEmpty())
        {
            dbg.nospace() << QLatin1String("{ ")    + targetString + QLatin1String(" } ") +
                             QLatin1String("-> { ") + sourceVertexTexts.join(QLatin1String(" }, { ")) +
                             QLatin1String(" }") << QT_ENDL;;
        }
        else if (g.data().outDegree(target) == 0)
        {
            dbg.nospace() << QLatin1String("Unconnected: { ") + targetString + QLatin1String(" }") << QT_ENDL;;
        }
    }

    return dbg;
}

} // namespace Digikam
