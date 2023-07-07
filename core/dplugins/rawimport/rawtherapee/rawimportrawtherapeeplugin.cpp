/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-18
 * Description : RawTherapee raw import plugin.
 *
 * SPDX-FileCopyrightText: 2019-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rawimportrawtherapeeplugin.h"

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

namespace DigikamRawImportRawTherapeePlugin
{

class Q_DECL_HIDDEN RawTherapeeRawImportPlugin::Private
{
public:

    explicit Private()
      : rawtherapee(nullptr)
    {
    }

    QProcess*          rawtherapee;
    DImg               decoded;
    LoadingDescription props;
    QString            tempName;
};

RawTherapeeRawImportPlugin::RawTherapeeRawImportPlugin(QObject* const parent)
    : DPluginRawImport(parent),
      d               (new Private)
{
}

RawTherapeeRawImportPlugin::~RawTherapeeRawImportPlugin()
{
    delete d;
}

QString RawTherapeeRawImportPlugin::name() const
{
    return QString::fromUtf8("Raw Import using RawTherapee");
}

QString RawTherapeeRawImportPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RawTherapeeRawImportPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-x-adobe-dng"));
}

QString RawTherapeeRawImportPlugin::description() const
{
    return QString::fromUtf8("A RAW import plugin based on RawTherapee");
}

QString RawTherapeeRawImportPlugin::details() const
{
    return QString::fromUtf8("<p>This RAW Import plugin use RawTherapee tool to pre-process file in Image Editor.</p>"
                             "<p>It requires at least RawTherapee version 5.2.0 to work.</p>"
                             "<p>See RawTherapee web site for details: <a href='https://rawtherapee.com/'>https://rawtherapee.com/</a></p>");
}

QString RawTherapeeRawImportPlugin::handbookSection() const
{
    return QLatin1String("setup_application");
}

QString RawTherapeeRawImportPlugin::handbookChapter() const
{
    return QLatin1String("editor_settings");
}

QString RawTherapeeRawImportPlugin::handbookReference() const
{
    return QLatin1String("setup-raw");
}

QList<DPluginAuthor> RawTherapeeRawImportPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2019-2022"))
            ;
}

void RawTherapeeRawImportPlugin::setup(QObject* const /*parent*/)
{
    // Nothing to do
}

bool RawTherapeeRawImportPlugin::run(const QString& filePath, const DRawDecoding& /*def*/)
{
    QFileInfo fileInfo(filePath);
    d->props       = LoadingDescription(fileInfo.filePath(), LoadingDescription::ConvertForEditor);
    d->decoded     = DImg();

    QTemporaryFile tempFile;
    tempFile.open();
    d->tempName    = tempFile.fileName();

    d->rawtherapee = new QProcess(this);
    d->rawtherapee->setProcessChannelMode(QProcess::MergedChannels);
    d->rawtherapee->setWorkingDirectory(fileInfo.path());
    d->rawtherapee->setProcessEnvironment(adjustedEnvironmentForAppImage());

    connect(d->rawtherapee, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(slotErrorOccurred(QProcess::ProcessError)));

    connect(d->rawtherapee, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(slotProcessFinished(int,QProcess::ExitStatus)));

    connect(d->rawtherapee, SIGNAL(readyRead()),
            this, SLOT(slotProcessReadyRead()));

    // --------

    d->rawtherapee->setProgram(DFileOperations::findExecutable(QLatin1String("rawtherapee")));
    d->rawtherapee->setArguments(QStringList() << QLatin1String("-gimp") // Special mode used initially as Gimp plugin
                                               << filePath               // Input file
                                               << d->tempName);          // Output file

    qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee arguments:" << d->rawtherapee->arguments();

    d->rawtherapee->start();

    return d->rawtherapee->waitForStarted(10000);
}

void RawTherapeeRawImportPlugin::slotErrorOccurred(QProcess::ProcessError error)
{
    switch (error)
    {
        case QProcess::FailedToStart:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee :: Process has failed to start";
            break;
        }

        case QProcess::Crashed:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee :: Process has crashed";
            break;
        }

        case QProcess::Timedout:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee :: Process time-out";
            break;
        }

        case QProcess::WriteError:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee :: Process write error";
            break;
        }

        case QProcess::ReadError:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee :: Process read error";
            break;
        }

        default:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee :: Process error unknown";
            break;
        }
    }
}

void RawTherapeeRawImportPlugin::slotProcessFinished(int code, QProcess::ExitStatus status)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee :: return code:" << code << ":: Exit status:" << status;

    d->decoded = DImg(d->tempName);
    d->decoded.setAttribute(QLatin1String("isReadOnly"), true);

    if (d->decoded.isNull())
    {
        QString message = i18n("Error to import RAW image with RawTherapee\nClose this dialog to load RAW image with native import tool");
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

        FilterAction action(QLatin1String("rawtherapee:RawConverter"), 1, FilterAction::DocumentedHistory);
        action.setDisplayableName(QString::fromUtf8(I18N_NOOP("RawTherapee Raw Conversion")));
        d->decoded.addFilterAction(action);

        Q_EMIT signalDecodedImage(d->props, d->decoded);
    }

    delete d->rawtherapee;
    d->rawtherapee = nullptr;

    QFile::remove(d->tempName);
}

void RawTherapeeRawImportPlugin::slotProcessReadyRead()
{
    QByteArray data   = d->rawtherapee->readAllStandardError();
    QStringList lines = QString::fromUtf8(data).split(QLatin1Char('\n'), QT_SKIP_EMPTY_PARTS);

    Q_FOREACH (const QString& one, lines)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee ::" << one;
    }
}

} // namespace DigikamRawImportRawTherapeePlugin
