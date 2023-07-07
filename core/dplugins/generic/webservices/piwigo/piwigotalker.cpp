/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-30
 * Description : a tool to export items to Piwigo web service
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006      by Colin Guthrie <kde at colin dot guthr dot ie>
 * SPDX-FileCopyrightText: 2006-2023 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008      by Andrea Diamantini <adjam7 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2019 by Frederic Coiffier <frederic dot coiffier at free dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "piwigotalker.h"

// Qt includes

#include <QByteArray>
#include <QImage>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include <QCryptographicHash>
#include <QUuid>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmetadata.h"
#include "digikam_debug.h"
#include "piwigoitem.h"
#include "digikam_version.h"
#include "wstoolutils.h"
#include "networkmanager.h"
#include "previewloadthread.h"

namespace DigikamGenericPiwigoPlugin
{

class Q_DECL_HIDDEN PiwigoTalker::Private
{
public:

    explicit Private()
      : parent      (nullptr),
        state       (PG_LOGOUT),
        netMngr     (nullptr),
        reply       (nullptr),
        loggedIn    (false),
        chunkId     (0),
        nbOfChunks  (0),
        version     (-1),
        albumId     (0),
        photoId     (0),
        iface       (nullptr)
    {
    }

    QWidget*               parent;
    State                  state;
    QString                cookie;
    QUrl                   url;
    QNetworkAccessManager* netMngr;
    QNetworkReply*         reply;
    bool                   loggedIn;
    QByteArray             talker_buffer;
    uint                   chunkId;
    uint                   nbOfChunks;
    int                    version;

    QByteArray             md5sum;
    QString                path;
    QString                tmpPath;    ///< If set, contains a temporary file which must be deleted
    int                    albumId;
    int                    photoId;    ///< Filled when the photo already exist
    QString                comment;    ///< Synchronized with Piwigo comment
    QString                title;      ///< Synchronized with Piwigo name
    QString                author;     ///< Synchronized with Piwigo author
    QDateTime              date;       ///< Synchronized with Piwigo date
    DInfoInterface*        iface;
};

QString PiwigoTalker::s_authToken = QLatin1String("");

PiwigoTalker::PiwigoTalker(DInfoInterface* const iface, QWidget* const parent)
    : d(new Private)
{
    d->parent  = parent;
    d->iface   = iface;
    d->netMngr = NetworkManager::instance()->getNetworkManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));
}

PiwigoTalker::~PiwigoTalker()
{
    cancel();
    WSToolUtils::removeTemporaryDir("piwigo");

    delete d;
}

void PiwigoTalker::cancel()
{
    deleteTemporaryFile();

    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }
}

QString PiwigoTalker::getAuthToken()
{
    return s_authToken;
}

QByteArray PiwigoTalker::computeMD5Sum(const QString& filepath)
{
    QFile file(filepath);

    if (!file.open(QIODevice::ReadOnly))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "File open error:" << filepath;

        return QByteArray();
    }

    QByteArray md5sum = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5);
    file.close();

    return md5sum;
}

bool PiwigoTalker::loggedIn() const
{
    return d->loggedIn;
}

