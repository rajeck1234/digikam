/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-09
 * Description : a presentation tool.
 *
 * SPDX-FileCopyrightText: 2008-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "presentationdlg.h"

// Qt includes

#include <QFileInfo>
#include <QFont>
#include <QCloseEvent>
#include <QIcon>
#include <QMenu>
#include <QTabWidget>
#include <QDialogButtonBox>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "presentationcontainer.h"
#include "presentation_mainpage.h"
#include "presentation_captionpage.h"
#include "presentation_advpage.h"
#include "thumbnailloadthread.h"

#ifdef HAVE_MEDIAPLAYER
#   include "presentation_audiopage.h"
#endif

namespace DigikamGenericPresentationPlugin
{

class Q_DECL_HIDDEN PresentationDlg::Private
{

public:

    explicit Private()
      : buttonBox   (nullptr),
        startButton (nullptr),
        tab         (nullptr),
        sharedData  (nullptr)
    {
    }

    QDialogButtonBox*      buttonBox;
    QPushButton*           startButton;
    QTabWidget*            tab;
    PresentationContainer* sharedData;
};

PresentationDlg::PresentationDlg(QWidget* const parent,
                                 PresentationContainer* const sharedData)
    : DPluginDialog(parent, QLatin1String("Presentation Settings")),
      d            (new Private)
{
    setWindowTitle(i18nc("@title:window", "Presentation"));
    setModal(false);

    d->sharedData = sharedData;

    m_buttons->addButton(QDialogButtonBox::Close);
    m_buttons->addButton(QDialogButtonBox::Ok);
    m_buttons->button(QDialogButtonBox::Ok)->setText(i18nc("@action: play presentation", "Start"));
    m_buttons->button(QDialogButtonBox::Ok)->setToolTip(i18nc("@info:tooltip", "Start Presentation"));
    m_buttons->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme(QLatin1String("media-playback-start")));
    m_buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    // --- Pages settings ---

    d->tab = new QTabWidget(this);

    d->sharedData->mainPage = new PresentationMainPage(this, d->sharedData);
    d->tab->addTab(d->sharedData->mainPage,
                   QIcon::fromTheme(QLatin1String("view-presentation")),
                   i18nc("@title: main settings for the presentation", "Main Settings"));

    d->sharedData->captionPage = new PresentationCaptionPage(this, d->sharedData);
    d->tab->addTab(d->sharedData->captionPage,
                   QIcon::fromTheme(QLatin1String("draw-freehand")),
                   i18nc("@title: caption settings for the presentation", "Caption"));

#ifdef HAVE_MEDIAPLAYER

    d->sharedData->soundtrackPage = new PresentationAudioPage(this, d->sharedData);
    d->tab->addTab(d->sharedData->soundtrackPage,
                   QIcon::fromTheme(QLatin1String("speaker")),
                   i18nc("@title: soundtrack settings for the presentation", "Soundtrack"));

#endif

    d->sharedData->advancedPage = new PresentationAdvPage(this, d->sharedData);
    d->tab->addTab(d->sharedData->advancedPage,
                   QIcon::fromTheme(QLatin1String("configure")),
                   i18nc("@title: advanced presentation settings", "Advanced"));

    QVBoxLayout* const mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(d->tab);
    mainLayout->addWidget(m_buttons);
    setLayout(mainLayout);

    // Slot connections

    connect(m_buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotStartClicked()));

    connect(m_buttons->button(QDialogButtonBox::Close), SIGNAL(clicked()),
            this, SLOT(reject()));

    readSettings();
}

PresentationDlg::~PresentationDlg ()
{
    delete d;
}

