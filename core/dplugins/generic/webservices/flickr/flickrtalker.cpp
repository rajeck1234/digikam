/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-07-07
 * Description : a tool to export images to Flickr web service
 *
 * SPDX-FileCopyrightText: 2005-2009 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2017-2019 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "flickrtalker.h"

// Qt includes

#include <QDomDocument>
#include <QDomElement>
#include <QByteArray>
#include <QFileInfo>
#include <QFile>
#include <QMap>
#include <QImage>
#include <QPointer>
#include <QUrlQuery>
#include <QStringList>
#include <QMessageBox>
#include <QInputDialog>
#include <QApplication>
#include <QProgressDialog>
#include <QDesktopServices>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_version.h"
#include "previewloadthread.h"
#include "webbrowserdlg.h"
#include "flickrwindow.h"
#include "flickrmpform.h"
#include "wstoolutils.h"
#include "dmetadata.h"

// OAuth2 library includes

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wextra-semi"
#endif

#include "o1.h"
#include "o0globals.h"
#include "o1requestor.h"
#include "o0settingsstore.h"
#include "networkmanager.h"

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

namespace DigikamGenericFlickrPlugin
{

class Q_DECL_HIDDEN FlickrTalker::Private
{
public:

    explicit Private()
      : parent      (nullptr),
        netMngr     (nullptr),
        reply       (nullptr),
        settings    (nullptr),
        state       (FE_LOGOUT),
        iface       (nullptr),
        o1          (nullptr),
        store       (nullptr),
        requestor   (nullptr),
        browser     (nullptr)
    {
        apiUrl      = QLatin1String("https://www.flickr.com/services/rest/");
        authUrl     = QLatin1String("https://www.flickr.com/services/oauth/authorize?perms=write");
        tokenUrl    = QLatin1String("https://www.flickr.com/services/oauth/request_token");
        accessUrl   = QLatin1String("https://www.flickr.com/services/oauth/access_token");
        uploadUrl   = QLatin1String("https://up.flickr.com/services/upload/");
        callbackUrl = QLatin1String("https://www.flickr.com");

        apikey      = QLatin1String("74f882bf4dabe22baaaace1f6d33c66b");
        secret      = QLatin1String("537d58e3ead2d6d5");
    }

    QWidget*               parent;

    QString                serviceName;
    QString                apiUrl;
    QString                authUrl;
    QString                tokenUrl;
    QString                accessUrl;
    QString                uploadUrl;
    QString                callbackUrl;
    QString                apikey;
    QString                secret;
    QString                maxSize;
    QString                username;
    QString                userId;
    QString                lastTmpFile;

    QNetworkAccessManager* netMngr;
    QNetworkReply*         reply;

    QSettings*             settings;

    State                  state;

    DInfoInterface*        iface;

