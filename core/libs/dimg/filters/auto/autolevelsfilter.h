/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : auto levels image filter.
 *
 * SPDX-FileCopyrightText: 2005-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_AUTO_LEVELS_FILTER_H
#define DIGIKAM_AUTO_LEVELS_FILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT AutoLevelsFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit AutoLevelsFilter(QObject* const parent = nullptr);
    AutoLevelsFilter(DImg* const orgImage,
                     const DImg* const refImage,
                     QObject* const parent = nullptr);
    ~AutoLevelsFilter()                                                       override;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:AutoLevelsFilter");
    }

    static QList<int>       SupportedVersions()
    {
        return QList<int>() << 1;
    }

    static int              CurrentVersion()
    {
        return 1;
    }

    static QString DisplayableName();

    QString         filterIdentifier()                                  const override
    {
        return FilterIdentifier();
    }

    FilterAction    filterAction()                                            override;

    void                    readParameters(const FilterAction& action)        override;

private:

    void filterImage()                                                        override;
    void autoLevelsCorrectionImage();

private:

    DImg m_refImage;
};

} // namespace Digikam

#endif // DIGIKAM_AUTO_LEVELS_FILTER_H
