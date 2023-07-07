/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-26
 * Description : a tool to export items to Facebook web service
 *
 * SPDX-FileCopyrightText: 2008-2010 by Luka Renko <lure at kubuntu dot org>
 * SPDX-FileCopyrightText: 2011      by Dirk Tilger <dirk dot kde at miriup dot de>
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fbtalker.h"

// Qt includes

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <QtAlgorithms>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QList>
#include <QApplication>
#include <QPushButton>
#include <QDialog>
#include <QDialogButtonBox>
#include <QUrlQuery>
#include <QSettings>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "fbmpform.h"
#include "wstoolutils.h"
#include "webbrowserdlg.h"
#include "networkmanager.h"


using namespace Digikam;

namespace DigikamGenericFaceBookPlugin
{

bool operator< (const FbUser& first, const FbUser& second)
{
    return first.name < second.name;
}

bool operator< (const FbAlbum& first, const FbAlbum& second)
{
    return first.title < second.title;
}

// -----------------------------------------------------------------------------

class Q_DECL_HIDDEN FbTalker::Private
{
public:

    enum State
    {
        FB_GETLOGGEDINUSER = 0,
        FB_LOGOUTUSER,
        FB_LISTALBUMS,
        FB_CREATEALBUM,
        FB_ADDPHOTO
    };

public:

    explicit Private()
      : dialog(nullptr),
        parent(nullptr),
        settings(nullptr),
        netMngr(nullptr),
        reply(nullptr),
        browser(nullptr),
        state(FB_GETLOGGEDINUSER)
    {
        apiURL       = QLatin1String("https://graph.facebook.com/%1/%2");
        authUrl      = QLatin1String("https://www.facebook.com/dialog/oauth");
        tokenUrl     = QLatin1String("https://graph.facebook.com/oauth/access_token");
        redirectUrl  = QLatin1String("https://www.facebook.com/connect/login_success.html");
        scope        = QLatin1String("user_photos,publish_pages,manage_pages"); //publish_to_groups,user_friends not necessary?
        apikey       = QLatin1String("400589753481372");
        clientSecret = QLatin1String("5b0b5cd096e110cd4f4c72f517e2c544");

        serviceName  = QLatin1String("Facebook");
        serviceTime  = QLatin1String("token_time");
        serviceKey   = QLatin1String("access_token");
    }

    QString                apiURL;
    QString                authUrl;
    QString                tokenUrl;
    QString                redirectUrl;
    QString                scope;
    QString                apikey;
    QString                clientSecret;
    QString                accessToken;
    QString                serviceName;
    QString                serviceTime;
    QString                serviceKey;

    QDateTime              expiryTime;

    QDialog*               dialog;
    QWidget*               parent;

    QSettings*             settings;

    QNetworkAccessManager* netMngr;
    QNetworkReply*         reply;

    WebBrowserDlg*         browser;

    State                  state;

    FbUser                 user;
};

// -----------------------------------------------------------------------------

FbTalker::FbTalker(QWidget* const parent)
    : d(new Private())
{
    d->parent   = parent;
    d->netMngr  = NetworkManager::instance()->getNetworkManager(this);
    d->settings = WSToolUtils::getOauthSettings(this);

    connect(this, SIGNAL(linkingSucceeded()),
            this, SLOT(slotLinkingSucceeded()));

    connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slotFinished(QNetworkReply*)));
}

FbTalker::~FbTalker()
{
    if (d->reply)
    {
        d->reply->abort();
    }

    delete d;
}

// ----------------------------------------------------------------------------------------------

void FbTalker::link()
{
    Q_EMIT signalBusy(true);
    Q_EMIT signalLoginProgress(1, 3);

    QUrl url(d->authUrl);
    QUrlQuery query(url);
    query.addQueryItem(QLatin1String("client_id"), d->apikey);
    query.addQueryItem(QLatin1String("response_type"), QLatin1String("token"));
    query.addQueryItem(QLatin1String("redirect_uri"), d->redirectUrl);
    query.addQueryItem(QLatin1String("scope"), d->scope);
    url.setQuery(query);

    delete d->browser;
    d->browser = new WebBrowserDlg(url, d->parent, true);
    d->browser->setModal(true);

    connect(d->browser, SIGNAL(urlChanged(QUrl)),
            this, SLOT(slotCatchUrl(QUrl)));

    connect(d->browser, SIGNAL(closeView(bool)),
            this, SIGNAL(signalBusy(bool)));

    d->browser->show();
}

