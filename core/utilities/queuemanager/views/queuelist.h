/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-21
 * Description : Batch Queue Manager items list.
 *
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_QUEUE_LIST_H
#define DIGIKAM_BQM_QUEUE_LIST_H

// Qt includes

#include <QTreeWidget>
#include <QWidget>
#include <QPixmap>
#include <QIcon>

// Local includes

#include "batchtool.h"
#include "batchtoolutils.h"
#include "iteminfo.h"
#include "iteminfolist.h"
#include "loadingdescription.h"
#include "queuesettingsview.h"

namespace Digikam
{

class QueueListView;

class QueueListViewItem : public QTreeWidgetItem
{

public:

    QueueListViewItem(QueueListView* const view, const ItemInfo& info);
    ~QueueListViewItem()              override;

    bool hasValidThumbnail()    const;

    void setInfo(const ItemInfo& info);
    ItemInfo info()             const;

    void assignTool(int index, const BatchToolSet& set);
    void unassignTool(int index);

    void setThumb(const QPixmap& pix, bool hasThumb = true);

    void animProgress();

    void setCanceled();
    void setFailed();

    void setDone();
    bool isDone()               const;

    void setBusy();
    bool isBusy()               const;

    void reset();

    void    setDestFileName(const QString& str);
    QString destFileName()      const;
    QString destBaseName()      const;
    QString destSuffix()        const;

private:

    void setPixmap(const QPixmap& pix);

private:

    class Private;
    Private* const d;

private:

    Q_DISABLE_COPY(QueueListViewItem)
};

// -------------------------------------------------------------------------

class CollectionImageChangeset;

class QueueListView : public QTreeWidget
{
    Q_OBJECT

public:

    explicit QueueListView(QWidget* const parent);
    ~QueueListView()                                                    override;

    void cancelItems();
    int  itemsCount();
    void removeItemByInfo(const ItemInfo& info);
    void removeItemById(qlonglong id);

    ItemInfoList pendingItemsList();
    int          pendingItemsCount();
    int          pendingTasksCount();

    QueueListViewItem* findItemByUrl(const QUrl& url);
    QueueListViewItem* findItemById(qlonglong id);

    void setSettings(const QueueSettings& settings);
    QueueSettings settings()                                      const;

    void setAssignedTools(const AssignedBatchTools& tools);
    AssignedBatchTools assignedTools()                            const;

    void setEnableToolTips(bool val);

    void reloadThumbs(const QUrl& url);

    void setItemBusy(qlonglong id);
    QPixmap progressPixmapForIndex(int index)                     const;

Q_SIGNALS:

    void signalQueueContentsChanged();

public Q_SLOTS:

    void slotClearList();
    void slotRemoveSelectedItems();
    void slotRemoveItemsDone();
    void slotAddItems(const ItemInfoList&);
    void slotAssignedToolsChanged(const AssignedBatchTools&);

private Q_SLOTS:

    void slotCollectionImageChange(const CollectionImageChangeset&);
    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);
    void slotContextMenu();
    void slotToolTip();
    void slotProgressTimerDone();

private:

    bool findItemByInfo(const ItemInfo& info);

    void removeItems(int removeType);
    void updateDestFileNames();
    void hideToolTip();
    bool acceptToolTip(const QPoint& pos)                         const;

#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))
    QMimeData*      mimeData(const QList<QTreeWidgetItem*>& items) const override;
#else
    // cppcheck-suppress passedByValue
    QMimeData*      mimeData(const QList<QTreeWidgetItem*> items)  const override;      // clazy:exclude=function-args-by-ref
#endif

    Qt::DropActions supportedDropActions()                        const override;
    void            startDrag(Qt::DropActions supportedActions)         override;

    void dragMoveEvent(QDragMoveEvent*)                                 override;
    void dragEnterEvent(QDragEnterEvent*)                               override;
    void dropEvent(QDropEvent*)                                         override;
    void mouseMoveEvent(QMouseEvent*)                                   override;
    void wheelEvent(QWheelEvent*)                                       override;
    void keyPressEvent(QKeyEvent*)                                      override;
    void focusOutEvent(QFocusEvent*)                                    override;
    void leaveEvent(QEvent*)                                            override;
    void resetQueue();

    void drawRow(QPainter* p,
                 const QStyleOptionViewItem& opt,
                 const QModelIndex& index)                        const override;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_BQM_QUEUE_LIST_H
