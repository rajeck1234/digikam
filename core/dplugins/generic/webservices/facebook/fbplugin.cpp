/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to Facebook web-service.
 *
 * SPDX-FileCopyrightText: 2018-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fbplugin.h"

// Qt includes

#include <QPointer>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "fbwindow.h"

namespace DigikamGenericFaceBookPlugin
{

FbPlugin::FbPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

FbPlugin::~FbPlugin()
{
}

void FbPlugin::cleanUp()
{
    delete m_toolDlg;
}

QString FbPlugin::name() const
{
    return i18n("Facebook");
}

QString FbPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon FbPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("dk-facebook"));
}

QString FbPlugin::description() const
{
    return i18n("A tool to export to Facebook web-service");
}

QString FbPlugin::details() const
{
    return i18n("<p>This tool allows users to export items to Facebook web-service.</p>"
                "<p>See Facebook web site for details: <a href='https://www.facebook.com/'>https://www.facebook.com/</a></p>");
}

QString FbPlugin::handbookSection() const
{
    return QLatin1String("export_tools");
}

QString FbPlugin::handbookChapter() const
{
    return QLatin1String("face_book");
}

QList<DPluginAuthor> FbPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Luka Renko"),
                             QString::fromUtf8("lure at kubuntu dot org"),
                             QString::fromUtf8("(C) 2009"))
            << DPluginAuthor(QString::fromUtf8("Vardhman Jain"),
                             QString::fromUtf8("vardhman at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2008"))
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2017-2021"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2012-2021"))
            ;
}

void FbPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to &Facebook..."));
    ac->setObjectName(QLatin1String("export_facebook"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_F);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotFaceBook()));

    addAction(ac);
}

void FbPlugin::slotFaceBook()
{
    if (!reactivateToolDialog(m_toolDlg))
    {
        QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Information, i18nc("@title:window", "Facebook"),
                 i18n("<p>Currently, as an open source project, we are unable to meet "
                      "the Facebook requirements for reactivating this plugin.</p>"
                      "<p>For more information look here: "
                      "<a href='https://bugs.kde.org/show_bug.cgi?id=182838'>"
                      "KDE Bugtracking System</a></p>"));

        msgBox->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        msgBox->exec();
        delete msgBox;

        delete m_toolDlg;
        m_toolDlg = new FbWindow(infoIface(sender()), nullptr);
        m_toolDlg->setPlugin(this);
        m_toolDlg->show();
    }
}

} // namespace DigikamGenericFaceBookPlugin
