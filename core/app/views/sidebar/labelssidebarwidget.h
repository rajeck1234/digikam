/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : Labels sidebar widgets
 *
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_LABELS_SIDE_BAR_WIDGET_H
#define DIGIKAM_LABELS_SIDE_BAR_WIDGET_H

// Local includes

#include "album.h"
#include "sidebarwidget.h"
#include "labelstreeview.h"

namespace Digikam
{

class LabelsSideBarWidget : public SidebarWidget
{
    Q_OBJECT

public:

    explicit LabelsSideBarWidget(QWidget* const parent);
    ~LabelsSideBarWidget() override;

    LabelsTreeView* labelsTree();

    void          setActive(bool active)                             override;
    void          applySettings()                                    override;
    void          changeAlbumFromHistory(const QList<Album*>& album) override;
    void          doLoadState()                                      override;
    void          doSaveState()                                      override;
    const QIcon   getIcon()                                          override;
    const QString getCaption()                                       override;

    QHash<LabelsTreeView::Labels, QList<int> > selectedLabels();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_LABELS_SIDE_BAR_WIDGET_H
