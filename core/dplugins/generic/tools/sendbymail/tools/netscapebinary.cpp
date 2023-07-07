/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect Netscape Messenger binary program
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "netscapebinary.h"

// KDE includes

#include <klocalizedstring.h>

namespace DigikamGenericSendByMailPlugin
{

NetscapeBinary::NetscapeBinary(QObject* const)
    : DBinaryIface(
                   QLatin1String("netscape"),
                   QLatin1String("Netscape Messenger"),
                   QLatin1String("https://en.wikipedia.org/wiki/Netscape_Mail_%26_Newsgroups"),
                   QLatin1String("SendByMail"),
                   QStringList(QLatin1String("-v")),
                   i18n("Netscape Mail Client.")
                  )
{
    setup();
}

NetscapeBinary::~NetscapeBinary()
{
}

} // namespace DigikamGenericSendByMailPlugin
