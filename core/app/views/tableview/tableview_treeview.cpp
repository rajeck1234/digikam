/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-03-02
 * Description : Table view: Tree view subelement
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tableview_treeview.h"

// Qt includes

#include <QContextMenuEvent>
#include <QHeaderView>
#include <QTreeView>
#include <QMenu>
#include <QAction>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "contextmenuhelper.h"
#include "iteminfo.h"
#include "itemmodel.h"
#include "tableview_column_configuration_dialog.h"
#include "tableview_columnfactory.h"
#include "tableview_model.h"
#include "tableview_selection_model_syncer.h"
#include "tableview_treeview_delegate.h"
#include "thumbnailsize.h"

namespace Digikam
{

class Q_DECL_HIDDEN TableViewTreeView::Private
{
public:

    explicit Private()
      : headerContextMenuActiveColumn         (-1),
        actionHeaderContextMenuRemoveColumn   (nullptr),
        actionHeaderContextMenuConfigureColumn(nullptr),
        dragDropThumbnailSize                 ()
    {
    }

public:

    int           headerContextMenuActiveColumn;
    QAction*      actionHeaderContextMenuRemoveColumn;
    QAction*      actionHeaderContextMenuConfigureColumn;
    ThumbnailSize dragDropThumbnailSize;
};

TableViewTreeView::TableViewTreeView(TableViewShared* const tableViewShared, QWidget* const parent)
    : QTreeView(parent),
      d        (new Private()),
      s        (tableViewShared)
{
    setModel(s->tableViewModel);
    setSelectionModel(s->tableViewSelectionModel);

    s->itemDelegate = new TableViewItemDelegate(s, this);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setItemDelegate(s->itemDelegate);
    setAlternatingRowColors(true);
    setAllColumnsShowFocus(true);
    setUniformRowHeights(true);
    setSortingEnabled(true);
    setDragEnabled(true);
    setAcceptDrops(true);
    setWordWrap(true);
/*
    viewport()->setAcceptDrops(true);
*/
    d->actionHeaderContextMenuRemoveColumn    = new QAction(QIcon::fromTheme(QLatin1String("edit-table-delete-column")), i18n("Remove this column"), this);

    connect(d->actionHeaderContextMenuRemoveColumn, SIGNAL(triggered(bool)),
            this, SLOT(slotHeaderContextMenuActionRemoveColumnTriggered()));

    d->actionHeaderContextMenuConfigureColumn = new QAction(QIcon::fromTheme(QLatin1String("configure")), i18n("Configure this column"), this);

    connect(d->actionHeaderContextMenuConfigureColumn, SIGNAL(triggered(bool)),
            this, SLOT(slotHeaderContextMenuConfigureColumn()));

    header()->installEventFilter(this);

    slotModelGroupingModeChanged();

    connect(s->tableViewModel, SIGNAL(signalGroupingModeChanged()),
            this, SLOT(slotModelGroupingModeChanged()));
}

TableViewTreeView::~TableViewTreeView()
{
}

bool TableViewTreeView::eventFilter(QObject* watched, QEvent* event)
{
    QHeaderView* const headerView = header();

    if ((watched == headerView) && (event->type() == QEvent::ContextMenu))
    {
        showHeaderContextMenu(event);

        return true;
    }

    return QObject::eventFilter(watched, event);
}

void TableViewTreeView::addColumnDescriptionsToMenu(const QList<TableViewColumnDescription>& columnDescriptions, QMenu* const menu)
{
    for (int i = 0 ; i < columnDescriptions.count() ; ++i)
    {
        const TableViewColumnDescription& desc = columnDescriptions.at(i);
        QAction* const action                  = new QAction(desc.columnTitle, menu);

        if (!desc.columnIcon.isEmpty())
        {
            action->setIcon(QIcon::fromTheme(desc.columnIcon));
        }

        if (desc.subColumns.isEmpty())
        {
            connect(action, SIGNAL(triggered(bool)),
                    this, SLOT(slotHeaderContextMenuAddColumn()));

            action->setData(QVariant::fromValue<TableViewColumnDescription>(desc));
        }
        else
        {
            QMenu* const subMenu = new QMenu(menu);
            addColumnDescriptionsToMenu(desc.subColumns, subMenu);

            action->setMenu(subMenu);
        }

        menu->addAction(action);
    }
}

void TableViewTreeView::showHeaderContextMenu(QEvent* const event)
{
    QContextMenuEvent* const e                = static_cast<QContextMenuEvent*>(event);
    QHeaderView* const headerView             = header();
    d->headerContextMenuActiveColumn          = headerView->logicalIndexAt(e->pos());
    const TableViewColumn* const columnObject = s->tableViewModel->getColumnObject(d->headerContextMenuActiveColumn);
    QMenu* const menu                         = new QMenu(this);

    d->actionHeaderContextMenuRemoveColumn->setEnabled(s->tableViewModel->columnCount(QModelIndex())>1);
    menu->addAction(d->actionHeaderContextMenuRemoveColumn);
    const bool columnCanConfigure = columnObject->getColumnFlags().testFlag(TableViewColumn::ColumnHasConfigurationWidget);
    d->actionHeaderContextMenuConfigureColumn->setEnabled(columnCanConfigure);
    menu->addAction(d->actionHeaderContextMenuConfigureColumn);
    menu->addSeparator();

    // add actions for all columns

    QList<TableViewColumnDescription> columnDescriptions = s->columnFactory->getColumnDescriptionList();
    addColumnDescriptionsToMenu(columnDescriptions, menu);

    menu->exec(e->globalPos());
}

void TableViewTreeView::slotHeaderContextMenuAddColumn()
{
    QAction* const triggeredAction = qobject_cast<QAction*>(sender());

    const QVariant actionData = triggeredAction->data();

    if (!actionData.canConvert<TableViewColumnDescription>())
    {
        return;
    }

    const TableViewColumnDescription desc = actionData.value<TableViewColumnDescription>();
    qCDebug(DIGIKAM_GENERAL_LOG) << "clicked: " << desc.columnTitle;
    const int newColumnLogicalIndex       = d->headerContextMenuActiveColumn+1;
    s->tableViewModel->addColumnAt(desc, newColumnLogicalIndex);

    // since the header column order is not the same as the model's column order, we need
    // to make sure the new column is moved directly behind the current column in the header:

    const int clickedVisualIndex   = header()->visualIndex(d->headerContextMenuActiveColumn);
    const int newColumnVisualIndex = header()->visualIndex(newColumnLogicalIndex);
    int newColumnVisualTargetIndex = clickedVisualIndex + 1;

    // If the column is inserted before the clicked column, we have to
    // subtract one from the target index because it looks like QHeaderView first removes
    // the column and then inserts it.

    if (newColumnVisualIndex < clickedVisualIndex)
    {
        newColumnVisualTargetIndex--;
    }

    if (newColumnVisualIndex!=newColumnVisualTargetIndex)
    {
        header()->moveSection(newColumnVisualIndex, newColumnVisualTargetIndex);
    }

    // Ensure that the newly created column is visible.
    // This is especially important if the new column is the last one,
    // because then it can be outside of the viewport.

    const QModelIndex topIndex = indexAt(QPoint(0, 0));
    const QModelIndex targetIndex = s->tableViewModel->index(topIndex.row(), newColumnLogicalIndex, topIndex.parent());
    scrollTo(targetIndex, EnsureVisible);
}

void TableViewTreeView::slotHeaderContextMenuActionRemoveColumnTriggered()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "remove column " << d->headerContextMenuActiveColumn;
    s->tableViewModel->removeColumnAt(d->headerContextMenuActiveColumn);
}

