/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-04-29
 * Description : digiKam XML GUI window
 *
 * SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dxmlguiwindow_p.h"

namespace Digikam
{

DXmlGuiWindow::DXmlGuiWindow(QWidget* const parent, Qt::WindowFlags f)
    : KXmlGuiWindow(parent, f),
      m_winLoaded  (false),
      m_maximized  (false),
      d            (new Private)
{
    m_animLogo = nullptr;

    installEventFilter(this);
}

DXmlGuiWindow::~DXmlGuiWindow()
{
    delete d;
}

void DXmlGuiWindow::setConfigGroupName(const QString& name)
{
    d->configGroupName = name;
}

QString DXmlGuiWindow::configGroupName() const
{
    return d->configGroupName;
}

void DXmlGuiWindow::showEvent(QShowEvent* e)
{
    KXmlGuiWindow::showEvent(e);

    restoreWindowSize();
}

void DXmlGuiWindow::closeEvent(QCloseEvent* e)
{
    checkFullScreenBeforeClosing();

    if (!testAttribute(Qt::WA_DeleteOnClose))
    {

#ifdef Q_OS_WIN

        m_maximized = (windowState() &
                       Qt::WindowMaximized);

#endif

        e->ignore();
        return;
    }

    saveWindowSize();

    KXmlGuiWindow::closeEvent(e);
}

bool DXmlGuiWindow::slotClose()
{
    checkFullScreenBeforeClosing();

    return close();
}

void DXmlGuiWindow::slotShowMenuBar()
{
    menuBar()->setVisible(d->showMenuBarAction->isChecked());
}

void DXmlGuiWindow::slotShowStatusBar()
{
    statusBar()->setVisible(d->showStatusBarAction->isChecked());
}

void DXmlGuiWindow::slotConfNotifications()
{

#ifdef HAVE_KNOTIFYCONFIG

    QDialog* const dialog             = new QDialog(this);
    dialog->setWindowTitle(i18n("Configure Notifications"));

    KNotifyConfigWidget* const w      = new KNotifyConfigWidget(dialog);

    QDialogButtonBox* const buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel| QDialogButtonBox::Help);
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

    QVBoxLayout* const layout         = new QVBoxLayout(dialog);
    layout->addWidget(w);
    layout->addWidget(buttonBox);

    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            w, SLOT(save()));

    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            w, SLOT(save()));

    connect(buttonBox->button(QDialogButtonBox::Help), &QPushButton::clicked,
            this, []()
        {
            openOnlineDocumentation(QLatin1String("setup_application"), QLatin1String("notifications_settings"));
        }
    );

    connect(w, SIGNAL(changed(bool)),
            buttonBox->button(QDialogButtonBox::Apply), SLOT(setEnabled(bool)));

    connect(buttonBox, SIGNAL(accepted()),
            dialog, SLOT(accept()));

    connect(buttonBox, SIGNAL(rejected()),
            dialog, SLOT(reject()));

    w->setApplication(QString());
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();

#endif

}

void DXmlGuiWindow::editKeyboardShortcuts(KActionCollection* const extraac, const QString& actitle)
{
    KShortcutsDialog dialog(KShortcutsEditor::AllActions,
                            KShortcutsEditor::LetterShortcutsAllowed, this);
    dialog.addCollection(actionCollection(), i18nc("general keyboard shortcuts", "General"));

    QDialogButtonBox* const btnbox = dialog.findChild<QDialogButtonBox*>();

    if (btnbox)
    {
        btnbox->setStandardButtons(btnbox->standardButtons() | QDialogButtonBox::Help);

        connect(btnbox->button(QDialogButtonBox::Help), &QPushButton::clicked,
                this, []()
            {
                openOnlineDocumentation(QLatin1String("setup_application"), QLatin1String("shortcuts_settings"));
            }
        );
    }

    if (extraac)
    {
        dialog.addCollection(extraac, actitle);
    }

    dialog.configure();
}

