/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-17-06
 * Description : a tool to export images to Flickr web service
 *
 * SPDX-FileCopyrightText: 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Luka Renko <lure at kubuntu dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "flickrwindow.h"

// Qt includes

#include <QPushButton>
#include <QProgressDialog>
#include <QPixmap>
#include <QCheckBox>
#include <QStringList>
#include <QSpinBox>
#include <QPointer>
#include <QApplication>
#include <QMenu>
#include <QMessageBox>
#include <QWindow>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "dprogresswdg.h"
#include "flickrtalker.h"
#include "flickritem.h"
#include "flickrlist.h"
#include "wsselectuserdlg.h"
#include "digikam_debug.h"
#include "flickrnewalbumdlg.h"
#include "previewloadthread.h"
#include "flickrwidget_p.h"

namespace DigikamGenericFlickrPlugin
{

class Q_DECL_HIDDEN FlickrWindow::Private
{
public:

    explicit Private()
      : uploadCount                 (0),
        uploadTotal                 (0),
        newAlbumBtn                 (nullptr),
        changeUserButton            (nullptr),
        removeAccount               (nullptr),
        albumsListComboBox          (nullptr),
        publicCheckBox              (nullptr),
        familyCheckBox              (nullptr),
        friendsCheckBox             (nullptr),
        exportHostTagsCheckBox      (nullptr),
        stripSpaceTagsCheckBox      (nullptr),
        addExtraTagsCheckBox        (nullptr),
        originalCheckBox            (nullptr),
        resizeCheckBox              (nullptr),
        dimensionSpinBox            (nullptr),
        imageQualitySpinBox         (nullptr),
        extendedPublicationButton   (nullptr),
        extendedTagsButton          (nullptr),
        contentTypeComboBox         (nullptr),
        safetyLevelComboBox         (nullptr),
        userNameDisplayLabel        (nullptr),
        authProgressDlg             (nullptr),
        tagsLineEdit                (nullptr),
        widget                      (nullptr),
        talker                      (nullptr),
        imglst                      (nullptr),
        select                      (nullptr),
        albumDlg                    (nullptr),
        iface                       (nullptr)
    {
    }

    unsigned int                     uploadCount;
    unsigned int                     uploadTotal;

    QString                          serviceName;

    QPushButton*                     newAlbumBtn;
    QPushButton*                     changeUserButton;
    QPushButton*                     removeAccount;

    QComboBox*                       albumsListComboBox;
    QCheckBox*                       publicCheckBox;
    QCheckBox*                       familyCheckBox;
    QCheckBox*                       friendsCheckBox;
    QCheckBox*                       exportHostTagsCheckBox;
    QCheckBox*                       stripSpaceTagsCheckBox;
    QCheckBox*                       addExtraTagsCheckBox;
    QCheckBox*                       originalCheckBox;
    QCheckBox*                       resizeCheckBox;

    QSpinBox*                        dimensionSpinBox;
    QSpinBox*                        imageQualitySpinBox;

    QPushButton*                     extendedPublicationButton;
    QPushButton*                     extendedTagsButton;
    WSComboBoxIntermediate*          contentTypeComboBox;
    WSComboBoxIntermediate*          safetyLevelComboBox;

    QString                          username;
    QString                          userId;
    QString                          lastSelectedAlbum;

    QLabel*                          userNameDisplayLabel;

    QProgressDialog*                 authProgressDlg;

    QList< QPair<QUrl, FPhotoInfo> > uploadQueue;

    DTextEdit*                       tagsLineEdit;

    FlickrWidget*                    widget;
    FlickrTalker*                    talker;

    FlickrList*                      imglst;
    WSSelectUserDlg*                 select;
    FlickrNewAlbumDlg*               albumDlg;

