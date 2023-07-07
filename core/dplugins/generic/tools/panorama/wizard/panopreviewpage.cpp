/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 *
 * SPDX-FileCopyrightText: 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "panopreviewpage.h"

// Qt includes

#include <QLabel>
#include <QMutex>
#include <QMutexLocker>
#include <QTextDocument>
#include <QStandardPaths>
#include <QVBoxLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dpreviewmanager.h"
#include "dprogresswdg.h"
#include "dhistoryview.h"
#include "panomanager.h"
#include "panoactionthread.h"
#include "enblendbinary.h"
#include "makebinary.h"
#include "nonabinary.h"
#include "pto2mkbinary.h"
#include "huginexecutorbinary.h"
#include "dlayoutbox.h"

namespace DigikamGenericPanoramaPlugin
{

class Q_DECL_HIDDEN PanoPreviewPage::Private
{
public:

    explicit Private(PanoManager* const m)
      : title           (nullptr),
        previewWidget   (nullptr),
        previewBusy     (false),
        previewDone     (false),
        stitchingBusy   (false),
        stitchingDone   (false),
        postProcessing  (nullptr),
        progressBar     (nullptr),
        curProgress     (0),
        totalProgress   (0),
        canceled        (false),
        mngr            (m),
        dlg             (nullptr)
    {
    }

    QLabel*                title;

    DPreviewManager*       previewWidget;
    bool                   previewBusy;
    bool                   previewDone;
    bool                   stitchingBusy;
    bool                   stitchingDone;
    DHistoryView*          postProcessing;
    DProgressWdg*          progressBar;
    int                    curProgress;
    int                    totalProgress;
    QMutex                 previewBusyMutex;      ///< This is a precaution in case the user does a back / next action at the wrong moment
    bool                   canceled;

    QString                output;

    PanoManager*           mngr;

    QWizard*               dlg;
};

PanoPreviewPage::PanoPreviewPage(PanoManager* const mngr, QWizard* const dlg)
    : DWizardPage(dlg, i18nc("@title:window", "<b>Preview and Post-Processing</b>")),
      d(new Private(mngr))
{
    d->dlg            = dlg;

    DVBox* const vbox = new DVBox(this);

    d->title          = new QLabel(vbox);
    d->title->setOpenExternalLinks(true);
    d->title->setWordWrap(true);

    d->previewWidget  = new DPreviewManager(vbox);
    d->previewWidget->setButtonText(i18nc("@action:button", "Details..."));

    d->postProcessing = new DHistoryView(vbox);
    d->progressBar    = new DProgressWdg(vbox);

    setPageWidget(vbox);

    QPixmap leftPix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/assistant-hugin.png")));
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));

    connect(d->progressBar, SIGNAL(signalProgressCanceled()),
            this, SLOT(slotCancel()));
}

PanoPreviewPage::~PanoPreviewPage()
{
    delete d;
}