    O1*                    o1;
    O0SettingsStore*       store;
    O1Requestor*           requestor;
    WebBrowserDlg*         browser;
};

FlickrTalker::FlickrTalker(QWidget* const parent,
                           const QString& serviceName,
                           DInfoInterface* const iface)
    : d(new Private)
{
    d->parent          = parent;
    d->serviceName     = serviceName;
    d->iface           = iface;
    m_photoSetsList    = nullptr;
    m_authProgressDlg  = nullptr;

    d->netMngr         = NetworkManager::instance()->getNetworkManager(this);

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));

    // Initialize selected photo set as empty.

    m_selectedPhotoSet = FPhotoSet();

    // Initialize photo sets list.

    m_photoSetsList    = new QList<FPhotoSet>();

    d->o1              = new O1(this);
    d->o1->setLocalPort(80);
    d->o1->setClientId(d->apikey);
    d->o1->setClientSecret(d->secret);
    d->o1->setCallbackUrl(d->callbackUrl);
    d->o1->setAuthorizeUrl(QUrl(d->authUrl));
    d->o1->setAccessTokenUrl(QUrl(d->accessUrl));
    d->o1->setRequestTokenUrl(QUrl(d->tokenUrl));
    d->o1->setUseExternalWebInterceptor(true);

    d->settings = WSToolUtils::getOauthSettings(this);
    d->store    = new O0SettingsStore(d->settings, QLatin1String(O2_ENCRYPTION_KEY), this);
    d->store->setGroupKey(d->serviceName);
    d->o1->setStore(d->store);

    connect(d->o1, SIGNAL(linkingFailed()),
            this, SLOT(slotLinkingFailed()));

    connect(d->o1, SIGNAL(linkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(d->o1, SIGNAL(openBrowser(QUrl)),
            this, SLOT(slotOpenBrowser(QUrl)));

    d->requestor = new O1Requestor(d->netMngr, d->o1, this);
}

FlickrTalker::~FlickrTalker()
{
    if (d->reply)
    {
        d->reply->abort();
    }

    WSToolUtils::removeTemporaryDir(d->serviceName.toLatin1().constData());

    delete m_photoSetsList;
    delete d;
}

void FlickrTalker::link(const QString& userName)
{
    Q_EMIT signalBusy(true);

    if (userName.isEmpty())
    {
        d->store->setGroupKey(d->serviceName);
    }
    else
    {
        d->store->setGroupKey(d->serviceName + userName);
    }

    d->o1->link();
}

void FlickrTalker::unLink()
{
    d->o1->unlink();
}

void FlickrTalker::removeUserName(const QString& userName)
{
    if (userName.startsWith(d->serviceName))
    {
        d->settings->beginGroup(userName);
        d->settings->remove(QString());
        d->settings->endGroup();
    }
}

void FlickrTalker::slotCatchUrl(const QUrl& url)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Received URL from webview:" << url;

    QString   str = url.toString();
    QUrlQuery query(str.section(QLatin1Char('?'), -1, -1));

    if (query.hasQueryItem(QLatin1String("oauth_token")))
    {
        QMap<QString, QString> queryParams;
        queryParams.insert(QLatin1String("oauth_token"),
                                         query.queryItemValue(QLatin1String("oauth_token")));
        queryParams.insert(QLatin1String("oauth_verifier"),
                                         query.queryItemValue(QLatin1String("oauth_verifier")));

        d->o1->onVerificationReceived(queryParams);
    }
}

void FlickrTalker::slotLinkingFailed()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Flickr fail";
    d->username = QString();
    Q_EMIT signalBusy(false);
}

void FlickrTalker::slotLinkingSucceeded()
{
    if (!d->o1->linked())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to Flickr ok";
        d->username = QString();
        return;
    }

    if (d->browser)
    {
        d->browser->close();
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Flickr ok";

    d->username = d->o1->extraTokens()[QLatin1String("username")].toString();
    d->userId   = d->o1->extraTokens()[QLatin1String("user_nsid")].toString();

    if (d->store->groupKey() == d->serviceName)
    {
        d->settings->beginGroup(d->serviceName);
        QStringList keys = d->settings->allKeys();
        d->settings->endGroup();

        Q_FOREACH (const QString& key, keys)
        {
            d->settings->beginGroup(d->serviceName);
            QVariant value = d->settings->value(key);
            d->settings->endGroup();
            d->settings->beginGroup(d->serviceName + d->username);
            d->settings->setValue(key, value);
            d->settings->endGroup();
        }

        d->store->setGroupKey(d->serviceName + d->username);
        removeUserName(d->serviceName);
    }

    Q_EMIT signalLinkingSucceeded();
}

void FlickrTalker::slotOpenBrowser(const QUrl& url)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Open Browser... (" << url << ")";

#ifdef HAVE_QWEBENGINE

    delete d->browser;
    d->browser = new WebBrowserDlg(url, d->parent, true);
    d->browser->setModal(true);

    connect(d->browser, SIGNAL(urlChanged(QUrl)),
            this, SLOT(slotCatchUrl(QUrl)));

    connect(d->browser, SIGNAL(closeView(bool)),
            this, SIGNAL(signalBusy(bool)));

    d->browser->show();