void FbTalker::unlink()
{
    d->accessToken = QString();
    d->user        = FbUser();

    d->settings->beginGroup(d->serviceName);
    d->settings->remove(QString());
    d->settings->endGroup();

    Q_EMIT linkingSucceeded();
}

void FbTalker::cancel()
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    Q_EMIT signalBusy(false);
}

void FbTalker::slotLinkingSucceeded()
{
    if (d->accessToken.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "UNLINK to Facebook";
        Q_EMIT signalBusy(false);
        return;
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "LINK to Facebook";

    if (d->browser)
    {
        d->browser->close();
    }

    getLoggedInUser();
}

void FbTalker::slotCatchUrl(const QUrl& url)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Received URL from webview:" << url;

    QString   str = url.toString();
    QUrlQuery query(str.section(QLatin1Char('#'), -1, -1));

    if (query.hasQueryItem(QLatin1String("access_token")))
    {
        d->accessToken = query.queryItemValue(QLatin1String("access_token"));
        int seconds    = query.queryItemValue(QLatin1String("expires_in")).toInt();
        d->expiryTime  = QDateTime::currentDateTime().addSecs(seconds);

        writeSettings();

        qDebug(DIGIKAM_WEBSERVICES_LOG) << "Access token received";
        Q_EMIT linkingSucceeded();
    }
    else
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "No access token in URL";
        Q_EMIT signalBusy(false);
    }
}

FbUser FbTalker::getUser() const
{
    return d->user;
}

bool FbTalker::linked()
{
    return (!d->accessToken.isEmpty());
}

void FbTalker::getLoggedInUser()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "getLoggedInUser called";

    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    Q_EMIT signalBusy(true);
    Q_EMIT signalLoginProgress(2, 3);

    QUrl url(d->apiURL.arg(QLatin1String("me")).arg(QString()));

    QUrlQuery q;
    q.addQueryItem(QLatin1String("access_token"), d->accessToken);
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String("application/x-www-form-urlencoded"));

    d->reply = d->netMngr->get(netRequest);

    d->state = Private::FB_GETLOGGEDINUSER;
}

// ----------------------------------------------------------------------------------------------

void FbTalker::logout()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "logout called";

    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    Q_EMIT signalBusy(true);

    QUrl url(QLatin1String("https://www.facebook.com/logout.php"));
    QUrlQuery q;
    q.addQueryItem(QLatin1String("next"), d->redirectUrl);
    q.addQueryItem(QLatin1String("access_token"), d->accessToken);
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String("application/x-www-form-urlencoded"));

    d->reply = d->netMngr->get(netRequest);

    d->state = Private::FB_LOGOUTUSER;
}

//----------------------------------------------------------------------------------------------------

void FbTalker::listAlbums(long long userID)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Requesting albums for user" << userID;

    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    Q_EMIT signalBusy(true);
    Q_EMIT signalLoginProgress(1, 3);

    QUrl url;

    /*
     * If userID is specified, load albums of that user,
     * else load albums of current user
     */
    if (!userID)
    {
        url = QUrl(d->apiURL.arg(d->user.id)
                            .arg(QLatin1String("albums")));
    }
    else
    {
        url = QUrl(d->apiURL.arg(userID)
                            .arg(QLatin1String("albums")));
    }

    QUrlQuery q;
    q.addQueryItem(QLatin1String("fields"),
                   QLatin1String("id,name,description,privacy,link,location"));
    q.addQueryItem(QLatin1String("access_token"), d->accessToken);
    url.setQuery(q);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String("application/x-www-form-urlencoded"));

    d->reply = d->netMngr->get(netRequest);

    d->state = Private::FB_LISTALBUMS;
}

