/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export items to Google web services
 *
 * SPDX-FileCopyrightText: 2013      by Pankaj Kumar <me at panks dot me>
 * SPDX-FileCopyrightText: 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gswindow.h"

// Qt includes

#include <QWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QProgressDialog>
#include <QPixmap>
#include <QCheckBox>
#include <QStringList>
#include <QSpinBox>
#include <QFileInfo>
#include <QPointer>
#include <QDesktopServices>
#include <QUrl>
#include <QScopedPointer>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "wstoolutils.h"
#include "ditemslist.h"
#include "digikam_version.h"
#include "dprogresswdg.h"
#include "gdtalker.h"
#include "gsitem.h"
#include "gsnewalbumdlg.h"
#include "gswidget.h"
#include "gptalker.h"
#include "gsreplacedlg.h"
#include "digikam_debug.h"
#include "dfileoperations.h"

namespace DigikamGenericGoogleServicesPlugin
{

class Q_DECL_HIDDEN GSWindow::Private
{
public:

    explicit Private()
      : imagesCount   (0),
        imagesTotal   (0),
        renamingOpt   (0),
        service       (GoogleService::GPhotoImport),
        widget        (nullptr),
        albumDlg      (nullptr),
        gphotoAlbumDlg(nullptr),
        talker        (nullptr),
        gphotoTalker  (nullptr),
        iface         (nullptr)
    {
    }

    unsigned int                  imagesCount;
    unsigned int                  imagesTotal;
    int                           renamingOpt;

    QString                       serviceName;
    QString                       toolName;
    GoogleService                 service;
    QString                       tmp;

    GSWidget*                     widget;
    GSNewAlbumDlg*                albumDlg;
    GSNewAlbumDlg*                gphotoAlbumDlg;

    GDTalker*                     talker;
    GPTalker*                     gphotoTalker;

    QString                       currentAlbumId;
    QString                       newFolderTitle;

    QList< QPair<QUrl, GSPhoto> > transferQueue;
    QList< QPair<QUrl, GSPhoto> > uploadQueue;