void PresentationDlg::readSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group(QLatin1String("Presentation Settings"));

    d->sharedData->opengl            = grp.readEntry("OpenGL",                   false);
    d->sharedData->openGlFullScale   = grp.readEntry("OpenGLFullScale",          false);
    d->sharedData->delay             = grp.readEntry("Delay",                    1500);
    d->sharedData->printFileName     = grp.readEntry("Print Filename",           true);
    d->sharedData->printProgress     = grp.readEntry("Print Progress Indicator", true);
    d->sharedData->printFileComments = grp.readEntry("Print Comments",           false);
    d->sharedData->loop              = grp.readEntry("Loop",                     false);
    d->sharedData->shuffle           = grp.readEntry("Shuffle",                  false);
    d->sharedData->effectName        = grp.readEntry("Effect Name",              "Random");
    d->sharedData->effectNameGL      = grp.readEntry("Effect Name (OpenGL)",     "Random");

    d->sharedData->delayMsMaxValue   = 120000;
    d->sharedData->delayMsMinValue   = 100;
    d->sharedData->delayMsLineStep   = 100;

    // Comments tab settings

    QFont* const savedFont = new QFont();
    savedFont->setFamily(    grp.readEntry("Comments Font Family"));
    savedFont->setPointSize( grp.readEntry("Comments Font Size",        10));
    savedFont->setBold(      grp.readEntry("Comments Font Bold",        false));
    savedFont->setItalic(    grp.readEntry("Comments Font Italic",      false));
    savedFont->setUnderline( grp.readEntry("Comments Font Underline",   false));
    savedFont->setOverline(  grp.readEntry("Comments Font Overline",    false));
    savedFont->setStrikeOut( grp.readEntry("Comments Font StrikeOut",   false));
    savedFont->setFixedPitch(grp.readEntry("Comments Font FixedPitch",  false));

    d->sharedData->captionFont = savedFont;

    d->sharedData->commentsFontColor   = grp.readEntry("Comments Font Color",   0xffffff);
    d->sharedData->commentsBgColor     = grp.readEntry("Comments Bg Color",     0x000000);
    d->sharedData->commentsDrawOutline = grp.readEntry("Comments Text Outline", true);
    d->sharedData->bgOpacity           = grp.readEntry("Background Opacity",    10);
    d->sharedData->commentsLinesLength = grp.readEntry("Comments Lines Length", 72);

#ifdef HAVE_MEDIAPLAYER

    // Soundtrack tab

    d->sharedData->soundtrackLoop             = grp.readEntry("Soundtrack Loop",                     false);
    d->sharedData->soundtrackPlay             = grp.readEntry("Soundtrack Auto Play",                false);
    d->sharedData->soundtrackPath             = QUrl::fromLocalFile(grp.readEntry("Soundtrack Path", ""));
    d->sharedData->soundtrackRememberPlaylist = grp.readEntry("Soundtrack Remember Playlist",        false);

#endif

    // Advanced tab

    d->sharedData->useMilliseconds     = grp.readEntry("Use Milliseconds",      false);
    d->sharedData->enableMouseWheel    = grp.readEntry("Enable Mouse Wheel",    true);

    d->sharedData->kbDisableFadeInOut  = grp.readEntry("KB Disable FadeInOut",  false);
    d->sharedData->kbDisableCrossFade  = grp.readEntry("KB Disable Crossfade",  false);

    d->sharedData->enableCache         = grp.readEntry("Enable Cache",          false);
    d->sharedData->cacheSize           = grp.readEntry("Cache Size",            5);

    if (d->sharedData->soundtrackRememberPlaylist)
    {
        QString groupName(QLatin1String("Presentation Settings") + QLatin1String(" Soundtrack "));
        KConfigGroup soundGrp = config->group(groupName);

        // load and check playlist files, if valid, add to tracklist widget

        QList<QUrl> playlistFiles = soundGrp.readEntry("Tracks", QList<QUrl>());

        Q_FOREACH (const QUrl& playlistFile, playlistFiles)
        {
            QUrl file(playlistFile);
            QFileInfo fi(file.toLocalFile());

            if (fi.isFile())
            {
                d->sharedData->soundtrackUrls << file;
            }
        }
    }

    d->sharedData->mainPage->readSettings();
    d->sharedData->captionPage->readSettings();
    d->sharedData->advancedPage->readSettings();

#ifdef HAVE_MEDIAPLAYER

    d->sharedData->soundtrackPage->readSettings();

#endif

}