    DInfoInterface*                  iface;
};

FlickrWindow::FlickrWindow(DInfoInterface* const iface,
                           QWidget* const /*parent*/,
                           const QString& serviceName)
    : WSToolDialog(nullptr, QString::fromLatin1("%1Export Dialog").arg(serviceName)),
      d           (new Private)
{
    d->iface                = iface;
    d->serviceName          = serviceName;
    setWindowTitle(i18nc("@title:window", "Export to %1 Web Service", d->serviceName));
    setModal(false);

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group(QString::fromLatin1("%1Export Settings").arg(d->serviceName));

    if (grp.exists())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << QString::fromLatin1("%1Export Settings").arg(d->serviceName) << " exists, deleting it";
        grp.deleteGroup();
    }

    d->select                    = new WSSelectUserDlg(nullptr, serviceName);
    d->uploadCount               = 0;
    d->uploadTotal               = 0;
    d->widget                    = new FlickrWidget(this, iface, serviceName);
    d->albumDlg                  = new FlickrNewAlbumDlg(this, QLatin1String("Flickr"));
    d->albumsListComboBox        = d->widget->getAlbumsCoB();
    d->newAlbumBtn               = d->widget->getNewAlbmBtn();
    d->originalCheckBox          = d->widget->getOriginalCheckBox();
    d->resizeCheckBox            = d->widget->getResizeCheckBox();
    d->publicCheckBox            = d->widget->d->publicCheckBox;
    d->familyCheckBox            = d->widget->d->familyCheckBox;
    d->friendsCheckBox           = d->widget->d->friendsCheckBox;
    d->dimensionSpinBox          = d->widget->getDimensionSpB();
    d->imageQualitySpinBox       = d->widget->getImgQualitySpB();
    d->extendedTagsButton        = d->widget->d->extendedTagsButton;
    d->addExtraTagsCheckBox      = d->widget->d->addExtraTagsCheckBox;
    d->extendedPublicationButton = d->widget->d->extendedPublicationButton;
    d->safetyLevelComboBox       = d->widget->d->safetyLevelComboBox;
    d->contentTypeComboBox       = d->widget->d->contentTypeComboBox;
    d->tagsLineEdit              = d->widget->d->tagsLineEdit;
    d->exportHostTagsCheckBox    = d->widget->d->exportHostTagsCheckBox;
    d->stripSpaceTagsCheckBox    = d->widget->d->stripSpaceTagsCheckBox;
    d->changeUserButton          = d->widget->getChangeUserBtn();
    d->removeAccount             = d->widget->d->removeAccount;
    d->userNameDisplayLabel      = d->widget->getUserNameLabel();
    d->imglst                    = d->widget->d->imglst;

    startButton()->setText(i18nc("@action:button", "Start Uploading"));
    startButton()->setToolTip(QString());

    setMainWidget(d->widget);
    d->widget->setMinimumSize(800, 600);

