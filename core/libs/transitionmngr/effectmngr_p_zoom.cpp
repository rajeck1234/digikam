/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-24
 * Description : video frame effects manager.
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "effectmngr_p.h"

namespace Digikam
{

int EffectMngr::Private::effectKenBurnsZoomIn(bool aInit)
{
    if (aInit)
    {
        eff_step = 0;
    }

    QRectF fRect(eff_image.rect());

    // This effect zoom in on the center of image from 100 to 80 percents.
    double nx    = eff_step * ((eff_image.width() - eff_image.width() * 0.8) / eff_imgFrames);
    double ny    = nx / ((double)eff_image.width() / (double)eff_image.height());
    fRect.setTopLeft(QPointF(nx, ny));
    fRect.setBottomRight(QPointF((double)eff_image.width()-nx, (double)eff_image.height()-ny));

    updateCurrentFrame(fRect);

    eff_step++;

    if (eff_step != eff_imgFrames)
    {
        return 15;
    }

    return -1;
}

int EffectMngr::Private::effectKenBurnsZoomOut(bool aInit)
{
    if (aInit)
    {
        eff_step = eff_imgFrames;
    }

    QRectF fRect(eff_image.rect());

    // This effect zoom out on the center of image from 80 to 100 percents.
    double nx    = eff_step * ((eff_image.width() - eff_image.width() * 0.8) / eff_imgFrames);
    double ny    = nx / ((double)eff_image.width() / (double)eff_image.height());
    fRect.setTopLeft(QPointF(nx, ny));
    fRect.setBottomRight(QPointF((double)eff_image.width()-nx, (double)eff_image.height()-ny));

    updateCurrentFrame(fRect);

    eff_step--;

    if (eff_step != 0)
    {
        return 15;
    }

    return -1;
}

} // namespace Digikam