void PiwigoTalker::login(const QUrl& url, const QString& name, const QString& passwd)
{
    d->url   = url;
    d->state = PG_LOGIN;
    d->talker_buffer.resize(0);

    // Add the page to the URL

    if (!d->url.url().endsWith(QLatin1String(".php")))
    {
        d->url.setPath(d->url.path() + QLatin1Char('/') + QLatin1String("ws.php"));
    }

    s_authToken = QLatin1String(QUuid::createUuid().toByteArray().toBase64());

    QStringList qsl;
    qsl.append(QLatin1String("password=") + QString::fromUtf8(passwd.toUtf8().toPercentEncoding()));
    qsl.append(QLatin1String("method=pwg.session.login"));
    qsl.append(QLatin1String("username=") + QString::fromUtf8(name.toUtf8().toPercentEncoding()));
    QString dataParameters = qsl.join(QLatin1Char('&'));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    QNetworkRequest netRequest(d->url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    d->reply = d->netMngr->post(netRequest, buffer);

    Q_EMIT signalBusy(true);
}

void PiwigoTalker::listAlbums()
{
    d->state = PG_LISTALBUMS;
    d->talker_buffer.resize(0);

    QStringList qsl;
    qsl.append(QLatin1String("method=pwg.categories.getList"));
    qsl.append(QLatin1String("recursive=true"));
    QString dataParameters = qsl.join(QLatin1Char('&'));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    QNetworkRequest netRequest(d->url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    d->reply = d->netMngr->post(netRequest, buffer);

    Q_EMIT signalBusy(true);
}

bool PiwigoTalker::addPhoto(int   albumId,
                            const QString& mediaPath,
                            bool  rescale,
                            int   maxWidth,
                            int   maxHeight,
                            int   quality)
{
    d->state       = PG_CHECKPHOTOEXIST;
    d->talker_buffer.resize(0);

    d->path        = mediaPath;           // By default, d->path contains the original file
    d->tmpPath     = QLatin1String("");   // By default, no temporary file (except with rescaling)
    d->albumId     = albumId;

    d->md5sum      = computeMD5Sum(mediaPath);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << mediaPath << " " << d->md5sum.toHex();

    if (mediaPath.endsWith(QLatin1String(".mp4"))  || mediaPath.endsWith(QLatin1String(".MP4"))  ||
        mediaPath.endsWith(QLatin1String(".ogg"))  || mediaPath.endsWith(QLatin1String(".OGG"))  ||
        mediaPath.endsWith(QLatin1String(".webm")) || mediaPath.endsWith(QLatin1String(".WEBM")))
    {
        // Video management
        // Nothing to do
    }
    else
    {
        // Image management

        QImage image = PreviewLoadThread::loadHighQualitySynchronously(mediaPath).copyQImage();

        if (image.isNull())
        {
            if (!image.load(mediaPath) || image.isNull())
            {
                // Invalid image

                return false;
            }
        }

        if (!rescale)
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Upload the original version: " << d->path;
        }
        else
        {
            // Rescale the image

            if ((image.width() > maxWidth) || (image.height() > maxHeight))
            {
                image = image.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }

            d->path = WSToolUtils::makeTemporaryDir("piwigo")
                                                    .filePath(QUrl::fromLocalFile(mediaPath).fileName());
            d->tmpPath = d->path;
            image.save(d->path, "JPEG", quality);

            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Upload a resized version: " << d->path;

            // Restore all metadata with EXIF
            // in the resized version

            QScopedPointer<DMetadata> meta(new DMetadata);

            if (meta->load(mediaPath))
            {
                meta->setItemDimensions(image.size());
                meta->setItemOrientation(MetaEngine::ORIENTATION_NORMAL);
                meta->setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);
                meta->save(d->path, true);
            }
            else
            {
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Image " << mediaPath << " has no exif data";
            }
        }
    }

    // Metadata management

    // Complete name and comment for summary sending

    QFileInfo fi(mediaPath);
    d->title   = fi.completeBaseName();
    d->comment = QString();
    d->author  = QString();
    d->date    = fi.birthTime();

    // Look in the host database

    DItemInfo info(d->iface->itemInfo(QUrl::fromLocalFile(mediaPath)));

    if (!info.title().isEmpty())
    {
        d->title = info.title();
    }

    if (!info.comment().isEmpty())
    {
        d->comment = info.comment();
    }

    if (!info.creators().isEmpty())
    {
        d->author = info.creators().join(QLatin1String(" / "));
    }

    if (!info.dateTime().isNull())
    {
        d->date = info.dateTime();
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Title: "   << d->title;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Comment: " << d->comment;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Author: "  << d->author;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Date: "    << d->date;

    QStringList qsl;
    qsl.append(QLatin1String("method=pwg.images.exist"));
    qsl.append(QLatin1String("md5sud->list=") + QLatin1String(d->md5sum.toHex()));
    QString dataParameters = qsl.join(QLatin1Char('&'));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    QNetworkRequest netRequest(d->url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    d->reply = d->netMngr->post(netRequest, buffer);

    Q_EMIT signalProgressInfo(i18n("Check if %1 already exists", QUrl(mediaPath).fileName()));

    Q_EMIT signalBusy(true);

    return true;
}

void PiwigoTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != d->reply)
    {
        return;
    }

    d->reply    = nullptr;
    State state = d->state; // Can change in the treatment itself, so we cache it

    if (reply->error() != QNetworkReply::NoError)
    {
        if      (state == PG_LOGIN)
        {
            Q_EMIT signalLoginFailed(reply->errorString());
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << reply->errorString();
        }
        else if (state == PG_GETVERSION)
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << reply->errorString();

            // Version isn't mandatory and errors can be ignored
            // As login succeeded, albums can be listed

            listAlbums();
        }
        else if ((state == PG_CHECKPHOTOEXIST) || (state == PG_GETINFO)       ||
                 (state == PG_SETINFO)         || (state == PG_ADDPHOTOCHUNK) ||
                 (state == PG_ADDPHOTOSUMMARY))
        {
            deleteTemporaryFile();
            Q_EMIT signalAddPhotoFailed(reply->errorString());
        }
        else
        {
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18nc("@title:window", "Error"), reply->errorString());
        }

        Q_EMIT signalBusy(false);
        reply->deleteLater();

        return;
    }

    d->talker_buffer.append(reply->readAll());

    switch (state)
    {
        case (PG_LOGIN):
        {
            parseResponseLogin(d->talker_buffer);
            break;
        }

        case (PG_GETVERSION):
        {
            parseResponseGetVersion(d->talker_buffer);
            break;
        }

        case (PG_LISTALBUMS):
        {
            parseResponseListAlbums(d->talker_buffer);
            break;
        }

        case (PG_CHECKPHOTOEXIST):
        {
            parseResponseDoesPhotoExist(d->talker_buffer);
            break;
        }

        case (PG_GETINFO):
        {
            parseResponseGetInfo(d->talker_buffer);
            break;
        }

        case (PG_SETINFO):
        {
            parseResponseSetInfo(d->talker_buffer);
            break;
        }

        case (PG_ADDPHOTOCHUNK):
        {
            // Support for Web API >= 2.4
            parseResponseAddPhotoChunk(d->talker_buffer);
            break;
        }

        case (PG_ADDPHOTOSUMMARY):
        {
            parseResponseAddPhotoSummary(d->talker_buffer);
            break;
        }

        default:   // PG_LOGOUT
        {
            break;
        }
    }

    if ((state == PG_GETVERSION) && d->loggedIn)
    {
        listAlbums();
    }

    Q_EMIT signalBusy(false);
    reply->deleteLater();
}