    connect(d->imglst, SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    // --------------------------------------------------------------------------

    d->talker = new FlickrTalker(this, serviceName, d->iface);

    connect(d->talker, SIGNAL(signalError(QString)),
            d->talker, SLOT(slotError(QString)));

    connect(d->talker, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    connect(d->talker, SIGNAL(signalAddPhotoSucceeded(QString)),
            this, SLOT(slotAddPhotoSucceeded(QString)));

    connect(d->talker, SIGNAL(signalAddPhotoFailed(QString)),
            this, SLOT(slotAddPhotoFailed(QString)));

    connect(d->talker, SIGNAL(signalAddPhotoSetSucceeded()),
            this, SLOT(slotAddPhotoSetSucceeded()));

    connect(d->talker, SIGNAL(signalListPhotoSetsSucceeded()),
            this, SLOT(slotPopulatePhotoSetComboBox()));

    connect(d->talker, SIGNAL(signalListPhotoSetsFailed(QString)),
            this, SLOT(slotListPhotoSetsFailed(QString)));

    connect(d->talker, SIGNAL(signalLinkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(d->widget->progressBar(), SIGNAL(signalProgressCanceled()),
            this, SLOT(slotAddPhotoCancelAndClose()));

    connect(d->widget->getReloadBtn(), SIGNAL(clicked()),
            this, SLOT(slotReloadPhotoSetRequest()));

    // --------------------------------------------------------------------------

    connect(d->changeUserButton, SIGNAL(clicked()),
            this, SLOT(slotUserChangeRequest()));

    connect(d->removeAccount, SIGNAL(clicked()),
            this, SLOT(slotRemoveAccount()));

    connect(d->newAlbumBtn, SIGNAL(clicked()),
            this, SLOT(slotCreateNewPhotoSet()));

    // --------------------------------------------------------------------------

    d->authProgressDlg = new QProgressDialog(this);
    d->authProgressDlg->setModal(true);
    d->authProgressDlg->setAutoReset(true);
    d->authProgressDlg->setAutoClose(true);
    d->authProgressDlg->setMaximum(0);
    d->authProgressDlg->reset();

    connect(d->authProgressDlg, SIGNAL(canceled()),
            this, SLOT(slotAuthCancel()));

    d->talker->m_authProgressDlg = d->authProgressDlg;

    // --------------------------------------------------------------------------

    connect(this, &QDialog::finished,
            this, &FlickrWindow::slotFinished);

    connect(this, SIGNAL(cancelClicked()),
            this, SLOT(slotCancelClicked()));

    connect(startButton(), &QPushButton::clicked,
            this, &FlickrWindow::slotUser1);

    d->select->reactivate();
    readSettings(d->select->getUserName());
    d->talker->link(d->select->getUserName());
}

FlickrWindow::~FlickrWindow()
{
    delete d->select;
    delete d->authProgressDlg;
    delete d->talker;
    delete d->widget;
    delete d;
}

void FlickrWindow::setItemsList(const QList<QUrl>& urls)
{
    d->widget->imagesList()->slotAddImages(urls);
}

void FlickrWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

void FlickrWindow::slotFinished()
{
    writeSettings();
    d->imglst->listView()->clear();
}

void FlickrWindow::setUiInProgressState(bool inProgress)
{
    setRejectButtonMode(inProgress ? QDialogButtonBox::Cancel : QDialogButtonBox::Close);

    if (inProgress)
    {
        d->widget->progressBar()->show();
    }
    else
    {
        d->widget->progressBar()->hide();
        d->widget->progressBar()->progressCompleted();
    }
}

void FlickrWindow::slotCancelClicked()
{
    d->talker->cancel();
    d->uploadQueue.clear();
    setUiInProgressState(false);
}

void FlickrWindow::slotAddPhotoCancelAndClose()
{
    writeSettings();
    d->imglst->listView()->clear();
    d->uploadQueue.clear();
    d->widget->progressBar()->reset();
    setUiInProgressState(false);
    d->talker->cancel();
    reject();
}

void FlickrWindow::reactivate()
{
    d->userNameDisplayLabel->setText(QString());
    readSettings(d->select->getUserName());
    d->talker->link(d->select->getUserName());

    d->widget->d->imglst->loadImagesFromCurrentSelection();
    show();
}

void FlickrWindow::readSettings(const QString& uname)
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    QString groupName       = QString::fromLatin1("%1%2Export Settings").arg(d->serviceName, uname);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Group name is:" << groupName;
    KConfigGroup grp        = config->group(groupName);

    d->exportHostTagsCheckBox->setChecked(grp.readEntry("Export Host Tags",                     false));
    d->extendedTagsButton->setChecked(grp.readEntry("Show Extended Tag Options",                false));
    d->addExtraTagsCheckBox->setChecked(grp.readEntry("Add Extra Tags",                         false));
    d->stripSpaceTagsCheckBox->setChecked(grp.readEntry("Strip Space From Tags",                false));
    d->publicCheckBox->setChecked(grp.readEntry("Public Sharing",                               false));
    d->familyCheckBox->setChecked(grp.readEntry("Family Sharing",                               false));
    d->friendsCheckBox->setChecked(grp.readEntry("Friends Sharing",                             false));
    d->extendedPublicationButton->setChecked(grp.readEntry("Show Extended Publication Options", false));

    int safetyLevel = d->safetyLevelComboBox->findData(QVariant(grp.readEntry("Safety Level", 0)));

    if (safetyLevel == -1)
    {
        safetyLevel = 0;
    }

    d->safetyLevelComboBox->setCurrentIndex(safetyLevel);

    int contentType = d->contentTypeComboBox->findData(QVariant(grp.readEntry("Content Type", 0)));

    if (contentType == -1)
    {
        contentType = 0;
    }

    d->contentTypeComboBox->setCurrentIndex(contentType);

    d->originalCheckBox->setChecked(grp.readEntry("Upload Original", false));
    d->resizeCheckBox->setChecked(grp.readEntry("Resize",            false));
    d->dimensionSpinBox->setValue(grp.readEntry("Maximum Width",     1600));
    d->imageQualitySpinBox->setValue(grp.readEntry("Image Quality",  85));
}

void FlickrWindow::writeSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    QString groupName       = QString::fromLatin1("%1%2Export Settings").arg(d->serviceName, d->username);
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Group name is:" << groupName;

    if (QString::compare(QString::fromLatin1("%1Export Settings").arg(d->serviceName), groupName) == 0)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Not writing entry of group" << groupName;
        return;
    }

    KConfigGroup grp = config->group(groupName);

    grp.writeEntry("username",                          d->username);
    grp.writeEntry("Export Host Tags",                  d->exportHostTagsCheckBox->isChecked());
    grp.writeEntry("Show Extended Tag Options",         d->extendedTagsButton->isChecked());
    grp.writeEntry("Add Extra Tags",                    d->addExtraTagsCheckBox->isChecked());
    grp.writeEntry("Strip Space From Tags",             d->stripSpaceTagsCheckBox->isChecked());
    grp.writeEntry("Public Sharing",                    d->publicCheckBox->isChecked());
    grp.writeEntry("Family Sharing",                    d->familyCheckBox->isChecked());
    grp.writeEntry("Friends Sharing",                   d->friendsCheckBox->isChecked());
    grp.writeEntry("Show Extended Publication Options", d->extendedPublicationButton->isChecked());
    int safetyLevel = d->safetyLevelComboBox->itemData(d->safetyLevelComboBox->currentIndex()).toInt();
    grp.writeEntry("Safety Level",                      safetyLevel);
    int contentType = d->contentTypeComboBox->itemData(d->contentTypeComboBox->currentIndex()).toInt();
    grp.writeEntry("Content Type",                      contentType);
    grp.writeEntry("Resize",                            d->resizeCheckBox->isChecked());
    grp.writeEntry("Upload Original",                   d->originalCheckBox->isChecked());
    grp.writeEntry("Maximum Width",                     d->dimensionSpinBox->value());
    grp.writeEntry("Image Quality",                     d->imageQualitySpinBox->value());
}

void FlickrWindow::slotLinkingSucceeded()
{
    d->username = d->talker->getUserName();
    d->userId   = d->talker->getUserId();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "SlotLinkingSucceeded invoked setting user Display name to" << d->username;
    d->userNameDisplayLabel->setText(QString::fromLatin1("<b>%1</b>").arg(d->username));

    KSharedConfigPtr config = KSharedConfig::openConfig();

    Q_FOREACH (const QString& group, config->groupList())
    {
        if (!(group.contains(d->serviceName)))
        {
            continue;
        }

        KConfigGroup grp = config->group(group);

        if (group.contains(d->username))
        {
            readSettings(d->username);
            break;
        }
    }

    writeSettings();
    d->talker->listPhotoSets();
}

void FlickrWindow::slotBusy(bool val)
{
    if (val)
    {
        setCursor(Qt::WaitCursor);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
    }
}

void FlickrWindow::slotError(const QString& msg)
{
    QMessageBox::critical(this, i18nc("@title:window", "Error"), msg);
}

void FlickrWindow::slotUserChangeRequest()
{
    writeSettings();
    d->userNameDisplayLabel->setText(QString());
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Slot Change User Request";
    d->select->reactivate();
    readSettings(d->select->getUserName());

    d->talker->link(d->select->getUserName());
}

void FlickrWindow::slotRemoveAccount()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    QString groupName       = QString::fromLatin1("%1%2Export Settings").arg(d->serviceName, d->username);
    KConfigGroup grp        = config->group(groupName);

