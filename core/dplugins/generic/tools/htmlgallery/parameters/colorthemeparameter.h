/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_COLOR_THEME_PARAMETER_H
#define DIGIKAM_COLOR_THEME_PARAMETER_H

// Local includes

#include "abstractthemeparameter.h"

namespace DigikamGenericHtmlGalleryPlugin
{

/**
 * A theme parameter to select a color.
 */
class ColorThemeParameter : public AbstractThemeParameter
{
public:

    QWidget* createWidget(QWidget* parent, const QString& value) const override;
    QString  valueFromWidget(QWidget*)                           const override;
};

} // namespace DigikamGenericHtmlGalleryPlugin

#endif // DIGIKAM_COLOR_THEME_PARAMETER_H