    DInfoInterface*               iface;
};

GSWindow::GSWindow(DInfoInterface* const iface,
                   QWidget* const /*parent*/,
                   const QString& serviceName)
    : WSToolDialog(nullptr, QString::fromLatin1("%1Export Dialog").arg(serviceName)),
      d           (new Private)
{
    d->iface       = iface;
    d->serviceName = serviceName;

    if      (QString::compare(d->serviceName, QLatin1String("googledriveexport"),
                              Qt::CaseInsensitive) == 0)
    {
        d->service  = GoogleService::GDrive;
        d->toolName = QLatin1String("Google Drive");
    }
    else if (QString::compare(d->serviceName, QLatin1String("googlephotoexport"),
                              Qt::CaseInsensitive) == 0)
    {
        d->service  = GoogleService::GPhotoExport;
        d->toolName = QLatin1String("Google Photos");
    }
    else
    {
        d->service  = GoogleService::GPhotoImport;
        d->toolName = QLatin1String("Google Photos");
    }

    d->tmp         = WSToolUtils::makeTemporaryDir("google").absolutePath() + QLatin1Char('/');;
    d->widget      = new GSWidget(this, d->iface, d->service, d->toolName);

    setMainWidget(d->widget);
    setModal(false);

    switch (d->service)
    {
        case GoogleService::GDrive:
        {
            setWindowTitle(i18nc("@title:window", "Export to Google Drive"));

            startButton()->setText(i18nc("@action:button", "Start Upload"));
            startButton()->setToolTip(i18nc("@info:tooltip, button", "Start upload to Google Drive"));

            d->widget->setMinimumSize(700, 500);

            d->albumDlg = new GSNewAlbumDlg(this, d->serviceName, d->toolName);
            d->talker   = new GDTalker(this);

            connect(d->talker,SIGNAL(signalBusy(bool)),
                    this,SLOT(slotBusy(bool)));

            connect(d->talker,SIGNAL(signalAccessTokenObtained()),
                    this,SLOT(slotAccessTokenObtained()));

            connect(d->talker, SIGNAL(signalAuthenticationRefused()),
                    this,SLOT(slotAuthenticationRefused()));

            connect(d->talker,SIGNAL(signalSetUserName(QString)),
                    this,SLOT(slotSetUserName(QString)));

            connect(d->talker,SIGNAL(signalListAlbumsDone(int,QString,QList<GSFolder>)),
                    this,SLOT(slotListAlbumsDone(int,QString,QList<GSFolder>)));

            connect(d->talker,SIGNAL(signalCreateFolderDone(int,QString)),
                    this,SLOT(slotCreateFolderDone(int,QString)));

            connect(d->talker,SIGNAL(signalAddPhotoDone(int,QString)),
                    this,SLOT(slotAddPhotoDone(int,QString)));

            connect(d->talker, SIGNAL(signalUploadPhotoDone(int,QString,QStringList)),
                    this, SLOT(slotUploadPhotoDone(int,QString,QStringList)));

            readSettings();
            buttonStateChange(false);

            d->talker->doOAuth();

            break;
        }

        case GoogleService::GPhotoImport:
        case GoogleService::GPhotoExport:
        {
            if (d->service == GoogleService::GPhotoExport)
            {
                setWindowTitle(i18nc("@title:window", "Export to Google Photos Service"));

                startButton()->setText(i18nc("@action:button", "Start Upload"));
                startButton()->setToolTip(i18nc("@info:tooltip, button", "Start upload to Google Photos service"));

                d->widget->setMinimumSize(700, 500);
            }
            else
            {
                setWindowTitle(i18nc("@title:window", "Import from Google Photos Service"));

                startButton()->setText(i18nc("@action:button", "Start Download"));
                startButton()->setToolTip(i18nc("@info:tooltip, button", "Start download from Google Photos service"));

                d->widget->setMinimumSize(300, 400);
            }

            d->gphotoAlbumDlg = new GSNewAlbumDlg(this, d->serviceName, d->toolName);
            d->gphotoTalker   = new GPTalker(this);

            connect(d->gphotoTalker, SIGNAL(signalBusy(bool)),
                    this, SLOT(slotBusy(bool)));

            connect(d->gphotoTalker,SIGNAL(signalSetUserName(QString)),
                    this,SLOT(slotSetUserName(QString)));

            connect(d->gphotoTalker, SIGNAL(signalAccessTokenObtained()),
                    this, SLOT(slotAccessTokenObtained()));

            connect(d->gphotoTalker, SIGNAL(signalAuthenticationRefused()),
                    this,SLOT(slotAuthenticationRefused()));

            connect(d->gphotoTalker, SIGNAL(signalListAlbumsDone(int,QString,QList<GSFolder>)),
                    this, SLOT(slotListAlbumsDone(int,QString,QList<GSFolder>)));

            connect(d->gphotoTalker, SIGNAL(signalCreateAlbumDone(int,QString,QString)),
                    this, SLOT(slotCreateFolderDone(int,QString,QString)));

            connect(d->gphotoTalker, SIGNAL(signalAddPhotoDone(int,QString)),
                    this, SLOT(slotAddPhotoDone(int,QString)));

            connect(d->gphotoTalker, SIGNAL(signalUploadPhotoDone(int,QString,QStringList)),
                    this, SLOT(slotUploadPhotoDone(int,QString,QStringList)));

            connect(d->gphotoTalker, SIGNAL(signalGetPhotoDone(int,QString,QByteArray,QString)),
                    this, SLOT(slotGetPhotoDone(int,QString,QByteArray,QString)));

            readSettings();
            buttonStateChange(false);

            d->gphotoTalker->doOAuth();

            break;
        }
    }

    connect(d->widget->imagesList(), SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    connect(d->widget->getChangeUserBtn(), SIGNAL(clicked()),
            this, SLOT(slotUserChangeRequest()));

    connect(d->widget->getNewAlbmBtn(), SIGNAL(clicked()),
            this,SLOT(slotNewAlbumRequest()));

    connect(d->widget->getReloadBtn(), SIGNAL(clicked()),
            this, SLOT(slotReloadAlbumsRequest()));

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotStartTransfer()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));
}

GSWindow::~GSWindow()
{
    d->transferQueue.clear();

    delete d->gphotoTalker;
    delete d->talker;
    delete d;
}

void GSWindow::reactivate()
{
    d->widget->imagesList()->loadImagesFromCurrentSelection();
    d->widget->progressBar()->hide();

    show();
}

void GSWindow::readSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp;

    switch (d->service)
    {
        case GoogleService::GDrive:
            grp = config->group("Google Drive Settings");
            break;

        default:
            grp = config->group("Google Photo Settings");
            break;
    }

    d->currentAlbumId = grp.readEntry("Current Album", QString());

    if (grp.readEntry("Resize", false))
    {
        d->widget->getResizeCheckBox()->setChecked(true);
        d->widget->getDimensionSpB()->setEnabled(true);
    }
    else
    {
        d->widget->getResizeCheckBox()->setChecked(false);
        d->widget->getDimensionSpB()->setEnabled(false);
    }

    d->widget->getOriginalCheckBox()->setChecked(grp.readEntry("Upload Original", false));
    d->widget->getPhotoIdCheckBox()->setChecked(grp.readEntry("Write PhotoID",    true));
    d->widget->getDimensionSpB()->setValue(grp.readEntry("Maximum Width",         1600));
    d->widget->getImgQualitySpB()->setValue(grp.readEntry("Image Quality",        90));

    if (d->service == GoogleService::GPhotoExport && d->widget->m_tagsBGrp)
    {
        d->widget->m_tagsBGrp->button(grp.readEntry("Tag Paths", 0))->setChecked(true);
    }
}