void PiwigoTalker::parseResponseLogin(const QByteArray& data)
{
    QXmlStreamReader ts(data);
    QString line;
    bool foundResponse = false;
    d->loggedIn        = false;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseLogin: " << QString::fromUtf8(data);

    while (!ts.atEnd())
    {
        ts.readNext();

        if (ts.isStartElement())
        {
            foundResponse = true;

            if ((ts.name() == QLatin1String("rsp")) &&
                (ts.attributes().value(QLatin1String("stat")) == QLatin1String("ok")))
            {
                d->loggedIn = true;

                /** Request Version */

                d->state          = PG_GETVERSION;
                d->talker_buffer.resize(0);
                d->version        = -1;

                QByteArray buffer = "method=pwg.getVersion";

                QNetworkRequest netRequest(d->url);
                netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
                netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

                d->reply = d->netMngr->post(netRequest, buffer);

                Q_EMIT signalBusy(true);

                return;
            }
        }
    }

    if (!foundResponse)
    {
        Q_EMIT signalLoginFailed(i18n("Piwigo URL probably incorrect"));

        return;
    }

    if (!d->loggedIn)
    {
        Q_EMIT signalLoginFailed(i18n("Incorrect username or password specified"));
    }
}

void PiwigoTalker::parseResponseGetVersion(const QByteArray& data)
{
    QXmlStreamReader ts(data);
    QString line;
    QRegularExpression verrx(QRegularExpression::anchoredPattern(QLatin1String(".*?(\\d+)\\.(\\d+).*")));
    verrx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);

    bool foundResponse = false;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseGetVersion: " << QString::fromUtf8(data);

    while (!ts.atEnd())
    {
        ts.readNext();

        if (ts.isStartElement())
        {
            foundResponse = true;

            if ((ts.name() == QLatin1String("rsp")) &&
                (ts.attributes().value(QLatin1String("stat")) == QLatin1String("ok")))
            {
                QString v                     = ts.readElementText();
                QRegularExpressionMatch match = verrx.match(v);

                if (match.hasMatch())
                {
                    QStringList qsl = match.capturedTexts();
                    d->version      = qsl[1].toInt() * 100 + qsl[2].toInt();
                    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Version: " << d->version;

                    break;
                }
            }
        }
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "foundResponse : " << foundResponse;

    if (d->version < PIWIGO_VER_2_4)
    {
        d->loggedIn = false;
        Q_EMIT signalLoginFailed(i18n("Upload to Piwigo version inferior to 2.4 is no longer supported"));

        return;
    }
}

