/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-21
 * Description : a simple web browser dialog based on Qt WebEngine.
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "webbrowserdlg.h"
#include "digikam_config.h"

// Qt includes

#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QToolBar>
#include <QDesktopServices>
#include <QNetworkCookieJar>

#ifdef HAVE_QWEBENGINE
#   include <QWebEngineView>
#   include <QWebEnginePage>
#   include <QWebEngineProfile>
#   include <QWebEngineCookieStore>
#   include <QWebEngineFindTextResult>
#else
#   include <qwebview.h>
#endif

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "statusprogressbar.h"
#include "searchtextbar.h"
#include "dxmlguiwindow.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN WebBrowserDlg::Private
{
public:

    explicit Private()
      : browser    (nullptr),
        toolbar    (nullptr),
        progressbar(nullptr),
        searchbar  (nullptr)
    {
    }

public:

    QUrl               home;

#ifdef HAVE_QWEBENGINE

    QWebEngineView*    browser;

#else

    QWebView*          browser;

#endif

    QToolBar*          toolbar;
    StatusProgressBar* progressbar;
    SearchTextBar*     searchbar;
};

WebBrowserDlg::WebBrowserDlg(const QUrl& url, QWidget* const parent, bool hideDeskBrowser)
    : QDialog(parent),
      d      (new Private)
{
    setModal(false);
    setAttribute(Qt::WA_DeleteOnClose, true);

    d->home    = url;

#ifdef HAVE_QWEBENGINE

    d->browser = new QWebEngineView(this);
    d->browser->page()->profile()->cookieStore()->deleteAllCookies();

#else

    d->browser = new QWebView(this);
    d->browser->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true);
    d->browser->page()->networkAccessManager()->setCookieJar(new QNetworkCookieJar());

#endif

    // --------------------------

    d->toolbar = new QToolBar(this);
    d->toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

#ifdef HAVE_QWEBENGINE

    d->toolbar->addAction(d->browser->pageAction(QWebEnginePage::Back));
    d->toolbar->addAction(d->browser->pageAction(QWebEnginePage::Forward));
    d->toolbar->addAction(d->browser->pageAction(QWebEnginePage::Reload));
    d->toolbar->addAction(d->browser->pageAction(QWebEnginePage::Stop));

#else

    d->toolbar->addAction(d->browser->pageAction(QWebPage::Back));
    d->toolbar->addAction(d->browser->pageAction(QWebPage::Forward));
    d->toolbar->addAction(d->browser->pageAction(QWebPage::Reload));
    d->toolbar->addAction(d->browser->pageAction(QWebPage::Stop));

#endif

    QAction* const gohome  = new QAction(QIcon::fromTheme(QLatin1String("go-home")),
                                         i18n("Home"), this);
    gohome->setToolTip(i18n("Go back to Home page"));
    d->toolbar->addAction(gohome);

    QAction* const deskweb = new QAction(QIcon::fromTheme(QLatin1String("internet-web-browser")),
                                         i18n("Desktop Browser"), this);
    deskweb->setToolTip(i18n("Open Home page with default desktop Web browser"));

    if (!hideDeskBrowser)
    {
        d->toolbar->addAction(deskweb);
    }

    // --------------------------

    d->searchbar   = new SearchTextBar(this, QLatin1String("WebBrowserDlgSearchBar"));
    d->searchbar->setHighlightOnResult(true);

    d->progressbar = new StatusProgressBar(this);
    d->progressbar->setProgressTotalSteps(100);
    d->progressbar->setAlignment(Qt::AlignLeft);
    d->progressbar->setNotify(false);

    // ----------------------

    QGridLayout* const grid = new QGridLayout(this);
    grid->setSpacing(qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                             QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing)));
    grid->addWidget(d->toolbar,     0, 0, 1, 1);
    grid->addWidget(d->searchbar,   0, 2, 1, 1);
    grid->addWidget(d->browser,     1, 0, 1, 3);
    grid->addWidget(d->progressbar, 2, 0, 1, 3);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(1, 10);
    setLayout(grid);

    // ----------------------