void TableViewTreeView::slotHeaderContextMenuConfigureColumn()
{
    TableViewConfigurationDialog* const configurationDialog = new TableViewConfigurationDialog(s, d->headerContextMenuActiveColumn, this);
    const int result                                        = configurationDialog->exec();

    if (result!=QDialog::Accepted)
    {
        return;
    }

    const TableViewColumnConfiguration newConfiguration     = configurationDialog->getNewConfiguration();
    s->tableViewModel->getColumnObject(d->headerContextMenuActiveColumn)->setConfiguration(newConfiguration);
}

AbstractItemDragDropHandler* TableViewTreeView::dragDropHandler() const
{
    qCDebug(DIGIKAM_GENERAL_LOG)<<s->imageModel->dragDropHandler();

    return s->imageModel->dragDropHandler();
}

QModelIndex TableViewTreeView::mapIndexForDragDrop(const QModelIndex& index) const
{
    // "index" is a TableViewModel index.
    // We are using the drag-drop-handler of ItemModel, thus
    // we have to convert it to an index of ItemModel.

    // map to ItemModel

    const QModelIndex imageModelIndex = s->tableViewModel->toItemModelIndex(index);

    return imageModelIndex;
}

QPixmap TableViewTreeView::pixmapForDrag(const QList< QModelIndex >& indexes) const
{
    const QModelIndex& firstIndex = indexes.at(0);
    const ItemInfo info           = s->tableViewModel->imageInfo(firstIndex);
    const QString path            = info.filePath();

    QPixmap thumbnailPixmap;

    /// @todo The first thumbnail load always fails. We have to add thumbnail pre-generation
    ///       like in ItemModel. Getting thumbnails from ItemModel does not help, because it
    ///       does not necessarily prepare them the same way.
    /// @todo Make a central drag-drop thumbnail generator?

    if (!s->thumbnailLoadThread->find(info.thumbnailIdentifier(), thumbnailPixmap, d->dragDropThumbnailSize.size()))
    {
        /// @todo better default pixmap?

        thumbnailPixmap.fill();
    }

    /// @todo Decorate the pixmap like the other drag-drop implementations?
    /// @todo Write number of images onto the pixmap

    return thumbnailPixmap;
/*
    const QModelIndex& firstIndex      = indexes.at(0);
    const QModelIndex& imageModelIndex = s->sortModel->toItemModelIndex(firstIndex);
    ItemModel* const imageModel        = s->imageFilterModel->sourceItemModel();

    /// @todo Determine how other views choose the size

    const QSize thumbnailSize(60, 60);

    imageModel->setData(imageModelIndex, qMax(thumbnailSize.width(), thumbnailSize.height()), ItemModel::ThumbnailRole);
    QVariant thumbnailData  = imageModel->data(imageModelIndex, ItemModel::ThumbnailRole);
    imageModel->setData(imageModelIndex, QVariant(), ItemModel::ThumbnailRole);

    QPixmap thumbnailPixmap = thumbnailData.value<QPixmap>();

    /// @todo Write number of images onto the pixmap

    return thumbnailPixmap;
*/
}