void GSWindow::writeSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp;

    switch (d->service)
    {
        case GoogleService::GDrive:
            grp = config->group("Google Drive Settings");
            break;

        default:
            grp = config->group("Google Photo Settings");
            break;
    }

    grp.writeEntry("Current Album",   d->currentAlbumId);
    grp.writeEntry("Resize",          d->widget->getResizeCheckBox()->isChecked());
    grp.writeEntry("Upload Original", d->widget->getOriginalCheckBox()->isChecked());
    grp.writeEntry("Write PhotoID",   d->widget->getPhotoIdCheckBox()->isChecked());
    grp.writeEntry("Maximum Width",   d->widget->getDimensionSpB()->value());
    grp.writeEntry("Image Quality",   d->widget->getImgQualitySpB()->value());

    if ((d->service == GoogleService::GPhotoExport) && d->widget->m_tagsBGrp)
    {
        grp.writeEntry("Tag Paths", d->widget->m_tagsBGrp->checkedId());
    }
}

void GSWindow::slotSetUserName(const QString& msg)
{
    d->widget->updateLabels(msg);
}

void GSWindow::slotListPhotosDoneForDownload(int errCode,
                                             const QString& errMsg,
                                             const QList <GSPhoto>& photosList)
{
    disconnect(d->gphotoTalker, SIGNAL(signalListPhotosDone(int,QString,QList<GSPhoto>)),
               this, SLOT(slotListPhotosDoneForDownload(int,QString,QList<GSPhoto>)));

    if (errCode == 0)
    {
        QMessageBox::critical(this, i18nc("@title:window", "Error"),
                              i18nc("@info", "Google Photos call failed: %1\n", errMsg));
        return;
    }

    typedef QPair<QUrl, GSPhoto> Pair;
    d->transferQueue.clear();
    QList<GSPhoto>::const_iterator itPWP;

    for (itPWP = photosList.begin() ; itPWP != photosList.end() ; ++itPWP)
    {
        d->transferQueue.append(Pair((*itPWP).originalURL, (*itPWP)));
    }

    if (d->transferQueue.isEmpty())
    {
        return;
    }

    d->currentAlbumId = d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString();
    d->imagesTotal    = d->transferQueue.count();
    d->imagesCount    = 0;

    d->widget->progressBar()->setFormat(i18nc("@info: progress bar", "%v / %m"));
    d->widget->progressBar()->show();

    d->renamingOpt    = 0;

    // start download with first photo in queue

    downloadNextPhoto();
}

void GSWindow::slotListAlbumsDone(int code, const QString& errMsg, const QList <GSFolder>& list)
{
    switch (d->service)
    {
        case GoogleService::GDrive:

            if (code == 0)
            {
                QMessageBox::critical(this, i18nc("@title:window", "Error"),
                                      i18nc("@info", "Google Drive call failed: %1\n", errMsg));
                return;
            }

            d->widget->getAlbumsCoB()->clear();

            for (int i = 0 ; i < list.size() ; ++i)
            {
                d->widget->getAlbumsCoB()->addItem(QIcon::fromTheme(QLatin1String("system-users")),
                                                   list.value(i).title, list.value(i).id);

                if (d->currentAlbumId == list.value(i).id)
                {
                    d->widget->getAlbumsCoB()->setCurrentIndex(i);
                }
            }

            buttonStateChange(true);
            d->talker->getUserName();
            break;

        default:

            if (code == 0)
            {
                QMessageBox::critical(this, i18nc("@title:window", "Error"),
                                      i18nc("@info", "Google Photos call failed: %1\n", errMsg));
                return;
            }

            d->widget->getAlbumsCoB()->clear();

            for (int i = 0 ; i < list.size() ; ++i)
            {
                if ((d->service == GoogleService::GPhotoImport)        &&
                    (list.at(i).title == QLatin1String("<auto-create>")))
                {
                    // remove <auto-create> album
                    continue;
                }

                QString albumIcon;

                if (list.at(i).isWriteable)
                {
                    albumIcon = QLatin1String("folder");
                }
                else
                {
                    albumIcon = QLatin1String("folder-locked");
                }

                d->widget->getAlbumsCoB()->addItem(QIcon::fromTheme(albumIcon), list.at(i).title, list.at(i).id);

                if (d->currentAlbumId == list.at(i).id)
                {
                    d->widget->getAlbumsCoB()->setCurrentIndex(i);
                }

                buttonStateChange(true);
            }
            break;
    }
}

