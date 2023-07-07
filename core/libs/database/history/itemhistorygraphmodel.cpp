/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-27
 * Description : Model to an ItemHistoryGraph
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemhistorygraphmodel.h"

// Qt includes

#include <QAbstractItemModel>
#include <QTreeWidgetItem>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dcategorizedsortfilterproxymodel.h"
#include "dimgfiltermanager.h"
#include "itemlistmodel.h"
#include "itemhistorygraphdata.h"

namespace Digikam
{

class Q_DECL_HIDDEN HistoryTreeItem
{
public:

    enum HistoryTreeItemType
    {
        UnspecifiedType,
        VertexItemType,
        FilterActionItemType,
        HeaderItemType,
        CategoryItemType,
        SeparatorItemType
    };

public:

    explicit HistoryTreeItem();
    virtual ~HistoryTreeItem();

    virtual HistoryTreeItemType type()  const
    {
        return UnspecifiedType;
    }

    bool isType(HistoryTreeItemType t)  const
    {
        return (type() == t);
    }

    void addItem(HistoryTreeItem* child);

    int childCount() const
    {
        return children.size();
    }

    HistoryTreeItem* child(int index)   const
    {
        return children.at(index);
    }

public:

    HistoryTreeItem*        parent;
    QList<HistoryTreeItem*> children;

private:

    Q_DISABLE_COPY(HistoryTreeItem)
};

// ------------------------------------------------------------------------

class Q_DECL_HIDDEN VertexItem : public HistoryTreeItem
{
public:

    VertexItem()
    {
    }

    explicit VertexItem(const HistoryGraph::Vertex& v)
        : vertex  (v),
          category(HistoryImageId::InvalidType)
    {
    }

    HistoryTreeItemType type() const override
    {
        return VertexItemType;
    }

public:

    HistoryGraph::Vertex  vertex;
    QModelIndex           index;
    HistoryImageId::Types category;

private:

    Q_DISABLE_COPY(VertexItem)
};

// ------------------------------------------------------------------------

class Q_DECL_HIDDEN FilterActionItem : public HistoryTreeItem
{
public:

    FilterActionItem()
    {
    }

    explicit FilterActionItem(const FilterAction& action)
        : action(action)
    {
    }

    HistoryTreeItemType type() const override
    {
        return FilterActionItemType;
    }

public:

    FilterAction action;

private:

    Q_DISABLE_COPY(FilterActionItem)
};

// ------------------------------------------------------------------------

class Q_DECL_HIDDEN HeaderItem : public HistoryTreeItem
{
public:

    explicit HeaderItem(const QString& title)
        : title(title)
    {
    }

    HistoryTreeItemType type() const override
    {
        return HeaderItemType;
    }

public:

    QString title;

private:

    Q_DISABLE_COPY(HeaderItem)
};

// ------------------------------------------------------------------------

class Q_DECL_HIDDEN CategoryItem : public HistoryTreeItem
{
public:

    explicit CategoryItem(const QString& title)
        : title(title)
    {
    }

    HistoryTreeItemType type() const override
    {
        return CategoryItemType;
    }

public:

    QString title;

private:

    Q_DISABLE_COPY(CategoryItem)
};

// ------------------------------------------------------------------------

class Q_DECL_HIDDEN SeparatorItem : public HistoryTreeItem
{
public:

    SeparatorItem() = default;

    HistoryTreeItemType type() const override
    {
        return SeparatorItemType;
    }

private:

    Q_DISABLE_COPY(SeparatorItem)
};

// ------------------------------------------------------------------------

#define if_isItem(class, name, pointer)                                                             \
    if (pointer && static_cast<HistoryTreeItem*>(pointer)->type() == HistoryTreeItem:: class##Type) \
        for (class* name = static_cast<class*>(pointer) ; name ; name = nullptr)

// ------------------------------------------------------------------------

HistoryTreeItem::HistoryTreeItem()
    : parent(nullptr)
{
}

HistoryTreeItem::~HistoryTreeItem()
{
    qDeleteAll(children);
}

void HistoryTreeItem::addItem(HistoryTreeItem* child)
{
    children << child;
    child->parent = this;
}

// ------------------------------------------------------------------------

static bool oldestInfoFirst(const ItemInfo&a, const ItemInfo& b)
{
    return (a.modDateTime() < b.modDateTime());
}

static bool newestInfoFirst(const ItemInfo&a, const ItemInfo& b)
{
    return (a.modDateTime() > b.modDateTime());
}

template <typename ItemInfoLessThan>

class Q_DECL_HIDDEN LessThanOnVertexItemInfo
{
public:

    LessThanOnVertexItemInfo(const HistoryGraph& graph, ItemInfoLessThan imageInfoLessThan)
        : graph            (graph),
          imageInfoLessThan(imageInfoLessThan)
    {
    }

    bool operator()(const HistoryGraph::Vertex& a, const HistoryGraph::Vertex& b) const
    {
        const HistoryVertexProperties& propsA = graph.properties(a);
        const HistoryVertexProperties& propsB = graph.properties(b);

        if      (propsA.infos.isEmpty())
        {
            return false;
        }
        else if (propsB.infos.isEmpty())
        {
            return true;
        }

        return imageInfoLessThan(propsA.infos.first(), propsB.infos.first());
    }

public:

    const HistoryGraph& graph;
    ItemInfoLessThan    imageInfoLessThan;
};

// ------------------------------------------------------------------------

class Q_DECL_HIDDEN ItemHistoryGraphModel::Private
{
public:

    explicit Private()
        : mode    (ItemHistoryGraphModel::CombinedTreeMode),
          rootItem(nullptr)
    {
    }

    ItemHistoryGraphModel::Mode                        mode;

    ItemHistoryGraph                                   historyGraph;
    ItemInfo                                           info;

    HistoryTreeItem*                                   rootItem;
    QList<VertexItem*>                                 vertexItems;
    ItemListModel                                      imageModel;
    QList<HistoryGraph::Vertex>                        path;
    QHash<HistoryGraph::Vertex, HistoryImageId::Types> categories;

public:

    inline const ItemHistoryGraphData& graph() const
    {
        return historyGraph.data();
    }

    inline HistoryTreeItem* historyItem(const QModelIndex& index) const
    {
        return (index.isValid() ? static_cast<HistoryTreeItem*>(index.internalPointer())
                                : rootItem);
    }

    void build();
    void buildImagesList();
    void buildImagesTree();
    void buildCombinedTree(const HistoryGraph::Vertex& ref);
    void addCombinedItemCategory(HistoryTreeItem* parentItem, QList<HistoryGraph::Vertex>& vertices,
                                 const QString& title, const HistoryGraph::Vertex& showActionsFrom,
                                 QList<HistoryGraph::Vertex>& added);
    void addItemSubgroup(VertexItem* parent, const QList<HistoryGraph::Vertex>& vertices, const QString& title, bool flat = false);
    void addIdenticalItems(HistoryTreeItem* parentItem, const HistoryGraph::Vertex& vertex,
                           const QList<ItemInfo>& infos, const QString& title);

    VertexItem* createVertexItem(const HistoryGraph::Vertex& v, const ItemInfo& info = ItemInfo());
    FilterActionItem* createFilterActionItem(const FilterAction& action);

    template <typename ItemInfoLessThan> LessThanOnVertexItemInfo<ItemInfoLessThan>

    sortBy(ItemInfoLessThan imageInfoLessThan)
    {
        return LessThanOnVertexItemInfo<ItemInfoLessThan>(graph(), imageInfoLessThan);
    }
};

// ------------------------------------------------------------------------

VertexItem* ItemHistoryGraphModel::Private::createVertexItem(const HistoryGraph::Vertex& v,
                                                             const ItemInfo& givenInfo)
{
    const HistoryVertexProperties& props = graph().properties(v);
    ItemInfo info                        = givenInfo.isNull() ? props.firstItemInfo() : givenInfo;
    QModelIndex index                    = imageModel.indexForItemInfo(info);
/*
    qCDebug(DIGIKAM_DATABASE_LOG) << "Added" << info.id() << index;
*/
    VertexItem* item                     = new VertexItem(v);
    item->index                          = index;
    item->category                       = categories.value(v);
    vertexItems << item;
/*
    qCDebug(DIGIKAM_DATABASE_LOG) << "Adding vertex item" << graph().properties(v).firstItemInfo().id() << index;
*/
    return item;
}

FilterActionItem* ItemHistoryGraphModel::Private::createFilterActionItem(const FilterAction& action)
{
/*
    qCDebug(DIGIKAM_DATABASE_LOG) << "Adding vertex item for" << action.displayableName();
*/
    return new FilterActionItem(action);
}

void ItemHistoryGraphModel::Private::build()
{
    delete rootItem;
    vertexItems.clear();
    rootItem = new HistoryTreeItem;
/*
    qCDebug(DIGIKAM_DATABASE_LOG) << historyGraph;
*/
    HistoryGraph::Vertex ref = graph().findVertexByProperties(info);
    path                     = graph().longestPathTouching(ref, sortBy(newestInfoFirst));
    categories               = graph().categorize();

    if (path.isEmpty())
    {
        return;
    }

    if      (mode == ItemHistoryGraphModel::ImagesListMode)
    {
        buildImagesList();
    }
    else if (mode == ItemHistoryGraphModel::ImagesTreeMode)
    {
        buildImagesTree();
    }
    else if (mode == CombinedTreeMode)
    {
        buildCombinedTree(ref);
    }
}

void ItemHistoryGraphModel::Private::buildImagesList()
{
    QList<HistoryGraph::Vertex> verticesOrdered = graph().verticesDepthFirstSorted(path.first(), sortBy(oldestInfoFirst));  // clazy:exclude=missing-typeinfo

    Q_FOREACH (const HistoryGraph::Vertex& v, verticesOrdered)
    {
        rootItem->addItem(createVertexItem(v));
    }
}

void ItemHistoryGraphModel::Private::buildImagesTree()
{
    QList<HistoryGraph::Vertex> verticesOrdered = graph().verticesDepthFirstSorted(path.first(), sortBy(oldestInfoFirst));  // clazy:exclude=missing-typeinfo

    QMap<HistoryGraph::Vertex, int> distances   = graph().shortestDistancesFrom(path.first());                              // clazy:exclude=missing-typeinfo


    QList<HistoryGraph::Vertex> sources;                            // clazy:exclude=missing-typeinfo
    int previousLevel                           = 0;
    HistoryTreeItem* parent                     = rootItem;
    VertexItem* item                            = nullptr;
    VertexItem* previousItem                    = nullptr;

    Q_FOREACH (const HistoryGraph::Vertex& v, verticesOrdered)
    {
        int currentLevel = distances.value(v);

        if (currentLevel == -1)
        {
            // unreachable from first root

            if (graph().isRoot(v) && (parent == rootItem))
            {
                // other first-level root?

                parent->addItem(createVertexItem(v));
            }
            else
            {
                // add later as sources

                sources << v;
            }

            continue;
        }

        item = createVertexItem(v);

        if (!sources.isEmpty())
        {
            addItemSubgroup(item, sources, i18nc("@title", "Source Images"));
        }

        if      (currentLevel == previousLevel)
        {
            parent->addItem(item);
        }
        else if (currentLevel > previousLevel && previousItem) // check pointer, prevent crash is distances are faulty
        {
            previousItem->addItem(item);
            parent = previousItem;
        }
        else if (currentLevel < previousLevel)
        {
            for (int level = currentLevel ; level < previousLevel ; ++level)
            {
                parent = parent->parent;
            }
            parent->addItem(item);
        }

        previousItem  = item;
        previousLevel = currentLevel;
    }
}

void ItemHistoryGraphModel::Private::buildCombinedTree(const HistoryGraph::Vertex& ref)
{
    VertexItem* item                            = nullptr;
    CategoryItem *categoryItem                  = new CategoryItem(i18nc("@title", "Image History"));
    rootItem->addItem(categoryItem);

    QList<HistoryGraph::Vertex> added;                                                          // clazy:exclude=missing-typeinfo
    QList<HistoryGraph::Vertex> currentVersions = categories.keys(HistoryImageId::Current);     // clazy:exclude=missing-typeinfo
    QList<HistoryGraph::Vertex> leavesFromRef   = graph().leavesFrom(ref);                      // clazy:exclude=missing-typeinfo

    bool onePath = (leavesFromRef.size() <= 1);

    for (int i = 0 ; i < path.size() ; ++i)
    {
        const HistoryGraph::Vertex& v       = path.at(i);
        HistoryGraph::Vertex previous       = i ? path.at(i-1) : HistoryGraph::Vertex();
/*
        HistoryGraph::Vertex next           = i < path.size() - 1 ? path[i+1] : HistoryGraph::Vertex();
*/
/*
        qCDebug(DIGIKAM_DATABASE_LOG) << "Vertex on path" << path[i];
*/
        // create new item

        item                                = createVertexItem(v);

        QList<HistoryGraph::Vertex> vertices;       // clazy:exclude=missing-typeinfo

        // any extra sources?

        QList<HistoryGraph::Vertex> sources = graph().adjacentVertices(item->vertex, HistoryGraph::EdgesToRoot);    // clazy:exclude=missing-typeinfo

        Q_FOREACH (const HistoryGraph::Vertex& source, sources)
        {
            if (source != previous)
            {
                rootItem->addItem(createVertexItem(source));
            }
        }

/*
        // Any other egdes off the main path?

        QList<HistoryGraph::Vertex> branches = graph().adjacentVertices(v, HistoryGraph::EdgesToLeaf);
        QList<HistoryGraph::Vertex> subgraph;

        Q_FOREACH (const HistoryGraph::Vertex& branch, branches)
        {
            if (branch != next)
            {
                subgraph << graph().verticesDominatedByDepthFirstSorted(branch, v, sortBy(oldestInfoFirst));
            }
        }

        addItemSubgroup(item, subgraph, i18nc("@title", "More Derived Images"));
*/

        // Add filter actions above item

        HistoryEdgeProperties props = graph().properties(v, previous);

        Q_FOREACH (const FilterAction& action, props.actions)
        {
            rootItem->addItem(createFilterActionItem(action));
        }

        // now, add item

        rootItem->addItem(item);
        added << v;

        // If there are multiple derived images, we display them in the next section

        if ((v == ref) && !onePath)
        {
            break;
        }
    }

    Q_FOREACH (const HistoryGraph::Vertex& v, added)
    {
        leavesFromRef.removeOne(v);
    }

    if (!leavesFromRef.isEmpty())
    {
        addCombinedItemCategory(rootItem, leavesFromRef, i18nc("@title", "Derived Images"), ref, added);
    }

    Q_FOREACH (const HistoryGraph::Vertex& v, added)
    {
        currentVersions.removeOne(v);
    }

    if (!currentVersions.isEmpty())
    {
        addCombinedItemCategory(rootItem, currentVersions, i18nc("@title", "Related Images"), path.first(), added);
    }

    QList<ItemInfo> allInfos = graph().properties(ref).infos;

    if (allInfos.size() > 1)
    {
        addIdenticalItems(rootItem, ref, allInfos, i18nc("@title", "Identical Images"));
    }
}

void ItemHistoryGraphModel::Private::addCombinedItemCategory(HistoryTreeItem* parentItem,
                                                             QList<HistoryGraph::Vertex>& vertices,
                                                             const QString& title,
                                                             const HistoryGraph::Vertex& showActionsFrom,
                                                             QList<HistoryGraph::Vertex>& added)
{
    parentItem->addItem(new CategoryItem(title));

    std::sort(vertices.begin(), vertices.end(), sortBy(oldestInfoFirst));
    bool isFirst     = true;
    VertexItem* item = nullptr;

    Q_FOREACH (const HistoryGraph::Vertex& v, vertices)
    {
        if (isFirst)
        {
            isFirst = false;
        }
        else
        {
            parentItem->addItem(new SeparatorItem);
        }

        item                                     = createVertexItem(v);

        QList<HistoryGraph::Vertex> shortestPath = graph().shortestPath(showActionsFrom, v);     // clazy:exclude=missing-typeinfo

        // add all filter actions showActionsFrom -> v above item

        for (int i = 1 ; i < shortestPath.size() ; ++i)
        {
            HistoryEdgeProperties props = graph().properties(shortestPath.at(i), shortestPath.at(i-1));

            Q_FOREACH (const FilterAction& action, props.actions)
            {
                parentItem->addItem(createFilterActionItem(action));
            }
        }

        parentItem->addItem(item);
        added << v;

        // Provide access to intermediates

        shortestPath.removeOne(showActionsFrom);
        shortestPath.removeOne(v);

        Q_FOREACH (const HistoryGraph::Vertex& addedVertex, added)
        {
            shortestPath.removeOne(addedVertex);
        }

        addItemSubgroup(item, shortestPath, i18nc("@title", "Intermediate Steps:"), true);
    }
}

void ItemHistoryGraphModel::Private::addItemSubgroup(VertexItem* parent,
                                                     const QList<HistoryGraph::Vertex>& vertices,
                                                     const QString& title,
                                                     bool flat)
{
    if (vertices.isEmpty())
    {
        return;
    }

    HeaderItem* const header         = new HeaderItem(title);
    parent->addItem(header);
    HistoryTreeItem* const addToItem = flat ? static_cast<HistoryTreeItem*>(parent) : static_cast<HistoryTreeItem*>(header);

    Q_FOREACH (const HistoryGraph::Vertex& v, vertices)
    {
        addToItem->addItem(createVertexItem(v));
    }
}

void ItemHistoryGraphModel::Private::addIdenticalItems(HistoryTreeItem* parentItem,
                                                       const HistoryGraph::Vertex& vertex,
                                                       const QList<ItemInfo>& infos,
                                                       const QString& title)
{
    parentItem->addItem(new CategoryItem(title));

    // the properties image info list is already sorted by proximity to subject

    VertexItem* item = nullptr;
    bool isFirst     = true;

    for (int i = 1 ; i < infos.size() ; ++i)
    {
        if (isFirst)
        {
            isFirst = false;
        }
        else
        {
            parentItem->addItem(new SeparatorItem);
        }

        item = createVertexItem(vertex, infos.at(i));
        parentItem->addItem(item);
    }
}

// ------------------------------------------------------------------------

ItemHistoryGraphModel::ItemHistoryGraphModel(QObject* const parent)
    : QAbstractItemModel(parent),
      d                 (new Private)
{
    d->rootItem = new HistoryTreeItem;
}

ItemHistoryGraphModel::~ItemHistoryGraphModel()
{
    delete d->rootItem;
    delete d;
}

void ItemHistoryGraphModel::setMode(Mode mode)
{
    if (d->mode == mode)
    {
        return;
    }

    d->mode = mode;
    setHistory(d->info, d->historyGraph);
}

ItemHistoryGraphModel::Mode ItemHistoryGraphModel::mode() const
{
    return d->mode;
}

void ItemHistoryGraphModel::setHistory(const ItemInfo& subject, const ItemHistoryGraph& graph)
{
    beginResetModel();

    d->info = subject;

    if (graph.isNull())
    {
        d->historyGraph = ItemHistoryGraph::fromInfo(subject);
    }
    else
    {
        d->historyGraph = graph;
        d->historyGraph.prepareForDisplay(subject);
    }

    // fill helper model

    d->imageModel.clearItemInfos();
    d->imageModel.addItemInfos(d->historyGraph.allImages());

    d->build();

    endResetModel();
}

ItemInfo ItemHistoryGraphModel::subject() const
{
    return d->info;
}

bool ItemHistoryGraphModel::isImage(const QModelIndex& index) const
{
    HistoryTreeItem* const item = d->historyItem(index);

    return (item && item->isType(HistoryTreeItem::VertexItemType));
}

bool ItemHistoryGraphModel::isFilterAction(const QModelIndex& index) const
{
    HistoryTreeItem* const item = d->historyItem(index);

    return (item && item->isType(HistoryTreeItem::FilterActionItemType));
}

FilterAction ItemHistoryGraphModel::filterAction(const QModelIndex& index) const
{
    HistoryTreeItem* const item = d->historyItem(index);

    if_isItem(FilterActionItem, filterActionItem, item)
    {
        return filterActionItem->action;
    }

    return FilterAction();
}

bool ItemHistoryGraphModel::hasImage(const ItemInfo& info)
{
    return d->imageModel.hasImage(info);
}

ItemInfo ItemHistoryGraphModel::imageInfo(const QModelIndex& index) const
{
    QModelIndex imageIndex = imageModelIndex(index);

    return ItemModel::retrieveItemInfo(imageIndex);
}

QModelIndex ItemHistoryGraphModel::indexForInfo(const ItemInfo& info) const
{
    if (info.isNull())
    {
        return QModelIndex();
    }

    // try with primary info

    Q_FOREACH (VertexItem* const item, d->vertexItems)
    {
        if (ItemModel::retrieveItemInfo(item->index) == info)
        {    // cppcheck-suppress useStlAlgorithm
            return createIndex(item->parent->children.indexOf(item), 0, item);
        }
    }

    // try all associated infos

    Q_FOREACH (VertexItem* const item, d->vertexItems)
    {
        if (d->graph().properties(item->vertex).infos.contains(info))
        {
            return createIndex(item->parent->children.indexOf(item), 0, item);
        }
    }

    return QModelIndex();
}

QVariant ItemHistoryGraphModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    HistoryTreeItem* const item = d->historyItem(index);

    if_isItem(VertexItem, vertexItem, item)
    {
        if (vertexItem->index.isValid())
        {
            QVariant data = vertexItem->index.data(role);

            switch (role)
            {
                case IsImageItemRole:
                {
                    return true;
                }

                case IsSubjectImageRole:
                {
                    return (bool)d->graph().properties(vertexItem->vertex).infos.contains(d->info);
                }

                case Qt::DisplayRole:
                {
                    if (vertexItem->category & HistoryImageId::Original)
                    {
                        return i18nc("@item filename", "%1\n(Original Image)", data.toString());
                    }

                    if (vertexItem->category & HistoryImageId::Source)
                    {
                        return i18nc("@item filename", "%1\n(Source Image)", data.toString());
                    }

                    break;
                }
            }

            return data;
        }

        // else: read HistoryImageId from d->graph().properties(vertexItem->vertex)?
    }
    else if_isItem(FilterActionItem, filterActionItem, item)
    {
        switch (role)
        {
            case IsFilterActionItemRole:
            {
                return true;
            }

            case Qt::DisplayRole:
            {
                return DImgFilterManager::instance()->i18nDisplayableName(filterActionItem->action);
            }

            case Qt::DecorationRole:
            {
                QString iconName = DImgFilterManager::instance()->filterIcon(filterActionItem->action);
                return QIcon::fromTheme(iconName);
            }

            case FilterActionRole:
            {
                return QVariant::fromValue(filterActionItem->action);
            }

            default:
            {
                break;
            }
        }
    }
    else if_isItem(HeaderItem, headerItem, item)
    {
        switch (role)
        {
            case IsHeaderItemRole:
                return true;

            case Qt::DisplayRole:
/*
            case Qt::ToolTipRole:
*/
                return headerItem->title;
                break;
        }
    }
    else if_isItem(CategoryItem, categoryItem, item)
    {
        switch (role)
        {
            case IsCategoryItemRole:
                return true;

            case Qt::DisplayRole:
            case DCategorizedSortFilterProxyModel::CategoryDisplayRole:
/*
            case Qt::ToolTipRole:
*/
                return categoryItem->title;
        }
    }
    else if_isItem(SeparatorItem, separatorItem, item)
    {
        switch (role)
        {
            case IsSeparatorItemRole:
                return true;
        }
    }

    switch (role)
    {
        case IsImageItemRole:
        case IsFilterActionItemRole:
        case IsHeaderItemRole:
        case IsCategoryItemRole:
        case IsSubjectImageRole:
            return false;

        default:
            return QVariant();
    }
}

bool ItemHistoryGraphModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    HistoryTreeItem* const item = d->historyItem(index);

