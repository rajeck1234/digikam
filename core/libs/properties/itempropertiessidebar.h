/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : item properties side bar (without support of digiKam database).
 *
 * SPDX-FileCopyrightText: 2004-2023 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ITEM_PROPERTIES_SIDEBAR_H
#define DIGIKAM_ITEM_PROPERTIES_SIDEBAR_H

// Qt includes

#include <QUrl>
#include <QWidget>
#include <QRect>
#include <QStackedWidget>

// Local includes

#include "digikam_config.h"
#include "sidebar.h"
#include "digikam_export.h"
#include "searchtextbar.h"

namespace Digikam
{

class DImg;
class SidebarSplitter;
class ItemPropertiesTab;
class ItemSelectionPropertiesTab;
class ItemPropertiesMetadataTab;
class ItemPropertiesColorsTab;

#ifdef HAVE_MARBLE

class ItemPropertiesGPSTab;

#endif // HAVE_MARBLE

class DIGIKAM_EXPORT ItemPropertiesSideBar : public Sidebar
{
    Q_OBJECT

public:
    explicit ItemPropertiesSideBar(QWidget* const parent,
                                   SidebarSplitter* const splitter,
                                   Qt::Edge side = Qt::LeftEdge,
                                   bool mimimizedDefault = false);
    ~ItemPropertiesSideBar() override;

    virtual void itemChanged(const QUrl& url, const QRect& rect = QRect(), DImg* const img = nullptr);

Q_SIGNALS:

    void signalSetupMetadataFilters(int);
    void signalSetupExifTool();

public Q_SLOTS:

    void slotLoadMetadataFilters();
    void slotImageSelectionChanged(const QRect& rect);
    virtual void slotNoCurrentItem();

protected Q_SLOTS:

    virtual void slotChangedTab(QWidget* tab);

protected:

    /**
     * load the last view state from disk - called by StateSavingObject#loadState()
     */
    void doLoadState() override;

    /**
     * save the view state to disk - called by StateSavingObject#saveState()
     */
    void doSaveState() override;

    virtual void setImagePropertiesInformation(const QUrl& url);

protected:

    bool                        m_dirtyPropertiesTab;
    bool                        m_dirtyMetadataTab;
    bool                        m_dirtyColorTab;
    bool                        m_dirtyGpsTab;
    bool                        m_dirtyHistoryTab;

    QRect                       m_currentRect;

    QUrl                        m_currentURL;

    DImg*                       m_image;

    QStackedWidget*             m_propertiesStackedView;

    ItemPropertiesTab*          m_propertiesTab;
    ItemSelectionPropertiesTab* m_selectionPropertiesTab;
    ItemPropertiesMetadataTab*  m_metadataTab;
    ItemPropertiesColorsTab*    m_colorTab;

#ifdef HAVE_MARBLE

    ItemPropertiesGPSTab*       m_gpsTab;

#endif // HAVE_MARBLE

};

} // namespace Digikam

#endif // DIGIKAM_ITEM_PROPERTIES_SIDEBAR_H
