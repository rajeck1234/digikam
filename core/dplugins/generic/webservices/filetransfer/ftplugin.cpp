/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export and import items with a remote location.
 *
 * SPDX-FileCopyrightText: 2018-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ftplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ftexportwindow.h"
#include "ftimportwindow.h"

namespace DigikamGenericFileTransferPlugin
{

FTPlugin::FTPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

FTPlugin::~FTPlugin()
{
}

void FTPlugin::cleanUp()
{
    delete m_toolDlgExport;
    delete m_toolDlgImport;
}

QString FTPlugin::name() const
{
    return i18nc("@title", "File Transfer");
}

QString FTPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon FTPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("folder-html"));
}

QString FTPlugin::description() const
{
    return i18nc("@info", "A tool to export and import items with a remote location");
}

QString FTPlugin::details() const
{
    return i18nc("@info", "This tool allows users to export and import items with a remote location.\n\n"
                 "Many protocols can be used, as FTP, SFTP, SAMBA, etc.");
}

QString FTPlugin::handbookSection() const
{
    return QLatin1String("export_tools");
}

QString FTPlugin::handbookChapter() const
{
    return QLatin1String("file_transfert");
}

QList<DPluginAuthor> FTPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Johannes Wienke"),
                             QString::fromUtf8("languitar at semipol dot de"),
                             QString::fromUtf8("(C) 2009"))
            << DPluginAuthor(QString::fromUtf8("Maik Qualmann"),
                             QString::fromUtf8("metzpinguin at gmail dot com"),
                             QString::fromUtf8("(C) 2017-2021"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2010-2021"))
            ;
}

void FTPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Export to remote storage..."));
    ac->setObjectName(QLatin1String("export_filetransfer"));
    ac->setActionCategory(DPluginAction::GenericExport);
    ac->setShortcut(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_K);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotFileTransferExport()));

    addAction(ac);

    DPluginAction* const ac2 = new DPluginAction(parent);
    ac2->setIcon(icon());
    ac2->setText(i18nc("@action", "Import from remote storage..."));
    ac2->setObjectName(QLatin1String("import_filetransfer"));
    ac2->setActionCategory(DPluginAction::GenericImport);
    ac2->setShortcut(Qt::ALT | Qt::SHIFT | Qt::Key_K);

    connect(ac2, SIGNAL(triggered(bool)),
            this, SLOT(slotFileTransferImport()));

    addAction(ac2);

}

void FTPlugin::slotFileTransferExport()
{
    if (!reactivateToolDialog(m_toolDlgExport))
    {
        delete m_toolDlgExport;
        m_toolDlgExport = new FTExportWindow(infoIface(sender()), nullptr);
        m_toolDlgExport->setPlugin(this);
        m_toolDlgExport->show();
    }
}

void FTPlugin::slotFileTransferImport()
{
    if (!reactivateToolDialog(m_toolDlgImport))
    {
        delete m_toolDlgImport;
        m_toolDlgImport = new FTImportWindow(infoIface(sender()), nullptr);
        m_toolDlgImport->setPlugin(this);
        m_toolDlgImport->show();
    }
}

} // namespace DigikamGenericFileTransferPlugin
