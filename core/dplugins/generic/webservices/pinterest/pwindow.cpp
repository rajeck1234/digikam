/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Pinterest web service
 *
 * SPDX-FileCopyrightText: 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "pwindow.h"

// Qt includes

#include <QPointer>
#include <QWindow>
#include <QSpinBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QCloseEvent>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "ditemslist.h"
#include "digikam_version.h"
#include "ptalker.h"
#include "pitem.h"
#include "pnewalbumdlg.h"
#include "pwidget.h"

namespace DigikamGenericPinterestPlugin
{

class Q_DECL_HIDDEN PWindow::Private
{
public:

    explicit Private()
      : imagesCount(0),
        imagesTotal(0),
        widget     (nullptr),
        albumDlg   (nullptr),
        talker     (nullptr)
    {
    }

    unsigned int  imagesCount;
    unsigned int  imagesTotal;

    PWidget*      widget;
    PNewAlbumDlg* albumDlg;
    PTalker*      talker;

    QString       currentAlbumName;
    QList<QUrl>   transferQueue;
};

PWindow::PWindow(DInfoInterface* const iface,
                 QWidget* const /*parent*/)
    : WSToolDialog(nullptr, QLatin1String("Pinterest Export Dialog")),
      d           (new Private)
{
    d->widget = new PWidget(this, iface, QLatin1String("Pinterest"));

    d->widget->imagesList()->setIface(iface);

    setMainWidget(d->widget);
    setModal(false);
    setWindowTitle(i18nc("@title:window", "Export to Pinterest"));

    startButton()->setText(i18nc("@action:button", "Start Upload"));
    startButton()->setToolTip(i18nc("@info:tooltip, button", "Start upload to Pinterest"));

    d->widget->setMinimumSize(700, 500);

    connect(d->widget->imagesList(), SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    connect(d->widget->getChangeUserBtn(), SIGNAL(clicked()),
            this, SLOT(slotUserChangeRequest()));

    connect(d->widget->getNewAlbmBtn(), SIGNAL(clicked()),
            this, SLOT(slotNewBoardRequest()));

    connect(d->widget->getReloadBtn(), SIGNAL(clicked()),
            this, SLOT(slotReloadBoardsRequest()));

    connect(startButton(), SIGNAL(clicked()),
            this, SLOT(slotStartTransfer()));

    d->albumDlg = new PNewAlbumDlg(this, QLatin1String("Pinterest"));
    d->talker   = new PTalker(this);

    connect(d->talker,SIGNAL(signalBusy(bool)),
            this,SLOT(slotBusy(bool)));

    connect(d->talker,SIGNAL(signalLinkingFailed()),
            this,SLOT(slotSignalLinkingFailed()));

    connect(d->talker,SIGNAL(signalNetworkError()),
            this,SLOT(slotTransferCancel()));

    connect(d->talker,SIGNAL(signalLinkingSucceeded()),
            this,SLOT(slotSignalLinkingSucceeded()));

    connect(d->talker,SIGNAL(signalSetUserName(QString)),
            this,SLOT(slotSetUserName(QString)));

    connect(d->talker,SIGNAL(signalListBoardsFailed(QString)),
            this,SLOT(slotListBoardsFailed(QString)));

    connect(d->talker,SIGNAL(signalListBoardsDone(QList<QPair<QString,QString> >)), // krazy:exclude=normalize
            this,SLOT(slotListBoardsDone(QList<QPair<QString,QString> >)));         // krazy:exclude=normalize

    connect(d->talker,SIGNAL(signalCreateBoardFailed(QString)),
            this,SLOT(slotCreateBoardFailed(QString)));

    connect(d->talker,SIGNAL(signalCreateBoardSucceeded()),
            this,SLOT(slotCreateBoardSucceeded()));

    connect(d->talker,SIGNAL(signalAddPinFailed(QString)),
            this,SLOT(slotAddPinFailed(QString)));

    connect(d->talker,SIGNAL(signalAddPinSucceeded()),
            this,SLOT(slotAddPinSucceeded()));

    connect(this, SIGNAL(finished(int)),
            this, SLOT(slotFinished()));

    readSettings();
    buttonStateChange(false);

    d->talker->readSettings();
}

PWindow::~PWindow()
{
    delete d->widget;
    delete d->albumDlg;
    delete d->talker;
    delete d;
}

void PWindow::readSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group("Pinterest Settings");
    d->currentAlbumName     = grp.readEntry("Current Album", QString());

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

    d->widget->getDimensionSpB()->setValue(grp.readEntry("Maximum Width",  1600));
    d->widget->getImgQualitySpB()->setValue(grp.readEntry("Image Quality", 90));
}

void PWindow::writeSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group("Pinterest Settings");

