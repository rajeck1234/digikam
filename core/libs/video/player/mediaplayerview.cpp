/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-20-12
 * Description : a view to embed QtAv media player.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mediaplayerview.h"

// Qt includes

#include <QApplication>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QProxyStyle>
#include <QPushButton>
#include <QFileInfo>
#include <QToolBar>
#include <QAction>
#include <QSlider>
#include <QLabel>
#include <QFrame>
#include <QEvent>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "thememanager.h"
#include "dlayoutbox.h"
#include "metaengine.h"
#include "dmetadata.h"
#include "WidgetRenderer.h"
#include "AudioDecoder.h"
#include "VideoDecoder.h"
#include "VideoCapture.h"
#include "QtAV.h"
#include "QtAV_Version.h"
#include "AVPlayerConfigMngr.h"
#include "DecoderConfigPage.h"

using namespace QtAV;

namespace Digikam
{

class Q_DECL_HIDDEN MediaPlayerMouseClickFilter : public QObject
{
    Q_OBJECT

public:

    explicit MediaPlayerMouseClickFilter(QObject* const parent)
        : QObject (parent),
          m_parent(parent)
    {
    }

protected:

    bool eventFilter(QObject* obj, QEvent* event) override
    {
        if ((event->type() == QEvent::MouseButtonRelease) || (event->type() == QEvent::MouseButtonDblClick))
        {
            bool singleClick              = qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick);
            QMouseEvent* const mouseEvent = dynamic_cast<QMouseEvent*>(event);

            if (m_parent && mouseEvent)
            {
                MediaPlayerView* const mplayer = dynamic_cast<MediaPlayerView*>(m_parent);

                if (mplayer)
                {
                    if      ((mouseEvent->button() == Qt::LeftButton) &&
                             ((singleClick && (event->type() == QEvent::MouseButtonRelease)) ||
                             (!singleClick && (event->type() == QEvent::MouseButtonDblClick))))
                    {
                        mplayer->slotEscapePressed();
                    }
                    else if ((mouseEvent->button() == Qt::RightButton) &&
                             (event->type() == QEvent::MouseButtonRelease))
                    {
                        mplayer->slotRotateVideo();
                    }

                    return true;
                }
            }
        }

        return QObject::eventFilter(obj, event);
    }

private:

    QObject* m_parent;
};

// --------------------------------------------------------

class Q_DECL_HIDDEN PlayerVideoStyle : public QProxyStyle
{
    Q_OBJECT

public:

    using QProxyStyle::QProxyStyle;

    int styleHint(QStyle::StyleHint hint,
                  const QStyleOption* option = nullptr,
                  const QWidget* widget = nullptr,
                  QStyleHintReturn* returnData = nullptr) const override
    {
        if (hint == QStyle::SH_Slider_AbsoluteSetButtons)
        {
            return (Qt::LeftButton | Qt::MiddleButton | Qt::RightButton);
        }

        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

// --------------------------------------------------------

class Q_DECL_HIDDEN MediaPlayerView::Private
{

public:

    enum MediaPlayerViewMode
    {
        ErrorView = 0,
        PlayerView
    };

public:

    explicit Private()
      : errorView       (nullptr),
        playerView      (nullptr),
        prevAction      (nullptr),
        nextAction      (nullptr),
        playAction      (nullptr),
        grabAction      (nullptr),
        loopPlay        (nullptr),
        toolBar         (nullptr),
        iface           (nullptr),
        videoWidget     (nullptr),
        player          (nullptr),
        slider          (nullptr),
        volume          (nullptr),
        tlabel          (nullptr),
        videoOrientation(0),
        capturePosition (0),
        sliderTime      (0)
    {
    }

    QFrame*              errorView;
    QFrame*              playerView;

    QAction*             prevAction;
    QAction*             nextAction;
    QAction*             playAction;
    QAction*             grabAction;

    QPushButton*         loopPlay;

    QToolBar*            toolBar;

    DInfoInterface*      iface;

    WidgetRenderer*      videoWidget;
    AVPlayerCore*        player;

    QSlider*             slider;
    QSlider*             volume;
    QLabel*              tlabel;
    QUrl                 currentItem;


    int                  videoOrientation;
    qint64               capturePosition;
    qint64               sliderTime;
};

MediaPlayerView::MediaPlayerView(QWidget* const parent)
    : QStackedWidget(parent),
      d             (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    const int spacing      = qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                             QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));

