/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-18
 * Description : ART raw import plugin.
 *
 * SPDX-FileCopyrightText: 2019-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rawimportartplugin.h"

// Qt includes

#include <QMessageBox>
#include <QApplication>
#include <QPointer>
#include <QByteArray>
#include <QFileInfo>
#include <QSettings>
#include <QTemporaryFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated
#include "dimg.h"
#include "filteraction.h"
#include "dfileoperations.h"
#include "loadingdescription.h"

namespace DigikamRawImportARTPlugin
{

class Q_DECL_HIDDEN ARTRawImportPlugin::Private
{
public:

    explicit Private()
      : art(nullptr)
    {
    }

    QProcess*          art;
    DImg               decoded;
    LoadingDescription props;
    QString            tempName;
};

ARTRawImportPlugin::ARTRawImportPlugin(QObject* const parent)
    : DPluginRawImport(parent),
      d               (new Private)
{
}

ARTRawImportPlugin::~ARTRawImportPlugin()
{
    delete d;
}

QString ARTRawImportPlugin::name() const
{
    return QString::fromUtf8("Raw Import using ART");
}

QString ARTRawImportPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ARTRawImportPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-x-adobe-dng"));
}

QString ARTRawImportPlugin::description() const
{
    return QString::fromUtf8("A RAW import plugin based on ART");
}

QString ARTRawImportPlugin::details() const
{
    return QString::fromUtf8("<p>This RAW Import plugin use ART tool to pre-process file in Image Editor.</p>"
                             "<p>It requires at least ART version 1.8.2 to work.</p>"
                             "<p>See ART web site for details: <a href='https://bitbucket.org/agriggio/art/wiki/Home'>"
                             "https://bitbucket.org/agriggio/art/wiki/Home</a></p>");
}

QString ARTRawImportPlugin::handbookSection() const
{
    return QLatin1String("setup_application");
}

QString ARTRawImportPlugin::handbookChapter() const
{
    return QLatin1String("editor_settings");
}

QString ARTRawImportPlugin::handbookReference() const
{
    return QLatin1String("setup-raw");
}

QList<DPluginAuthor> ARTRawImportPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2019-2022"))
            ;
}

void ARTRawImportPlugin::setup(QObject* const /*parent*/)
{
    // Nothing to do
}

bool ARTRawImportPlugin::run(const QString& filePath, const DRawDecoding& /*def*/)
{
    QFileInfo fileInfo(filePath);
    d->props       = LoadingDescription(fileInfo.filePath(), LoadingDescription::ConvertForEditor);
    d->decoded     = DImg();

    QTemporaryFile tempFile;
    tempFile.open();
    d->tempName    = tempFile.fileName();

    d->art = new QProcess(this);
    d->art->setProcessChannelMode(QProcess::MergedChannels);
    d->art->setWorkingDirectory(fileInfo.path());
    d->art->setProcessEnvironment(adjustedEnvironmentForAppImage());

    connect(d->art, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(slotErrorOccurred(QProcess::ProcessError)));

    connect(d->art, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(slotProcessFinished(int,QProcess::ExitStatus)));

    connect(d->art, SIGNAL(readyRead()),
            this, SLOT(slotProcessReadyRead()));

    // --------

    d->art->setProgram(DFileOperations::findExecutable(QLatin1String("ART")));
    d->art->setArguments(QStringList() << QLatin1String("-gimp") // Special mode used initially as Gimp plugin
                                       << filePath               // Input file
                                       << d->tempName);          // Output file

    qCDebug(DIGIKAM_GENERAL_LOG) << "ART arguments:" << d->art->arguments();

    d->art->start();

    return d->art->waitForStarted(10000);
}

void ARTRawImportPlugin::slotErrorOccurred(QProcess::ProcessError error)
{
    switch (error)
    {
        case QProcess::FailedToStart:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "ART :: Process has failed to start";
            break;
        }

        case QProcess::Crashed:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "ART :: Process has crashed";
            break;
        }

        case QProcess::Timedout:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "ART :: Process time-out";
            break;
        }

        case QProcess::WriteError:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "ART :: Process write error";
            break;
        }

        case QProcess::ReadError:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "ART :: Process read error";
            break;
        }

        default:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "ART :: Process error unknown";
            break;
        }
    }
}

void ARTRawImportPlugin::slotProcessFinished(int code, QProcess::ExitStatus status)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "ART :: return code:" << code << ":: Exit status:" << status;

    d->decoded = DImg(d->tempName);
    d->decoded.setAttribute(QLatin1String("isReadOnly"), true);

    if (d->decoded.isNull())
    {
        QString message = i18n("Error to import RAW image with ART\nClose this dialog to load RAW image with native import tool");
        QMessageBox::information(nullptr, qApp->applicationName(), message);

        qCDebug(DIGIKAM_GENERAL_LOG) << "Decoded image is null! Load with Native tool...";
        qCDebug(DIGIKAM_GENERAL_LOG) << d->props.filePath;

        Q_EMIT signalLoadRaw(d->props);
    }
    else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Decoded image is not null...";
        qCDebug(DIGIKAM_GENERAL_LOG) << d->props.filePath;
        d->props = LoadingDescription(d->tempName, LoadingDescription::ConvertForEditor);

        FilterAction action(QLatin1String("art:RawConverter"), 1, FilterAction::DocumentedHistory);
        action.setDisplayableName(QString::fromUtf8(I18N_NOOP("ART Raw Conversion")));
        d->decoded.addFilterAction(action);

        Q_EMIT signalDecodedImage(d->props, d->decoded);
    }

    delete d->art;
    d->art = nullptr;

    QFile::remove(d->tempName);
}

void ARTRawImportPlugin::slotProcessReadyRead()
{
    QByteArray data   = d->art->readAllStandardError();
    QStringList lines = QString::fromUtf8(data).split(QLatin1Char('\n'), QT_SKIP_EMPTY_PARTS);

    Q_FOREACH (const QString& one, lines)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "ART ::" << one;
    }
}

} // namespace DigikamRawImportARTPlugin
