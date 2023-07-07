/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "vidslidefinalpage.h"

// Qt includes

#include <QIcon>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <QStyle>
#include <QTimer>
#include <QDir>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "vidslidewizard.h"
#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "dprogresswdg.h"
#include "dhistoryview.h"
#include "vidslidethread.h"
#include "vidplayerdlg.h"

namespace DigikamGenericVideoSlideShowPlugin
{

class Q_DECL_HIDDEN VidSlideFinalPage::Private
{
public:

    explicit Private(QWizard* const dialog)
      : progressView(nullptr),
        progressBar(nullptr),
        complete(false),
        encoder(nullptr),
        wizard(nullptr),
        settings(nullptr),
        iface(nullptr)
    {
        wizard = dynamic_cast<VidSlideWizard*>(dialog);

        if (wizard)
        {
            iface    = wizard->iface();
            settings = wizard->settings();
        }
    }

    DHistoryView*     progressView;
    DProgressWdg*     progressBar;
    bool              complete;
    VidSlideThread*   encoder;
    VidSlideWizard*   wizard;
    VidSlideSettings* settings;
    DInfoInterface*   iface;
};

VidSlideFinalPage::VidSlideFinalPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    setObjectName(QLatin1String("FinalPage"));

    DVBox* const vbox = new DVBox(this);
    d->progressView   = new DHistoryView(vbox);
    d->progressBar    = new DProgressWdg(vbox);

    vbox->setStretchFactor(d->progressBar, 10);
    vbox->setContentsMargins(QMargins());
    vbox->setSpacing(qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                             QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing)));

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("system-run")));
}

VidSlideFinalPage::~VidSlideFinalPage()
{
    if (d->encoder)
        d->encoder->cancel();

    delete d;
}

void VidSlideFinalPage::initializePage()
{
    d->complete = false;
    Q_EMIT completeChanged();
    QTimer::singleShot(0, this, SLOT(slotProcess()));
}

void VidSlideFinalPage::slotProcess()
{
    if (!d->wizard)
    {
        d->progressView->addEntry(i18n("Internal Error"),
                                  DHistoryView::ErrorEntry);
        return;
    }

    d->progressView->clear();
    d->progressBar->reset();

    d->progressView->addEntry(i18n("Starting to generate video slideshow..."),
                              DHistoryView::ProgressEntry);

    d->progressView->addEntry(i18n("%1 input images to process", d->settings->inputImages.count()),
                                  DHistoryView::ProgressEntry);

    Q_FOREACH (const QUrl& url, d->settings->inputImages)
    {
        d->progressView->addEntry(QDir::toNativeSeparators(url.toLocalFile()),
                                  DHistoryView::ProgressEntry);
    }

    if (!d->settings->inputAudio.isEmpty())
    {
        d->progressView->addEntry(i18n("%1 input audio stream to process:",
                                       d->settings->inputAudio.count()),
                                  DHistoryView::ProgressEntry);

        Q_FOREACH (const QUrl& url, d->settings->inputAudio)
        {
            d->progressView->addEntry(QDir::toNativeSeparators(url.toLocalFile()),
                                      DHistoryView::ProgressEntry);
        }
    }

    d->progressBar->setMinimum(0);
    d->progressBar->setMaximum(d->settings->inputImages.count());

    d->encoder = new VidSlideThread(this);

    connect(d->encoder, SIGNAL(signalProgress(int)),
            d->progressBar, SLOT(setValue(int)));

    connect(d->encoder, SIGNAL(signalMessage(QString,bool)),
            this, SLOT(slotMessage(QString,bool)));

    connect(d->encoder, SIGNAL(signalDone(bool)),
            this, SLOT(slotDone(bool)));

    d->encoder->processStream(d->settings);
    d->encoder->start();
}

void VidSlideFinalPage::cleanupPage()
{
    if (d->encoder)
        d->encoder->cancel();
}

void VidSlideFinalPage::slotMessage(const QString& mess, bool err)
{
    d->progressView->addEntry(mess, err ? DHistoryView::ErrorEntry
                                        : DHistoryView::ProgressEntry);
}

void VidSlideFinalPage::slotDone(bool completed)
{
    d->progressBar->progressCompleted();
    d->complete = completed;

    if (!d->complete)
    {
        d->progressView->addEntry(i18n("Video Slideshow is not completed"),
                                  DHistoryView::WarningEntry);
    }
    else
    {
        d->progressView->addEntry(i18n("Video Slideshow completed."),
                                  DHistoryView::ProgressEntry);

        if (d->settings->outputPlayer != VidSlideSettings::NOPLAYER)
        {
            d->progressView->addEntry(i18n("Opening video stream in player..."),
                                      DHistoryView::ProgressEntry);

            if (d->settings->outputPlayer == VidSlideSettings::INTERNAL)
            {
                VidPlayerDlg* const player = new VidPlayerDlg(d->settings->outputVideo, this);
                player->show();
                player->resize(800, 600);
            }
            else
            {
                QDesktopServices::openUrl(QUrl::fromLocalFile(d->settings->outputVideo));
            }
        }
    }

    Q_EMIT completeChanged();
}

bool VidSlideFinalPage::isComplete() const
{
    return d->complete;
}

} // namespace DigikamGenericVideoSlideShowPlugin