void GSWindow::slotBusy(bool val)
{
    setCursor(val ? Qt::WaitCursor
                  : Qt::ArrowCursor);

    d->widget->imagesList()->enableControlButtons(!val);
    d->widget->imagesList()->enableDragAndDrop(!val);
    d->widget->getChangeUserBtn()->setEnabled(!val);
    d->widget->getOptionsBox()->setEnabled(!val);
    buttonStateChange(!val);
}

void GSWindow::slotStartTransfer()
{
    d->widget->imagesList()->clearProcessedStatus();

    switch (d->service)
    {
        case GoogleService::GDrive:
        case GoogleService::GPhotoExport:
        {
            if (d->widget->imagesList()->imageUrls().isEmpty())
            {
                QMessageBox::critical(this, i18nc("@title:window", "Error"),
                                      i18nc("@info", "No image selected. Please select which images should be uploaded."));
                return;
            }

            break;
        }

        case GoogleService::GPhotoImport:
            break;
    }

    switch (d->service)
    {
        case GoogleService::GDrive:
        {
            if (!(d->talker->authenticated()))
            {
                QPointer<QMessageBox> warn = new QMessageBox(QMessageBox::Warning,
                                 i18nc("@title:window", "Warning"),
                                 i18nc("@info", "Authentication failed. Click \"Continue\" to authenticate."),
                                 QMessageBox::Yes | QMessageBox::No);

                (warn->button(QMessageBox::Yes))->setText(i18nc("@action:button", "Continue"));
                (warn->button(QMessageBox::No))->setText(i18nc("@action:button", "Cancel"));

                if (warn->exec() == QMessageBox::Yes)
                {
                    d->talker->doOAuth();
                    delete warn;
                    return;
                }
                else
                {
                    delete warn;
                    return;
                }
            }

            break;
        }

        default:
        {
            d->gphotoTalker->cancel();

            if (!(d->gphotoTalker->authenticated()))
            {
                QPointer<QMessageBox> warn = new QMessageBox(QMessageBox::Warning,
                                 i18nc("@title:window", "Warning"),
                                 i18nc("@info", "Authentication failed. Click \"Continue\" to authenticate."),
                                 QMessageBox::Yes | QMessageBox::No);

                (warn->button(QMessageBox::Yes))->setText(i18nc("@action:button", "Continue"));
                (warn->button(QMessageBox::No))->setText(i18nc("@action:button", "Cancel"));

                if (warn->exec() == QMessageBox::Yes)
                {
                    d->gphotoTalker->doOAuth();
                    delete warn;
                    return;
                }
                else
                {
                    delete warn;
                    return;
                }
            }

            if (d->service == GoogleService::GPhotoImport)
            {
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Google Photo Transfer invoked";

                // list photos of the album, then start download

                connect(d->gphotoTalker, SIGNAL(signalListPhotosDone(int,QString,QList<GSPhoto>)),
                        this, SLOT(slotListPhotosDoneForDownload(int,QString,QList<GSPhoto>)));

                d->gphotoTalker->listPhotos(
                    d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString());

                return;
            }
        }
    }

    typedef QPair<QUrl, GSPhoto> Pair;

    for (int i = 0 ; i < (d->widget->imagesList()->imageUrls().size()) ; ++i)
    {
        DItemInfo info(d->iface->itemInfo(d->widget->imagesList()->imageUrls().value(i)));
        GSPhoto temp;
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "in start transfer info" <<info.title() << info.comment();

        switch (d->service)
        {
            case GoogleService::GDrive:
                temp.title       = info.title();
                temp.description = info.comment().section(QLatin1String("\n"), 0, 0);
                break;

            default:
                temp.title = info.name();

                // Google Photo doesn't support image titles. Include it in descriptions if needed.

                QStringList descriptions({info.title()});

                if (info.title() != info.comment())
                {
                    descriptions << info.comment();
                }

                descriptions.removeAll(QLatin1String(""));
                temp.description         = descriptions.join(QLatin1String("\n\n"));
                temp.description.replace(QLatin1Char('"'), QLatin1String("\\\""));
                temp.description         = temp.description.left(1000);
                break;
        }

        temp.gpsLat.setNum(info.latitude());
        temp.gpsLon.setNum(info.longitude());
        temp.tags        = info.tagsPath();

        d->transferQueue.append(Pair(d->widget->imagesList()->imageUrls().value(i), temp));
    }

    d->currentAlbumId = d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString();
    d->imagesTotal    = d->transferQueue.count();
    d->imagesCount    = 0;

    d->widget->progressBar()->setFormat(i18nc("@info: progress bar", "%v / %m"));
    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(0);
    d->widget->progressBar()->show();

    switch (d->service)
    {
        case GoogleService::GDrive:
            d->widget->progressBar()->progressScheduled(i18nc("@info", "Google Drive export"), true, true);
            d->widget->progressBar()->progressThumbnailChanged(
                QIcon::fromTheme(QLatin1String("dk-googledrive")).pixmap(22, 22));
            break;

        default:
            d->widget->progressBar()->progressScheduled(i18nc("@info", "Google Photo export"), true, true);
            d->widget->progressBar()->progressThumbnailChanged(
                QIcon::fromTheme((QLatin1String("dk-googlephoto"))).pixmap(22, 22));
            break;
    }

    uploadNextPhoto();
}

