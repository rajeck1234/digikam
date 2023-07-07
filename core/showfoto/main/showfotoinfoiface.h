/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : interface to item information for Showfoto
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef SHOWFOTO_INFO_IFACE_H
#define SHOWFOTO_INFO_IFACE_H

// Local includes

#include "dmetainfoiface.h"

using namespace Digikam;

namespace ShowFoto
{

class ShowfotoInfoIface : public DMetaInfoIface
{
    Q_OBJECT

public:

    explicit ShowfotoInfoIface(QObject* const parent,
                               const QList<QUrl>& lst);
    ~ShowfotoInfoIface()               override;

    void openSetupPage(SetupPage page) override;
};

} // namespace ShowFoto

#endif // SHOWFOTO_INFO_IFACE_H