    grp.writeEntry("Current Album", d->currentAlbumName);
    grp.writeEntry("Resize",        d->widget->getResizeCheckBox()->isChecked());
    grp.writeEntry("Maximum Width", d->widget->getDimensionSpB()->value());
    grp.writeEntry("Image Quality", d->widget->getImgQualitySpB()->value());
}

void PWindow::reactivate()
{
    d->widget->imagesList()->loadImagesFromCurrentSelection();
    d->widget->progressBar()->hide();

    show();
}

void PWindow::setItemsList(const QList<QUrl>& urls)
{
    d->widget->imagesList()->slotAddImages(urls);
}

void PWindow::slotBusy(bool val)
{
    if (val)
    {
        setCursor(Qt::WaitCursor);
        d->widget->getChangeUserBtn()->setEnabled(false);
        buttonStateChange(false);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        d->widget->getChangeUserBtn()->setEnabled(true);
        buttonStateChange(true);
    }
}

void PWindow::slotSetUserName(const QString& msg)
{
    d->widget->updateLabels(msg, QLatin1String(""));
}

void PWindow::slotListBoardsDone(const QList<QPair<QString,QString> >& list)
{
    d->widget->getAlbumsCoB()->clear();

    for (int i = 0 ; i < list.size() ; ++i)
    {
        d->widget->getAlbumsCoB()->addItem(
            QIcon::fromTheme(QLatin1String("system-users")),
        list.value(i).second, list.value(i).first);

        if (d->currentAlbumName == list.value(i).second)
        {
            d->widget->getAlbumsCoB()->setCurrentIndex(i);
        }
    }

    buttonStateChange(true);
    d->talker->getUserName();
}

void PWindow::slotStartTransfer()
{
    d->widget->imagesList()->clearProcessedStatus();

    if (d->widget->imagesList()->imageUrls().isEmpty())
    {
        QMessageBox::critical(this, i18nc("@title:window", "Error"),
                              i18n("No image selected. Please select which images should be uploaded."));
        return;
    }

    if (!(d->talker->authenticated()))
    {
        QPointer<QMessageBox> warn = new QMessageBox(QMessageBox::Warning,
            i18nc("@title:window", "Warning"),
            i18n("Authentication failed. Click \"Continue\" to authenticate."),
            QMessageBox::Yes | QMessageBox::No);

        warn->button(QMessageBox::Yes)->setText(i18nc("@action:button", "Continue"));
        warn->button(QMessageBox::No)->setText(i18nc("@action:button", "Cancel"));

        if (warn->exec() == QMessageBox::Yes)
        {
            d->talker->link();
            delete warn;
            return;
        }
        else
        {
            delete warn;
            return;
        }
    }

    d->transferQueue = d->widget->imagesList()->imageUrls();

    if (d->transferQueue.isEmpty())
    {
        return;
    }

    d->currentAlbumName = d->widget->getAlbumsCoB()->currentText();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "StartTransfer:" << d->currentAlbumName
                                     << "INDEX:" << d->widget->getAlbumsCoB()->currentIndex();
    d->imagesTotal      = d->transferQueue.count();
    d->imagesCount      = 0;

    d->widget->progressBar()->setFormat(i18n("%v / %m"));
    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(0);
    d->widget->progressBar()->show();
    d->widget->progressBar()->progressScheduled(i18n("Pinterest export"), true, true);
    d->widget->progressBar()->progressThumbnailChanged(
        QIcon::fromTheme(QLatin1String("dk-pinterest")).pixmap(22, 22));

    uploadNextPhoto();
}

