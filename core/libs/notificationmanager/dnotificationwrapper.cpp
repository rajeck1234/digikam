/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-03
 * Description : A wrapper send desktop notifications
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dnotificationwrapper.h"
#include "digikam_config.h"

// Qt includes

#include <QProcess>
#include <QApplication>
#include <QStandardPaths>
#include <QIcon>

#ifdef HAVE_DBUS
#   include <QDBusConnection>
#   include <QDBusConnectionInterface>
#endif

// KDE includes

#ifdef HAVE_KNOTIFICATIONS
#    include <knotification.h>
#endif

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dnotificationpopup.h"

namespace Digikam
{

/**
 * Re-implementation of DNotificationPopup to move pop-up notification
 * window on the bottom right corner of parent window. The goal is to simulate
 * the position of KDE notifier pop-up from task bar if this one is not available,
 * as for ex under Windows, Gnome, or using a remote connection through ssh.
 */
class Q_DECL_HIDDEN NotificationPassivePopup : public DNotificationPopup
{
    Q_OBJECT

public:

    explicit NotificationPassivePopup(QWidget* const parent)
        : DNotificationPopup(parent),
          m_parent          (parent)
    {
    }

    void showNotification(const QString& caption, const QString& text, const QPixmap& icon)
    {
        setView(caption, text, icon);
        QPoint ppos = m_parent->pos();
        QSize psize = m_parent->frameSize();
        int offsetx = minimumSizeHint().width()  + 30;
        int offsety = minimumSizeHint().height() + 30;
        show(QPoint(ppos.x() + psize.width()  - offsetx,
                    ppos.y() + psize.height() - offsety));
    }

private:

    QWidget* m_parent;
};

// ----------------------------------------------------------------------------------------------

#if defined HAVE_KNOTIFICATIONS && defined HAVE_DBUS

static inline bool detectKDEDesktopIsRunning()
{
    const QByteArray xdgCurrentDesktop = qgetenv("XDG_CURRENT_DESKTOP");

    if (!xdgCurrentDesktop.isEmpty())
    {
        return (xdgCurrentDesktop.toUpper() == "KDE");
    }

    // Classic fallbacks

    if (!qEnvironmentVariableIsEmpty("KDE_FULL_SESSION"))
    {
        return true;
    }

    return false;
}

#endif

// ----------------------------------------------------------------------------------------------

void DNotificationWrapper(const QString& eventId, const QString& message,
                          QWidget* const parent, const QString& windowTitle,
                          const QPixmap& pixmap)
{
    QPixmap logoPixmap = pixmap;

    if (logoPixmap.isNull())
    {
        if (QApplication::applicationName() == QLatin1String("digikam"))
        {
            logoPixmap = QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48));
        }
        else
        {
            logoPixmap = QIcon::fromTheme(QLatin1String("showfoto")).pixmap(QSize(48,48));
        }
    }

#if defined HAVE_KNOTIFICATIONS && defined HAVE_DBUS

    // NOTE: This detection of KDE desktop is not perfect because KNotification service may never be started.
    //       But in a regular KDE session, KNotification service should be running already.

    if (detectKDEDesktopIsRunning() &&
        QDBusConnection::sessionBus().interface()->
            isServiceRegistered(QLatin1String("org.kde.StatusNotifierWatcher")).value())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Event is dispatched to KDE desktop notifier";

        if (eventId.isEmpty())
        {
            KNotification::event(KNotification::Notification, message, logoPixmap, parent);
        }
        else
        {
            KNotification::event(eventId, message, logoPixmap, parent);
        }
    }
    else

#else
    {
        Q_UNUSED(eventId);
    }

#endif

#ifdef Q_OS_DARWIN

    // OSX support

    if (MacNativeDispatchNotify(windowTitle, message))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Event is dispatched to OSX desktop notifier";
        return;
    }
    else

#endif // Q_OS_DARWIN

    {
        // Other Linux Desktops

        QProcess proc;

        proc.setProcessEnvironment(adjustedEnvironmentForAppImage());
        proc.start(QLatin1String("notify-send"),
                   QStringList() << windowTitle
                                 << message
                                 << QLatin1String("-a")
                                 << QApplication::applicationName());

        if (proc.waitForFinished() && proc.exitCode() == 0)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Event is dispatched to desktop notifier through DBUS";
            return;
        }
        else
        {
            if (!parent)
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "parent is null";
                return;
            }

            qCDebug(DIGIKAM_GENERAL_LOG) << "Event is dispatched through a passive pop-up";

            NotificationPassivePopup* const popup = new NotificationPassivePopup(parent);
            popup->showNotification(windowTitle, message, logoPixmap);
        }
    }
}

} // namespace Digikam

#include "dnotificationwrapper.moc"