void FbTalker::createAlbum(const FbAlbum& album)
{
    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    Q_EMIT signalBusy(true);

    QUrlQuery params;
    params.addQueryItem(QLatin1String("access_token"), d->accessToken);
    params.addQueryItem(QLatin1String("name"), album.title);

    if (!album.location.isEmpty())
        params.addQueryItem(QLatin1String("location"), album.location);
    /*
     * description is deprecated and now a param of message
     */
    if (!album.description.isEmpty())
        params.addQueryItem(QLatin1String("message"), album.description);

    // TODO (Dirk): Wasn't that a requested feature in Bugzilla?
    switch (album.privacy)
    {
        case FB_ME:
            params.addQueryItem(QLatin1String("privacy"), QLatin1String("{'value':'SELF'}"));
            break;
        case FB_FRIENDS:
            params.addQueryItem(QLatin1String("privacy"), QLatin1String("{'value':'ALL_FRIENDS'}"));
            break;
        case FB_FRIENDS_OF_FRIENDS:
            params.addQueryItem(QLatin1String("privacy"), QLatin1String("{'value':'FRIENDS_OF_FRIENDS'}"));
            break;
        case FB_EVERYONE:
            params.addQueryItem(QLatin1String("privacy"), QLatin1String("{'value':'EVERYONE'}"));
            break;
        case FB_CUSTOM:
            //TODO
            params.addQueryItem(QLatin1String("privacy"), QLatin1String("{'value':'CUSTOM'}"));
            break;
    }

    QUrl url(QUrl(d->apiURL.arg(d->user.id)
                           .arg(QLatin1String("albums"))));
    url.setQuery(params);

    QNetworkRequest netRequest(url);
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                         QLatin1String("application/x-www-form-urlencoded"));

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "url to create new album" << netRequest.url() << params.query();

    d->reply = d->netMngr->post(netRequest, params.query().toUtf8());

    d->state = Private::FB_CREATEALBUM;
}

void FbTalker::addPhoto(const QString& imgPath, const QString& albumID, const QString& caption)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Adding photo" << imgPath << "to album with id"
                                     << albumID << "using caption '" << caption << "'";

    if (d->reply)
    {
        d->reply->abort();
        d->reply = nullptr;
    }

    Q_EMIT signalBusy(true);

    QMap<QString, QString> args;
    args[QLatin1String("access_token")] = d->accessToken;

    if (!caption.isEmpty())
        args[QLatin1String("message")]  = caption;

    FbMPForm form;

    for (QMap<QString, QString>::const_iterator it = args.constBegin() ;
        it != args.constEnd() ; ++it)
    {
        form.addPair(it.key(), it.value());
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "FORM:" << QT_ENDL << form.formData();

    if (!form.addFile(QUrl::fromLocalFile(imgPath).fileName(), imgPath))
    {
        Q_EMIT signalAddPhotoDone(666, i18n("Cannot open file"));
        Q_EMIT signalBusy(false);
        return;
    }

    form.finish();

    QVariant arg_1;

    if (albumID.isEmpty())
    {
        arg_1 = d->user.id;
    }
    else
    {
        arg_1 = albumID;
    }

    QNetworkRequest netRequest(QUrl(d->apiURL.arg(arg_1.toString()).arg(QLatin1String("photos"))));
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, form.contentType());

    d->reply = d->netMngr->post(netRequest, form.formData());

    d->state = Private::FB_ADDPHOTO;
}

//----------------------------------------------------------------------------------------------------

QString FbTalker::errorToText(int errCode, const QString& errMsg)
{
    QString transError;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "errorToText:" << errCode << ":" << errMsg;

    switch (errCode)
    {
        case 0:
            transError = QLatin1String("");
            break;
        case 2:
            transError = i18n("The service is not available at this time.");
            break;
        case 4:
            transError = i18n("The application has reached the maximum number of requests allowed.");
            break;
        case 102:
            transError = i18n("Invalid session key or session expired. Try to log in again.");
            break;
        case 120:
            transError = i18n("Invalid album ID.");
            break;
        case 321:
            transError = i18n("Album is full.");
            break;
        case 324:
            transError = i18n("Missing or invalid file.");
            break;
        case 325:
            transError = i18n("Too many unapproved photos pending.");
            break;
        default:
            transError = errMsg;
            break;
    }

    return transError;
}