    if (grp.exists())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Removing Account having group" << groupName;
        grp.deleteGroup();
    }

    d->talker->unLink();
    d->talker->removeUserName(d->serviceName + d->username);

    d->userNameDisplayLabel->setText(QString());
    d->username = QString();
}

/**
 * Try to guess a sensible set name from the urls given.
 * Currently, it extracs the last path name component, and returns the most
 * frequently seen. The function could be expanded to, for example, only
 * accept the path if it occurs at least 50% of the time. It could also look
 * further up in the path name.
 */
QString FlickrWindow::guessSensibleSetName(const QList<QUrl>& urlList) const
{
    QMap<QString, int> nrFolderOccurences;

    // Extract last component of directory

    Q_FOREACH (const QUrl& url, urlList)
    {
        QString dir      = url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).toLocalFile();
        QStringList list = dir.split(QLatin1Char('/'));

        if (list.isEmpty())
        {
            continue;
        }

        nrFolderOccurences[list.last()]++;
    }

    int maxCount   = 0;
    int totalCount = 0;
    QString name;

    for (QMap<QString, int>::const_iterator it = nrFolderOccurences.constBegin() ;
         it != nrFolderOccurences.constEnd() ; ++it)
    {
        totalCount += it.value();

        if (it.value() > maxCount)
        {
            maxCount = it.value();
            name     = it.key();
        }
    }

    // If there is only one entry or one name appears at least twice, return the suggestion

    if ((totalCount == 1) || (maxCount > 1))
    {
        return name;
    }

    return QString();
}

