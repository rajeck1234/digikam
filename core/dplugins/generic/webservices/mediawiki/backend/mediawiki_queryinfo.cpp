/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mediawiki_queryinfo.h"

// Qt includes

#include <QDateTime>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>
#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QNetworkReply>
#include <QNetworkRequest>

// Local includes

#include "mediawiki_iface.h"
#include "mediawiki_job_p.h"

namespace MediaWiki
{

class Q_DECL_HIDDEN QueryInfoPrivate : public JobPrivate
{
public:

    explicit QueryInfoPrivate(Iface& MediaWiki)
        : JobPrivate(MediaWiki)
    {
    }

    QVector<Protection>    protections;
    QMap<QString, QString> requestParameter;
    Page                   page;
};

QueryInfo::QueryInfo(Iface& MediaWiki, QObject* const parent)
    : Job(*new QueryInfoPrivate(MediaWiki), parent)
{
}

QueryInfo::~QueryInfo()
{
}

void QueryInfo::setPageName(const QString& title)
{
    Q_D(QueryInfo);
    d->requestParameter[QStringLiteral("titles")] = title;
}

void QueryInfo::setToken(const QString& token)
{
    Q_D(QueryInfo);
    d->requestParameter[QStringLiteral("meta")] = token;
}

void QueryInfo::setPageId(unsigned int id)
{
    Q_D(QueryInfo);
    d->requestParameter[QStringLiteral("pageids")] = QString::number(id);
}

void QueryInfo::setRevisionId(unsigned int id)
{
    Q_D(QueryInfo);
    d->requestParameter[QStringLiteral("revids")] = QString::number(id);
}

void QueryInfo::start()
{
    QTimer::singleShot(0, this, SLOT(doWorkSendRequest()));
}

void QueryInfo::doWorkSendRequest()
{
    Q_D(QueryInfo);

    // Set the url

    QUrl url = d->MediaWiki.url();
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("format"),        QStringLiteral("xml"));
    query.addQueryItem(QStringLiteral("action"),        QStringLiteral("query"));
    query.addQueryItem(QStringLiteral("prop"),          QStringLiteral("info"));
    query.addQueryItem(QStringLiteral("inprop"),        QStringLiteral("protection|talkid|watched|subjectid|url|preload"));
    query.addQueryItem(QStringLiteral("intestactions"), QStringLiteral("read"));

    QMapIterator<QString, QString> it(d->requestParameter);

    while (it.hasNext())
    {
        it.next();
        query.addQueryItem(it.key(), it.value());
    }

    url.setQuery(query);

    // Set the request

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", d->MediaWiki.userAgent().toUtf8());
    QByteArray cookie = "";
    QList<QNetworkCookie> MediaWikiCookies = d->manager->cookieJar()->cookiesForUrl(d->MediaWiki.url());

    for (int i = 0 ; i < MediaWikiCookies.size() ; ++i)
    {
        cookie += MediaWikiCookies.at(i).toRawForm(QNetworkCookie::NameAndValueOnly);
        cookie += ';';
    }

    request.setRawHeader( "Cookie", cookie );

    // Send the request

    d->reply = d->manager->get(request);
    connectReply();

    connect(d->reply, SIGNAL(finished()),
            this, SLOT(doWorkProcessReply()));
}

void QueryInfo::doWorkProcessReply()
{
    Q_D(QueryInfo);

    disconnect(d->reply, SIGNAL(finished()),
               this, SLOT(doWorkProcessReply()));

    if (d->reply->error() == QNetworkReply::NoError)
    {
        // Replace & in &amp;

        QString content = QString::fromUtf8(d->reply->readAll());
        QRegularExpression regex(QStringLiteral("&(?!\\w+;)"));
        content.replace(regex, QStringLiteral("&amp;"));
        QXmlStreamReader reader(content);
        QVector<Protection> protect;

        while (!reader.atEnd() && !reader.hasError())
        {
            QXmlStreamReader::TokenType token = reader.readNext();
            QXmlStreamAttributes attrs = reader.attributes();

            if (token == QXmlStreamReader::StartElement)
            {
                if      (reader.name() == QLatin1String("page"))
                {
                    d->page.setPageId(attrs.value(QStringLiteral("pageid")).toString().toUInt());
                    d->page.setTitle(attrs.value(QStringLiteral("title")).toString());
                    d->page.setNs(attrs.value(QStringLiteral("ns")).toString().toUInt());
                    d->page.setTouched(QDateTime::fromString(attrs.value(QStringLiteral("touched")).toString(), QStringLiteral("yyyy'-'MM'-'dd'T'hh':'mm':'ss'Z'")));
                    d->page.setLastRevId(attrs.value(QStringLiteral("lastrevid")).toString().toUInt());
                    d->page.setCounter(attrs.value(QStringLiteral("counter")).toString().toUInt());
                    d->page.setLength(attrs.value(QStringLiteral("length")).toString().toUInt());
                    d->page.setStarttimestamp(QDateTime::fromString(attrs.value(QStringLiteral("starttimestamp")).toString(), QStringLiteral("yyyy'-'MM'-'dd'T'hh':'mm':'ss'Z'")));
                    d->page.setTalkid(attrs.value(QStringLiteral("talkid")).toString().toUInt());
                    d->page.setFullurl(QUrl(attrs.value(QStringLiteral("fullurl")).toString()));
                    d->page.setEditurl(QUrl(attrs.value(QStringLiteral("editurl")).toString()));
                    d->page.setPreload(attrs.value(QStringLiteral("preload")).toString());
                }
                else if (reader.name() == QLatin1String("protection"))
                {
                    protect.clear();
                }
                else if (reader.name() == QLatin1String("tokens"))
                {
                    d->page.setEditToken(attrs.value(QStringLiteral("csrftoken")).toString());
                }
                else if (reader.name() == QLatin1String("actions"))
                {
                    d->page.setReadable(attrs.value(QStringLiteral("read")).toString());
                }
                else if (reader.name() == QLatin1String("pr"))
                {
                    QString expiry(attrs.value(QStringLiteral("expiry")).toString());
                    QString level(attrs.value(QStringLiteral("level")).toString());
                    QString type(attrs.value(QStringLiteral("type")).toString());
                    QString source;

                    if      (!attrs.value(QStringLiteral("source")).toString().isEmpty())
                    {
                        source = attrs.value(QStringLiteral("source")).toString();
                    }
                    else if (!attrs.value(QStringLiteral("cascade")).toString().isEmpty())
                    {
                        source = attrs.value(QStringLiteral("cascade")).toString();
                    }

                    Protection p;
                    p.setExpiry(expiry);
                    p.setLevel(level);
                    p.setType(type);
                    p.setSource(source);
                    protect.push_back(p);
                }
            }
            else if (token == QXmlStreamReader::EndElement)
            {
                if (reader.name() == QLatin1String("page"))
                {
                    d->protections = protect;
                }
            }
        }
        if (!reader.hasError())
        {
            setError(KJob::NoError);
            Q_EMIT protection(protect);
            Q_EMIT page(d->page);
        }
        else
        {
            setError(Job::XmlError);
        }
    }
    else
    {
        setError(Job::NetworkError);
    }

    emitResult();
}

} // namespace MediaWiki
