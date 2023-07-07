/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-05
 * Description : Side Bar Widget for the tag view.
 *
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TAG_VIEW_SIDE_BAR_WIDGET_H
#define DIGIKAM_TAG_VIEW_SIDE_BAR_WIDGET_H

// Local includes

#include "digikam_config.h"
#include "albummodel.h"
#include "sidebarwidget.h"

namespace Digikam
{

template <class T>
class AlbumPointer;

class TagViewSideBarWidget : public SidebarWidget
{

    Q_OBJECT

public:

    explicit TagViewSideBarWidget(QWidget* const parent, TagModel* const model);
    ~TagViewSideBarWidget() override;

    void          setActive(bool active)                             override;
    void          doLoadState()                                      override;
    void          doSaveState()                                      override;
    void          applySettings()                                    override;
    void          changeAlbumFromHistory(const QList<Album*>& album) override;
    const QIcon   getIcon()                                          override;
    const QString getCaption()                                       override;

    AlbumPointer<TAlbum> currentAlbum() const;

private:

    void setNoTagsAlbum();

public Q_SLOTS:

    void setCurrentAlbum(TAlbum* album);
    void slotOpenTagManager();
    void slotToggleTagsSelection(int radioClicked);

Q_SIGNALS:

    void signalFindDuplicates(const QList<TAlbum*>& albums);

public:

    // Declared as public due to use by Private
    class Private;

private:

    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_TAG_VIEW_SIDE_BAR_WIDGET_H