    d->prevAction          = new QAction(QIcon::fromTheme(QLatin1String("go-previous")),
                                         i18nc("go to previous image", "Back"),   this);
    d->nextAction          = new QAction(QIcon::fromTheme(QLatin1String("go-next")),
                                         i18nc("go to next image", "Forward"),    this);
    d->playAction          = new QAction(QIcon::fromTheme(QLatin1String("media-playback-start")),
                                         i18nc("pause/play video", "Pause/Play"), this);
    d->grabAction          = new QAction(QIcon::fromTheme(QLatin1String("view-preview")),
                                         i18nc("capture video frame", "Capture"), this);

    d->errorView           = new QFrame(this);
    QLabel* const errorMsg = new QLabel(i18n("An error has occurred with the media player..."), this);

    errorMsg->setAlignment(Qt::AlignCenter);
    d->errorView->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    d->errorView->setLineWidth(1);

    QVBoxLayout* const vbox1 = new QVBoxLayout(d->errorView);
    vbox1->addWidget(errorMsg, 10);
    vbox1->setContentsMargins(QMargins());
    vbox1->setSpacing(spacing);

    insertWidget(Private::ErrorView, d->errorView);

    // --------------------------------------------------------------------------

    d->playerView     = new QFrame(this);
    d->videoWidget    = new WidgetRenderer(this);

    DHBox* const hbox = new DHBox(this);
    d->slider         = new QSlider(Qt::Horizontal, hbox);
    d->slider->setStyle(new PlayerVideoStyle());
    d->slider->setRange(0, 0);
    d->tlabel         = new QLabel(hbox);
    d->tlabel->setText(QLatin1String("00:00:00 / 00:00:00"));
    d->loopPlay       = new QPushButton(hbox);
    d->loopPlay->setIcon(QIcon::fromTheme(QLatin1String("media-playlist-normal")));
    d->loopPlay->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    d->loopPlay->setToolTip(i18n("Toggle playing in a loop"));
    d->loopPlay->setFocusPolicy(Qt::NoFocus);
    d->loopPlay->setMinimumSize(22, 22);
    d->loopPlay->setCheckable(true);
    QLabel* const spk = new QLabel(hbox);
    spk->setPixmap(QIcon::fromTheme(QLatin1String("audio-volume-high")).pixmap(22, 22));
    d->volume         = new QSlider(Qt::Horizontal, hbox);
    d->volume->setRange(0, 100);
    d->volume->setValue(50);
    hbox->setContentsMargins(0, spacing, 0, 0);
    hbox->setStretchFactor(d->slider, 10);
    hbox->setSpacing(4);

    d->videoWidget->setOutAspectRatioMode(QtAV::VideoAspectRatio);
    d->videoWidget->setMouseTracking(true);

    d->playerView->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    d->playerView->setLineWidth(1);

    QVBoxLayout* const vbox2 = new QVBoxLayout(d->playerView);
    vbox2->addWidget(d->videoWidget, 10);
    vbox2->addWidget(hbox,            0);
    vbox2->setContentsMargins(0, 0, 0, spacing);
    vbox2->setSpacing(spacing);

    insertWidget(Private::PlayerView, d->playerView);

    d->toolBar = new QToolBar(this);
    d->toolBar->addAction(d->prevAction);
    d->toolBar->addAction(d->nextAction);
    d->toolBar->addAction(d->playAction);
    d->toolBar->addAction(d->grabAction);
    d->toolBar->setStyleSheet(toolButtonStyleSheet());

    setPreviewMode(Private::PlayerView);

    d->errorView->installEventFilter(new MediaPlayerMouseClickFilter(this));
    d->videoWidget->installEventFilter(new MediaPlayerMouseClickFilter(this));

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group("Media Player Settings");

    d->volume->setValue(group.readEntry("Volume", 50));

    slotPlayerConfigChanged();

