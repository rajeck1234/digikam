/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a rename dialog for the AdvancedRename utility
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2023 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "advancedrenamedialog.h"

// Qt includes

#include <QAction>
#include <QActionGroup>
#include <QEvent>
#include <QFileInfo>
#include <QGridLayout>
#include <QMoveEvent>
#include <QSet>
#include <QString>
#include <QTreeWidget>
#include <QMenu>
#include <QApplication>
#include <QStyle>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHeaderView>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_globals.h"
#include "advancedrenamewidget.h"
#include "contextmenuhelper.h"
#include "parser.h"
#include "parsesettings.h"
#include "advancedrenamemanager.h"
#include "advancedrenameprocessdialog.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN AdvancedRenameListItem::Private
{
public:

    explicit Private()
    {
    }

    QUrl    imageUrl;
    QString completeFileName;
};

// --------------------------------------------------------

AdvancedRenameListItem::AdvancedRenameListItem(QTreeWidget* const view)
    : QTreeWidgetItem(view),
      d              (new Private)
{
}

AdvancedRenameListItem::AdvancedRenameListItem(QTreeWidget* const view, const QUrl& url)
    : QTreeWidgetItem(view),
      d              (new Private)
{
    setImageUrl(url);
}

AdvancedRenameListItem:: ~AdvancedRenameListItem()
{
    delete d;
}

void AdvancedRenameListItem::setImageUrl(const QUrl& url)
{
    d->imageUrl          = url;

    QFileInfo fi(d->imageUrl.toLocalFile());
    d->completeFileName  = fi.fileName();

    setName(d->completeFileName);
    setNewName(d->completeFileName);
}

QUrl AdvancedRenameListItem::imageUrl() const
{
    return d->imageUrl;
}

void AdvancedRenameListItem::setName(const QString& name)
{
    setText(OldName, name);
}

QString AdvancedRenameListItem::name() const
{
    return text(OldName);
}

void AdvancedRenameListItem::setNewName(const QString& name)
{
    setText(NewName, name);
}

QString AdvancedRenameListItem::newName() const
{
    return text(NewName);
}

void AdvancedRenameListItem::markInvalid(bool invalid)
{
    QColor normalText = qApp->palette().text().color();
    setForeground(OldName, invalid ? Qt::red : normalText);
    setForeground(NewName, invalid ? Qt::red : normalText);
}

bool AdvancedRenameListItem::isNameEqual() const
{
    return (name() == newName());
}

// --------------------------------------------------------

class Q_DECL_HIDDEN AdvancedRenameDialog::Private
{
public:

    explicit Private()
      : singleFileMode       (false),
        minSizeDialog        (450),
        sortActionName       (nullptr),
        sortActionDate       (nullptr),
        sortActionSize       (nullptr),
        sortActionAscending  (nullptr),
        sortActionDescending (nullptr),
        sortGroupActions     (nullptr),
        sortGroupDirections  (nullptr),
        listView             (nullptr),
        buttons              (nullptr),
        advancedRenameManager(nullptr),
        advancedRenameWidget (nullptr)
    {
    }

    static const QString   configGroupName;
    static const QString   configLastUsedRenamePatternEntry;
    static const QString   configDialogSizeEntry;

    QString                singleFileModeOldFilename;

    bool                   singleFileMode;
    int                    minSizeDialog;

    QAction*               sortActionName;
    QAction*               sortActionDate;
    QAction*               sortActionSize;

    QAction*               sortActionAscending;
    QAction*               sortActionDescending;

    QActionGroup*          sortGroupActions;
    QActionGroup*          sortGroupDirections;

    QTreeWidget*           listView;
    QDialogButtonBox*      buttons;

    AdvancedRenameManager* advancedRenameManager;
    AdvancedRenameWidget*  advancedRenameWidget;
    NewNamesList           newNamesList;
};

const QString AdvancedRenameDialog::Private::configGroupName(QLatin1String("AdvancedRename Dialog"));
const QString AdvancedRenameDialog::Private::configLastUsedRenamePatternEntry(QLatin1String("Last Used Rename Pattern"));
const QString AdvancedRenameDialog::Private::configDialogSizeEntry(QLatin1String("Dialog Size"));

// --------------------------------------------------------

AdvancedRenameDialog::AdvancedRenameDialog(QWidget* const parent)
    : QDialog(parent),
      d      (new Private)
{
    setWindowFlags((windowFlags() & ~Qt::Dialog) |
                   Qt::Window                    |
                   Qt::WindowCloseButtonHint     |
                   Qt::WindowMinMaxButtonsHint);

    setModal(true);
    setupWidgets();
    setupConnections();

    initDialog();
    readSettings();
}

AdvancedRenameDialog::~AdvancedRenameDialog()
{
    writeSettings();
    delete d->advancedRenameManager;
    delete d;
}

