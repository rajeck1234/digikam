/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-07-13
 * Description : Qt categorized item view for camera items
 *
 * SPDX-FileCopyrightText: 2012 by Islam Wazery <wazery at ubuntu dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_IMPORT_CATEGORIZED_VIEW_H
#define DIGIKAM_IMPORT_CATEGORIZED_VIEW_H

// Local includes

#include "itemviewcategorized.h"
#include "importimagemodel.h"
#include "importfiltermodel.h"
#include "importthumbnailmodel.h"
#include "itemdelegateoverlay.h"
#include "camiteminfo.h"
#include "digikam_export.h"

namespace Digikam
{

class ImportDelegate;
class ICCSettingsContainer;

class DIGIKAM_GUI_EXPORT ImportCategorizedView : public ItemViewCategorized
{
    Q_OBJECT

public:

    explicit ImportCategorizedView(QWidget* const parent = nullptr);
    ~ImportCategorizedView() override;

    void setModels(ImportItemModel* model,
                   ImportSortFilterModel* filterModel);

    ImportItemModel*       importItemModel()                  const;
    ImportSortFilterModel* importSortFilterModel()            const;

    QItemSelectionModel*   getSelectionModel()                const;

    /**
     * Returns any ImportFilterModel in chain. May not be sourceModel()
     */
    ImportFilterModel*     importFilterModel()                const;

    /**
     * Returns 0 if the ImportItemModel is not an ImportThumbnailModel
     */
    ImportThumbnailModel*  importThumbnailModel()             const;

    ImportDelegate*        delegate()                         const;

    CamItemInfo            currentInfo()                      const;
    QUrl                   currentUrl()                       const;

    QList<CamItemInfo>     selectedCamItemInfos()             const;
    QList<CamItemInfo>     selectedCamItemInfosCurrentFirst() const;
    QList<QUrl>            selectedUrls()                     const;

    QList<CamItemInfo>     camItemInfos()                     const;
    QList<QUrl>            urls()                             const;

    /**
     * Selects the index as current and scrolls to it
     */
    void toIndex(const QUrl& url);

    /** Returns the n-th info after the given one.
     *  Specifically, return the previous info for nth = -1
     *  and the next info for n = 1.
     *  Returns a null info if either startingPoint or the nth info are
     *  not contained in the model
     */
    CamItemInfo nextInOrder(const CamItemInfo& startingPoint, int nth);

    CamItemInfo previousInfo(const CamItemInfo& info)
    {
        return nextInOrder(info, -1);
    }

    CamItemInfo nextInfo(const CamItemInfo& info)
    {
        return nextInOrder(info, 1);
    }

    /**
     * Add and remove an overlay. It will as well be removed automatically when destroyed.
     * Unless you pass a different delegate, the current delegate will be used.
     */
    void addOverlay(ItemDelegateOverlay* overlay, ImportDelegate* delegate = nullptr);
    void removeOverlay(ItemDelegateOverlay* overlay);

    void addSelectionOverlay(ImportDelegate* delegate = nullptr);

    ThumbnailSize thumbnailSize()                             const;

    virtual void setThumbnailSize(const ThumbnailSize& size);

public Q_SLOTS:

    void setThumbnailSize(int size);

    /**
     * Scroll the view to the given item when it becomes available
     */
    void setCurrentWhenAvailable(qlonglong camItemId);

    /**
     * Set as current item the item identified by its file url
     */
    void setCurrentUrl(const QUrl& url);

    /**
     * Set as current item the item identified by the CamItemInfo
     */
    void setCurrentInfo(const CamItemInfo& info);

    /**
     * Set selected items identified by their file urls
     */
    void setSelectedUrls(const QList<QUrl>& urlList);

    /**
     * Set selected items
     */
    void setSelectedCamItemInfos(const QList<CamItemInfo>& infos);

    /**
     * Does something to gain attention for info, but not changing current selection
     */
    void hintAt(const CamItemInfo& info);

Q_SIGNALS:

    void currentChanged(const CamItemInfo& info);

    /**
     * Emitted when new items are selected. The parameter includes only the newly selected infos,
     * there may be other already selected infos.
     */
    void selected(const QList<CamItemInfo>& newSelectedInfos);

    /**
     * Emitted when items are deselected. There may be other selected infos left.
     * This signal is not emitted when the model is reset; then only selectionCleared is emitted.
     */
    void deselected(const QList<CamItemInfo>& nowDeselectedInfos);

    /**
     * Emitted when the given CamItemInfo is activated. Info is never null.
     */
    void camItemInfoActivated(const CamItemInfo& info);

    /**
     * Emitted when a new model is set
     */
    void modelChanged();

protected Q_SLOTS:

    void slotCamItemInfosAdded();

protected:

    /// reimplemented from parent class
    QSortFilterProxyModel*       filterModel()                                                                       const override;
    AbstractItemDragDropHandler* dragDropHandler()                                                                   const override;
    QModelIndex                  nextIndexHint(const QModelIndex& indexToAnchor, const QItemSelectionRange& removed) const override;

    void setItemDelegate(ImportDelegate* delegate);
    void indexActivated(const QModelIndex& index, Qt::KeyboardModifiers modifiers)                                         override;
    void currentChanged(const QModelIndex& index, const QModelIndex& previous)                                             override;
    void paintEvent(QPaintEvent* e)                                                                                        override;
    void selectionChanged(const QItemSelection&, const QItemSelection&)                                                    override;
    void updateGeometries()                                                                                                override;

    /// Reimplement these in a subclass
    virtual void activated(const CamItemInfo& info, Qt::KeyboardModifiers modifiers);
    virtual void showContextMenuOnInfo(QContextMenuEvent* event, const CamItemInfo& info);
    void showContextMenuOnIndex(QContextMenuEvent* event, const QModelIndex& index)                                        override;

private Q_SLOTS:

    void slotFileChanged(const QString& filePath);
    void slotDelayedEnter();
    void slotIccSettingsChanged(const ICCSettingsContainer&, const ICCSettingsContainer&);

private:

    void scrollToStoredItem();

private:

    // Disable
    ImportCategorizedView(const ImportCategorizedView&)            = delete;
    ImportCategorizedView& operator=(const ImportCategorizedView&) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_IMPORT_CATEGORIZED_VIEW_H