    // --------------------------------------------------------------------------

    connect(&AVPlayerConfigMngr::instance(), SIGNAL(changed()),
            this, SLOT(slotPlayerConfigChanged()));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(d->prevAction, SIGNAL(triggered()),
            this, SIGNAL(signalPrevItem()));

    connect(d->nextAction, SIGNAL(triggered()),
            this, SIGNAL(signalNextItem()));

    connect(d->playAction, SIGNAL(triggered()),
            this, SLOT(slotPausePlay()));

    connect(d->grabAction, SIGNAL(triggered()),
            this, SLOT(slotCapture()));

    connect(d->slider, SIGNAL(sliderMoved(int)),
            this, SLOT(slotPosition(int)));

    connect(d->slider, SIGNAL(valueChanged(int)),
            this, SLOT(slotPosition(int)));

    connect(d->volume, SIGNAL(valueChanged(int)),
            this, SLOT(slotVolumeChanged(int)));

    connect(d->loopPlay, SIGNAL(toggled(bool)),
            this, SLOT(slotLoopToggled(bool)));

    connect(d->player, SIGNAL(stateChanged(QtAV::AVPlayerCore::State)),
            this, SLOT(slotPlayerStateChanged(QtAV::AVPlayerCore::State)));

    connect(d->player, SIGNAL(mediaStatusChanged(QtAV::MediaStatus)),
            this, SLOT(slotMediaStatusChanged(QtAV::MediaStatus)));

    connect(d->player, SIGNAL(positionChanged(qint64)),
            this, SLOT(slotPositionChanged(qint64)),
            Qt::QueuedConnection);

    connect(d->player, SIGNAL(durationChanged(qint64)),
            this, SLOT(slotDurationChanged(qint64)));

    connect(d->player, SIGNAL(error(QtAV::AVError)),
            this, SLOT(slotHandlePlayerError(QtAV::AVError)));

    connect(d->player->videoCapture(), SIGNAL(imageCaptured(QImage)),
            this, SLOT(slotImageCaptured(QImage)));


    qCDebug(DIGIKAM_GENERAL_LOG) << "Audio output backends:"
                                 << d->player->audio()->backendsAvailable();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Number of supported audio codecs:"
                                 << AudioDecoder::supportedCodecs().count();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Number of supported video codecs:"
                                 << VideoDecoder::supportedCodecs().count();
}

MediaPlayerView::~MediaPlayerView()
{
    escapePreview();
    delete d;
}

void MediaPlayerView::setInfoInterface(DInfoInterface* const iface)
{
    d->iface = iface;
}

void MediaPlayerView::reload()
{
    d->player->stop();
    d->player->setFile(d->currentItem.toLocalFile());
    d->player->play();
}

void MediaPlayerView::slotPlayerStateChanged(QtAV::AVPlayerCore::State state)
{
    if      (state == QtAV::AVPlayerCore::PlayingState)
    {
        int rotate = 0;

#if QTAV_VERSION > QTAV_VERSION_CHK(1, 12, 0)

        // fix wrong rotation from QtAV git/master

        rotate     = d->player->statistics().video_only.rotate;

#endif

        d->videoWidget->setOrientation((-rotate) + d->videoOrientation);
        qCDebug(DIGIKAM_GENERAL_LOG) << "Found video orientation:"
                                     << d->videoOrientation;

        d->playAction->setIcon(QIcon::fromTheme(QLatin1String("media-playback-pause")));
    }
    else if ((state == QtAV::AVPlayerCore::PausedState) ||
             (state == QtAV::AVPlayerCore::StoppedState))
    {
        d->playAction->setIcon(QIcon::fromTheme(QLatin1String("media-playback-start")));
    }
}

void MediaPlayerView::slotMediaStatusChanged(QtAV::MediaStatus status)
{
    if (status == QtAV::InvalidMedia)
    {
        setPreviewMode(Private::ErrorView);
    }
}

void MediaPlayerView::escapePreview()
{
    d->player->stop();
    d->player->audio()->close();
    d->player->setFile(QString());
}

