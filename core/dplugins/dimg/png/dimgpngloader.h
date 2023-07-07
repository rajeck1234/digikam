/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-11-01
 * Description : a PNG image loader for DImg framework.
 *
 * SPDX-FileCopyrightText: 2005-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DIMG_PNG_LOADER_H
#define DIGIKAM_DIMG_PNG_LOADER_H

// C++ includes

#include <cstdarg>

// Local includes

#include "dimg.h"
#include "dimgloader.h"

using namespace Digikam;

namespace DigikamPNGDImgPlugin
{

class DImgPNGLoader : public DImgLoader
{
public:

    explicit DImgPNGLoader(DImg* const image);
    ~DImgPNGLoader()                                                       override;

    bool load(const QString& filePath, DImgLoaderObserver* const observer) override;
    bool save(const QString& filePath, DImgLoaderObserver* const observer) override;

    bool hasAlpha()                                                  const override;
    bool sixteenBit()                                                const override;
    bool isReadOnly()                                                const override;

private:

    bool m_sixteenBit;
    bool m_hasAlpha;
};

} // namespace DigikamPNGDImgPlugin

#endif // DIGIKAM_DIMG_PNG_LOADER_H
