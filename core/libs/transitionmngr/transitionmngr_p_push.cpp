/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-24
 * Description : images transition manager.
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "transitionmngr_p.h"

namespace Digikam
{

int TransitionMngr::Private::transitionPushL2R(bool aInit)
{
    if (aInit)
    {
        eff_fx = eff_outSize.width() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(eff_i,                     0, eff_inImage);
    bufferPainter.drawImage(eff_i-eff_outSize.width(), 0, eff_outImage);
    bufferPainter.end();

    eff_i = eff_i + lround(eff_fx);

    if (eff_i <= eff_outSize.width())
    {
        return 15;
    }

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::transitionPushR2L(bool aInit)
{
    if (aInit)
    {
        eff_fx = eff_outSize.width() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(eff_i,                     0, eff_inImage);
    bufferPainter.drawImage(eff_i+eff_outSize.width(), 0, eff_outImage);
    bufferPainter.end();

    eff_i = eff_i - lround(eff_fx);

    if (eff_i >= -eff_outSize.width())
    {
        return 15;
    }

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::transitionPushT2B(bool aInit)
{
    if (aInit)
    {
        eff_fy = eff_outSize.height() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(0, eff_i,                      eff_inImage);
    bufferPainter.drawImage(0, eff_i-eff_outSize.height(), eff_outImage);
    bufferPainter.end();

    eff_i = eff_i + lround(eff_fy);

    if (eff_i <= eff_outSize.height())
    {
        return 15;
    }

    eff_curFrame = eff_outImage;

    return -1;
}

int TransitionMngr::Private::transitionPushB2T(bool aInit)
{
    if (aInit)
    {
        eff_fy = eff_outSize.height() / 25.0;
        eff_i  = 0;
    }

    QPainter bufferPainter(&eff_curFrame);
    bufferPainter.drawImage(0, eff_i,                      eff_inImage);
    bufferPainter.drawImage(0, eff_i+eff_outSize.height(), eff_outImage);
    bufferPainter.end();

    eff_i = eff_i - lround(eff_fy);

    if (eff_i >= -eff_outSize.height())
    {
        return 15;
    }

    eff_curFrame = eff_outImage;

    return -1;
}

} // namespace Digikam