void FbTalker::slotFinished(QNetworkReply* reply)
{
    if (reply != d->reply)
    {
        return;
    }

    d->reply = nullptr;

    if (reply->error() != QNetworkReply::NoError)
    {
        if (d->state == Private::FB_ADDPHOTO)
        {
            Q_EMIT signalBusy(false);
            Q_EMIT signalAddPhotoDone(reply->error(), reply->errorString());
        }
        else
        {
            Q_EMIT signalBusy(false);
            QMessageBox::critical(QApplication::activeWindow(),
                                  i18nc("@title:window", "Error"), reply->errorString());
        }

        reply->deleteLater();
        return;
    }

    QByteArray buffer = reply->readAll();

    switch (d->state)
    {
        case (Private::FB_GETLOGGEDINUSER):
            parseResponseGetLoggedInUser(buffer);
            break;
        case (Private::FB_LOGOUTUSER):
            parseResponseLogoutUser();
            break;
        case (Private::FB_LISTALBUMS):
            parseResponseListAlbums(buffer);
            break;
        case (Private::FB_CREATEALBUM):
            parseResponseCreateAlbum(buffer);
            break;
        case (Private::FB_ADDPHOTO):
            parseResponseAddPhoto(buffer);
            break;
    }

    reply->deleteLater();
}

int FbTalker::parseErrorResponse(const QDomElement& e, QString& errMsg)
{
    int errCode = -1;

    for (QDomNode node = e.firstChild() ;
         !node.isNull() ; node = node.nextSibling())
    {
        if (!node.isElement())
            continue;

        if (node.nodeName() == QLatin1String("error_code"))
        {
            errCode = node.toElement().text().toInt();
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error Code:" << errCode;
        }
        else if (node.nodeName() == QLatin1String("error_msg"))
        {
            errMsg = node.toElement().text();
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error Text:" << errMsg;
        }
    }

    return errCode;
}

void FbTalker::parseResponseGetLoggedInUser(const QByteArray& data)
{
    QString errMsg;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Logged in data" << doc;

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();
    d->user.id             = jsonObject[QLatin1String("id")].toString();

    if (!(QString::compare(jsonObject[QLatin1String("id")].toString(), QLatin1String(""), Qt::CaseInsensitive) == 0))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "ID found in response of GetLoggedInUser";
    }

    d->user.name       = jsonObject[QLatin1String("name")].toString();
    d->user.profileURL = jsonObject[QLatin1String("link")].toString();

    Q_EMIT signalLoginDone(0, QString());
}

void FbTalker::parseResponseAddPhoto(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) <<"Parse Add Photo data is" << data;
    int errCode       = -1;
    QString errMsg;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();

    if (jsonObject.contains(QLatin1String("id")))
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Id of photo exported is"
                                         << jsonObject[QLatin1String("id")].toString();
        errCode = 0;
    }

    if (jsonObject.contains(QLatin1String("error")))
    {
        QJsonObject obj = jsonObject[QLatin1String("error")].toObject();
        errCode         = obj[QLatin1String("code")].toInt();
        errMsg          = obj[QLatin1String("message")].toString();
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "add photo:" << doc;

    Q_EMIT signalBusy(false);
    Q_EMIT signalAddPhotoDone(errCode, errorToText(errCode, errMsg));
}