void AdvancedRenameDialog::setupWidgets()
{
    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    d->advancedRenameManager = new AdvancedRenameManager();
    d->advancedRenameWidget  = new AdvancedRenameWidget(this);
    d->advancedRenameManager->setWidget(d->advancedRenameWidget);

    // --------------------------------------------------------

    d->sortActionName = new QAction(i18n("By Name"), this);
    d->sortActionDate = new QAction(i18n("By Date"), this);
    d->sortActionSize = new QAction(i18n("By File Size"), this);

    d->sortActionName->setCheckable(true);
    d->sortActionDate->setCheckable(true);
    d->sortActionSize->setCheckable(true);

    // --------------------------------------------------------

    d->sortActionAscending  = new QAction(i18nc("@action: sort type", "Ascending"),  this);
    d->sortActionDescending = new QAction(i18nc("@action: sort type", "Descending"), this);

    d->sortActionAscending->setCheckable(true);
    d->sortActionDescending->setCheckable(true);
    d->sortActionAscending->setChecked(true);

    // --------------------------------------------------------

    d->sortGroupActions     = new QActionGroup(this);
    d->sortGroupDirections  = new QActionGroup(this);

    d->sortGroupActions->addAction(d->sortActionName);
    d->sortGroupActions->addAction(d->sortActionDate);
    d->sortGroupActions->addAction(d->sortActionSize);

    d->sortGroupDirections->addAction(d->sortActionAscending);
    d->sortGroupDirections->addAction(d->sortActionDescending);

    // --------------------------------------------------------

    d->listView = new QTreeWidget(this);
    d->listView->setRootIsDecorated(false);
    d->listView->setSelectionMode(QAbstractItemView::NoSelection);
    d->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setSortingEnabled(false);
    d->listView->setColumnCount(2);
    d->listView->setHeaderLabels(QStringList() << i18n("Current Name") << i18n("New Name"));
    d->listView->setContextMenuPolicy(Qt::CustomContextMenu);
    d->listView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    d->listView->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    d->listView->setWhatsThis(i18n("This list shows the results for your renaming pattern. Red items indicate a "
                                   "name collision, either because the new name is equal to the current name, "
                                   "or because the name has already been assigned to another item."));

    // --------------------------------------------------------

    QWidget* const mainWidget     = new QWidget(this);
    QGridLayout* const mainLayout = new QGridLayout;
    mainLayout->addWidget(d->listView,             0, 0, 1, 1);
    mainLayout->addWidget(d->advancedRenameWidget, 1, 0, 1, 1);
    mainLayout->setRowStretch(0, 10);
    mainWidget->setLayout(mainLayout);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(mainWidget);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    setMinimumWidth(d->advancedRenameWidget->minimumWidth());
}

void AdvancedRenameDialog::setupConnections()
{
    connect(d->advancedRenameWidget, SIGNAL(signalTextChanged(QString)),
            this, SLOT(slotParseStringChanged(QString)));

    connect(d->advancedRenameWidget, SIGNAL(signalReturnPressed()),
            this, SLOT(slotReturnPressed()));

    connect(d->advancedRenameManager, SIGNAL(signalSortingChanged(QList<QUrl>)),
            this, SLOT(slotAddImages(QList<QUrl>)));

    connect(d->listView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotShowContextMenu(QPoint)));

    connect(d->sortGroupActions, SIGNAL(triggered(QAction*)),
            this, SLOT(slotSortActionTriggered(QAction*)));

    connect(d->sortGroupDirections, SIGNAL(triggered(QAction*)),
            this, SLOT(slotSortDirectionTriggered(QAction*)));

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));
}

void AdvancedRenameDialog::initDialog()
{
    QStringList fileList = d->advancedRenameManager->fileList();
    int count            = fileList.size();

    QString title = i18np("Rename", "Rename (%1 images)", count);
    setWindowTitle(title);

    if (count < 1)
    {
        d->listView->clear();
        return;
    }

    d->singleFileMode = (count == 1);

    Q_FOREACH (const QString& file, fileList)
    {
        QUrl url = QUrl::fromLocalFile(file);
        new AdvancedRenameListItem(d->listView, url);
    }

    // set current filename if only one image has been added

    if (d->singleFileMode)
    {
        QFileInfo info(fileList.first());
        d->advancedRenameWidget->setParseString(info.completeBaseName());
        d->advancedRenameWidget->setParseTimerDuration(50);
        d->advancedRenameWidget->focusLineEdit();
        d->advancedRenameWidget->highlightLineEdit();
        d->singleFileModeOldFilename = info.fileName();
    }

    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(checkNewNames());
}

void AdvancedRenameDialog::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    QSize s = group.readEntry(d->configDialogSizeEntry, QSize(d->minSizeDialog, d->minSizeDialog));
    resize(s);
    d->advancedRenameWidget->setParseString(group.readEntry(d->configLastUsedRenamePatternEntry, QString()));
}

void AdvancedRenameDialog::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configDialogSizeEntry, size());

    if (d->singleFileMode)
    {
        d->advancedRenameWidget->clear();
    }
    else
    {
        group.writeEntry(d->configLastUsedRenamePatternEntry, d->advancedRenameWidget->parseString());
    }
}