void PiwigoTalker::parseResponseListAlbums(const QByteArray& data)
{
    QString str        = QString::fromUtf8(data);
    QXmlStreamReader ts(data);
    QString line;
    bool foundResponse = false;
    bool success       = false;

    typedef QList<PiwigoAlbum> PiwigoAlbumList;
    PiwigoAlbumList albumList;
    PiwigoAlbumList::iterator iter = albumList.begin();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseListAlbums";

    while (!ts.atEnd())
    {
        ts.readNext();

        if (ts.isEndElement() && (ts.name() == QLatin1String("categories")))
        {
            break;
        }

        if (ts.isStartElement())
        {
            if ((ts.name() == QLatin1String("rsp")) &&
                (ts.attributes().value(QLatin1String("stat")) == QLatin1String("ok")))
            {
                foundResponse = true;
            }

            if (ts.name() == QLatin1String("categories"))
            {
                success = true;
            }

            if (ts.name() == QLatin1String("category"))
            {
                PiwigoAlbum album;
                album.m_refNum       = ts.attributes().value(QLatin1String("id")).toString().toInt();
                album.m_parentRefNum = -1;

                qCDebug(DIGIKAM_WEBSERVICES_LOG) << album.m_refNum << "\n";

                iter                 = albumList.insert(iter, album);
            }

            if (ts.name() == QLatin1String("name"))
            {
                (*iter).m_name = ts.readElementText();
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << (*iter).m_name << "\n";
            }

            if (ts.name() == QLatin1String("uppercats"))
            {
                QString uppercats   = ts.readElementText();
                QStringList catlist = uppercats.split(QLatin1Char(','));

                if ((catlist.size() > 1) && (catlist.at((uint)catlist.size() - 2).toInt() != (*iter).m_refNum))
                {
                    (*iter).m_parentRefNum = catlist.at((uint)catlist.size() - 2).toInt();
                    qCDebug(DIGIKAM_WEBSERVICES_LOG) << (*iter).m_parentRefNum << "\n";
                }
            }
        }
    }

    if (!foundResponse)
    {
        Q_EMIT signalError(i18n("Invalid response received from remote Piwigo"));

        return;
    }

    if (!success)
    {
        Q_EMIT signalError(i18n("Failed to list albums"));

        return;
    }

    // We need parent albums to come first for rest of the code to work

    std::sort(albumList.begin(), albumList.end());

    Q_EMIT signalAlbums(albumList);
}

