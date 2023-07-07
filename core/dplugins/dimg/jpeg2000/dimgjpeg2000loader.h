/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-06-14
 * Description : A JPEG-2000 IO file for DImg framework
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Others Linux JPEG-2000 Loader implementation:
 *    https://github.com/ImageMagick/ImageMagick/blob/master/coders/jp2.c
 *    https://github.com/GNOME/gimp/blob/master/plug-ins/common/file-jp2-load.c
 *    https://invent.kde.org/kde/krita/tree/fc1d4dce1c12a1a663d02436cde15a77e067af2c/plugins/impex/jp2
 *    https://github.com/darktable-org/darktable/blob/master/src/imageio/format/j2k.c
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DIMG_JPEG_2000_LOADER_H
#define DIGIKAM_DIMG_JPEG_2000_LOADER_H

// Local includes

#include "dimg.h"
#include "dimgloader.h"

using namespace Digikam;

namespace DigikamJPEG2000DImgPlugin
{

class DImgJPEG2000Loader : public DImgLoader
{

public:

    explicit DImgJPEG2000Loader(DImg* const image);
    ~DImgJPEG2000Loader()                                                  override;

    bool load(const QString& filePath, DImgLoaderObserver* const observer) override;
    bool save(const QString& filePath, DImgLoaderObserver* const observer) override;

    bool hasAlpha()                                                  const override;
    bool sixteenBit()                                                const override;
    bool isReadOnly()                                                const override;

private:

    int  initJasper();
    void cleanupJasper();

private:

    bool m_sixteenBit;
    bool m_hasAlpha;
};

} // namespace DigikamJPEG2000DImgPlugin

#endif // DIGIKAM_DIMG_JPEG_2000_LOADER_H