void MediaPlayerView::slotThemeChanged()
{
    QPalette palette;
    palette.setColor(d->errorView->backgroundRole(), qApp->palette().color(QPalette::Base));
    d->errorView->setPalette(palette);

    QPalette palette2;
    palette2.setColor(d->playerView->backgroundRole(), qApp->palette().color(QPalette::Base));
    d->playerView->setPalette(palette2);
}

void MediaPlayerView::slotEscapePressed()
{
    escapePreview();
    Q_EMIT signalEscapePreview();
}

void MediaPlayerView::slotRotateVideo()
{
    if (d->player->isPlaying())
    {
        int orientation = 0;

        switch (d->videoWidget->orientation())
        {
            case 0:
                orientation = 90;
                break;

            case 90:
                orientation = 180;
                break;

            case 180:
                orientation = 270;
                break;

            default:
                orientation = 0;
        }

        d->videoWidget->setOrientation(orientation);
    }
}

void MediaPlayerView::slotPausePlay()
{
    if (!d->player->isPlaying())
    {
        d->player->play();
        return;
    }

    d->player->pause(!d->player->isPaused());
}

void MediaPlayerView::slotCapture()
{
    if (d->player->isPlaying())
    {
        d->player->videoCapture()->setAutoSave(false);
        d->capturePosition = d->player->position();
        d->player->videoCapture()->capture();
    }
}

void MediaPlayerView::slotImageCaptured(const QImage& image)
{
    if (!image.isNull() && d->currentItem.isValid())
    {
        QFileInfo info(d->currentItem.toLocalFile());
        QString tempPath = QString::fromUtf8("%1/%2-%3.digikamtempfile.jpg")
                          .arg(info.path())
                          .arg(info.baseName())
                          .arg(d->capturePosition);

        if (image.save(tempPath, "JPG", 100))
        {
            QScopedPointer<DMetadata> meta(new DMetadata);

            if (meta->load(tempPath))
            {
                QDateTime dateTime;
                MetaEngine::ImageOrientation orientation = MetaEngine::ORIENTATION_NORMAL;

                if (d->iface)
                {
                    DItemInfo dinfo(d->iface->itemInfo(d->currentItem));

                    dateTime    = dinfo.dateTime();
                    orientation = (MetaEngine::ImageOrientation)dinfo.orientation();
                }
                else
                {
                    QScopedPointer<DMetadata> meta2(new DMetadata);

                    if (meta2->load(d->currentItem.toLocalFile()))
                    {
                        dateTime    = meta2->getItemDateTime();
                        orientation = meta2->getItemOrientation();
                    }
                }

                if (dateTime.isValid())
                {
                    dateTime = dateTime.addMSecs(d->capturePosition);
                }
                else
                {
                    dateTime = QDateTime::currentDateTime();
                }

                if (orientation == MetaEngine::ORIENTATION_UNSPECIFIED)
                {
                    orientation = MetaEngine::ORIENTATION_NORMAL;
                }

                meta->setImageDateTime(dateTime, true);
                meta->setItemDimensions(image.size());
                meta->setItemOrientation(orientation);
                meta->save(tempPath, true);
            }

            QString finalPath = QString::fromUtf8("%1/%2-%3.jpg")
                               .arg(info.path())
                               .arg(info.baseName())
                               .arg(d->capturePosition);

            if (QFile::rename(tempPath, finalPath))
            {
                if (d->iface)
                {
                    d->iface->slotMetadataChangedForUrl(QUrl::fromLocalFile(finalPath));
                }
            }
            else
            {
                QFile::remove(tempPath);
            }
        }
    }
}

int MediaPlayerView::previewMode()
{
    return indexOf(currentWidget());
}

void MediaPlayerView::setPreviewMode(int mode)
{
    if ((mode != Private::ErrorView) && (mode != Private::PlayerView))
    {
        return;
    }

    setCurrentIndex(mode);

    d->toolBar->adjustSize();
    d->toolBar->raise();
}

