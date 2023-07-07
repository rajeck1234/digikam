/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-23
 * Description : Graph data class for item history
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ITEM_HISTORY_GRAPH_H
#define DIGIKAM_ITEM_HISTORY_GRAPH_H

// Qt includes

#include <QFlags>
#include <QSharedDataPointer>
#include <QDebug>

// Local includes

#include "iteminfo.h"
#include "historyimageid.h"
#include "digikam_export.h"

namespace Digikam
{

class ItemHistoryGraphData;
class DImageHistory;

class DIGIKAM_DATABASE_EXPORT ItemHistoryGraph
{
public:

    enum HistoryLoadingFlag
    {
        /// Load the relation cloud to the graph. Will give all edges, but no further info
        LoadRelationCloud  = 1 << 0,

        /// Will load the DImageHistory of the given subject
        LoadSubjectHistory = 1 << 1,

        /// Will load the DImageHistory of all leave vertices of the graph
        LoadLeavesHistory  = 1 << 2,

        LoadAll            = LoadRelationCloud | LoadSubjectHistory | LoadLeavesHistory
    };
    Q_DECLARE_FLAGS(HistoryLoadingMode, HistoryLoadingFlag)

    enum ProcessingMode
    {
        NoProcessing,
        PrepareForDisplay
    };

public:

    ItemHistoryGraph();
    ItemHistoryGraph(const ItemHistoryGraph& other);
    ~ItemHistoryGraph();

    ItemHistoryGraph& operator=(const ItemHistoryGraph& other);

    bool isNull()                                                       const;
    bool isEmpty()                                                      const;
    bool isSingleVertex()                                               const;

    /**
     * Returns if the graph contains any edges. Because loops are not allowed,
     * this also means (!isEmpty() && !isSingleVertex()).
     */
    bool hasEdges()                                                     const;

    ItemHistoryGraphData& data();
    const ItemHistoryGraphData& data()                                  const;

    /**
     * Convenience: Reads all available history for the given info from the database
     * and returns the created graph.
     * Depending on mode, the graph will be preparedForDisplay().
     * If no history is recorded and no relations found, a single-vertex graph is returned.
     */
    static ItemHistoryGraph fromInfo(const ItemInfo& info,
                                      HistoryLoadingMode loadingMode = LoadAll,
                                      ProcessingMode processingMode  = PrepareForDisplay);

    /**
     * Add the given history.
     * The optionally given info or id is used as the "current" image of the history.
     * If you read a history from a file's metadata or the database, you shall give the
     * relevant subject.
     */
    void addHistory(const DImageHistory& history, const ItemInfo& historySubject = ItemInfo());
    void addHistory(const DImageHistory& history, const HistoryImageId& historySubject = HistoryImageId());

    /**
     * This is very similar to addHistory. The only difference is that
     * no attempt is made to retrieve an ItemInfo for the historySubjectId.
     * Can be useful in the context of scanning
     */
    void addScannedHistory(const DImageHistory& history, qlonglong historySubjectId);

    /**
     * Add images and their relations from the given pairs.
     * Each pair (a,b) means "a is derived from b".
     */
    void addRelations(const QList<QPair<qlonglong, qlonglong> >& pairs);

    /**
     * Clears this graph.
     */
    void clear();

    /**
     * Remove edges which provide only duplicate information
     * (performs a transitive reduction).
     * Especially call this when addRelations() was used.
     */
    void reduceEdges();

    /**
     * Returns true if for any entry no ItemInfo could be located.
     */
    bool hasUnresolvedEntries()                                         const;

    /**
     * Remove all vertices from the graph for which no existing ItemInfo
     * could be found in the database
     */
    void dropUnresolvedEntries();

    /**
     * Sort vertex information prioritizing for the given vertex
     */
    void sortForInfo(const ItemInfo& subject);

    /**
     * Combines reduceEdges(), dropOrphans() and sortForInfo()
     */
    void prepareForDisplay(const ItemInfo& subject);

    /**
     * Returns all possible relations between images in this graph,
     * the edges of the transitive closure.
     * The first variant returns (1,2),(3,4),(6,8), the second (1,3,6)(2,4,8).
     */
    QList<QPair<qlonglong, qlonglong> > relationCloud()                 const;
    QPair<QList<qlonglong>, QList<qlonglong> > relationCloudParallel()  const;

    /**
     * Returns image infos / ids from all vertices in this graph
     */
    QList<ItemInfo> allImages()                                         const;
    QList<qlonglong> allImageIds()                                      const;

    /**
     * Returns image infos / ids from all root vertices in this graph,
     * i.e. vertices with no precedent history.
     */
    QList<ItemInfo> rootImages()                                        const;

    /**
     * Returns image infos / ids from all leaf vertices in this graph,
     * i.e. vertices with no subsequent history.
     */
    QList<ItemInfo> leafImages()                                        const;

    /**
     * Attempts at a categorization of all images in the graph
     * into the types defined by HistoryImageId.
     * The type will be invalid if no decision can be made due to conflicting data.
     */
    QHash<ItemInfo, HistoryImageId::Types> categorize()                 const;

private:

    QSharedDataPointer<ItemHistoryGraphData> d;
};

QDebug DIGIKAM_DATABASE_EXPORT operator<<(QDebug dbg, const ItemHistoryGraph& g);

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::ItemHistoryGraph::HistoryLoadingMode)

#endif // DIGIKAM_ITEM_HISTORY_GRAPH_H
