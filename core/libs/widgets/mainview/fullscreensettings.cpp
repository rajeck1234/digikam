/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-04-29
 * Description : a full screen settings widget
 *
 * SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fullscreensettings.h"

// Qt includes

#include <QString>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dxmlguiwindow.h"

namespace Digikam
{

class Q_DECL_HIDDEN FullScreenSettings::Private
{
public:

    explicit Private()
      : options      (FS_NONE),
        hideToolBars (nullptr),
        hideThumbBar (nullptr),
        hideSideBars (nullptr),
        hideStatusBar(nullptr)
    {
    }

    int        options;

    QCheckBox* hideToolBars;
    QCheckBox* hideThumbBar;
    QCheckBox* hideSideBars;
    QCheckBox* hideStatusBar;
};

FullScreenSettings::FullScreenSettings(int options, QWidget* const parent)
    : QGroupBox(i18n("Full-screen Options"), parent),
      d(new Private)
{
    const int spacing        = qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                             QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));

    d->options               = options;
    QVBoxLayout* const vlay  = new QVBoxLayout(this);
    d->hideToolBars          = new QCheckBox(i18n("H&ide toolbars"),  this);
    d->hideToolBars->setWhatsThis(i18n("Hide all toolbars when window switch in full-screen mode."));

    d->hideThumbBar          = new QCheckBox(i18n("Hide &thumbbar"),  this);
    d->hideThumbBar->setWhatsThis(i18n("Hide thumbbar view when window switch in full-screen mode."));

    d->hideSideBars          = new QCheckBox(i18n("Hide &sidebars"),  this);
    d->hideSideBars->setWhatsThis(i18n("Hide all side-bars when window switch in full-screen mode."));

    d->hideStatusBar         = new QCheckBox(i18n("Hide st&atusbar"), this);
    d->hideStatusBar->setWhatsThis(i18n("Hide statusbar when window switch in full-screen mode."));

    vlay->addWidget(d->hideToolBars);
    vlay->addWidget(d->hideThumbBar);
    vlay->addWidget(d->hideSideBars);
    vlay->addWidget(d->hideStatusBar);
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(0);

    if (!(options & FS_TOOLBARS))
    {
        d->hideToolBars->hide();
    }

    if (!(options & FS_THUMBBAR))
    {
        d->hideThumbBar->hide();
    }

    if (!(options & FS_SIDEBARS))
    {
        d->hideSideBars->hide();
    }

    if (!(options & FS_STATUSBAR))
    {
        d->hideStatusBar->hide();
    }
}

FullScreenSettings::~FullScreenSettings()
{
    delete d;
}

void FullScreenSettings::readSettings(const KConfigGroup& group)
{
    if (d->options & FS_TOOLBARS)
    {
        d->hideToolBars->setChecked(group.readEntry(s_configFullScreenHideToolBarsEntry,   false));
    }

    if (d->options & FS_THUMBBAR)
    {
        d->hideThumbBar->setChecked(group.readEntry(s_configFullScreenHideThumbBarEntry,   true));
    }

    if (d->options & FS_SIDEBARS)
    {
        d->hideSideBars->setChecked(group.readEntry(s_configFullScreenHideSideBarsEntry,   false));
    }

    if (d->options & FS_STATUSBAR)
    {
        d->hideStatusBar->setChecked(group.readEntry(s_configFullScreenHideStatusBarEntry, false));
    }
}

void FullScreenSettings::saveSettings(KConfigGroup& group)
{
    if (d->options & FS_TOOLBARS)
    {
        group.writeEntry(s_configFullScreenHideToolBarsEntry,  d->hideToolBars->isChecked());
    }

    if (d->options & FS_THUMBBAR)
    {
        group.writeEntry(s_configFullScreenHideThumbBarEntry,  d->hideThumbBar->isChecked());
    }

    if (d->options & FS_SIDEBARS)
    {
        group.writeEntry(s_configFullScreenHideSideBarsEntry,  d->hideSideBars->isChecked());
    }

    if (d->options & FS_STATUSBAR)
    {
        group.writeEntry(s_configFullScreenHideStatusBarEntry, d->hideStatusBar->isChecked());
    }

    group.sync();
}

} // namespace Digikam