void PanoPreviewPage::computePreview()
{
    // Cancel any stitching being processed

    if (d->stitchingBusy)
    {
        cleanupPage();
    }

    QMutexLocker lock(&d->previewBusyMutex);

    connect(d->mngr->thread(), SIGNAL(stepFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
            this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

    connect(d->mngr->thread(), SIGNAL(jobCollectionFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
            this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

    d->canceled = false;

    d->previewWidget->setBusy(true, i18n("Processing Panorama Preview..."));
    d->previewDone = false;
    d->previewBusy = true;

    d->mngr->resetPreviewPto();
    d->mngr->resetPreviewUrl();
    d->mngr->resetPreviewMkUrl();
    d->mngr->thread()->generatePanoramaPreview(d->mngr->viewAndCropOptimisePtoData(),
                                               d->mngr->previewPtoUrl(),
                                               d->mngr->previewMkUrl(),
                                               d->mngr->previewUrl(),
                                               d->mngr->preProcessedMap(),
                                               d->mngr->makeBinary().path(),
                                               d->mngr->pto2MkBinary().path(),
                                               d->mngr->huginExecutorBinary().path(),
                                               d->mngr->hugin2015(),
                                               d->mngr->enblendBinary().path(),
                                               d->mngr->nonaBinary().path());
}

void PanoPreviewPage::startStitching()
{
    QMutexLocker lock(&d->previewBusyMutex);

    if (d->previewBusy)
    {
        // The real beginning of the stitching starts after preview has finished / failed

        connect(this, SIGNAL(signalPreviewFinished()), this, SLOT(slotStartStitching()));
        cleanupPage(lock);

        return;
    }

    connect(d->mngr->thread(), SIGNAL(starting(DigikamGenericPanoramaPlugin::PanoActionData)),
            this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

    connect(d->mngr->thread(), SIGNAL(stepFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
            this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

    connect(d->mngr->thread(), SIGNAL(jobCollectionFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
            this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

    d->canceled      = false;
    d->stitchingBusy = true;
    d->curProgress   = 0;

    if (d->mngr->hugin2015())
    {
        d->totalProgress = 1;
    }
    else
    {
        d->totalProgress = d->mngr->preProcessedMap().size() + 1;
    }

    d->previewWidget->hide();

    QSize panoSize      = d->mngr->viewAndCropOptimisePtoData()->project.size;
    QRect panoSelection = d->mngr->viewAndCropOptimisePtoData()->project.crop;

    if (d->previewDone)
    {
        QSize previewSize = d->mngr->previewPtoData()->project.size;
        QRectF selection  = d->previewWidget->getSelectionArea();
        QRectF proportionSelection(selection.x()      / previewSize.width(),
                                   selection.y()      / previewSize.height(),
                                   selection.width()  / previewSize.width(),
                                   selection.height() / previewSize.height());

        // At this point, if no selection area was created, proportionSelection is null,
        // hence panoSelection becomes a null rectangle

        panoSelection     = QRect(proportionSelection.x()      * panoSize.width(),
                                  proportionSelection.y()      * panoSize.height(),
                                  proportionSelection.width()  * panoSize.width(),
                                  proportionSelection.height() * panoSize.height());
    }

    d->title->setText(i18n("<qt>"
                           "<p><h1>Panorama Post-Processing</h1></p>"
                           "</qt>"));

    d->progressBar->reset();
    d->progressBar->setMaximum(d->totalProgress);
    d->progressBar->progressScheduled(i18nc("@title:group", "Panorama Post-Processing"), true, true);
    d->progressBar->progressThumbnailChanged(QIcon::fromTheme(QLatin1String("panorama")).pixmap(22, 22));
    d->progressBar->show();
    d->postProcessing->show();

    d->mngr->resetPanoPto();
    d->mngr->resetMkUrl();
    d->mngr->resetPanoUrl();
    d->mngr->thread()->compileProject(d->mngr->viewAndCropOptimisePtoData(),
                                      d->mngr->panoPtoUrl(),
                                      d->mngr->mkUrl(),
                                      d->mngr->panoUrl(),
                                      d->mngr->preProcessedMap(),
                                      d->mngr->format(),
                                      panoSelection,
                                      d->mngr->makeBinary().path(),
                                      d->mngr->pto2MkBinary().path(),
                                      d->mngr->huginExecutorBinary().path(),
                                      d->mngr->hugin2015(),
                                      d->mngr->enblendBinary().path(),
                                      d->mngr->nonaBinary().path());
}

void PanoPreviewPage::preInitializePage()
{
    d->title->setText(QString());
    d->previewWidget->show();
    d->progressBar->progressCompleted();
    d->progressBar->hide();
    d->postProcessing->hide();

    setComplete(true);

    Q_EMIT completeChanged();
}

void PanoPreviewPage::initializePage()
{
    preInitializePage();

    computePreview();
}

bool PanoPreviewPage::validatePage()
{
    if (d->stitchingDone)
    {
        return true;
    }

    setComplete(false);
    startStitching();

    return false;
}

void PanoPreviewPage::cleanupPage()
{
    QMutexLocker lock(&d->previewBusyMutex);
    cleanupPage(lock);
}

#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))
void PanoPreviewPage::cleanupPage(QMutexLocker<QMutex>& /*lock*/)
#else
void PanoPreviewPage::cleanupPage(QMutexLocker& /*lock*/)
#endif
{
    d->canceled = true;

    d->mngr->thread()->cancel();
    d->progressBar->progressCompleted();

    if      (d->previewBusy)
    {
        d->previewBusy = false;
        d->previewWidget->setBusy(false);
        d->previewWidget->setText(i18n("Preview Processing Cancelled."));
    }
    else if (d->stitchingBusy)
    {
        d->stitchingBusy = false;
    }
}

void PanoPreviewPage::slotCancel()
{
    d->dlg->reject();
}

void PanoPreviewPage::slotStartStitching()
{
    disconnect(this, SIGNAL(signalPreviewFinished()),
               this, SLOT(slotStartStitching()));

    startStitching();
}

void PanoPreviewPage::slotPanoAction(const DigikamGenericPanoramaPlugin::PanoActionData& ad)
{
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "SlotPanoAction (preview)";
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "\tstarting, success, canceled, action: " << ad.starting << ad.success << d->canceled << ad.action;

    QString      text;

    QMutexLocker lock(&d->previewBusyMutex);

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "\tbusy (preview / stitch):" << d->previewBusy << d->stitchingBusy;

    if (!ad.starting)           // Something is complete...
    {
        if (!ad.success)        // Something is failed...
        {
            switch (ad.action)
            {
                case PANO_CREATEPREVIEWPTO:
                case PANO_NONAFILEPREVIEW:
                case PANO_STITCHPREVIEW:
                case PANO_CREATEMKPREVIEW:
                case PANO_HUGINEXECUTORPREVIEW:
                {
                    if (!d->previewBusy)
                    {
                        lock.unlock();
                        Q_EMIT signalPreviewFinished();
                        return;
                    }

                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    d->output      = ad.message;
                    d->previewWidget->setBusy(false);
                    d->previewBusy = false;

                    qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Preview compilation failed: " << ad.message;
                    QString errorString(i18n("<h1><b>Error</b></h1><p>%1</p>",
                                              ad.message));
                    d->previewWidget->setText(errorString);
                    d->previewWidget->setSelectionAreaPossible(false);

                    setComplete(false);
                    Q_EMIT completeChanged();

                    lock.unlock();
                    Q_EMIT signalPreviewFinished();

                    break;
                }

                case PANO_CREATEMK:
                {
                    if (!d->stitchingBusy)
                    {
                        return;
                    }

                    disconnect(d->mngr->thread(), SIGNAL(starting(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    d->stitchingBusy = false;
                    QString message  = i18nc("Here a makefile is a script for GNU Make",
                                             "<p><b>Cannot create makefile: </b></p><p>%1</p>",
                                             ad.message);
                    qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "pto2mk call failed";
                    d->postProcessing->addEntry(message, DHistoryView::ErrorEntry);

                    setComplete(false);
                    Q_EMIT completeChanged();

                    break;
                }

                case PANO_CREATEFINALPTO:
                {
                    if (!d->stitchingBusy)
                    {
                        return;
                    }

                    disconnect(d->mngr->thread(), SIGNAL(starting(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    d->stitchingBusy = false;
                    QString message  = i18nc("a project file is a .pto file, as given to hugin to build a panorama",
                                             "<p><b>Cannot create project file: </b></p><p>%1</p>",
                                             ad.message);
                    qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "pto creation failed";
                    d->postProcessing->addEntry(message, DHistoryView::ErrorEntry);

                    setComplete(false);
                    Q_EMIT completeChanged();

                    break;
                }

                case PANO_NONAFILE:
                {
                    if (!d->stitchingBusy)
                    {
                        return;
                    }

                    disconnect(d->mngr->thread(), SIGNAL(starting(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    d->stitchingBusy = false;
                    QString message  = i18nc("Error message for image file number %1 out of %2",
                                             "<p><b>Processing file %1 / %2: </b></p><p>%3</p>",
                                             ad.id + 1,
                                             d->totalProgress - 1,
                                             ad.message);
                    qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Nona call failed for file #" << ad.id;
                    d->postProcessing->addEntry(message, DHistoryView::ErrorEntry);

                    setComplete(false);
                    Q_EMIT completeChanged();

                    break;
                }

                case PANO_STITCH:
                case PANO_HUGINEXECUTOR:
                {
                    if (!d->stitchingBusy)
                    {
                        return;
                    }

                    disconnect(d->mngr->thread(), SIGNAL(starting(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    d->stitchingBusy = false;
                    d->postProcessing->addEntry(i18nc("Error message for panorama compilation",
                                                         "<p><b>Panorama compilation: </b></p><p>%1</p>",
                                                         ad.message.toHtmlEscaped()), DHistoryView::ErrorEntry);
                    qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Enblend call failed";

                    setComplete(false);
                    Q_EMIT completeChanged();

                    break;
                }

                default:
                {
                    qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Unknown action (preview) " << ad.action;
                    break;
                }
            }
        }
        else                    // Something is done...
        {
            switch (ad.action)
            {
                case PANO_CREATEPREVIEWPTO:
                case PANO_CREATEMKPREVIEW:
                case PANO_NONAFILEPREVIEW:
                case PANO_CREATEFINALPTO:
                case PANO_CREATEMK:
                {
                    // Nothing to do yet, a step is finished, that's all
                    break;
                }

                case PANO_STITCHPREVIEW:
                case PANO_HUGINEXECUTORPREVIEW:
                {
                    if (d->previewBusy)
                    {
                        disconnect(d->mngr->thread(), SIGNAL(stepFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                                   this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                        disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                                   this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));
                    }

                    d->previewBusy = false;
                    d->previewDone = true;

                    lock.unlock();
                    Q_EMIT signalPreviewFinished();

                    d->title->setText(i18n("<qt>"
                                           "<h1>Panorama Preview</h1>"
                                           "<p>Draw a rectangle if you want to crop the image.</p>"
                                           "<p>Pressing the <i>Next</i> button will then launch the final "
                                           "stitching process.</p>"
                                           "</qt>"));
                    d->previewWidget->setSelectionAreaPossible(true);
/*
                    d->previewWidget->load(QUrl::fromLocalFile(d->mngr->previewUrl().toLocalFile()), true);
*/
                    d->previewWidget->load(d->mngr->previewUrl(), true);
                    QSize panoSize    = d->mngr->viewAndCropOptimisePtoData()->project.size;
                    QRect panoCrop    = d->mngr->viewAndCropOptimisePtoData()->project.crop;
                    QSize previewSize = d->mngr->previewPtoData()->project.size;
                    d->previewWidget->setSelectionArea(QRectF(
                        ((double) panoCrop.left())   / panoSize.width()  * previewSize.width(),
                        ((double) panoCrop.top())    / panoSize.height() * previewSize.height(),
                        ((double) panoCrop.width())  / panoSize.width()  * previewSize.width(),
                        ((double) panoCrop.height()) / panoSize.height() * previewSize.height()
                    ));

                    break;
                }

                case PANO_NONAFILE:
                {
                    QString message = i18nc("Success for image file number %1 out of %2",
                                            "Processing file %1 / %2",
                                            ad.id + 1,
                                            d->totalProgress - 1);
                    d->postProcessing->addEntry(message, DHistoryView::SuccessEntry);
                    d->curProgress++;
                    d->progressBar->setValue(d->curProgress);
                    d->progressBar->setMaximum(d->totalProgress);

                    break;
                }

                case PANO_STITCH:
                case PANO_HUGINEXECUTOR:
                {
                    if (!d->stitchingBusy)
                    {
                        return;
                    }

                    disconnect(d->mngr->thread(), SIGNAL(starting(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(DigikamGenericPanoramaPlugin::PanoActionData)),
                               this, SLOT(slotPanoAction(DigikamGenericPanoramaPlugin::PanoActionData)));

                    d->stitchingBusy = false;
                    d->postProcessing->addEntry(i18nc("Success for panorama compilation", "Panorama compilation"), DHistoryView::SuccessEntry);
                    d->curProgress++;
                    d->progressBar->setValue(d->curProgress);
                    d->progressBar->setMaximum(d->totalProgress);
                    d->progressBar->progressCompleted();
                    d->progressBar->hide();
                    d->postProcessing->hide();
                    d->stitchingDone = true;

                    Q_EMIT signalStitchingFinished();
                    preInitializePage();

                    break;
                }

                default:
                {
                    qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Unknown action (preview) " << ad.action;

                    break;
                }
            }
        }
    }
    else           // Some step is started...
    {
        switch (ad.action)
        {
            case PANO_CREATEPREVIEWPTO:
            case PANO_CREATEMKPREVIEW:
            case PANO_NONAFILEPREVIEW:
            case PANO_STITCHPREVIEW:
            case PANO_CREATEFINALPTO:
            case PANO_CREATEMK:
            case PANO_HUGINEXECUTORPREVIEW:
            {
                // Nothing to do...

                break;
            }

            case PANO_NONAFILE:
            {
                QString message = i18nc("Compilation started for image file number %1 out of %2",
                                        "Processing file %1 / %2",
                                        ad.id + 1,
                                        d->totalProgress - 1);
                d->postProcessing->addEntry(message, DHistoryView::StartingEntry);

                break;
            }

            case PANO_STITCH:
            case PANO_HUGINEXECUTOR:
            {
                lock.unlock();
                d->postProcessing->addEntry(i18nc("Panorama compilation started", "Panorama compilation"), DHistoryView::StartingEntry);

                break;
            }

            default:
            {
                qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Unknown starting action (preview) " << ad.action;

                break;
            }
        }
    }
}

} // namespace DigikamGenericPanoramaPlugin