#else

    QDesktopServices::openUrl(url);
    QPointer<QInputDialog> textDlg = new QInputDialog(d->parent);

    textDlg->setWindowTitle(i18nc("@title:window", "Enter Flickr Login URL"));
    textDlg->setLabelText(i18n("Copy the full URL from the external browser "
                               "that contains the \"oauth_token\" entry:"));
    textDlg->resize(770, textDlg->sizeHint().height());
    textDlg->setInputMode(QInputDialog::TextInput);
    textDlg->setTextEchoMode(QLineEdit::Normal);
    textDlg->setModal(true);

    if (textDlg->exec() != QDialog::Accepted)
    {
        Q_EMIT signalBusy(false);
        delete textDlg;

        return;
    }

    slotCatchUrl(QUrl(textDlg->textValue().trimmed()));
    delete textDlg;

#endif

}

QString FlickrTalker::getMaxAllowedFileSize()
{
    return d->maxSize;
}

void FlickrTalker::maxAllowedFileSize()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    if (!d->o1->linked())
    {
        return;
    }

    QUrl url(d->apiUrl);
    QNetworkRequest netRequest(url);
    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_XFORM));

    reqParams << O0RequestParameter("method", "flickr.people.getLimits");

    QByteArray postData = O1::createQueryParameters(reqParams);

    d->reply = d->requestor->post(netRequest, reqParams, postData);

    d->state = FE_GETMAXSIZE;
    m_authProgressDlg->setLabelText(i18n("Getting the maximum allowed file size."));
    m_authProgressDlg->setMaximum(4);
    m_authProgressDlg->setValue(1);

    Q_EMIT signalBusy(true);
}

void FlickrTalker::listPhotoSets()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    if (!d->o1->linked())
    {
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "List photoset invoked";

    QUrl url(d->apiUrl);
    QNetworkRequest netRequest(url);
    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_XFORM));

    reqParams << O0RequestParameter("method", "flickr.photosets.getList");

    QByteArray postData = O1::createQueryParameters(reqParams);

    d->reply = d->requestor->post(netRequest, reqParams, postData);

    d->state = FE_LISTPHOTOSETS;
    Q_EMIT signalBusy(true);
}

void FlickrTalker::getPhotoProperty(const QString& method, const QStringList& argList)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    if (!d->o1->linked())
    {
        return;
    }

    QUrl url(d->apiUrl);
    QNetworkRequest netRequest(url);
    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_XFORM));

    reqParams << O0RequestParameter("method", method.toLatin1());

    for (QStringList::const_iterator it = argList.constBegin(); it != argList.constEnd(); ++it)
    {
        QStringList str = (*it).split(QLatin1Char('='), QT_SKIP_EMPTY_PARTS);
        reqParams << O0RequestParameter(str[0].toLatin1(), str[1].toLatin1());
    }

    QByteArray postData = O1::createQueryParameters(reqParams);

    d->reply = d->requestor->post(netRequest, reqParams, postData);

    d->state = FE_GETPHOTOPROPERTY;
    Q_EMIT signalBusy(true);
}

void FlickrTalker::listPhotos(const QString& /*albumName*/)
{
    // TODO
}

void FlickrTalker::createPhotoSet(const QString& /*albumName*/, const QString& albumTitle,
                                  const QString& albumDescription, const QString& primaryPhotoId)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    if (!d->o1->linked())
    {
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Create photoset invoked";

    QUrl url(d->apiUrl);
    QNetworkRequest netRequest(url);
    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_XFORM));

    reqParams << O0RequestParameter("method", "flickr.photosets.create");
    reqParams << O0RequestParameter("title", albumTitle.toLatin1());
    reqParams << O0RequestParameter("description", albumDescription.toLatin1());
    reqParams << O0RequestParameter("primary_photo_id", primaryPhotoId.toLatin1());

    QByteArray postData = O1::createQueryParameters(reqParams);

    d->reply = d->requestor->post(netRequest, reqParams, postData);

    d->state = FE_CREATEPHOTOSET;
    Q_EMIT signalBusy(true);
}

