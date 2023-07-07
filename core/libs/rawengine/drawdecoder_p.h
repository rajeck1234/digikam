/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-10-09
 * Description : internal private container for DRawDecoder
 *
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DRAW_DECODER_PRIVATE_H
#define DIGIKAM_DRAW_DECODER_PRIVATE_H

#include "digikam_config.h"

// Qt includes

#include <QByteArray>

// Pragma directives to reduce warnings from LibRaw and LCMS2 headers
#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#   pragma clang diagnostic ignored "-Wundef"
#   pragma clang diagnostic ignored "-Wdeprecated-register"
#   pragma clang diagnostic ignored "-Wmacro-redefined"
#endif

// LibRaw includes

#include "libraw.h"

// Restore warnings
#if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "drawinfo.h"
#include "drawdecoder.h"

namespace Digikam
{

extern "C"
{
    int  s_progressCallbackForLibRaw(void* context, enum LibRaw_progress p, int iteration, int expected);
    void s_exifParserCallbackForLibRaw(void* context, int tag, int type, int len, unsigned int ord, void* ifp, INT64 base);
}

class Q_DECL_HIDDEN DRawDecoder::Private
{

public:

    explicit Private(DRawDecoder* const p);
    ~Private();

public:

    int    progressCallback(enum LibRaw_progress p, int iteration, int expected);
    void   exifParserCallback(int tag, int type, int len, unsigned int ord, void* ifp, INT64 base);

    void   setProgress(double value);
    double progressValue() const;

    bool   loadFromLibraw(const QString& filePath, QByteArray& imageData,
                          int& width, int& height, int& rgbmax);

public:

    static void createPPMHeader(QByteArray& imgData, libraw_processed_image_t* const img);

    static void fillIndentifyInfo(LibRaw* const raw, DRawInfo& identify);

    static bool loadEmbeddedPreview(QByteArray&, LibRaw* const raw);

    static bool loadHalfPreview(QImage&, LibRaw* const raw, bool rotate = true);

private:

    double       m_progress;

    DRawDecoder* m_parent;

    friend class DRawDecoder;
};

} // namespace Digikam

#endif // DIGIKAM_DRAW_DECODER_PRIVATE_H