void AdvancedRenameDialog::slotReturnPressed()
{
    if (d->buttons->button(QDialogButtonBox::Ok)->isEnabled())
    {
        accept();
    }
}

void AdvancedRenameDialog::slotSortActionTriggered(QAction* action)
{
    if      (!action)
    {
        d->advancedRenameManager->setSortAction(AdvancedRenameManager::SortCustom);
    }
    else if (action == d->sortActionName)
    {
        d->advancedRenameManager->setSortAction(AdvancedRenameManager::SortName);
    }
    else if (action == d->sortActionDate)
    {
        d->advancedRenameManager->setSortAction(AdvancedRenameManager::SortDate);
    }
    else if (action == d->sortActionSize)
    {
        d->advancedRenameManager->setSortAction(AdvancedRenameManager::SortSize);
    }
}

void AdvancedRenameDialog::slotSortDirectionTriggered(QAction* direction)
{
    if      (direction == d->sortActionAscending)
    {
        d->advancedRenameManager->setSortDirection(AdvancedRenameManager::SortAscending);
    }
    else if (direction == d->sortActionDescending)
    {
        d->advancedRenameManager->setSortDirection(AdvancedRenameManager::SortDescending);
    }
}

void AdvancedRenameDialog::slotShowContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    menu.addSection(i18n("Sort Images"));

    ContextMenuHelper cmhelper(&menu);
    cmhelper.addAction(d->sortActionName);
    cmhelper.addAction(d->sortActionDate);
    cmhelper.addAction(d->sortActionSize);
    cmhelper.addSeparator();
    cmhelper.addAction(d->sortActionAscending);
    cmhelper.addAction(d->sortActionDescending);

    cmhelper.exec(d->listView->viewport()->mapToGlobal(pos));
}

void AdvancedRenameDialog::slotParseStringChanged(const QString& parseString)
{
    if (!d->advancedRenameManager)
    {
        return;
    }

    if (!d->singleFileMode)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
    }

    d->newNamesList.clear();

    // generate new file names

    ParseSettings settings;
    settings.useOriginalFileExtension = true;

    // settings.useOriginalFileExtension = d->singleFileMode ? false : true;

    d->advancedRenameManager->parseFiles(parseString, settings);

    // fill the tree widget with the updated files

    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        AdvancedRenameListItem* const item = dynamic_cast<AdvancedRenameListItem*>((*it));

        if (item)
        {
            QString newName = d->advancedRenameManager->newName(item->imageUrl().toLocalFile());
            item->setNewName(newName);
            d->newNamesList << NewNameInfo(item->imageUrl(), newName);
        }

        ++it;
    }

    bool enableBtn = checkNewNames() && !parseString.isEmpty();
    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(enableBtn);

    d->listView->viewport()->update();

    if (!d->singleFileMode)
    {
        QApplication::restoreOverrideCursor();
    }
}

void AdvancedRenameDialog::slotAddImages(const QList<QUrl>& urls)
{
    if (urls.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "No item to process";
        return;
    }

    d->listView->clear();
    d->advancedRenameManager->reset();
    QList<ParseSettings> files;

    Q_FOREACH (const QUrl& url, urls)
    {
        ParseSettings ps;
        ps.fileUrl = url;
        files << ps;
        qCDebug(DIGIKAM_GENERAL_LOG) << url;
    }

    d->advancedRenameManager->addFiles(files);

    initDialog();
    slotParseStringChanged(d->advancedRenameWidget->parseString());
}

NewNamesList AdvancedRenameDialog::newNames() const
{
    return filterNewNames();
}

bool AdvancedRenameDialog::checkNewNames() const
{
    int numNames      = 0;
    int numEqualNames = 0;
    bool ok           = true;

    QSet<QString> tmpNewNames;
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        AdvancedRenameListItem* const item = dynamic_cast<AdvancedRenameListItem*>((*it));

        if (item)
        {
            ++numNames;
            QFileInfo fi(item->imageUrl().toLocalFile());

            QString completeNewName = fi.path();
            completeNewName.append(QLatin1Char('/'));
            completeNewName.append(item->newName());

            bool invalid            = tmpNewNames.contains(completeNewName);
            tmpNewNames << completeNewName;

            item->markInvalid(invalid);
            ok                     &= !invalid;

            if (item->isNameEqual())
            {
                ++ numEqualNames;
            }
        }

        ++it;
    }

    return (ok && !(numNames == numEqualNames));
}

NewNamesList AdvancedRenameDialog::filterNewNames() const
{
    NewNamesList filteredNames;
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        AdvancedRenameListItem* const item = dynamic_cast<AdvancedRenameListItem*>((*it));

        if (item && !item->isNameEqual())
        {
            filteredNames << NewNameInfo(item->imageUrl(), item->newName());
        }

        ++it;
    }

    return filteredNames;
}

void AdvancedRenameDialog::slotHelp()
{
    openOnlineDocumentation(QLatin1String("main_window"), QLatin1String("image_view"), QLatin1String("renaming-photograph"));
}

} // namespace Digikam