Album* TableViewTreeView::albumAt(const QPoint& pos) const
{
    Q_UNUSED(pos)

    ItemAlbumModel* const albumModel = qobject_cast<ItemAlbumModel*>(s->imageModel);

    if (albumModel)
    {
        if (!(albumModel->currentAlbums().isEmpty()))
        {
            return albumModel->currentAlbums().constFirst();
        }
    }

    return nullptr;
}

void TableViewTreeView::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        const int delta = event->angleDelta().y();

        if      (delta > 0)
        {
            Q_EMIT signalZoomInStep();
        }
        else if (delta < 0)
        {
            Q_EMIT signalZoomOutStep();
        }

        event->accept();

        return;
    }

    QTreeView::wheelEvent(event);
}

bool TableViewTreeView::hasHiddenGroupedImages(const ItemInfo& info) const
{
        return (
                info.hasGroupedImages() &&
                (
                 (s->tableViewModel->groupingMode() == s->tableViewModel->GroupingMode::GroupingHideGrouped)    ||
                  ((s->tableViewModel->groupingMode() == s->tableViewModel->GroupingMode::GroupingShowSubItems) &&
                   (!s->treeView->isExpanded(s->tableViewModel->indexFromImageId(info.id(), 0)))
                 )
                )
               );
}

void TableViewTreeView::slotModelGroupingModeChanged()
{
    setRootIsDecorated(s->tableViewModel->groupingMode() == TableViewModel::GroupingShowSubItems);
}

} // namespace Digikam
