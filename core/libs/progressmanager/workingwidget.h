/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-27
 * Description : Widget showing a throbber ("working" animation)
 *
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_WORKING_WIDGET_H
#define DIGIKAM_WORKING_WIDGET_H

// Qt includes

#include <QLabel>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT WorkingWidget : public QLabel
{
    Q_OBJECT

public:

    explicit WorkingWidget(QWidget* const parent = nullptr);
    ~WorkingWidget() override;

Q_SIGNALS:

    void animationStep();

public Q_SLOTS:

    void toggleTimer(bool turnOn = false);

private Q_SLOTS:

    void slotChangeImage();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_WORKING_WIDGET_H