bool DXmlGuiWindow::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == this)
    {
        if (ev && (ev->type() == QEvent::HoverMove) && fullScreenIsActive())
        {
            // We will handle a stand alone FullScreen button action on top/right corner of screen
            // only if managed window tool bar is hidden, and if we switched already in Full Screen mode.

            KToolBar* const mainbar = mainToolBar();

            if (mainbar)
            {
                if (((d->fsOptions & FS_TOOLBARS) && d->fullScreenHideToolBars) || !mainbar->isVisible())
                {
                    QHoverEvent* const mev = dynamic_cast<QHoverEvent*>(ev);

                    if (mev)
                    {

#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))

                        QPoint pos(mev->position().toPoint());

#else

                        QPoint pos(mev->pos());

#endif

                        QRect  desktopRect = windowHandle()->screen()->geometry();

                        QRect sizeRect(QPoint(0, 0), d->fullScreenBtn->size());
                        QRect topLeft, topRight;
                        QRect topRightLarger;

                        desktopRect        = QRect(desktopRect.y(), desktopRect.y(), desktopRect.width(), desktopRect.height());
                        topLeft            = sizeRect;
                        topRight           = sizeRect;

                        topLeft.moveTo(desktopRect.x(), desktopRect.y());
                        topRight.moveTo(desktopRect.x() + desktopRect.width() - sizeRect.width() - 1, topLeft.y());

                        topRightLarger     = topRight.adjusted(-25, 0, 0, 10);

                        if (topRightLarger.contains(pos))
                        {
                            d->fullScreenBtn->move(topRight.topLeft());
                            d->fullScreenBtn->show();
                        }
                        else
                        {
                            d->fullScreenBtn->hide();
                        }

                        return false;
                    }
                }
            }
        }
    }

    // pass the event on to the parent class

    return QObject::eventFilter(obj, ev);
}

void DXmlGuiWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        if (fullScreenIsActive())
        {
            d->fullScreenAction->activate(QAction::Trigger);
        }
    }
}

void DXmlGuiWindow::showSideBars(bool visible)
{
    Q_UNUSED(visible);
}

void DXmlGuiWindow::showThumbBar(bool visible)
{
    Q_UNUSED(visible);
}

bool DXmlGuiWindow::thumbbarVisibility() const
{
    return true;
}

void DXmlGuiWindow::slotHelpContents()
{
    openOnlineDocumentation(d->handbookSection);
}

void DXmlGuiWindow::unminimizeAndActivateWindow()
{
    if (isMinimized())
    {
        setWindowState(windowState() & ~Qt::WindowMinimized);
    }

#ifdef Q_OS_WIN

    if (m_maximized)
    {
        setWindowState(windowState() | Qt::WindowMaximized);
    }

#endif

    setWindowState(windowState() | Qt::WindowActive);
}

bool DXmlGuiWindow::restoreWindowSize(QWindow* const win, const KConfigGroup& group)
{

#ifdef Q_OS_WIN

    int  w   = group.readEntry(QLatin1String("DK Width"),     win->width());
    int  h   = group.readEntry(QLatin1String("DK Height"),    win->height());
    int  x   = group.readEntry(QLatin1String("DK PositionX"), win->geometry().x());
    int  y   = group.readEntry(QLatin1String("DK PositionY"), win->geometry().y());
    bool max = group.readEntry(QLatin1String("DK Maximized"), (bool)(win->windowState() &
                                                                     Qt::WindowMaximized));

    if (win->screen()->availableVirtualGeometry().contains(QRect(x, y, w, h)))
    {
        win->setPosition(x, y);
    }

    if (max)
    {
        // we don't use showMaximized() because it also calls
        // setVisible() it would create an infinite loop.

        win->setWindowState(Qt::WindowMaximized);
    }
    else
    {
        win->resize(w, h);
    }

    return max;

#else

    KWindowConfig::restoreWindowSize(win, group);

    return false;

#endif

}

void DXmlGuiWindow::saveWindowSize(QWindow* const win, KConfigGroup& group)
{

#ifdef Q_OS_WIN

    group.writeEntry(QLatin1String("DK Width"),     win->width());
    group.writeEntry(QLatin1String("DK Height"),    win->height());
    group.writeEntry(QLatin1String("DK PositionX"), win->geometry().x());
    group.writeEntry(QLatin1String("DK PositionY"), win->geometry().y());
    group.writeEntry(QLatin1String("DK Maximized"), (bool)(win->windowState() &
                                                           Qt::WindowMaximized));

#else

    KWindowConfig::saveWindowSize(win, group);

#endif

}