void PiwigoTalker::parseResponseDoesPhotoExist(const QByteArray& data)
{
    QString str        = QString::fromUtf8(data);
    QXmlStreamReader ts(data);
    QString line;
    bool foundResponse = false;
    bool success       = false;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseDoesPhotoExist: " << QString::fromUtf8(data);

    while (!ts.atEnd())
    {
        ts.readNext();

        if (ts.name() == QLatin1String("rsp"))
        {
            foundResponse = true;

            if (ts.attributes().value(QLatin1String("stat")) == QLatin1String("ok"))
            {
                success = true;
            }

            // Originally, first versions of Piwigo 2.4.x returned an invalid XML as the element started with a digit
            // New versions are corrected (starting with _) : This code works with both versions

            QRegularExpression md5rx(QRegularExpression::anchoredPattern(QLatin1String("_?([a-f0-9]+)>([0-9]+)</.+")));
            ts.readNext();
            QRegularExpressionMatch match = md5rx.match(QString::fromUtf8(data.mid(ts.characterOffset())));

            if (match.hasMatch())
            {
                QStringList qsl1 = match.capturedTexts();

                if (qsl1[1] == QLatin1String(d->md5sum.toHex()))
                {
                    d->photoId = qsl1[2].toInt();
                    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "d->photoId: " << d->photoId;

                    Q_EMIT signalProgressInfo(i18n("Photo '%1' already exists.", d->title));

                    d->state   = PG_GETINFO;
                    d->talker_buffer.resize(0);

                    QStringList qsl2;
                    qsl2.append(QLatin1String("method=pwg.images.getInfo"));
                    qsl2.append(QLatin1String("image_id=") + QString::number(d->photoId));
                    QString dataParameters = qsl2.join(QLatin1Char('&'));
                    QByteArray buffer;
                    buffer.append(dataParameters.toUtf8());

                    QNetworkRequest netRequest(d->url);
                    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
                    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

                    d->reply = d->netMngr->post(netRequest, buffer);

                    return;
                }
            }
        }
    }

    if (!foundResponse)
    {
        Q_EMIT signalAddPhotoFailed(i18n("Invalid response received from remote Piwigo"));

        return;
    }

    if (!success)
    {
        Q_EMIT signalAddPhotoFailed(i18n("Failed to upload photo"));

        return;
    }

    if (d->version >= PIWIGO_VER_2_4)
    {
        QFileInfo fi(d->path);

        d->state      = PG_ADDPHOTOCHUNK;
        d->talker_buffer.resize(0);

        // Compute the number of chunks for the image

        d->nbOfChunks = (fi.size() / CHUNK_MAX_SIZE) + 1;
        d->chunkId    = 0;

        addNextChunk();
    }
    else
    {
        Q_EMIT signalAddPhotoFailed(i18n("Upload to Piwigo version inferior to 2.4 is no longer supported"));

        return;
    }
}