void GSWindow::uploadNextPhoto()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "in upload nextphoto" << d->transferQueue.count();

    if (d->transferQueue.isEmpty())
    {
        //d->widget->progressBar()->hide();

        d->widget->progressBar()->progressCompleted();

        /**
         * Now all raw photos have been added,
         * for GPhoto: prepare to upload on user account
         * for GDrive: get listPhotoId to write metadata and finish upload
         */
        if (d->service == GoogleService::GPhotoExport)
        {
            Q_EMIT d->gphotoTalker->signalReadyToUpload();
        }
        else
        {
            Q_EMIT d->talker->signalReadyToUpload();
        }

        return;
    }

    typedef QPair<QUrl, GSPhoto> Pair;
    Pair pathComments = d->transferQueue.first();
    GSPhoto info      = pathComments.second;
    bool res          = true;
    d->widget->imagesList()->processing(pathComments.first);

    switch (d->service)
    {
        case GoogleService::GDrive:
        {
            res = d->talker->addPhoto(pathComments.first.toLocalFile(),
                                      info,
                                      d->currentAlbumId,
                                      d->widget->getOriginalCheckBox()->isChecked(),
                                      d->widget->getResizeCheckBox()->isChecked(),
                                      d->widget->getDimensionSpB()->value(),
                                      d->widget->getImgQualitySpB()->value());
            break;
        }

        case GoogleService::GPhotoExport:
        {
            bool bCancel = false;
            bool bAdd    = true;

            if (!info.id.isEmpty() && !info.editUrl.isEmpty())
            {
                switch (d->renamingOpt)
                {
                    case PWR_ADD_ALL:
                        bAdd = true;
                        break;

                    case PWR_REPLACE_ALL:
                        bAdd = false;
                        break;

                    default:
                    {
                        QPointer<ReplaceDialog> dlg = new ReplaceDialog(this, QLatin1String(""),
                                                                        d->iface, pathComments.first,
                                                                        info.thumbURL);
                        (void)dlg->exec();

                        switch (dlg->getResult())
                        {
                            case PWR_ADD_ALL:
                                d->renamingOpt = PWR_ADD_ALL;
                                break;

                            case PWR_ADD:
                                bAdd = true;
                                break;

                            case PWR_REPLACE_ALL:
                                d->renamingOpt = PWR_REPLACE_ALL;
                                break;

                            case PWR_REPLACE:
                                bAdd = false;
                                break;

                            case PWR_CANCEL:
                            default:
                                bCancel = true;
                                break;
                        }

                        delete dlg;
                        break;
                    }
                }
            }

            // adjust tags according to radio button clicked

            if (d->widget->m_tagsBGrp)
            {
                switch (d->widget->m_tagsBGrp->checkedId())
                {
                    case GPTagLeaf:
                    {
                        QStringList newTags;
                        QStringList::const_iterator itT;

                        for (itT = info.tags.constBegin() ; itT != info.tags.constEnd() ; ++itT)
                        {
                            QString strTmp = *itT;
                            int idx        = strTmp.lastIndexOf(QLatin1Char('/'));

                            if (idx > 0)
                            {
                                strTmp.remove(0, idx + 1);
                            }

                            newTags.append(strTmp);
                        }

                        info.tags = newTags;
                        break;
                    }

                    case GPTagSplit:
                    {
                        QSet<QString> newTagsSet;
                        QStringList::const_iterator itT;

                        for (itT = info.tags.constBegin() ; itT != info.tags.constEnd() ; ++itT)
                        {
                            QStringList strListTmp = itT->split(QLatin1Char('/'));
                            QStringList::const_iterator itT2;

                            for (itT2 = strListTmp.constBegin() ; itT2 != strListTmp.constEnd() ; ++itT2)
                            {
                                if (!newTagsSet.contains(*itT2))
                                {
                                    newTagsSet.insert(*itT2);
                                }
                            }
                        }

                        info.tags.clear();
                        QSet<QString>::const_iterator itT3;

                        for (itT3 = newTagsSet.constBegin() ; itT3 != newTagsSet.constEnd() ; ++itT3)
                        {
                            info.tags.append(*itT3);
                        }

                        break;
                    }

                    case GPTagCombined:
                    default:
                    {
                        break;
                    }
                }
            }

            if (bCancel)
            {
                slotTransferCancel();
                res = true;
            }
            else
            {
                if (bAdd)
                {
                    res = d->gphotoTalker->addPhoto(pathComments.first.toLocalFile(),
                                                    info,
                                                    d->currentAlbumId,
                                                    d->widget->getOriginalCheckBox()->isChecked(),
                                                    d->widget->getResizeCheckBox()->isChecked(),
                                                    d->widget->getDimensionSpB()->value(),
                                                    d->widget->getImgQualitySpB()->value());
                }
                else
                {
                    res = d->gphotoTalker->updatePhoto(pathComments.first.toLocalFile(),
                                                       info,
                                                       d->widget->getResizeCheckBox()->isChecked(),
                                                       d->widget->getDimensionSpB()->value(),
                                                       d->widget->getImgQualitySpB()->value());
                }
            }
            break;
        }

        case GoogleService::GPhotoImport:
            break;
    }

    if (!res)
    {
        slotAddPhotoDone(0, QLatin1String(""));
        return;
    }
}