void DXmlGuiWindow::restoreWindowSize()
{

#ifdef Q_OS_WIN

    if (!m_winLoaded)
    {
        winId();

        m_winLoaded               = true;
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        KConfigGroup group        = config->group(configGroupName());
        m_maximized               = restoreWindowSize(windowHandle(), group);
    }

#endif

}

void DXmlGuiWindow::saveWindowSize()
{

#ifdef Q_OS_WIN

    if (m_maximized)
    {
        windowHandle()->setWindowState(Qt::WindowMaximized);
    }

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());

    saveWindowSize(windowHandle(), group);

#endif

}

void DXmlGuiWindow::setGoodDefaultWindowSize(QWindow* const win)
{
    QScreen* const screen = win->screen();
    QRect srect           = screen->availableGeometry();
    int height            = qRound(log10(srect.height() / 60) * 600);
    int width             = qRound(log10(srect.width()  / 80) * 800);

    win->resize(width  > srect.width()  ? srect.width()  : width,
                height > srect.height() ? srect.height() : height);
}

void DXmlGuiWindow::slotRawCameraList()
{
    showRawCameraList();
}

void DXmlGuiWindow::slotSolidHardwareList()
{
    SolidHardwareDlg* const dlg = new SolidHardwareDlg(qApp->activeWindow());
    (void)dlg->exec();
}

void DXmlGuiWindow::slotDonateMoney()
{
    QUrl url(QLatin1String("https://www.digikam.org/donate/"));

#ifdef HAVE_QWEBENGINE

    WebBrowserDlg* const browser = new WebBrowserDlg(url, qApp->activeWindow());
    browser->show();

#else

    QDesktopServices::openUrl(url);

#endif

}

void DXmlGuiWindow::slotRecipesBook()
{
    QUrl url(QLatin1String("https://www.digikam.org/recipes_book/"));

#ifdef HAVE_QWEBENGINE

    WebBrowserDlg* const browser = new WebBrowserDlg(url, qApp->activeWindow());
    browser->show();

#else

    QDesktopServices::openUrl(url);

#endif

}

void DXmlGuiWindow::slotContribute()
{
    QUrl url(QLatin1String("https://www.digikam.org/contribute/"));

#ifdef HAVE_QWEBENGINE

    WebBrowserDlg* const browser = new WebBrowserDlg(url, qApp->activeWindow());
    browser->show();

#else

    QDesktopServices::openUrl(url);

#endif

}

void DXmlGuiWindow::setupIconTheme()
{
    // Let QStandardPaths handle this, it will look for app local stuff
    // this means e.g. for mac: "<APPDIR>/../Resources" and for win: "<APPDIR>/data".

    bool hasBreeze                = false;
    const QString breezeIcons     = QStandardPaths::locate(QStandardPaths::AppDataLocation,
                                                           QLatin1String("breeze.rcc"));

    if (!breezeIcons.isEmpty() && QFile::exists(breezeIcons))
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "Breeze icons resource file found";
        QResource::registerResource(breezeIcons);
        hasBreeze = true;
    }

    bool hasBreezeDark            = false;
    const QString breezeDarkIcons = QStandardPaths::locate(QStandardPaths::AppDataLocation,
                                                           QLatin1String("breeze-dark.rcc"));

    if (!breezeDarkIcons.isEmpty() && QFile::exists(breezeDarkIcons))
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "Breeze-dark icons resource file found";
        QResource::registerResource(breezeDarkIcons);
        hasBreezeDark = true;
    }

    if (hasBreeze || hasBreezeDark)
    {
        // Tell Qt about the theme

        QIcon::setThemeSearchPaths(QStringList() << QLatin1String(":/icons"));

        const QStringList iconsDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                                QLatin1String("icons"),
                                                                QStandardPaths::LocateDirectory);

        if (!iconsDirs.isEmpty())
        {
            QIcon::setThemeSearchPaths(QStringList() << QIcon::themeSearchPaths() << iconsDirs);
        }

        if      (hasBreeze)
        {
            QIcon::setThemeName(QLatin1String("breeze"));
        }
        else if (hasBreezeDark)
        {
            QIcon::setThemeName(QLatin1String("breeze-dark"));
        }
    }
    else
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "Use installed icons";
    }
}

} // namespace Digikam
