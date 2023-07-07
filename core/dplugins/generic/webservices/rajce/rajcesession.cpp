/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * SPDX-FileCopyrightText: 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rajcesession.h"

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericRajcePlugin
{

class Q_DECL_HIDDEN RajceSession::Private
{
public:

    explicit Private()
    {
        maxWidth      = 0;
        maxHeight     = 0;
        imageQuality  = 0;
        lastErrorCode = 0;
        lastCommand   = Logout;
    }

    unsigned            maxWidth;
    unsigned            maxHeight;
    unsigned            imageQuality;
    unsigned            lastErrorCode;

    QString             sessionToken;
    QString             nickname;
    QString             username;
    QString             albumToken;
    QString             lastErrorMessage;

    QVector<RajceAlbum> albums;

    RajceCommandType    lastCommand;
};

RajceSession::RajceSession()
    : d(new Private)
{
}

RajceSession::~RajceSession()
{
    delete d;
}

RajceSession::RajceSession(const RajceSession& other)
    : d(other.d)
{
}

RajceSession& RajceSession::operator=(const RajceSession& other)
{
    *d = *other.d;

    return *this;
}

QString& RajceSession::sessionToken()
{
    return d->sessionToken;
}

QString const& RajceSession::sessionToken() const
{
    return d->sessionToken;
}

QString& RajceSession::nickname()
{
    return d->nickname;
}

QString const& RajceSession::nickname() const
{
    return d->nickname;
}

QString& RajceSession::username()
{
    return d->username;
}

QString const& RajceSession::username() const
{
    return d->username;
}

QString& RajceSession::openAlbumToken()
{
    return d->albumToken;
}

QString const& RajceSession::openAlbumToken() const
{
    return d->albumToken;
}

QString& RajceSession::lastErrorMessage()
{
    return d->lastErrorMessage;
}

QString const& RajceSession::lastErrorMessage() const
{
    return d->lastErrorMessage;
}

unsigned& RajceSession::maxWidth()
{
    return d->maxWidth;
}

unsigned RajceSession::maxWidth() const
{
    return d->maxWidth;
}

unsigned& RajceSession::maxHeight()
{
    return d->maxHeight;
}

unsigned RajceSession::maxHeight() const
{
    return d->maxHeight;
}

unsigned& RajceSession::imageQuality()
{
    return d->imageQuality;
}

unsigned RajceSession::imageQuality() const
{
    return d->imageQuality;
}

unsigned& RajceSession::lastErrorCode()
{
    return d->lastErrorCode;
}

unsigned RajceSession::lastErrorCode() const
{
    return d->lastErrorCode;
}

QVector<RajceAlbum>& RajceSession::albums()
{
    return d->albums;
}

const QVector<RajceAlbum>& RajceSession::albums() const
{
    return d->albums;
}

RajceCommandType RajceSession::lastCommand() const
{
    return d->lastCommand;
}

RajceCommandType& RajceSession::lastCommand()
{
    return d->lastCommand;
}

} // namespace DigikamGenericRajcePlugin

QDebug operator<<(QDebug d, const DigikamGenericRajcePlugin::RajceSession& s)
{
    QString     ss;
    QTextStream str(&ss);

    str << "SessionState[";
    str << "sessionToken='"       << s.sessionToken()     << "'";
    str << ", nickname='"         << s.nickname()         << "'";
    str << ", username='"         << s.username()         << "'";
    str << ", albumToken='"       << s.openAlbumToken()   << "'";
    str << ", lastErrorMessage='" << s.lastErrorMessage() << "'";
    str << ", lastErrorCode="     << s.lastErrorCode();
    str << ", maxWidth="          << s.maxWidth();
    str << ", maxHeight="         << s.maxHeight();
    str << ", imageQuality="      << s.imageQuality();
    str << ", albums=[";

    DigikamGenericRajcePlugin::RajceAlbum a;

    Q_FOREACH (a, s.albums())
    {
        str << a << ", ";
    }

    str << "]";

    d << *str.string();

    return d;
}