void GSWindow::downloadNextPhoto()
{
    if (d->transferQueue.isEmpty())
    {
        d->widget->progressBar()->hide();
        d->widget->progressBar()->progressCompleted();
        return;
    }

    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(d->imagesCount);

    QString imgPath = d->transferQueue.first().first.url();

    d->gphotoTalker->getPhoto(imgPath);
}

void GSWindow::slotGetPhotoDone(int errCode, const QString& errMsg,
                                const QByteArray& photoData, const QString& fileName)
{
    if (d->transferQueue.isEmpty())
    {
        return;
    }

    GSPhoto item = d->transferQueue.first().second;

    /**
     * (Trung)
     * Google Photo API now does not support title for image,
     * so we use the file name from the header. As fallback
     * the creation time for image name instead.
     */
    QString itemName(item.title);
    QString suffix(item.mimeType.section(QLatin1Char('/'), -1));

    if (itemName.isEmpty() && !fileName.isEmpty())
    {
        QFileInfo info(fileName);
        itemName = info.completeBaseName();
        suffix   = info.suffix();
    }

    if (itemName.isEmpty())
    {
        itemName = QString::fromLatin1("image-%1").arg(item.creationTime);

        // Replace colon for Windows file systems

        itemName.replace(QLatin1Char(':'), QLatin1Char('-'));
    }

    QUrl tmpUrl  = QUrl::fromLocalFile(QString(d->tmp + itemName +
                                               QLatin1Char('.') +
                                               suffix));

    if (errCode == 1)
    {
        QString errText;
        QFile imgFile(tmpUrl.toLocalFile());

        if      (!imgFile.open(QIODevice::WriteOnly))
        {
            errText = imgFile.errorString();
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "error write";
        }
        else if (imgFile.write(photoData) != photoData.size())
        {
            errText = imgFile.errorString();
        }
        else
        {
            imgFile.close();
        }

        if (errText.isEmpty())
        {
            QScopedPointer<DMetadata> meta(new DMetadata);

            if (meta->load(tmpUrl.toLocalFile()))
            {
                if (meta->supportXmp() && meta->canWriteXmp(tmpUrl.toLocalFile()))
                {
                    meta->setXmpTagString("Xmp.digiKam.picasawebGPhotoId", item.id);
                    meta->setXmpKeywords(item.tags);
                }

                if (!item.gpsLat.isEmpty() && !item.gpsLon.isEmpty())
                {
                    meta->setGPSInfo(0.0, item.gpsLat.toDouble(), item.gpsLon.toDouble());
                }

                meta->setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);
                meta->save(tmpUrl.toLocalFile());
            }

            d->transferQueue.removeFirst();
            d->imagesCount++;
        }
        else
        {
            QPointer<QMessageBox> warn = new QMessageBox(QMessageBox::Warning,
                             i18nc("@title:window", "Warning"),
                             i18nc("@info", "Failed to save photo: %1\n"
                                   "Do you want to continue?", errText),
                             QMessageBox::Yes | QMessageBox::No);

            (warn->button(QMessageBox::Yes))->setText(i18nc("@action:button", "Continue"));
            (warn->button(QMessageBox::No))->setText(i18nc("@action:button", "Cancel"));

            if (warn->exec() != QMessageBox::Yes)
            {
                slotTransferCancel();
                delete warn;
                return;
            }

            delete warn;
        }
    }
    else
    {
        QPointer<QMessageBox> warn = new QMessageBox(QMessageBox::Warning,
                         i18nc("@title:window", "Warning"),
                         i18nc("@info", "Failed to download photo: %1\n"
                              "Do you want to continue?", errMsg),
                         QMessageBox::Yes | QMessageBox::No);

        (warn->button(QMessageBox::Yes))->setText(i18nc("@action:button", "Continue"));
        (warn->button(QMessageBox::No))->setText(i18nc("@action:button", "Cancel"));

        if (warn->exec() != QMessageBox::Yes)
        {
            slotTransferCancel();
            delete warn;
            return;
        }

        delete warn;
    }

    QUrl newUrl = QUrl::fromLocalFile(QString::fromLatin1("%1/%2").arg(d->widget->getDestinationPath())
                                                                  .arg(tmpUrl.fileName()));

    newUrl      = DFileOperations::getUniqueFileUrl(newUrl);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "location" << newUrl;

    if (!QFile::rename(tmpUrl.toLocalFile(), newUrl.toLocalFile()))
    {
        QMessageBox::critical(this, i18nc("@title:window", "Error"),
                              i18nc("@info", "Failed to save image to %1",
                                   newUrl.toLocalFile()));
    }

    Q_EMIT updateHostApp(newUrl);
    downloadNextPhoto();
}

