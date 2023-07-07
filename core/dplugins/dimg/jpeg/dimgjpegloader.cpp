/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : A JPEG IO file for DImg framework
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#define XMD_H

#include "dimgjpegloader.h"

// C ANSI includes

extern "C"
{
#include "iccjpeg.h"
}

// Qt includes

#include <QFile>
#include <QByteArray>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "dimgloaderobserver.h"

#ifdef Q_OS_WIN
#   include "jpegwin.h"
#endif

namespace DigikamJPEGDImgPlugin
{

// To manage Errors/Warnings handling provide by libjpeg

void DImgJPEGLoader::dimg_jpeg_error_exit(j_common_ptr cinfo)
{
    dimg_jpeg_error_mgr* const myerr = static_cast<dimg_jpeg_error_mgr*>(cinfo->err);

    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

    qCWarning(DIGIKAM_DIMG_LOG_JPEG) << buffer;

#ifdef __MINGW32__  // krazy:exclude=cpp

    __builtin_longjmp(myerr->setjmp_buffer, 1);

#else

    longjmp(myerr->setjmp_buffer, 1);

#endif

}

void DImgJPEGLoader::dimg_jpeg_emit_message(j_common_ptr cinfo, int msg_level)
{
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

    qCDebug(DIGIKAM_DIMG_LOG_JPEG) << buffer << " (" << msg_level << ")";
}

void DImgJPEGLoader::dimg_jpeg_output_message(j_common_ptr cinfo)
{
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);

    qCDebug(DIGIKAM_DIMG_LOG_JPEG) << buffer;
}

DImgJPEGLoader::DImgJPEGLoader(DImg* const image)
    : DImgLoader(image)
{
}

DImgJPEGLoader::~DImgJPEGLoader()
{
}

bool DImgJPEGLoader::hasAlpha() const
{
    return false;
}

bool DImgJPEGLoader::sixteenBit() const
{
    return false;
}

bool DImgJPEGLoader::isReadOnly() const
{
    return false;
}

} // namespace DigikamJPEGDImgPlugin
