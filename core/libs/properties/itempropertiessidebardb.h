/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : item properties side bar using data from digiKam database.
 *
 * SPDX-FileCopyrightText: 2004-2023 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ITEM_PROPERTIES_SIDEBAR_DB_H
#define DIGIKAM_ITEM_PROPERTIES_SIDEBAR_DB_H

// Qt includes

#include <QUrl>
#include <QWidget>
#include <QRect>

// Local includes

#include "dimagehistory.h"
#include "iteminfolist.h"
#include "itempropertiessidebar.h"
#include "digikam_export.h"
#include "digikam_config.h"

namespace Digikam
{

class DImg;
class SidebarSplitter;
class ItemInfo;
class ImageChangeset;
class ItemDescEditTab;
class ImageTagChangeset;
class ItemPropertiesVersionsTab;
class GPSItemInfo;

class ItemPropertiesSideBarDB : public ItemPropertiesSideBar
{
    Q_OBJECT

public:

    explicit ItemPropertiesSideBarDB(QWidget* const parent,
                                      SidebarSplitter* const splitter,
                                      Qt::Edge side=Qt::LeftEdge,
                                      bool mimimizedDefault=false);
    ~ItemPropertiesSideBarDB() override;

    void populateTags();
    void refreshTagsView();

    /// This is for image editor to be able to update the filter list in sidebar
    ItemPropertiesVersionsTab* getFiltersHistoryTab() const;
    ItemDescEditTab*           imageDescEditTab()     const;

    void itemChanged(const QUrl& url, const QRect& rect = QRect(), DImg* const img = nullptr) override;

    virtual void itemChanged(const ItemInfo& info, const QRect& rect = QRect(),
                             DImg* const img = nullptr, const DImageHistory& history = DImageHistory());

    virtual void itemChanged(const ItemInfoList& infos, const ItemInfoList& allInfos);


#ifdef HAVE_MARBLE

    static bool GPSItemInfofromItemInfo(const ItemInfo&, GPSItemInfo* const);

#endif // HAVE_MARBLE

Q_SIGNALS:

    void signalFirstItem();
    void signalPrevItem();
    void signalNextItem();
    void signalLastItem();

public Q_SLOTS:

    void slotAssignRating(int rating);
    void slotAssignRatingNoStar();
    void slotAssignRatingOneStar();
    void slotAssignRatingTwoStar();
    void slotAssignRatingThreeStar();
    void slotAssignRatingFourStar();
    void slotAssignRatingFiveStar();

    void slotPopupTagsView();

    void slotNoCurrentItem() override;

private Q_SLOTS:

    void slotChangedTab(QWidget* tab) override;
    void slotFileMetadataChanged(const QUrl& url);
    void slotImageChangeDatabase(const ImageChangeset& changeset);
    void slotImageTagChanged(const ImageTagChangeset& changeset);
    void slotPropertiesButtonPressed(int);

private:

    void changedTab(QWidget* const tab);
    void itemChanged(const QUrl& url, const ItemInfo& info,
                     const QRect& rect, DImg* const img, const DImageHistory& history);
    void itemChanged(const ItemInfoList& infos, const ItemInfoList& allInfos,
                     const QRect& rect, DImg* const img, const DImageHistory& history);
    void setImagePropertiesInformation(const QUrl& url) override;
    void setImageSelectionPropertiesInformation();

protected:

    /**
     * load the last view state from disk - called by StateSavingObject#loadState()
     */
    void doLoadState() override;

    /**
     * save the view state to disk - called by StateSavingObject#saveState()
     */
    void doSaveState() override;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_PROPERTIES_SIDEBAR_DB_H
