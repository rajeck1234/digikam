/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-20
 * Description : color selector widget
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DCOLOR_SELECTOR_H
#define DIGIKAM_DCOLOR_SELECTOR_H

// Qt includes

#include <QColor>
#include <QPushButton>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/**
 * A widget to chosse a color from a palette.
 */
class DIGIKAM_EXPORT DColorSelector : public QPushButton
{
    Q_OBJECT

public:

    explicit DColorSelector(QWidget* const parent = nullptr);
    ~DColorSelector() override;

    void setColor(const QColor& color);
    QColor color() const;

    void setAlphaChannelEnabled(bool);

Q_SIGNALS:

    void signalColorSelected(const QColor&);

private Q_SLOTS:

    void slotBtnClicked();

private:

    void paintEvent(QPaintEvent*) override;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_DCOLOR_SELECTOR_H