void FlickrTalker::addPhotoToPhotoSet(const QString& photoId,
                                      const QString& photoSetId)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    if (!d->o1->linked())
    {
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "AddPhotoToPhotoSet invoked";

    /*
     * If the photoset id starts with the special string "UNDEFINED_", it means
     * it doesn't exist yet on Flickr and needs to be created. Note that it's
     * not necessary to subsequently add the photo to the photo set, as this
     * is done in the set creation call to Flickr.
     */

    if (photoSetId.startsWith(QLatin1String("UNDEFINED_")))
    {
        createPhotoSet(QLatin1String(""), m_selectedPhotoSet.title, m_selectedPhotoSet.description, photoId);
    }
    else
    {
        QUrl url(d->apiUrl);
        QNetworkRequest netRequest(url);
        QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

        netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_XFORM));

        reqParams << O0RequestParameter("method", "flickr.photosets.addPhoto");
        reqParams << O0RequestParameter("photoset_id", photoSetId.toLatin1());
        reqParams << O0RequestParameter("photo_id", photoId.toLatin1());

        QByteArray postData = O1::createQueryParameters(reqParams);

        d->reply = d->requestor->post(netRequest, reqParams, postData);

        d->state = FE_ADDPHOTOTOPHOTOSET;
        Q_EMIT signalBusy(true);
    }
}

