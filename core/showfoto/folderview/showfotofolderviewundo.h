/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-08-27
 * Description : Showfoto folder view undo command.
 *
 * SPDX-FileCopyrightText: 2021-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef SHOWFOTO_FOLDER_VIEW_UNDO_H
#define SHOWFOTO_FOLDER_VIEW_UNDO_H

// Qt includes

#include <QUndoCommand>
#include <QString>

// Local includes

#include "showfotofolderviewsidebar.h"

namespace ShowFoto
{

class ShowfotoFolderViewUndo : public QUndoCommand
{

public:

    ShowfotoFolderViewUndo(ShowfotoFolderViewSideBar* const view,
                           const QString& newPath);
    ~ShowfotoFolderViewUndo();

    QString undoPath() const;

    void undo();
    void redo();

private:

    class Private;
    Private* const d;
};

} // namespace ShowFoto

#endif // SHOWFOTO_FOLDER_VIEW_UNDO_H