void PresentationDlg::saveSettings()
{
    d->sharedData->mainPage->saveSettings();
    d->sharedData->captionPage->saveSettings();
    d->sharedData->advancedPage->saveSettings();

#ifdef HAVE_MEDIAPLAYER

    d->sharedData->soundtrackPage->saveSettings();

#endif

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group(QLatin1String("Presentation Settings"));
    grp.writeEntry("OpenGL",                   d->sharedData->opengl);
    grp.writeEntry("OpenGLFullScale",          d->sharedData->openGlFullScale);
    grp.writeEntry("Delay",                    d->sharedData->delay);
    grp.writeEntry("Print Filename",           d->sharedData->printFileName);
    grp.writeEntry("Print Progress Indicator", d->sharedData->printProgress);
    grp.writeEntry("Print Comments",           d->sharedData->printFileComments);
    grp.writeEntry("Loop",                     d->sharedData->loop);
    grp.writeEntry("Shuffle",                  d->sharedData->shuffle);
    grp.writeEntry("Use Milliseconds",         d->sharedData->useMilliseconds);
    grp.writeEntry("Enable Mouse Wheel",       d->sharedData->enableMouseWheel);

    // Comments tab settings

    QFont* const commentsFont = d->sharedData->captionFont;
    grp.writeEntry("Comments Font Family",     commentsFont->family());
    grp.writeEntry("Comments Font Size",       commentsFont->pointSize());
    grp.writeEntry("Comments Font Bold",       commentsFont->bold());
    grp.writeEntry("Comments Font Italic",     commentsFont->italic());
    grp.writeEntry("Comments Font Underline",  commentsFont->underline());
    grp.writeEntry("Comments Font Overline",   commentsFont->overline());
    grp.writeEntry("Comments Font StrikeOut",  commentsFont->strikeOut());
    grp.writeEntry("Comments Font FixedPitch", commentsFont->fixedPitch());
    grp.writeEntry("Comments Font Color",      d->sharedData->commentsFontColor);
    grp.writeEntry("Comments Bg Color",        d->sharedData->commentsBgColor);
    grp.writeEntry("Comments Text Outline",    d->sharedData->commentsDrawOutline);
    grp.writeEntry("Background Opacity",       d->sharedData->bgOpacity);
    grp.writeEntry("Comments Lines Length",    d->sharedData->commentsLinesLength);
    grp.writeEntry("Effect Name (OpenGL)",     d->sharedData->effectNameGL);
    grp.writeEntry("Effect Name",              d->sharedData->effectName);

#ifdef HAVE_MEDIAPLAYER

    // Soundtrack tab

    grp.writeEntry("Soundtrack Loop",              d->sharedData->soundtrackLoop);
    grp.writeEntry("Soundtrack Auto Play",         d->sharedData->soundtrackPlay);
    grp.writeEntry("Soundtrack Path",              d->sharedData->soundtrackPath.toLocalFile());
    grp.writeEntry("Soundtrack Remember Playlist", d->sharedData->soundtrackRememberPlaylist);

#endif

    // Advanced settings

    grp.writeEntry("KB Disable FadeInOut", d->sharedData->kbDisableFadeInOut);
    grp.writeEntry("KB Disable Crossfade", d->sharedData->kbDisableCrossFade);
    grp.writeEntry("Enable Cache",         d->sharedData->enableCache);
    grp.writeEntry("Cache Size",           d->sharedData->cacheSize);

    // --------------------------------------------------------

    // only save tracks when option is set and tracklist is NOT empty, to prevent deletion
    // of older track entries

    if (d->sharedData->soundtrackRememberPlaylist && d->sharedData->soundtrackPlayListNeedsUpdate)
    {
        QString groupName(QLatin1String("Presentation Settings") + QLatin1String(" Soundtrack "));
        KConfigGroup soundGrp = config->group(groupName);
        soundGrp.writeEntry("Tracks", d->sharedData->soundtrackUrls);
    }

    config->sync();
}

void PresentationDlg::slotStartClicked()
{
    saveSettings();

    if (d->sharedData->mainPage->updateUrlList())
    {
        Q_EMIT buttonStartClicked();
    }

    return;
}

void PresentationDlg::closeEvent(QCloseEvent* e)
{
    saveSettings();
    e->accept();
}

} // namespace DigikamGenericPresentationPlugin