bool FlickrTalker::addPhoto(const QString& photoPath, const FPhotoInfo& info,
                            bool original, bool rescale, int maxDim, int imageQuality)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    if (!d->o1->linked())
    {
        return false;
    }

    Q_EMIT signalBusy(true);

    QUrl url(d->uploadUrl);
    QNetworkRequest netRequest(url);
    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    QString path = photoPath;
    FlickrMPForm  form;

    QString ispublic = (info.is_public == 1) ? QLatin1String("1") : QLatin1String("0");
    form.addPair(QLatin1String("is_public"), ispublic, QLatin1String("text/plain"));
    reqParams << O0RequestParameter("is_public", ispublic.toLatin1());

    QString isfamily = (info.is_family == 1) ? QLatin1String("1") : QLatin1String("0");
    form.addPair(QLatin1String("is_family"), isfamily, QLatin1String("text/plain"));
    reqParams << O0RequestParameter("is_family", isfamily.toLatin1());

    QString isfriend = (info.is_friend == 1) ? QLatin1String("1") : QLatin1String("0");
    form.addPair(QLatin1String("is_friend"), isfriend, QLatin1String("text/plain"));
    reqParams << O0RequestParameter("is_friend", isfriend.toLatin1());

    QString safetyLevel = QString::number(static_cast<int>(info.safety_level));
    form.addPair(QLatin1String("safety_level"), safetyLevel, QLatin1String("text/plain"));
    reqParams << O0RequestParameter("safety_level", safetyLevel.toLatin1());

    QString contentType = QString::number(static_cast<int>(info.content_type));
    form.addPair(QLatin1String("content_type"), contentType, QLatin1String("text/plain"));
    reqParams << O0RequestParameter("content_type", contentType.toLatin1());

    QString tags = QLatin1Char('"') + info.tags.join(QLatin1String("\" \"")) + QLatin1Char('"');

    if (tags.length() > 0)
    {
        form.addPair(QLatin1String("tags"), tags, QLatin1String("text/plain"));
        reqParams << O0RequestParameter("tags", tags.toUtf8());
    }

    if (!info.title.isEmpty())
    {
        form.addPair(QLatin1String("title"), info.title, QLatin1String("text/plain"));
        reqParams << O0RequestParameter("title", info.title.toUtf8());
    }

    if (!info.description.isEmpty())
    {
        form.addPair(QLatin1String("description"), info.description, QLatin1String("text/plain"));
        reqParams << O0RequestParameter("description", info.description.toUtf8());
    }

    if (!original)
    {
        QImage image = PreviewLoadThread::loadHighQualitySynchronously(photoPath).copyQImage();

        if (image.isNull())
        {
            image.load(photoPath);
        }

        if (!image.isNull())
        {
            if (!d->lastTmpFile.isEmpty())
            {
                QFile::remove(d->lastTmpFile);
            }

            path = WSToolUtils::makeTemporaryDir(d->serviceName.toLatin1().constData()).filePath(QFileInfo(photoPath)
                                                                          .baseName().trimmed() + QLatin1String(".jpg"));

            if (rescale && (image.width() > maxDim || image.height() > maxDim))
            {
                image = image.scaled(maxDim, maxDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }

            image.save(path, "JPEG", imageQuality);
            d->lastTmpFile = path;

            // Restore all metadata.

            QScopedPointer<DMetadata> meta(new DMetadata);

            if (meta->load(photoPath))
            {
                meta->setItemDimensions(image.size());
                meta->setItemOrientation(MetaEngine::ORIENTATION_NORMAL);

                // NOTE: see bug #153207: Flickr use IPTC keywords to create Tags in web interface
                //       As IPTC do not support UTF-8, we need to remove it.
                //       This function call remove all Application2 Tags.

                meta->removeIptcTags(QStringList() << QLatin1String("Application2"));

                // NOTE: see bug # 384260: Flickr use Xmp.dc.subject to create Tags
                //       in web interface, we need to remove it.
                //       This function call remove all Dublin Core Tags.

                meta->removeXmpTags(QStringList() << QLatin1String("dc"));

                meta->setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);
                meta->save(path, true);
            }
            else
            {
                qCWarning(DIGIKAM_WEBSERVICES_LOG) << "Flickr::Image do not have metadata";
            }

            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Resizing and saving to temp file: " << path;
        }
    }

    QFileInfo tempFileInfo(path);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "QUrl path is " << QUrl::fromLocalFile(path) << "Image size (in bytes) is "<< tempFileInfo.size();

    if (tempFileInfo.size() > (getMaxAllowedFileSize().toLongLong()))
    {
        Q_EMIT signalAddPhotoFailed(i18n("File Size exceeds maximum allowed file size."));
        Q_EMIT signalBusy(false);

        return false;
    }

    if (!form.addFile(QLatin1String("photo"), path))
    {
        Q_EMIT signalBusy(false);

        return false;
    }

    form.finish();

    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());

    d->reply = d->requestor->post(netRequest, reqParams, form.formData());
    d->state = FE_ADDPHOTO;

    return true;
}

void FlickrTalker::setGeoLocation(const QString& photoId, const QString& lat, const QString& lon)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    if (!d->o1->linked())
    {
        return;
    }

    QUrl url(d->apiUrl);
    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String(O2_MIME_TYPE_XFORM));

    QList<O0RequestParameter> reqParams = QList<O0RequestParameter>();

    reqParams << O0RequestParameter("method", "flickr.photos.geo.setLocation");
    reqParams << O0RequestParameter("photo_id", photoId.toLatin1());
    reqParams << O0RequestParameter("lat", lat.toLatin1());
    reqParams << O0RequestParameter("lon", lon.toLatin1());

    QByteArray postData = O1::createQueryParameters(reqParams);
    d->reply            = d->requestor->post(netRequest, reqParams, postData);
    d->state            = FE_SETGEO;

    Q_EMIT signalBusy(true);
}

QString FlickrTalker::getUserName() const
{
    return d->username;
}

QString FlickrTalker::getUserId() const
{
    return d->userId;
}

void FlickrTalker::cancel()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    if (m_authProgressDlg && !m_authProgressDlg->isHidden())
    {
        m_authProgressDlg->hide();
    }
}