/*
    connect(d->browser, SIGNAL(iconChanged(const QIcon&)),
            this, SLOT(slotIconChanged(const QIcon&)));
*/
    connect(d->browser, SIGNAL(titleChanged(QString)),
            this, SLOT(slotTitleChanged(QString)));

    connect(d->browser, SIGNAL(urlChanged(QUrl)),
            this, SLOT(slotUrlChanged(QUrl)));

    connect(d->browser, SIGNAL(loadStarted()),
            this, SLOT(slotLoadingStarted()));

    connect(d->browser, SIGNAL(loadFinished(bool)),
            this, SLOT(slotLoadingFinished(bool)));

    connect(d->searchbar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotSearchTextChanged(SearchTextSettings)));

    connect(d->browser, SIGNAL(loadProgress(int)),
            d->progressbar, SLOT(setProgressValue(int)));

    connect(gohome, SIGNAL(triggered()),
            this, SLOT(slotGoHome()));

    connect(deskweb, SIGNAL(triggered()),
            this, SLOT(slotDesktopWebBrowser()));

    // ----------------------

    KConfigGroup group = KSharedConfig::openConfig()->group("WebBrowserDlg");

    winId();
    windowHandle()->resize(800, 600);
    DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size());

    slotGoHome();
}

WebBrowserDlg::~WebBrowserDlg()
{
    delete d;
}

void WebBrowserDlg::closeEvent(QCloseEvent* e)
{
    KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("WebBrowserDlg"));
    DXmlGuiWindow::saveWindowSize(windowHandle(), group);

    Q_EMIT closeView(false);
    e->accept();
}

void WebBrowserDlg::slotUrlChanged(const QUrl& url)
{
    d->progressbar->setText(url.toString());
    Q_EMIT urlChanged(url);
}

void WebBrowserDlg::slotTitleChanged(const QString& title)
{
    setWindowTitle(title);
}

void WebBrowserDlg::slotIconChanged(const QIcon& icon)
{
    setWindowIcon(icon);
}

void WebBrowserDlg::slotLoadingStarted()
{
    d->progressbar->setProgressBarMode(StatusProgressBar::ProgressBarMode);
}

void WebBrowserDlg::slotLoadingFinished(bool b)
{
    QString curUrl = d->browser->url().toString();

    d->progressbar->setProgressBarMode(StatusProgressBar::TextMode, curUrl);

    if (!b)
    {
        d->progressbar->setText(i18n("Cannot load page %1", curUrl));
    }
}

void WebBrowserDlg::slotSearchTextChanged(const SearchTextSettings& settings)
{

#ifdef HAVE_QWEBENGINE

#   if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))

    d->browser->findText(settings.text,
                         (settings.caseSensitive == Qt::CaseSensitive) ? QWebEnginePage::FindCaseSensitively
                                                                       : QWebEnginePage::FindFlags(),
                         [this](const QWebEngineFindTextResult& result)
                            {
                                d->searchbar->slotSearchResult(result.activeMatch());
                            }
                        );

#   else

    d->browser->findText(settings.text,
                         (settings.caseSensitive == Qt::CaseSensitive) ? QWebEnginePage::FindCaseSensitively
                                                                       : QWebEnginePage::FindFlags(),
                         [this](bool found)
                            {
                                d->searchbar->slotSearchResult(found);
                            }
                        );

#   endif

#else

    bool found = d->browser->findText(
                    settings.text,
                    (settings.caseSensitive == Qt::CaseInsensitive) ? QWebPage::FindCaseSensitively
                                                                    : QWebPage::FindFlags());
    d->searchbar->slotSearchResult(found);

#endif

}

void WebBrowserDlg::slotGoHome()
{
    d->browser->setUrl(d->home);
}

void WebBrowserDlg::slotDesktopWebBrowser()
{
    QDesktopServices::openUrl(d->home);
}

} // namespace Digikam