/**
 * This method is called when the photo set creation button is pressed. It
 * summons a creation dialog for user input. When that is closed, it
 * creates a new photo set in the local list. The id gets the form of
 * UNDEFINED_ followed by a number, to indicate that it doesn't exist on
 * Flickr yet.
 */
void FlickrWindow::slotCreateNewPhotoSet()
{
    if (d->albumDlg->exec() == QDialog::Accepted)
    {
        FPhotoSet fps1;
        d->albumDlg->getFolderProperties(fps1);
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "in slotCreateNewPhotoSet()" << fps1.title;

        // Lets find an UNDEFINED_ style id that isn't taken yet.s

        QString id;
        int i                               = 0;
        id                                  = QLatin1String("UNDEFINED_") + QString::number(i);
        QList<FPhotoSet>::const_iterator it = d->talker->m_photoSetsList->constBegin();

        while (it != d->talker->m_photoSetsList->constEnd())
        {
            FPhotoSet fps2 = *it;

            if (fps2.id == id)
            {
                id = QLatin1String("UNDEFINED_") + QString::number(++i);
                it = d->talker->m_photoSetsList->constBegin();
            }

            ++it;
        }

        fps1.id = id;

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Created new photoset with temporary id" << id;

        // Append the new photoset to the list.

        d->talker->m_photoSetsList->prepend(fps1);
        d->talker->m_selectedPhotoSet = fps1;

        // Re-populate the photo sets combo box.

        slotPopulatePhotoSetComboBox();
    }
    else
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "New Photoset creation aborted";
    }
}

void FlickrWindow::slotAuthCancel()
{
    d->talker->cancel();
    d->authProgressDlg->hide();
}

void FlickrWindow::slotPopulatePhotoSetComboBox()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "slotPopulatePhotoSetComboBox invoked";

    if (d->talker && d->talker->m_photoSetsList)
    {
        QList<FPhotoSet>* const list = d->talker->m_photoSetsList;
        d->albumsListComboBox->clear();
        d->albumsListComboBox->insertItem(0, i18n("Photostream Only"));
        d->albumsListComboBox->insertSeparator(1);
        QList<FPhotoSet>::const_iterator it = list->constBegin();
        int index                           = 2;
        int curr_index                      = 0;

        while (it != list->constEnd())
        {
            FPhotoSet photoSet = *it;
            QString name       = photoSet.title;

            // Store the id as user data, because the title is not unique.

            QVariant id        = QVariant(photoSet.id);

            if (id == d->talker->m_selectedPhotoSet.id)
            {
                curr_index = index;
            }

            d->albumsListComboBox->insertItem(index++, name, id);
            ++it;
        }

        d->albumsListComboBox->setCurrentIndex(curr_index);
    }
}

