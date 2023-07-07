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

#include "dcolorselector.h"

// Qt includes

#include <QWidget>
#include <QPainter>
#include <QColorDialog>
#include <QStyleOptionButton>
#include <qdrawutil.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN DColorSelector::Private
{
public:

    explicit Private()
      : alpha(false)
    {
    }

    QColor color;
    bool   alpha;
};

DColorSelector::DColorSelector(QWidget* const parent)
    : QPushButton(parent),
      d(new Private)
{
    connect(this, SIGNAL(clicked()),
            this, SLOT(slotBtnClicked()));
}

DColorSelector::~DColorSelector()
{
    delete d;
}

void DColorSelector::setColor(const QColor& color)
{
    if (color.isValid())
    {
        d->color = color;
        update();
    }
}

QColor DColorSelector::color() const
{
    return d->color;
}

void DColorSelector::setAlphaChannelEnabled(bool b)
{
    d->alpha = b;
}

void DColorSelector::slotBtnClicked()
{
    QColor color = QColorDialog::getColor(d->color,
                                          this,
                                          QString(),
                                          d->alpha ? QColorDialog::ShowAlphaChannel
                                                   : QColorDialog::ColorDialogOptions());

    if (color.isValid())
    {
        setColor(color);
        Q_EMIT signalColorSelected(color);
    }
}

void DColorSelector::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QStyle* const style = QWidget::style();

    QStyleOptionButton opt;

    opt.initFrom(this);
    opt.state    |= isDown() ? QStyle::State_Sunken : QStyle::State_Raised;
    opt.features  = QStyleOptionButton::None;
    opt.icon      = QIcon();
    opt.text.clear();

    style->drawControl(QStyle::CE_PushButtonBevel, &opt, &painter, this);

    QRect labelRect = style->subElementRect(QStyle::SE_PushButtonContents, &opt, this);
    int shift       = style->pixelMetric(QStyle::PM_ButtonMargin, &opt, this) / 2;
    labelRect.adjust(shift, shift, -shift, -shift);
    int x, y, w, h;
    labelRect.getRect(&x, &y, &w, &h);

    if (isChecked() || isDown())
    {
        x += style->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &opt, this);
        y += style->pixelMetric(QStyle::PM_ButtonShiftVertical,   &opt, this);
    }

    QColor fillCol = isEnabled() ? d->color : palette().color(backgroundRole());
    qDrawShadePanel(&painter, x, y, w, h, palette(), true, 1, nullptr);

    if (fillCol.isValid())
    {
        const QRect rect(x + 1, y + 1, w - 2, h - 2);

        if (fillCol.alpha() < 255)
        {
            QPixmap chessboardPattern(16, 16);
            QPainter patternPainter(&chessboardPattern);
            patternPainter.fillRect(0, 0, 8, 8, Qt::black);
            patternPainter.fillRect(8, 8, 8, 8, Qt::black);
            patternPainter.fillRect(0, 8, 8, 8, Qt::white);
            patternPainter.fillRect(8, 0, 8, 8, Qt::white);
            patternPainter.end();
            painter.fillRect(rect, QBrush(chessboardPattern));
        }

        painter.fillRect(rect, fillCol);
    }

    if (hasFocus())
    {
        QRect focusRect = style->subElementRect(QStyle::SE_PushButtonFocusRect, &opt, this);
        QStyleOptionFocusRect focusOpt;
        focusOpt.initFrom(this);
        focusOpt.rect            = focusRect;
        focusOpt.backgroundColor = palette().window().color();
        style->drawPrimitive(QStyle::PE_FrameFocusRect, &focusOpt, &painter, this);
    }
}

} // namespace Digikam
