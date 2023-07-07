/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-18
 * Description : slideshow error view
 *
 * SPDX-FileCopyrightText: 2014-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "slideerror.h"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QPalette>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

namespace DigikamGenericSlideShowPlugin
{

class Q_DECL_HIDDEN SlideError::Private
{

public:

    explicit Private()
      : errorMsg(nullptr)
    {
    }

    QLabel* errorMsg;
};

SlideError::SlideError(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setMouseTracking(true);
    setAutoFillBackground(true);

    const int spacing       = qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                                   QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));

    QPalette palette;
    palette.setColor(backgroundRole(), Qt::black);
    setPalette(palette);

    d->errorMsg             = new QLabel(this);
    d->errorMsg->setAlignment(Qt::AlignCenter);

    QGridLayout* const grid = new QGridLayout(this);
    grid->addWidget(d->errorMsg, 1, 0, 1, 3 );
    grid->setColumnStretch(0, 10);
    grid->setColumnStretch(2, 10);
    grid->setRowStretch(0, 10);
    grid->setRowStretch(2, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);
}

SlideError::~SlideError()
{
    delete d;
}

void SlideError::setCurrentUrl(const QUrl& url)
{
    d->errorMsg->setText(i18n("An error has occurred to show item\n%1", url.fileName()));
}

} // namespace DigikamGenericSlideShowPlugin