/**
 * This slot is call when 'Start Uploading' button is pressed.
 */
void FlickrWindow::slotUser1()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "SlotUploadImages invoked";
/*
    d->widget->d->tab->setCurrentIndex(FlickrWidget::FILELIST);
*/
    if (d->imglst->imageUrls().isEmpty())
    {
        return;
    }

    typedef QPair<QUrl, FPhotoInfo> Pair;

    d->uploadQueue.clear();

    for (int i = 0 ; i < d->imglst->listView()->topLevelItemCount() ; ++i)
    {
        FlickrListViewItem* const lvItem = dynamic_cast<FlickrListViewItem*>(d->imglst->listView()->topLevelItem(i));

        if (lvItem)
        {
            DItemInfo info(d->iface->itemInfo(lvItem->url()));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Adding images" << lvItem->url() << " to the list";
            FPhotoInfo temp;

            temp.title                 = info.title();
            temp.description           = info.comment();
            temp.size                  = info.fileSize();
            temp.is_public             = lvItem->isPublic()  ? 1 : 0;
            temp.is_family             = lvItem->isFamily()  ? 1 : 0;
            temp.is_friend             = lvItem->isFriends() ? 1 : 0;
            temp.safety_level          = lvItem->safetyLevel();
            temp.content_type          = lvItem->contentType();
            QStringList tagsFromDialog = d->tagsLineEdit->text().split(QLatin1Char(','), QT_SKIP_EMPTY_PARTS);
            QStringList tagsFromList   = lvItem->extraTags();

            QStringList           allTags;
            QStringList::Iterator itTags;

            // Tags from the dialog

            itTags = tagsFromDialog.begin();

            while (itTags != tagsFromDialog.end())
            {
                allTags.append(*itTags);
                ++itTags;
            }

            // Tags from the database

            if (d->exportHostTagsCheckBox->isChecked())
            {
                QStringList tagsFromDatabase;

                tagsFromDatabase = info.keywords();
                itTags           = tagsFromDatabase.begin();

                while (itTags != tagsFromDatabase.end())
                {
                    allTags.append(*itTags);
                    ++itTags;
                }
            }

            // Tags from the list view.

            itTags = tagsFromList.begin();

            while (itTags != tagsFromList.end())
            {
                allTags.append(*itTags);
                ++itTags;
            }

            // Remove spaces if the user doesn't like them.

            if (d->stripSpaceTagsCheckBox->isChecked())
            {
                for (QStringList::iterator it = allTags.begin() ; it != allTags.end() ; ++it)
                {
                    *it = (*it).trimmed().remove(QLatin1Char(' '));
                }
            }

            // Debug the tag list.

            itTags = allTags.begin();

            while (itTags != allTags.end())
            {
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Tags list:" << (*itTags);
                ++itTags;
            }

            temp.tags = allTags;
            d->uploadQueue.append(Pair(lvItem->url(), temp));
        }
    }

    d->uploadTotal = d->uploadQueue.count();
    d->uploadCount = 0;
    d->widget->progressBar()->reset();
    slotAddPhotoNext();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "SlotUploadImages done";
}

