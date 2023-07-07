/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : A QImage loader for DImg framework.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2021 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DIMG_QIMAGE_LOADER_H
#define DIGIKAM_DIMG_QIMAGE_LOADER_H

// Local includes

#include "dimg.h"
#include "dimgloader.h"
#include "digikam_config.h"

using namespace Digikam;

namespace DigikamQImageDImgPlugin
{

class DImgQImageLoader : public DImgLoader
{
public:

    explicit DImgQImageLoader(DImg* const image);
    ~DImgQImageLoader()                                                    override;

    bool load(const QString& filePath, DImgLoaderObserver* const observer) override;
    bool save(const QString& filePath, DImgLoaderObserver* const observer) override;

    bool hasAlpha()                                                  const override;
    bool sixteenBit()                                                const override;
    bool isReadOnly()                                                const override;

private:

    bool m_hasAlpha;
    bool m_sixteenBit;
};

} // namespace DigikamQImageDImgPlugin

#endif // DIGIKAM_DIMG_QIMAGE_LOADER_H