    if_isItem(VertexItem, vertexItem, item)
    {
        if (vertexItem->index.isValid())
        {
            return d->imageModel.setData(vertexItem->index, value, role);
        }
    }

    return false;
}

ItemListModel* ItemHistoryGraphModel::imageModel() const
{
    return &d->imageModel;
}

QModelIndex ItemHistoryGraphModel::imageModelIndex(const QModelIndex& index) const
{
    HistoryTreeItem* const item = d->historyItem(index);

    if_isItem(VertexItem, vertexItem, item)
    {
        return vertexItem->index;
    }

    return QModelIndex();
}

QVariant ItemHistoryGraphModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)

    return QVariant();
}

int ItemHistoryGraphModel::rowCount(const QModelIndex& parent) const
{
    return d->historyItem(parent)->childCount();
}

int ItemHistoryGraphModel::columnCount(const QModelIndex&) const
{
    return 1;
}

Qt::ItemFlags ItemHistoryGraphModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    HistoryTreeItem* const item = d->historyItem(index);

    if_isItem(VertexItem, vertexItem, item)
    {
        return d->imageModel.flags(vertexItem->index);
    }

    if (item)
    {
        switch (item->type())
        {
            case HistoryTreeItem::FilterActionItemType:
                return (Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            case HistoryTreeItem::HeaderItemType:
            case HistoryTreeItem::CategoryItemType:
            case HistoryTreeItem::SeparatorItemType:
            default:
                return Qt::ItemIsEnabled;
        }
    }

    return Qt::ItemIsEnabled;
}

QModelIndex ItemHistoryGraphModel::index(int row, int column, const QModelIndex& parent) const
{
    if ((column != 0) || (row < 0))
    {
        return QModelIndex();
    }

    HistoryTreeItem* const item = d->historyItem(parent);

    if (row >= item->childCount())
    {
        return QModelIndex();
    }

    return createIndex(row, 0, item->child(row));
}

bool ItemHistoryGraphModel::hasChildren(const QModelIndex& parent) const
{
    return d->historyItem(parent)->childCount();
}

QModelIndex ItemHistoryGraphModel::parent(const QModelIndex& index) const
{
    HistoryTreeItem* const item   = d->historyItem(index);
    HistoryTreeItem* const parent = item->parent;

    if (!parent)
    {
        return QModelIndex();    // index was an invalid index
    }

    HistoryTreeItem* const grandparent = parent->parent;

    if (!grandparent)
    {
        return QModelIndex();    // index was a top-level index, was the invisible rootItem as parent
    }

    return createIndex(grandparent->children.indexOf(parent), 0, parent);
}

} // namespace Digikam
