/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-17
 * Description : A TIFF IO file for DImg framework
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// C ANSI includes
extern "C"
{
#include <tiffvers.h>
}

// C++ includes

#include <cstdio>

// Qt includes

#include <QFile>
#include <QByteArray>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "dimgloaderobserver.h"
#include "dimgtiffloader.h"     //krazy:exclude=includes

namespace DigikamTIFFDImgPlugin
{

// To manage Errors/Warnings handling provide by libtiff

void DImgTIFFLoader::dimg_tiff_warning(const char* module, const char* format, va_list warnings)
{
    if (DIGIKAM_DIMG_LOG_TIFF().isDebugEnabled())
    {
        char message[4096];
        vsnprintf(message, 4096, format, warnings);
        qCDebug(DIGIKAM_DIMG_LOG_TIFF) << module <<  "::" <<  message;
    }
}

void DImgTIFFLoader::dimg_tiff_error(const char* module, const char* format, va_list errors)
{
    if (DIGIKAM_DIMG_LOG_TIFF().isDebugEnabled())
    {
        char message[4096];
        vsnprintf(message, 4096, format, errors);
        qCDebug(DIGIKAM_DIMG_LOG_TIFF) << module << "::" << message;
    }
}

DImgTIFFLoader::DImgTIFFLoader(DImg* const image)
    : DImgLoader(image)
{
    m_hasAlpha   = false;
    m_sixteenBit = false;
}

DImgTIFFLoader::~DImgTIFFLoader()
{
}

bool DImgTIFFLoader::hasAlpha() const
{
    return m_hasAlpha;
}

bool DImgTIFFLoader::sixteenBit() const
{
    return m_sixteenBit;
}

bool DImgTIFFLoader::isReadOnly() const
{
    return false;
}

} // namespace DigikamTIFFDImgPlugin