void FlickrTalker::slotError(const QString& error)
{
    QString transError;
    int errorNo = error.toInt();

    switch (errorNo)
    {
        case 2:
            transError = i18n("No photo specified");
            break;

        case 3:
            transError = i18n("General upload failure");
            break;

        case 4:
            transError = i18n("Filesize was zero");
            break;

        case 5:
            transError = i18n("Filetype was not recognized");
            break;

        case 6:
            transError = i18n("User exceeded upload limit");
            break;

        case 96:
            transError = i18n("Invalid signature");
            break;

        case 97:
            transError = i18n("Missing signature");
            break;

        case 98:
            transError = i18n("Login Failed / Invalid auth token");
            break;

        case 100:
            transError = i18n("Invalid API Key");
            break;

        case 105:
            transError = i18n("Service currently unavailable");
            break;

        case 108:
            transError = i18n("Invalid Frob");
            break;

        case 111:
            transError = i18n("Format \"xxx\" not found");
            break;

        case 112:
            transError = i18n("Method \"xxx\" not found");
            break;

        case 114:
            transError = i18n("Invalid SOAP envelope");
            break;

        case 115:
            transError = i18n("Invalid XML-RPC Method Call");
            break;

        case 116:
            transError = i18n("The POST method is now required for all setters");
            break;

        default:
            transError = i18n("Unknown error");
            break;
    };

    QMessageBox::critical(QApplication::activeWindow(),
                          i18nc("@title:window", "Error"),
                          i18n("Error Occurred: %1\nCannot proceed any further.", transError));
}

void FlickrTalker::slotFinished(QNetworkReply* reply)
{
    Q_EMIT signalBusy(false);

    if (reply != d->reply)
    {
        return;
    }

    d->reply = nullptr;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->state == FE_ADDPHOTO)
        {
            Q_EMIT signalAddPhotoFailed(reply->errorString());
        }
        else
        {
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18nc("@title:window", "Error"), reply->errorString());
        }

        reply->deleteLater();

        return;
    }

    QByteArray buffer = reply->readAll();

    switch (d->state)
    {
        case (FE_LOGIN):
/*
            parseResponseLogin(buffer);
*/
            break;

        case (FE_LISTPHOTOSETS):
            parseResponseListPhotoSets(buffer);
            break;

        case (FE_LISTPHOTOS):
            parseResponseListPhotos(buffer);
            break;

        case (FE_GETPHOTOPROPERTY):
            parseResponsePhotoProperty(buffer);
            break;

        case (FE_ADDPHOTO):
            parseResponseAddPhoto(buffer);
            break;

        case (FE_ADDPHOTOTOPHOTOSET):
            parseResponseAddPhotoToPhotoSet(buffer);
            break;

        case (FE_CREATEPHOTOSET):
            parseResponseCreatePhotoSet(buffer);
            break;

        case (FE_GETMAXSIZE):
            parseResponseMaxSize(buffer);
            break;

        case (FE_SETGEO):
            parseResponseSetGeoLocation(buffer);
            break;

        default:  // FR_LOGOUT
            break;
    }

    reply->deleteLater();
}

