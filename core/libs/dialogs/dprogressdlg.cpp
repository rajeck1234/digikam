/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-30-08
 * Description : a progress dialog for digiKam
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dprogressdlg.h"

// Qt includes

#include <QHeaderView>
#include <QStyle>
#include <QLabel>
#include <QImage>
#include <QGridLayout>
#include <QProgressBar>
#include <QTreeWidget>
#include <QStandardPaths>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// Local includes

#include "dexpanderbox.h"

namespace Digikam
{

class Q_DECL_HIDDEN DProgressDlg::Private
{
public:

    explicit Private()
      : logo(nullptr),
        title(nullptr),
        label(nullptr),
        actionPix(nullptr),
        actionLabel(nullptr),
        progress(nullptr),
        buttons(nullptr)
    {
    }

    QLabel*           logo;
    QLabel*           title;
    QLabel*           label;

    QLabel*           actionPix;
    DAdjustableLabel* actionLabel;

    QProgressBar*     progress;

    QDialogButtonBox* buttons;
};

DProgressDlg::DProgressDlg(QWidget* const parent, const QString& caption)
    : QDialog(parent),
      d(new Private)
{
    setModal(true);
    setWindowTitle(caption);

    d->buttons              = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Cancel)->setDefault(true);

    QWidget* const page     = new QWidget(this);
    QGridLayout* const grid = new QGridLayout(page);

    d->actionPix            = new QLabel(page);
    d->actionLabel          = new DAdjustableLabel(page);
    d->logo                 = new QLabel(page);
    d->progress             = new QProgressBar(page);
    d->title                = new QLabel(page);
    d->label                = new QLabel(page);
    d->actionPix->setFixedSize(QSize(32, 32));

    d->logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48)));

    grid->addWidget(d->logo,        0, 0, 3, 1);
    grid->addWidget(d->label,       0, 1, 1, 2);
    grid->addWidget(d->actionPix,   1, 1, 1, 1);
    grid->addWidget(d->actionLabel, 1, 2, 1, 1);
    grid->addWidget(d->progress,    2, 1, 1, 2);
    grid->addWidget(d->title,       3, 1, 1, 2);
    grid->setSpacing(qMin(style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                          style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing)));
    grid->setContentsMargins(QMargins());
    grid->setColumnStretch(2, 10);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(slotCancel()));

    adjustSize();
    reset();
}

DProgressDlg::~DProgressDlg()
{
    delete d;
}

void DProgressDlg::slotCancel()
{
    Q_EMIT signalCancelPressed();

    close();
}

void DProgressDlg::setButtonText(const QString& text)
{
    d->buttons->button(QDialogButtonBox::Cancel)->setText(text);
}

void DProgressDlg::addedAction(const QPixmap& itemPix, const QString& text)
{
    QPixmap pix = itemPix;

    if (pix.isNull())
    {
        pix = QIcon::fromTheme(QLatin1String("image-missing")).pixmap(32);
    }
    else
    {
        pix = pix.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    d->actionPix->setPixmap(pix);
    d->actionLabel->setAdjustedText(text);
}

void DProgressDlg::reset()
{
    d->progress->setValue(0);
}

void DProgressDlg::setMaximum(int max)
{
    d->progress->setMaximum(max);
}

void DProgressDlg::incrementMaximum(int added)
{
    d->progress->setMaximum(d->progress->maximum() + added);
}

void DProgressDlg::setValue(int value)
{
    d->progress->setValue(value);
}

void DProgressDlg::advance(int offset)
{
    d->progress->setValue(d->progress->value() + offset);
}

int DProgressDlg::value() const
{
    return d->progress->value();
}

void DProgressDlg::setLabel(const QString& text)
{
    d->label->setText(text);
}

void DProgressDlg::setTitle(const QString& text)
{
    d->title->setText(text);
}

} // namespace Digikam
