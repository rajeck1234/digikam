/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-09-27
 * Description : Tool tip for Showfoto stack view item.
 *
 * SPDX-FileCopyrightText: 2021-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef SHOWFOTO_STACK_VIEW_TOOL_TIP_H
#define SHOWFOTO_STACK_VIEW_TOOL_TIP_H

// Qt includes

#include <QString>
#include <QRect>
#include <QModelIndex>

// Local includes

#include "ditemtooltip.h"
#include "showfotoiteminfo.h"

using namespace Digikam;

namespace ShowFoto
{

class ShowfotoStackViewList;

class ShowfotoStackViewToolTip : public DItemToolTip
{
    Q_OBJECT

public:

    explicit ShowfotoStackViewToolTip(ShowfotoStackViewList* const view);
    ~ShowfotoStackViewToolTip()          override;

    void setIndex(const QModelIndex& index);

private:

    QRect   repositionRect()              override;
    QString tipContents()                 override;

private:

    class Private;
    Private* const d;
};

} // namespace ShowFoto

#endif // SHOWFOTO_STACK_VIEW_TOOL_TIP_H
