/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to create calendar.
 *
 * SPDX-FileCopyrightText: 2018-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "calendarplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "calwizard.h"

namespace DigikamGenericCalendarPlugin
{

CalendarPlugin::CalendarPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

CalendarPlugin::~CalendarPlugin()
{
}

QString CalendarPlugin::name() const
{
    return i18n("Create Calendar");
}

QString CalendarPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon CalendarPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("view-calendar"));
}

QString CalendarPlugin::description() const
{
    return i18n("A tool to create calendar from images");
}

QString CalendarPlugin::details() const
{
    return i18n("<p>This tool allows users to compose items and create a calendar with your best photos.</p>"
                "<p>Different calendar layout are available and you can integrate a list of dates from ICS format to highlight holidays time.</p>"
                "<p>Finally, the tool will propose to export the result to your printer or in a PDF file.</p>");
}

QString CalendarPlugin::handbookSection() const
{
    return QLatin1String("post_processing");
}

QString CalendarPlugin::handbookChapter() const
{
    return QLatin1String("calendar_tool");
}

QList<DPluginAuthor> CalendarPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Renchi Raju"),
                             QString::fromUtf8("renchi dot raju at gmail dot com"),
                             QString::fromUtf8("(C) 2003-2005"),
                             i18n("Former Author"))
            << DPluginAuthor(QString::fromUtf8("Orgad Shaneh"),
                             QString::fromUtf8("orgads at gmail dot com"),
                             QString::fromUtf8("(C) 2007-2008"))
            << DPluginAuthor(QString::fromUtf8("Tom Albers"),
                             QString::fromUtf8("tomalbers at kde dot nl"),
                             QString::fromUtf8("(C) 2006"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2021"),
                             i18n("Developer and Maintainer"))
            ;
}

void CalendarPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Create Calendar..."));
    ac->setObjectName(QLatin1String("calendar"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotCalendar()));

    addAction(ac);
}

void CalendarPlugin::slotCalendar()
{
    QPointer<CalWizard> wzrd = new CalWizard(nullptr, infoIface(sender()));
    wzrd->setPlugin(this);
    wzrd->exec();
    delete wzrd;
}

} // namespace DigikamGenericCalendarPlugin