void FbTalker::parseResponseCreateAlbum(const QByteArray& data)
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) <<"Parse Create album data is" << data;
    int errCode       = -1;
    QString errMsg;
    QString newAlbumID;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();

    if (jsonObject.contains(QLatin1String("id")))
    {
        newAlbumID = jsonObject[QLatin1String("id")].toString();
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Id of album created is" << newAlbumID;
        errCode    = 0;
    }

    if (jsonObject.contains(QLatin1String("error")))
    {
        QJsonObject obj = jsonObject[QLatin1String("error")].toObject();
        errCode         = obj[QLatin1String("code")].toInt();
        errMsg          = obj[QLatin1String("message")].toString();
    }

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "error create photo:" << doc;

    Q_EMIT signalBusy(false);
    Q_EMIT signalCreateAlbumDone(errCode, errorToText(errCode, errMsg), newAlbumID);
}

void FbTalker::parseResponseListAlbums(const QByteArray& data)
{
    int errCode = -1;
    QString errMsg;
    QJsonParseError err;
    QList<FbAlbum> albumsList;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        Q_EMIT signalBusy(false);
        return;
    }

    QJsonObject jsonObject = doc.object();

    if (jsonObject.contains(QLatin1String("data")))
    {
        QJsonArray jsonArray = jsonObject[QLatin1String("data")].toArray();

        Q_FOREACH (const QJsonValue& value, jsonArray)
        {
            QJsonObject obj   = value.toObject();
            FbAlbum album;
            album.id          = obj[QLatin1String("id")].toString();
            album.title       = obj[QLatin1String("name")].toString();
            album.location    = obj[QLatin1String("location")].toString();
            album.url         = obj[QLatin1String("link")].toString();
            album.description = obj[QLatin1String("description")].toString();
            album.uploadable  = obj[QLatin1String("can_upload")].toBool();

            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "can_upload " << album.uploadable;

            if (QString::compare(obj[QLatin1String("privacy")].toString(),
                                 QLatin1String("ALL_FRIENDS"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_FRIENDS;
            }
            else if (QString::compare(obj[QLatin1String("privacy")].toString(),
                                      QLatin1String("FRIENDS_OF_FRIENDS"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_FRIENDS;
            }
            else if (QString::compare(obj[QLatin1String("privacy")].toString(),
                                      QLatin1String("EVERYONE"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_EVERYONE;
            }
            else if (QString::compare(obj[QLatin1String("privacy")].toString(),
                                      QLatin1String("CUSTOM"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_CUSTOM;
            }
            else if (QString::compare(obj[QLatin1String("privacy")].toString(),
                                      QLatin1String("SELF"), Qt::CaseInsensitive) == 0)
            {
                album.privacy = FB_ME;
            }

            albumsList.append(album);
        }

        errCode = 0;
    }

    if (jsonObject.contains(QLatin1String("error")))
    {
        QJsonObject obj = jsonObject[QLatin1String("error")].toObject();
        errCode         = obj[QLatin1String("code")].toInt();
        errMsg          = obj[QLatin1String("message")].toString();
    }

    std::sort(albumsList.begin(), albumsList.end());

    Q_EMIT signalBusy(false);
    Q_EMIT signalListAlbumsDone(errCode, errorToText(errCode, errMsg), albumsList);
}

void FbTalker::writeSettings()
{
    d->settings->beginGroup(d->serviceName);
    d->settings->setValue(d->serviceTime, d->expiryTime);
    d->settings->setValue(d->serviceKey,  d->accessToken);
    d->settings->endGroup();
}

void FbTalker::readSettings()
{
    d->settings->beginGroup(d->serviceName);
    d->expiryTime  = d->settings->value(d->serviceTime).toDateTime();
    d->accessToken = d->settings->value(d->serviceKey).toString();
    d->settings->endGroup();

    if (d->accessToken.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Access token is empty";
        Q_EMIT signalLoginDone(-1, QString());
    }
    else if (QDateTime::currentDateTime() > d->expiryTime)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Access token has expired";
        d->accessToken = QString();
        Q_EMIT signalLoginDone(-1, QString());
    }
    else
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Already Linked";
        Q_EMIT linkingSucceeded();
    }
}

void FbTalker::parseResponseLogoutUser()
{
    unlink();
    Q_EMIT signalLoginDone(-1, QString());
}

} // namespace DigikamGenericFaceBookPlugin