void GSWindow::slotAddPhotoDone(int err, const QString& msg)
{
    if (d->transferQueue.isEmpty())
    {
        return;
    }

    if (err == 0)
    {
        d->widget->imagesList()->processed(d->transferQueue.first().first,false);

        QPointer<QMessageBox> warn = new QMessageBox(QMessageBox::Warning,
                         i18nc("@title:window", "Warning"),
                         i18nc("@info", "Failed to upload photo to %1.\n%2\n"
                              "Do you want to continue?",
                              d->toolName, msg),
                         QMessageBox::Yes | QMessageBox::No);

        (warn->button(QMessageBox::Yes))->setText(i18nc("@action:button", "Continue"));
        (warn->button(QMessageBox::No))->setText(i18nc("@action:button", "Cancel"));

        if (warn->exec() != QMessageBox::Yes)
        {
            d->transferQueue.clear();
            d->widget->progressBar()->hide();
        }
        else
        {
            d->transferQueue.removeFirst();
            d->imagesTotal--;
            d->widget->progressBar()->setMaximum(d->imagesTotal);
            d->widget->progressBar()->setValue(d->imagesCount);
            uploadNextPhoto();
        }

        delete warn;
    }
    else
    {
        /**
         * (Trung) Take first item out of transferQueue and append to uploadQueue,
         * in order to use it again to write id in slotUploadPhotoDone
         */
        QPair<QUrl, GSPhoto> item = d->transferQueue.first();
        d->uploadQueue.append(item);

        // Remove photo uploaded from the transfer queue

        d->transferQueue.removeFirst();
        d->imagesCount++;
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "In slotAddPhotoSucceeded" << d->imagesCount;
        d->widget->progressBar()->setMaximum(d->imagesTotal);
        d->widget->progressBar()->setValue(d->imagesCount);
        uploadNextPhoto();
    }
}

void GSWindow::slotUploadPhotoDone(int err, const QString& msg, const QStringList& listPhotoId)
{
    if (err == 0)
    {
        QPointer<QMessageBox> warn = new QMessageBox(QMessageBox::Warning,
                                                     i18nc("@title:window", "Warning"),
                                                     i18nc("@info", "Failed to finish uploading photo to %1.\n%2\n"
                                                           "No image uploaded to your account.",
                                                           d->toolName, msg),
                                                     QMessageBox::Yes);

        (warn->button(QMessageBox::Yes))->setText(i18nc("@action:button", "OK"));

        d->uploadQueue.clear();
        d->widget->progressBar()->hide();

        delete warn;
    }
    else
    {
        Q_FOREACH (const QString& photoId, listPhotoId)
        {
            // Remove image from upload list and from UI

            QPair<QUrl, GSPhoto> item = d->uploadQueue.takeFirst();
            d->widget->imagesList()->removeItemByUrl(item.first);

            QUrl fileUrl = item.first;

            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "photoID:" << photoId;

            QScopedPointer<DMetadata> meta(new DMetadata);

            if (d->widget->getPhotoIdCheckBox()->isChecked() &&
                meta->supportXmp()                           &&
                meta->canWriteXmp(fileUrl.toLocalFile())     &&
                meta->load(fileUrl.toLocalFile())            &&
                !photoId.isEmpty())
            {
                meta->setXmpTagString("Xmp.digiKam.picasawebGPhotoId", photoId);
                meta->save(fileUrl.toLocalFile());
            }
        }

        if (!d->widget->imagesList()->imageUrls().isEmpty())
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "continue to upload";
            Q_EMIT d->gphotoTalker->signalReadyToUpload();
        }
    }
}

void GSWindow::slotImageListChanged()
{
    startButton()->setEnabled(!(d->widget->imagesList()->imageUrls().isEmpty()));
}

