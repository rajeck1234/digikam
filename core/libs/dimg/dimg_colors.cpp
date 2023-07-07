/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : digiKam 8/16 bits image management API.
 *               Color operations.
 *
 * SPDX-FileCopyrightText: 2005-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimg_p.h"

namespace Digikam
{

void DImg::convertToSixteenBit()
{
    convertDepth(64);
}

void DImg::convertToEightBit()
{
    convertDepth(32);
}

void DImg::convertToDepthOfImage(const DImg* const otherImage)
{
    if (otherImage->sixteenBit())
    {
        convertToSixteenBit();
    }
    else
    {
        convertToEightBit();
    }
}

void DImg::convertDepth(int depth)
{
    if (isNull())
    {
        return;
    }

    if ((depth != 32) && (depth != 64))
    {
        qCDebug(DIGIKAM_DIMG_LOG) << " : wrong color depth!";
        return;
    }

    if (((depth == 32) && !sixteenBit()) ||
        ((depth == 64) && sixteenBit()))
    {
        return;
    }

    if      (depth == 32)
    {
        // downgrading from 16 bit to 8 bit

        uchar*  data = new uchar[width()*height() * 4];
        uchar*  dptr = data;
        ushort* sptr = reinterpret_cast<ushort*>(bits());
        uint dim     = width() * height() * 4;

        for (uint i = 0 ; i < dim ; ++i)
        {
            *dptr++ = (*sptr++ * 256UL) / 65536UL;
        }

        delete [] m_priv->data;
        m_priv->data       = data;
        m_priv->sixteenBit = false;
    }
    else if (depth == 64)
    {
        // upgrading from 8 bit to 16 bit

        uchar*  data = new uchar[width()*height() * 8];
        ushort* dptr = reinterpret_cast<ushort*>(data);
        uchar*  sptr = bits();

        // use default seed of the generator

        RandomNumberGenerator generator;
        ushort noise = 0;

        uint dim = width() * height() * 4;

        for (uint i = 0 ; i < dim ; ++i)
        {
            if (i % 4 < 3)
            {
                noise = generator.number(0, 255);
            }
            else
            {
                noise = 0;
            }

            *dptr++ = (*sptr++ * 65536ULL) / 256ULL + noise;
        }

        delete [] m_priv->data;
        m_priv->data       = data;
        m_priv->sixteenBit = true;
    }
}

void DImg::fill(const DColor& color)
{
    if (isNull())
    {
        return;
    }

    // caching

    uint dim = width() * height() * 4;

    if (sixteenBit())
    {
        unsigned short* imgData16 = reinterpret_cast<unsigned short*>(m_priv->data);
        unsigned short red        = (unsigned short)color.red();
        unsigned short green      = (unsigned short)color.green();
        unsigned short blue       = (unsigned short)color.blue();
        unsigned short alpha      = (unsigned short)color.alpha();

        for (uint i = 0 ; i < dim ; i += 4)
        {
            imgData16[i    ] = blue;
            imgData16[i + 1] = green;
            imgData16[i + 2] = red;
            imgData16[i + 3] = alpha;
        }
    }
    else
    {
        uchar* imgData = m_priv->data;
        uchar red      = (uchar)color.red();
        uchar green    = (uchar)color.green();
        uchar blue     = (uchar)color.blue();
        uchar alpha    = (uchar)color.alpha();

        for (uint i = 0 ; i < dim ; i += 4)
        {
            imgData[i    ] = blue;
            imgData[i + 1] = green;
            imgData[i + 2] = red;
            imgData[i + 3] = alpha;
        }
    }
}

void DImg::removeAlphaChannel()
{
    removeAlphaChannel(DColor(0xFF, 0xFF, 0xFF, 0xFF, false));
}

void DImg::removeAlphaChannel(const DColor& destColor)
{
    if (isNull() || !hasAlpha())
    {
        return;
    }

    bitBlendImageOnColor(destColor);

    // unsure if alpha value is always 0xFF now

    m_priv->alpha = false;
}

} // namespace Digikam