void FlickrWindow::slotAddPhotoNext()
{
    if (d->uploadQueue.isEmpty())
    {
        d->widget->progressBar()->reset();
        setUiInProgressState(false);
        return;
    }

    typedef QPair<QUrl, FPhotoInfo> Pair;
    Pair pathComments = d->uploadQueue.first();
    FPhotoInfo info   = pathComments.second;

    QString selectedPhotoSetId = d->albumsListComboBox->itemData(d->albumsListComboBox->currentIndex()).toString();

    if (selectedPhotoSetId.isEmpty())
    {
        d->talker->m_selectedPhotoSet = FPhotoSet();
    }
    else
    {
        QList<FPhotoSet>::const_iterator it = d->talker->m_photoSetsList->constBegin();

        while (it != d->talker->m_photoSetsList->constEnd())
        {
            if (it->id == selectedPhotoSetId)
            {
                d->talker->m_selectedPhotoSet = *it;
                break;
            }

            ++it;
        }
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Max allowed file size is:"
                                     << d->talker->getMaxAllowedFileSize().toLongLong()
                                     << "File Size is" << info.size;

    bool res = d->talker->addPhoto(pathComments.first.toLocalFile(), //the file path
                                   info,
                                   d->originalCheckBox->isChecked(),
                                   d->resizeCheckBox->isChecked(),
                                   d->dimensionSpinBox->value(),
                                   d->imageQualitySpinBox->value());

    if (!res)
    {
        slotAddPhotoFailed(QLatin1String(""));
        return;
    }

    if (d->widget->progressBar()->isHidden())
    {
        setUiInProgressState(true);
        d->widget->progressBar()->progressScheduled(i18n("Flickr Export"), true, true);
        d->widget->progressBar()->progressThumbnailChanged(
            QIcon::fromTheme(QLatin1String("dk-flickr")).pixmap(22, 22));
    }
}

void FlickrWindow::slotAddPhotoSucceeded(const QString& photoId)
{
    QUrl photoUrl = d->uploadQueue.first().first;

    // Set location for uploaded photo

    DItemInfo info(d->iface->itemInfo(photoUrl));

    if (info.hasGeolocationInfo() && !photoId.isEmpty())
    {
        d->talker->setGeoLocation(photoId,
                                  QString::number(info.latitude()),
                                  QString::number(info.longitude()));
        return;
    }

    // Remove photo uploaded from the list

    d->imglst->removeItemByUrl(photoUrl);
    d->uploadQueue.removeFirst();
    d->uploadCount++;
    d->widget->progressBar()->setMaximum(d->uploadTotal);
    d->widget->progressBar()->setValue(d->uploadCount);
    slotAddPhotoNext();
}

void FlickrWindow::slotListPhotoSetsFailed(const QString& msg)
{
    QMessageBox::critical(this, QLatin1String("Error"),
                          i18n("Failed to Fetch Photoset information from %1. %2\n",
                               d->serviceName, msg));
}

void FlickrWindow::slotAddPhotoFailed(const QString& msg)
{
    QPointer<QMessageBox> warn = new QMessageBox(QMessageBox::Warning,
                     i18nc("@title:window", "Warning"),
                     i18n("Failed to upload photo into %1. %2\nDo you want to continue?",
                          d->serviceName, msg),
                     QMessageBox::Yes | QMessageBox::No);

    (warn->button(QMessageBox::Yes))->setText(i18nc("@action:button", "Continue"));
    (warn->button(QMessageBox::No))->setText(i18nc("@action:button", "Cancel"));

    if (warn->exec() != QMessageBox::Yes)
    {
        d->uploadQueue.clear();
        d->widget->progressBar()->reset();
        setUiInProgressState(false);
    }
    else
    {
        d->uploadQueue.removeFirst();
        d->uploadTotal--;
        d->widget->progressBar()->setMaximum(d->uploadTotal);
        d->widget->progressBar()->setValue(d->uploadCount);
        slotAddPhotoNext();
    }

    delete warn;
}

/**
 * Method called when a photo set has been successfully created on Flickr.
 * It functions to restart the normal flow after a photo set has been created
 * on Flickr.
 */
void FlickrWindow::slotAddPhotoSetSucceeded()
{
    slotPopulatePhotoSetComboBox();
    slotAddPhotoSucceeded(QLatin1String(""));
}

void FlickrWindow::slotImageListChanged()
{
    startButton()->setEnabled(!(d->widget->d->imglst->imageUrls().isEmpty()));
}

void FlickrWindow::slotReloadPhotoSetRequest()
{
    d->talker->listPhotoSets();
}

} // namespace DigikamGenericFlickrPlugin