void GSWindow::slotNewAlbumRequest()
{
    switch (d->service)
    {
        case GoogleService::GDrive:
        {
            if (d->albumDlg->exec() == QDialog::Accepted)
            {
                GSFolder newFolder;
                d->albumDlg->getAlbumProperties(newFolder);
                d->currentAlbumId = d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString();
                d->talker->createFolder(newFolder.title, d->currentAlbumId);
            }

            break;
        }

        default:
        {
            if (d->gphotoAlbumDlg->exec() == QDialog::Accepted)
            {
                GSFolder newFolder;
                d->gphotoAlbumDlg->getAlbumProperties(newFolder);
                d->gphotoTalker->createAlbum(newFolder);
                d->newFolderTitle = newFolder.title;
            }

            break;
        }
    }
}

void GSWindow::slotReloadAlbumsRequest()
{
    switch (d->service)
    {
        case GoogleService::GDrive:
            d->talker->listFolders();
            break;

        case GoogleService::GPhotoImport:
        case GoogleService::GPhotoExport:
            d->gphotoTalker->listAlbums();
            break;
    }
}

void GSWindow::slotAccessTokenObtained()
{
    switch (d->service)
    {
        case GoogleService::GDrive:
            d->talker->listFolders();
            break;

        case GoogleService::GPhotoImport:
        case GoogleService::GPhotoExport:
            d->gphotoTalker->getLoggedInUser();
            break;
    }
}

void GSWindow::slotAuthenticationRefused()
{
//     QMessageBox::critical(this, i18nc("@title:window", "Error"),
//                           i18n("An authentication error occurred: account failed to link"));

    // Clear list albums

    d->widget->getAlbumsCoB()->clear();

    // Clear user name

    d->widget->updateLabels(QString());

    return;
}

void GSWindow::slotCreateFolderDone(int code, const QString& msg, const QString& albumId)
{
    switch (d->service)
    {
        case GoogleService::GDrive:
        {
            if (code == 0)
            {
                QMessageBox::critical(this, i18nc("@title:window", "Error"),
                                      i18nc("@info", "Google Drive call failed:\n%1", msg));
            }
            else
            {
                d->currentAlbumId = albumId;
                d->talker->listFolders();
            }

            break;
        }

        case GoogleService::GPhotoImport:
        case GoogleService::GPhotoExport:
        {
            if (code == 0)
            {
                QMessageBox::critical(this, i18nc("@title:window", "Error"),
                                      i18nc("@info", "Google Photos call failed:\n%1", msg));
            }
            else
            {
                d->currentAlbumId = albumId;
                d->widget->getAlbumsCoB()->addItem(QIcon::fromTheme(QLatin1String("folder")),
                                                   d->newFolderTitle, d->currentAlbumId);
                d->widget->getAlbumsCoB()->setCurrentIndex(d->widget->getAlbumsCoB()->
                                               findData(d->currentAlbumId));
            }

            break;
        }
    }
}

void GSWindow::slotTransferCancel()
{
    d->transferQueue.clear();
    d->widget->progressBar()->hide();

    switch (d->service)
    {
        case GoogleService::GDrive:
            d->talker->cancel();
            break;

        case GoogleService::GPhotoImport:
        case GoogleService::GPhotoExport:
            d->gphotoTalker->cancel();
            break;
    }
}

void GSWindow::slotUserChangeRequest()
{
    QPointer<QMessageBox> warn = new QMessageBox(QMessageBox::Warning,
                                                    i18nc("@title:window", "Warning"),
                                                    i18nc("@info", "You will be logged out of your account, "
                                                          "click \"Continue\" to authenticate for another account"),
                                                    QMessageBox::Yes | QMessageBox::No);

    (warn->button(QMessageBox::Yes))->setText(i18nc("@action:button", "Continue"));
    (warn->button(QMessageBox::No))->setText(i18nc("@action:button", "Cancel"));

    if (warn->exec() == QMessageBox::Yes)
    {
        /**
         * We do not force user to logout from their account
         * We simply unlink user account and direct use to login page to login new account
         * (In the future, we may not unlink() user, but let them change account and
         * choose which one they want to use)
         * After unlink(), waiting actively until O2 completely unlink() account, before doOAuth() again
         */
        switch (d->service)
        {
            case GoogleService::GDrive:
                d->talker->unlink();
                while(d->talker->authenticated());
                d->talker->doOAuth();
                break;

            case GoogleService::GPhotoImport:
            case GoogleService::GPhotoExport:
                d->gphotoTalker->unlink();
                while(d->gphotoTalker->authenticated());
                d->gphotoTalker->doOAuth();
                break;
        }
    }

    delete warn;
}

void GSWindow::buttonStateChange(bool state)
{
    d->widget->getNewAlbmBtn()->setEnabled(state);
    d->widget->getReloadBtn()->setEnabled(state);
    startButton()->setEnabled(state);
}

void GSWindow::slotFinished()
{
    writeSettings();
    d->transferQueue.clear();
    d->widget->imagesList()->listView()->clear();
}

void GSWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

} // namespace DigikamGenericGoogleServicesPlugin
