/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : Searches folder view
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi at pooh dot tam dot uiuc dot edu>
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Johannes Wienke <languitar at semipol dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "searchfolderview.h"

// Qt includes

#include <QAction>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "contextmenuhelper.h"

namespace Digikam
{

class Q_DECL_HIDDEN NormalSearchTreeView::Private
{
public:

    explicit Private()
      : newAction   (nullptr),
        editAction  (nullptr)
    {
    }

    QAction* newAction;
    QAction* editAction;
};

NormalSearchTreeView::NormalSearchTreeView(QWidget* const parent,
                                           SearchModel* const searchModel,
                                           SearchModificationHelper* const searchModificationHelper)
    : EditableSearchTreeView(parent, searchModel, searchModificationHelper),
      d                     (new Private)
{

    d->newAction  = new QAction(QIcon::fromTheme(QLatin1String("document-new")), i18nc("Create new search",    "New..."),  this);
    d->editAction = new QAction(QIcon::fromTheme(QLatin1String("edit-find")),    i18nc("Edit selected search", "Edit..."), this);
}

NormalSearchTreeView::~NormalSearchTreeView()
{
    delete d;
}

void NormalSearchTreeView::addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album)
{
    cmh.addAction(d->newAction);
    cmh.addSeparator();

    EditableSearchTreeView::addCustomContextMenuActions(cmh, album);

    SAlbum* const salbum = dynamic_cast<SAlbum*>(album);

    d->editAction->setEnabled(salbum);
    cmh.addAction(d->editAction);
}

void NormalSearchTreeView::handleCustomContextMenuAction(QAction* action, const AlbumPointer<Album>& album)
{
    Album* a             = album;
    SAlbum* const salbum = dynamic_cast<SAlbum*>(a);

    if      ((action == d->newAction) && salbum)
    {
        Q_EMIT newSearch();
    }
    else if ((action == d->editAction) && salbum)
    {
        Q_EMIT editSearch(salbum);
    }
    else
    {
        EditableSearchTreeView::handleCustomContextMenuAction(action, album);
    }
}

} // namespace Digikam