void PWindow::uploadNextPhoto()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "uploadNextPhoto:" << d->transferQueue.count();

    if (d->transferQueue.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "empty";
        d->widget->progressBar()->progressCompleted();
        return;
    }

    QString imgPath = d->transferQueue.first().toLocalFile();
    QString boardID = d->widget->getAlbumsCoB()->itemData(d->widget->getAlbumsCoB()->currentIndex()).toString();

    bool result = d->talker->addPin(imgPath,
                                    boardID,
                                    d->widget->getResizeCheckBox()->isChecked(),
                                    d->widget->getDimensionSpB()->value(),
                                    d->widget->getImgQualitySpB()->value());

    if (!result)
    {
        slotAddPinFailed(QLatin1String(""));
        return;
    }
}

void PWindow::slotAddPinFailed(const QString& msg)
{
    if (QMessageBox::question(this, i18nc("@title:window", "Uploading Failed"),
                              i18n("Failed to upload photo to Pinterest."
                                   "\n%1\n"
                                   "Do you want to continue?", msg))
        != QMessageBox::Yes)
    {
        d->transferQueue.clear();
        d->widget->progressBar()->hide();
        d->widget->progressBar()->progressCompleted();
    }
    else
    {
        d->transferQueue.pop_front();
        d->imagesTotal--;
        d->widget->progressBar()->setMaximum(d->imagesTotal);
        d->widget->progressBar()->setValue(d->imagesCount);
        uploadNextPhoto();
    }
}

void PWindow::slotAddPinSucceeded()
{
    // Remove photo uploaded from the list

    d->widget->imagesList()->removeItemByUrl(d->transferQueue.first());
    d->transferQueue.pop_front();
    d->imagesCount++;
    d->widget->progressBar()->setMaximum(d->imagesTotal);
    d->widget->progressBar()->setValue(d->imagesCount);
    uploadNextPhoto();
}

void PWindow::slotImageListChanged()
{
    startButton()->setEnabled(!(d->widget->imagesList()->imageUrls().isEmpty()));
}

void PWindow::slotNewBoardRequest()
{
    if (d->albumDlg->exec() == QDialog::Accepted)
    {
        PFolder newFolder;
        d->albumDlg->getFolderTitle(newFolder);
        d->currentAlbumName = newFolder.title;
        d->talker->createBoard(d->currentAlbumName);
    }
}

void PWindow::slotReloadBoardsRequest()
{
    d->talker->listBoards();
}

void PWindow::slotSignalLinkingFailed()
{
    slotSetUserName(QLatin1String(""));
    d->widget->getAlbumsCoB()->clear();

    if (QMessageBox::question(this, i18nc("@title:window", "Login Failed"),
                              i18n("Authentication failed. Do you want to try again?"))
        == QMessageBox::Yes)
    {
        d->talker->link();
    }
}

void PWindow::slotSignalLinkingSucceeded()
{
    d->talker->listBoards();
}

void PWindow::slotListBoardsFailed(const QString& msg)
{
    QMessageBox::critical(this, QString(), i18n("Pinterest call failed:\n%1", msg));
}

void PWindow::slotCreateBoardFailed(const QString& msg)
{
    QMessageBox::critical(this, QString(), i18n("Pinterest call failed:\n%1", msg));
}

void PWindow::slotCreateBoardSucceeded()
{
    d->talker->listBoards();
}

void PWindow::slotTransferCancel()
{
    d->talker->cancel();
    d->transferQueue.clear();
    d->widget->progressBar()->hide();
    d->widget->progressBar()->progressCompleted();
}

void PWindow::slotUserChangeRequest()
{
    slotSetUserName(QLatin1String(""));
    d->widget->getAlbumsCoB()->clear();
    d->talker->unLink();
    d->talker->link();
}

void PWindow::buttonStateChange(bool state)
{
    d->widget->getNewAlbmBtn()->setEnabled(state);
    d->widget->getReloadBtn()->setEnabled(state);
    startButton()->setEnabled(state);
}

void PWindow::slotFinished()
{
    writeSettings();
    slotTransferCancel();
    d->widget->imagesList()->listView()->clear();
}

void PWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    slotFinished();
    e->accept();
}

} // namespace DigikamGenericPinterestPlugin