void PiwigoTalker::parseResponseGetInfo(const QByteArray& data)
{
    QString str        = QString::fromUtf8(data);
    QXmlStreamReader ts(data);
    QString line;
    bool foundResponse = false;
    bool success       = false;
    QList<int> categories;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseGetInfo: " << QString::fromUtf8(data);

    while (!ts.atEnd())
    {
        ts.readNext();

        if (ts.isStartElement())
        {
            if (ts.name() == QLatin1String("rsp"))
            {
                foundResponse = true;

                if (ts.attributes().value(QLatin1String("stat")) == QLatin1String("ok"))
                {
                    success = true;
                }
            }

            if (ts.name() == QLatin1String("category"))
            {
                if (ts.attributes().hasAttribute(QLatin1String("id")))
                {
                    QString id(ts.attributes().value(QLatin1String("id")).toString());
                    categories.append(id.toInt());
                }
            }
        }
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "success : " << success;

    if (!foundResponse)
    {
        Q_EMIT signalAddPhotoFailed(i18n("Invalid response received from remote Piwigo"));

        return;
    }

    if (categories.contains(d->albumId))
    {
        Q_EMIT signalAddPhotoFailed(i18n("Photo '%1' already exists in this album.", d->title));

        return;
    }
    else
    {
        categories.append(d->albumId);
    }

    d->state = PG_SETINFO;
    d->talker_buffer.resize(0);

    QStringList qsl_cat;

    for (int i = 0 ; i < categories.size() ; ++i)
    {
        qsl_cat.append(QString::number(categories.at(i)));
    }

    QStringList qsl;
    qsl.append(QLatin1String("method=pwg.images.setInfo"));
    qsl.append(QLatin1String("image_id=") + QString::number(d->photoId));
    qsl.append(QLatin1String("categories=") + QString::fromUtf8(qsl_cat.join(QLatin1Char(';')).toUtf8().toPercentEncoding()));
    QString dataParameters = qsl.join(QLatin1Char('&'));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    QNetworkRequest netRequest(d->url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    d->reply = d->netMngr->post(netRequest, buffer);

    return;
}

void PiwigoTalker::parseResponseSetInfo(const QByteArray& data)
{
    QString str        = QString::fromUtf8(data);
    QXmlStreamReader ts(data);
    QString line;
    bool foundResponse = false;
    bool success       = false;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseSetInfo: " << QString::fromUtf8(data);

    while (!ts.atEnd())
    {
        ts.readNext();

        if (ts.isStartElement())
        {
            if (ts.name() == QLatin1String("rsp"))
            {
                foundResponse = true;

                if (ts.attributes().value(QLatin1String("stat")) == QLatin1String("ok"))
                {
                    success = true;
                }

                break;
            }
        }
    }

    if (!foundResponse)
    {
        Q_EMIT signalAddPhotoFailed(i18n("Invalid response received from remote Piwigo"));

        return;
    }

    if (!success)
    {
        Q_EMIT signalAddPhotoFailed(i18n("Failed to upload photo"));

        return;
    }

    deleteTemporaryFile();

    Q_EMIT signalAddPhotoSucceeded();
}

void PiwigoTalker::addNextChunk()
{
    QFile imagefile(d->path);

    if (!imagefile.open(QIODevice::ReadOnly))
    {
        Q_EMIT signalProgressInfo(i18n("Error : Cannot open photo: %1", QUrl(d->path).fileName()));

        return;
    }

    d->chunkId++; // We start with chunk 1

    imagefile.seek((d->chunkId - 1) * CHUNK_MAX_SIZE);

    d->talker_buffer.resize(0);
    QStringList qsl;
    qsl.append(QLatin1String("method=pwg.images.addChunk"));
    qsl.append(QLatin1String("original_sum=") + QLatin1String(d->md5sum.toHex()));
    qsl.append(QLatin1String("position=") + QString::number(d->chunkId));
    qsl.append(QLatin1String("type=file"));
    qsl.append(QLatin1String("data=") + QString::fromUtf8(imagefile.read(CHUNK_MAX_SIZE).toBase64().toPercentEncoding()));
    QString dataParameters = qsl.join(QLatin1Char('&'));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    imagefile.close();

    QNetworkRequest netRequest(d->url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    d->reply = d->netMngr->post(netRequest, buffer);

    Q_EMIT signalProgressInfo(i18n("Upload the chunk %1/%2 of %3", d->chunkId, d->nbOfChunks, QUrl(d->path).fileName()));
}

void PiwigoTalker::parseResponseAddPhotoChunk(const QByteArray& data)
{
    QString str        = QString::fromUtf8(data);
    QXmlStreamReader ts(data);
    QString line;
    bool foundResponse = false;
    bool success       = false;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseAddPhotoChunk: " << QString::fromUtf8(data);

    while (!ts.atEnd())
    {
        ts.readNext();

        if (ts.isStartElement())
        {
            if (ts.name() == QLatin1String("rsp"))
            {
                foundResponse = true;

                if (ts.attributes().value(QLatin1String("stat")) == QLatin1String("ok"))
                {
                    success = true;
                }

                break;
            }
        }
    }

    if (!foundResponse || !success)
    {
        Q_EMIT signalProgressInfo(i18n("Warning : The full size photo cannot be uploaded."));
    }

    if (d->chunkId < d->nbOfChunks)
    {
        addNextChunk();
    }
    else
    {
        addPhotoSummary();
    }
}

void PiwigoTalker::addPhotoSummary()
{
    d->state = PG_ADDPHOTOSUMMARY;
    d->talker_buffer.resize(0);

    QStringList qsl;
    qsl.append(QLatin1String("method=pwg.images.add"));
    qsl.append(QLatin1String("original_sum=") + QLatin1String(d->md5sum.toHex()));
    qsl.append(QLatin1String("original_filename=") + QString::fromUtf8(QUrl(d->path).fileName().toUtf8().toPercentEncoding()));
    qsl.append(QLatin1String("name=") + QString::fromUtf8(d->title.toUtf8().toPercentEncoding()));

    if (!d->author.isEmpty())
    {
        qsl.append(QLatin1String("author=") + QString::fromUtf8(d->author.toUtf8().toPercentEncoding()));
    }

    if (!d->comment.isEmpty())
    {
        qsl.append(QLatin1String("comment=") + QString::fromUtf8(d->comment.toUtf8().toPercentEncoding()));
    }

    qsl.append(QLatin1String("categories=") + QString::number(d->albumId));
    qsl.append(QLatin1String("file_sum=") + QLatin1String(computeMD5Sum(d->path).toHex()));
    qsl.append(QLatin1String("date_creation=") +
               QString::fromUtf8(d->date.toString(QLatin1String("yyyy-MM-dd hh:mm:ss")).toUtf8().toPercentEncoding()));
/*
    qsl.append("tag_ids="); // TODO Implement this function
*/
    QString dataParameters = qsl.join(QLatin1Char('&'));
    QByteArray buffer;
    buffer.append(dataParameters.toUtf8());

    QNetworkRequest netRequest(d->url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));
    netRequest.setRawHeader("Authorization", s_authToken.toLatin1());

    d->reply = d->netMngr->post(netRequest, buffer);

    Q_EMIT signalProgressInfo(i18n("Upload the metadata of %1", QUrl(d->path).fileName()));
}

void PiwigoTalker::parseResponseAddPhotoSummary(const QByteArray& data)
{
    QString str        = QString::fromUtf8(data);
    QXmlStreamReader ts(data.mid(data.indexOf("<?xml")));
    QString line;
    bool foundResponse = false;
    bool success       = false;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseAddPhotoSummary: " << QString::fromUtf8(data);

    while (!ts.atEnd())
    {
        ts.readNext();

        if (ts.isStartElement())
        {
            if (ts.name() == QLatin1String("rsp"))
            {
                foundResponse = true;

                if (ts.attributes().value(QLatin1String("stat")) == QLatin1String("ok"))
                {
                   success = true;
                }

                break;
            }
        }
    }

    if (!foundResponse)
    {
        Q_EMIT signalAddPhotoFailed(i18n("Invalid response received from remote Piwigo (%1)", QString::fromUtf8(data)));

        return;
    }

    if (!success)
    {
        Q_EMIT signalAddPhotoFailed(i18n("Failed to upload photo"));

        return;
    }

    deleteTemporaryFile();

    Q_EMIT signalAddPhotoSucceeded();
}

void PiwigoTalker::deleteTemporaryFile()
{
    if (d->tmpPath.size())
    {
        QFile(d->tmpPath).remove();
        d->tmpPath = QLatin1String("");
    }
}

} // namespace DigikamGenericPiwigoPlugin
