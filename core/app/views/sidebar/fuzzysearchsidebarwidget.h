/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : Side Bar Widget for the fuzzy search.
 *
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_FUZZY_SEARCH_SIDE_BAR_WIDGET_H
#define DIGIKAM_FUZZY_SEARCH_SIDE_BAR_WIDGET_H

// Local includes

#include "digikam_config.h"
#include "albummodel.h"
#include "searchmodificationhelper.h"
#include "sidebarwidget.h"

namespace Digikam
{

class FuzzySearchSideBarWidget : public SidebarWidget
{
    Q_OBJECT

public:

    explicit FuzzySearchSideBarWidget(QWidget* const parent,
                                      SearchModel* const searchModel,
                                      SearchModificationHelper* const searchModificationHelper);
    ~FuzzySearchSideBarWidget() override;

    void          setActive(bool active)                             override;
    void          doLoadState()                                      override;
    void          doSaveState()                                      override;
    void          applySettings()                                    override;
    void          changeAlbumFromHistory(const QList<Album*>& album) override;
    const QIcon   getIcon()                                          override;
    const QString getCaption()                                       override;

    void newDuplicatesSearch(const QList<PAlbum*>& albums);
    void newDuplicatesSearch(const QList<TAlbum*>& albums);
    void newSimilarSearch(const ItemInfo& imageInfo);

Q_SIGNALS:

    void signalActive(bool);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_FUZZY_SEARCH_SIDE_BAR_WIDGET_H