void MediaPlayerView::setCurrentItem(const QUrl& url, bool hasPrevious, bool hasNext)
{
    d->prevAction->setEnabled(hasPrevious);
    d->nextAction->setEnabled(hasNext);

    if (url.isEmpty())
    {
        d->player->stop();
        d->currentItem = url;
        d->player->audio()->close();

        return;
    }

    if (d->currentItem == url)
    {
        return;
    }

    d->currentItem = url;

    d->player->stop();

    int orientation = 0;

    if (d->iface)
    {
        DItemInfo info(d->iface->itemInfo(url));

        orientation = info.orientation();
    }

    switch (orientation)
    {
        case MetaEngine::ORIENTATION_ROT_90:
        case MetaEngine::ORIENTATION_ROT_90_HFLIP:
        case MetaEngine::ORIENTATION_ROT_90_VFLIP:
            d->videoOrientation = 90;
            break;

        case MetaEngine::ORIENTATION_ROT_180:
            d->videoOrientation = 180;
            break;

        case MetaEngine::ORIENTATION_ROT_270:
            d->videoOrientation = 270;
            break;

        default:
            d->videoOrientation = 0;
            break;
    }

    d->player->setFile(d->currentItem.toLocalFile());
    setPreviewMode(Private::PlayerView);
    d->player->setPosition(10);
    d->player->play();
}

void MediaPlayerView::slotPositionChanged(qint64 position)
{
    if ((d->sliderTime < position)       &&
        ((d->sliderTime + 100) > position))
    {
        return;
    }

    d->sliderTime = position;

    if (!d->slider->isSliderDown())
    {
        d->slider->blockSignals(true);
        d->slider->setValue(position);
        d->slider->blockSignals(false);
    }

    d->tlabel->setText(QString::fromLatin1("%1 / %2")
                       .arg(QTime(0, 0, 0).addMSecs(position).toString(QLatin1String("HH:mm:ss")))
                       .arg(QTime(0, 0, 0).addMSecs(d->slider->maximum()).toString(QLatin1String("HH:mm:ss"))));
}

void MediaPlayerView::slotVolumeChanged(int volume)
{
    d->player->audio()->setVolume((qreal)volume / 100.0);

    if (objectName() != QLatin1String("main_media_player"))
    {
        return;
    }

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group("Media Player Settings");
    group.writeEntry("Volume", volume);
}

void MediaPlayerView::slotLoopToggled(bool loop)
{
    if (loop)
    {
        d->loopPlay->setIcon(QIcon::fromTheme(QLatin1String("media-playlist-repeat")));
        d->player->setRepeat(-1);
    }
    else
    {
        d->loopPlay->setIcon(QIcon::fromTheme(QLatin1String("media-playlist-normal")));
        d->player->setRepeat(0);
    }
}

void MediaPlayerView::slotDurationChanged(qint64 duration)
{
    qint64 max = qMax((qint64)1, duration);
    d->slider->setRange(0, max);
}

void MediaPlayerView::slotPosition(int position)
{
    if (d->player->isSeekable())
    {
        d->player->setPosition((qint64)position);
    }
}

void MediaPlayerView::slotHandlePlayerError(const QtAV::AVError& err)
{
    setPreviewMode(Private::ErrorView);
    qCDebug(DIGIKAM_GENERAL_LOG) << "Error: " << err.string();
}

void MediaPlayerView::slotPlayerConfigChanged()
{
    if (d->player)
    {
        d->player->stop();
        delete d->player;
    }

    d->player = new AVPlayerCore(this);
    d->player->setRenderer(d->videoWidget);

    d->player->setBufferMode(QtAV::BufferPackets);
    d->player->setBufferValue(AVPlayerConfigMngr::instance().bufferValue());
    d->player->setFrameRate(AVPlayerConfigMngr::instance().forceFrameRate());
    d->player->setInterruptOnTimeout(AVPlayerConfigMngr::instance().abortOnTimeout());
    d->player->setInterruptTimeout(AVPlayerConfigMngr::instance().timeout() * 1000.0);
    d->player->setPriority(DecoderConfigPage::idsFromNames(AVPlayerConfigMngr::instance().decoderPriorityNames()));

    d->player->audio()->setVolume((qreal)d->volume->value() / 100.0);
    d->player->audio()->close();
}

} // namespace Digikam

#include "mediaplayerview.moc"
