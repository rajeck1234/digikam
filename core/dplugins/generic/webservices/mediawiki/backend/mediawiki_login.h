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
 * SPDX-FileCopyrightText: 2011      by Peter Potrowl <peter dot potrowl at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Manuel Campomanes <campomanes dot manuel at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_MEDIAWIKI_LOGIN_H
#define DIGIKAM_MEDIAWIKI_LOGIN_H

// Qt includes

#include <QString>
#include <QNetworkCookieJar>

// Local includes

#include "mediawiki_job.h"


namespace MediaWiki
{

class Iface;
class LoginPrivate;
/**
 * @brief Login job.
 *
 * Uses for log in a user.
 */
class Login : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Login)

public:

    enum
    {
        /**
         * @brief You didn't set the login parameter
         */
        LoginMissing = Job::UserDefinedError + 1,

        /**
         * @brief You provided an illegal username
         */
        IllegalUsername,

        /**
         * @brief The username you provided doesn't exist
         */
        UsernameNotExists,

        /**
        * @brief You didn't set the lgpassword parameter or you left it empty
        */
        PasswordMissing,

        /**
        * @brief The password you provided is incorrect
        */
        WrongPassword,

        /**
        * @brief Same as WrongPass, returned when an authentication plugin rather than Iface itself rejected the password
        */
        WrongPluginPassword,

        /**
        * @brief The wiki tried to automatically create a new account for you, but your IP address has been blocked from account creation
        */
        IPAddressBlocked,

        /**
        * @brief You've logged in too many times in a short time.
        */
        TooManyConnections,

        /**
        * @brief User is blocked
        */
        UserBlocked,

        /**
        * @brief Either you did not provide the login token or the sessionid cookie. Request again with the token and cookie given in this response
        */
        TokenNeeded
    };

public:

    /**
     * @brief Constructs a Login job.
     * @param MediaWiki the MediaWiki concerned by the job
     * @param login the QObject parent
     * @param password the QObject parent
     * @param parent the QObject parent
     */
    explicit Login(Iface& MediaWiki,
                   const QString& login,
                   const QString& password,
                   QObject* const parent = nullptr);

    /**
     * @brief Destroys the Login job.
     */
    ~Login()     override;

    /**
     * @brief Starts the job asynchronously.
     */
    void start() override;

private Q_SLOTS:

    /**
     * @brief Send a request to get the token and the cookie.
     */
    void doWorkSendRequest();

    /**
     * @brief Reads the xml
     * if the attribute value is equal to "NeedToken", try to log in the user
     * else if the attribute value is equal to "Success", the user is logged in
     * @param success true if the connection was completed successfully.
     */
    void doWorkProcessReply();
};

} // namespace MediaWiki

#endif // DIGIKAM_MEDIAWIKI_LOGIN_H
