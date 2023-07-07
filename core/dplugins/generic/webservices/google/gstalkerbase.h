/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-21
 * Description : a tool to export items to Google web services
 *
 * SPDX-FileCopyrightText: 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * SPDX-FileCopyrightText: 2015-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_GS_TALKER_BASE_H
#define DIGIKAM_GS_TALKER_BASE_H

// Qt includes

#include <QUrl>
#include <QString>
#include <QObject>
#include <QStringList>
#include <QNetworkReply>
#include <QOAuth2AuthorizationCodeFlow>

namespace DigikamGenericGoogleServicesPlugin
{

class GSTalkerBase : public QObject
{
    Q_OBJECT

public:

    explicit GSTalkerBase(QObject* const parent, const QStringList& scope, const QString& serviceName);
    ~GSTalkerBase() override;

public:

    void link();
    void unlink();
    void doOAuth();
    bool authenticated() const;

Q_SIGNALS:

    void signalBusy(bool val);
    void signalLinkingSucceeded();
    void signalAccessTokenObtained();
    void signalAuthenticationRefused();

private Q_SLOTS:

    void slotLinkingFailed();
    void slotLinkingSucceeded();
    void slotOpenBrowser(const QUrl& url);
    void slotTokenChanged(const QString& token);

private:

    void parseResponseGetLoggedInUser(const QByteArray& data);

protected:

    QStringList                   m_scope;
    QString                       m_accessToken;
    QString                       m_bearerAccessToken;

    QString                       m_serviceName;

    QNetworkReply*                m_reply;
    QOAuth2AuthorizationCodeFlow* m_service;

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericGoogleServicesPlugin

#endif // DIGIKAM_GS_TALKER_BASE_H
