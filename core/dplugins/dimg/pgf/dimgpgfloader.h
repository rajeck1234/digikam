/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-03
 * Description : A PGF IO file for DImg framework
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This implementation use LibPGF API <https://www.libpgf.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DIMG_PGF_LOADER_H
#define DIGIKAM_DIMG_PGF_LOADER_H

// Local includes

#include "dimg.h"
#include "dimgloader.h"

using namespace Digikam;

namespace Digikam
{

class DImgPGFLoader : public DImgLoader
{

public:

    explicit DImgPGFLoader(DImg* const image);
    ~DImgPGFLoader()                                                       override;

    bool load(const QString& filePath, DImgLoaderObserver* const observer) override;
    bool save(const QString& filePath, DImgLoaderObserver* const observer) override;

    bool hasAlpha()                                                  const override;
    bool sixteenBit()                                                const override;
    bool isReadOnly()                                                const override;

private:

    bool progressCallback(double percent, bool escapeAllowed);

    static bool CallbackForLibPGF(double percent,
                                  bool escapeAllowed,
                                  void* data);

private:

    bool                m_sixteenBit;
    bool                m_hasAlpha;

    DImgLoaderObserver* m_observer;
};

} // namespace Digikam

#endif // DIGIKAM_DIMG_PGF_LOADER_H