void FlickrTalker::parseResponseMaxSize(const QByteArray& data)
{
    QString errorString;
    QDomDocument doc(QLatin1String("mydocument"));

    if (!doc.setContent(data))
    {
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode node       = docElem.firstChild();

    QDomElement e;

    while (!node.isNull())
    {
        if (node.isElement() && (node.nodeName() == QLatin1String("person")))
        {
            e                = node.toElement();
            QDomNode details = e.firstChild();

            while (!details.isNull())
            {
                if (details.isElement())
                {
                    e = details.toElement();

                    if (details.nodeName() == QLatin1String("videos"))
                    {
                        QDomAttr a = e.attributeNode(QLatin1String("maxupload"));
                        d->maxSize = a.value();
                        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Max upload size is"<<d->maxSize;
                    }
                }

                details = details.nextSibling();
            }
        }

        if (node.isElement() && (node.nodeName() == QLatin1String("err")))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Checking Error in response";
            errorString = node.toElement().attribute(QLatin1String("code"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error code=" << errorString;
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Msg=" << node.toElement().attribute(QLatin1String("msg"));
        }

        node = node.nextSibling();
    }

    m_authProgressDlg->hide();
}

void FlickrTalker::parseResponseCreatePhotoSet(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Parse response create photoset received " << data;

    //bool success = false;

    QDomDocument doc(QLatin1String("getListPhotoSets"));

    if (!doc.setContent(data))
    {
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode    node    = docElem.firstChild();
    QDomElement e;

    while (!node.isNull())
    {
        if (node.isElement() && (node.nodeName() == QLatin1String("photoset")))
        {
            // Parse the id from the response.

            QString new_id = node.toElement().attribute(QLatin1String("id"));

            // Set the new id in the photo sets list.

            QList<FPhotoSet>::iterator it = m_photoSetsList->begin();

            while (it != m_photoSetsList->end())
            {
                if (it->id == m_selectedPhotoSet.id)
                {
                    it->id = new_id;
                    break;
                }

                ++it;
            }

            // Set the new id in the selected photo set.

            m_selectedPhotoSet.id = new_id;

            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "PhotoSet created successfully with id" << new_id;
            Q_EMIT signalAddPhotoSetSucceeded();
        }

        if (node.isElement() && (node.nodeName() == QLatin1String("err")))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Checking Error in response";
            QString code = node.toElement().attribute(QLatin1String("code"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error code=" << code;
            QString msg  = node.toElement().attribute(QLatin1String("msg"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Msg=" << msg;
            QMessageBox::critical(QApplication::activeWindow(), i18nc("@title:window", "Error"), i18n("PhotoSet creation failed: ") + msg);
        }

        node = node.nextSibling();
    }
}

void FlickrTalker::parseResponseListPhotoSets(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseListPhotosets" << data;
    bool success = false;
    QDomDocument doc(QLatin1String("getListPhotoSets"));

    if (!doc.setContent(data))
    {
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode    node    = docElem.firstChild();
    QDomElement e;

    QString photoSet_id, photoSet_title, photoSet_description;
    m_photoSetsList->clear();

    while (!node.isNull())
    {
        if (node.isElement() && (node.nodeName() == QLatin1String("photosets")))
        {
            e                    = node.toElement();
            QDomNode details     = e.firstChild();
            FPhotoSet fps;
            QDomNode detailsNode = details;

            while (!detailsNode.isNull())
            {
                if (detailsNode.isElement())
                {
                    e = detailsNode.toElement();

                    if (detailsNode.nodeName() == QLatin1String("photoset"))
                    {
                        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "id=" << e.attribute(QLatin1String("id"));
                        photoSet_id              = e.attribute(QLatin1String("id"));     // this is what is obtained from data.
                        fps.id                   = photoSet_id;
                        QDomNode photoSetDetails = detailsNode.firstChild();
                        QDomElement e_detail;

                        while (!photoSetDetails.isNull())
                        {
                            e_detail = photoSetDetails.toElement();

                            if      (photoSetDetails.nodeName() == QLatin1String("title"))
                            {
                                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Title=" << e_detail.text();
                                photoSet_title = e_detail.text();
                                fps.title      = photoSet_title;
                            }
                            else if (photoSetDetails.nodeName() == QLatin1String("description"))
                            {
                                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Description =" << e_detail.text();
                                photoSet_description = e_detail.text();
                                fps.description      = photoSet_description;
                            }

                            photoSetDetails = photoSetDetails.nextSibling();
                        }

                        m_photoSetsList->append(fps);
                    }
                }

                detailsNode = detailsNode.nextSibling();
            }

            details = details.nextSibling();
            success = true;
        }

        if (node.isElement() && (node.nodeName() == QLatin1String("err")))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Checking Error in response";
            QString code = node.toElement().attribute(QLatin1String("code"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error code=" << code;
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Msg=" << node.toElement().attribute(QLatin1String("msg"));
            Q_EMIT signalError(code);
        }

        node = node.nextSibling();
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "GetPhotoList finished";

    if (!success)
    {
        Q_EMIT signalListPhotoSetsFailed(i18n("Failed to fetch list of photo sets."));
    }
    else
    {
        Q_EMIT signalListPhotoSetsSucceeded();
        maxAllowedFileSize();
    }
}

void FlickrTalker::parseResponseListPhotos(const QByteArray& data)
{
    QDomDocument doc(QLatin1String("getPhotosList"));

    if (!doc.setContent(data))
    {
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode node       = docElem.firstChild();

    //QDomElement e;
    //TODO
}

void FlickrTalker::parseResponseCreateAlbum(const QByteArray& data)
{
    QDomDocument doc(QLatin1String("getCreateAlbum"));

    if (!doc.setContent(data))
    {
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode node       = docElem.firstChild();

    //TODO
}

void FlickrTalker::parseResponseAddPhoto(const QByteArray& data)
{
    bool    success = false;
    QString line;
    QDomDocument doc(QLatin1String("AddPhoto Response"));

    if (!doc.setContent(data))
    {
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode node       = docElem.firstChild();
    QDomElement e;
    QString photoId;

    while (!node.isNull())
    {
        if (node.isElement() && (node.nodeName() == QLatin1String("photoid")))
        {
            e                = node.toElement();           // try to convert the node to an element.
            QDomNode details = e.firstChild();
            photoId          = e.text();
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Photoid= " << photoId;
            success          = true;
        }

        if (node.isElement() && (node.nodeName() == QLatin1String("err")))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Checking Error in response";
            QString code = node.toElement().attribute(QLatin1String("code"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error code=" << code;
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Msg=" << node.toElement().attribute(QLatin1String("msg"));
            Q_EMIT signalError(code);
        }

        node = node.nextSibling();
    }

    if (!success)
    {
        Q_EMIT signalAddPhotoFailed(i18n("Failed to upload photo"));
    }
    else
    {
        QString photoSetId = m_selectedPhotoSet.id;

        if (photoSetId == QLatin1String("-1"))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "PhotoSet Id not set, not adding the photo to any photoset";
            Q_EMIT signalAddPhotoSucceeded(photoId);
        }
        else
        {
            addPhotoToPhotoSet(photoId, photoSetId);
        }
    }
}

void FlickrTalker::parseResponsePhotoProperty(const QByteArray& data)
{
    bool         success = false;
    QString      line;
    QDomDocument doc(QLatin1String("Photos Properties"));

    if (!doc.setContent(data))
    {
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode    node    = docElem.firstChild();
    QDomElement e;

    while (!node.isNull())
    {
        if (node.isElement() && (node.nodeName() == QLatin1String("photoid")))
        {
            e                = node.toElement();                 // try to convert the node to an element.
            QDomNode details = e.firstChild();
            success          = true;
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Photoid=" << e.text();
        }

        if (node.isElement() && (node.nodeName() == QLatin1String("err")))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Checking Error in response";
            QString code = node.toElement().attribute(QLatin1String("code"));
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error code=" << code;
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Msg=" << node.toElement().attribute(QLatin1String("msg"));
            Q_EMIT signalError(code);
        }

        node = node.nextSibling();
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "GetToken finished";

    if (!success)
    {
        Q_EMIT signalAddPhotoFailed(i18n("Failed to query photo information"));
    }
    else
    {
        Q_EMIT signalAddPhotoSucceeded(QLatin1String(""));
    }
}

void FlickrTalker::parseResponseAddPhotoToPhotoSet(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseListPhotosets" << data;
    Q_EMIT signalAddPhotoSucceeded(QLatin1String(""));
}

void FlickrTalker::parseResponseSetGeoLocation(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "parseResponseSetGeoLocation" << data;
    Q_EMIT signalAddPhotoSucceeded(QLatin1String(""));
}

} // namespace DigikamGenericFlickrPlugin
