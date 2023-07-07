/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a rename dialog for the AdvancedRename utility
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ADVANCED_RENAME_DIALOG_H
#define DIGIKAM_ADVANCED_RENAME_DIALOG_H

// Qt includes

#include <QList>
#include <QPair>
#include <QTreeWidgetItem>
#include <QUrl>
#include <QDialog>

// Local includes

#include "iteminfo.h"
#include "digikam_export.h"

class QEvent;
class QMoveEvent;
class QString;
class QTreeWidget;

namespace Digikam
{

class Parser;

class DIGIKAM_GUI_EXPORT AdvancedRenameListItem : public QTreeWidgetItem
{
public:

    enum Column
    {
        OldName = 0,
        NewName
    };

public:

    explicit AdvancedRenameListItem(QTreeWidget* const view);
    AdvancedRenameListItem(QTreeWidget* const view, const QUrl& info);
    ~AdvancedRenameListItem() override;

    void setImageUrl(const QUrl& url);
    QUrl imageUrl() const;

    void setName(const QString& name);
    QString name() const;

    void setNewName(const QString& name);
    QString newName() const;

    void markInvalid(bool invalid);
    bool isNameEqual() const;

private:

    // Disable
    AdvancedRenameListItem(const AdvancedRenameListItem&)            = delete;
    AdvancedRenameListItem& operator=(const AdvancedRenameListItem&) = delete;

private:

    class Private;
    Private* const d;
};

// --------------------------------------------------------

typedef QPair<QUrl, QString> NewNameInfo;
typedef QList<NewNameInfo>   NewNamesList;

class DIGIKAM_GUI_EXPORT AdvancedRenameDialog : public QDialog
{
    Q_OBJECT

public:

    explicit AdvancedRenameDialog(QWidget* const parent = nullptr);
    ~AdvancedRenameDialog() override;

    NewNamesList newNames() const;

public Q_SLOTS:

    void slotAddImages(const QList<QUrl>& urls);

private Q_SLOTS:

    void slotParseStringChanged(const QString&);
    void slotReturnPressed();

    void slotSortActionTriggered(QAction*);
    void slotSortDirectionTriggered(QAction*);

    void slotShowContextMenu(const QPoint&);
    void slotHelp();

private:

    void setupWidgets();
    void setupConnections();

    void initDialog();
    void readSettings();
    void writeSettings();
    bool checkNewNames() const;

    NewNamesList filterNewNames() const;

private:

    // Disable
    AdvancedRenameDialog(const AdvancedRenameDialog&)            = delete;
    AdvancedRenameDialog& operator=(const AdvancedRenameDialog&) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_ADVANCED_RENAME_DIALOG_H
