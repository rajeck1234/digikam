/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-18
 * Description : slideshow end view
 *
 * SPDX-FileCopyrightText: 2014-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_SLIDE_END_PLUGIN_H
#define DIGIKAM_SLIDE_END_PLUGIN_H

// Qt includes

#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace DigikamGenericSlideShowPlugin
{

class SlideEnd : public QWidget
{
    Q_OBJECT

public:

    explicit SlideEnd(QWidget* const parent = nullptr);
    ~SlideEnd() override;
};

} // namespace DigikamGenericSlideShowPlugin

#endif // DIGIKAM_SLIDE_END_PLUGIN_H
