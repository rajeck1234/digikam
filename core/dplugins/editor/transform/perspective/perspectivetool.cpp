/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-02-17
 * Description : a tool to change image perspective .
 *
 * SPDX-FileCopyrightText: 2005-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "perspectivetool.h"

// Qt includes

#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "perspectivewidget.h"
#include "dexpanderbox.h"

namespace DigikamEditorPerspectiveToolPlugin
{

class Q_DECL_HIDDEN PerspectiveTool::Private
{
public:

    explicit Private()
      : newWidthLabel           (nullptr),
        newHeightLabel          (nullptr),
        topLeftAngleLabel       (nullptr),
        topRightAngleLabel      (nullptr),
        bottomLeftAngleLabel    (nullptr),
        bottomRightAngleLabel   (nullptr),
        drawWhileMovingCheckBox (nullptr),
        drawGridCheckBox        (nullptr),
        inverseTransformation   (nullptr),
        previewWidget           (nullptr),
        gboxSettings            (nullptr)
    {
    }

    static const QString configGroupName;
    static const QString configDrawWhileMovingEntry;
    static const QString configDrawGridEntry;
    static const QString configInverseTransformationEntry;

    QLabel*              newWidthLabel;
    QLabel*              newHeightLabel;
    QLabel*              topLeftAngleLabel;
    QLabel*              topRightAngleLabel;
    QLabel*              bottomLeftAngleLabel;
    QLabel*              bottomRightAngleLabel;

    QCheckBox*           drawWhileMovingCheckBox;
    QCheckBox*           drawGridCheckBox;
    QCheckBox*           inverseTransformation;

    PerspectiveWidget*   previewWidget;
    EditorToolSettings*  gboxSettings;
};

const QString PerspectiveTool::Private::configGroupName(QLatin1String("perspective Tool"));
const QString PerspectiveTool::Private::configDrawWhileMovingEntry(QLatin1String("Draw While Moving"));
const QString PerspectiveTool::Private::configDrawGridEntry(QLatin1String("Draw Grid"));
const QString PerspectiveTool::Private::configInverseTransformationEntry(QLatin1String("Inverse Transformation"));

// --------------------------------------------------------

PerspectiveTool::PerspectiveTool(QObject* const parent)
    : EditorTool(parent),
      d         (new Private)
{
    setObjectName(QLatin1String("perspective"));

    // -------------------------------------------------------------

    QFrame* const frame  = new QFrame(nullptr);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* const l = new QVBoxLayout(frame);
    d->previewWidget     = new PerspectiveWidget(525, 350, frame);
    l->addWidget(d->previewWidget);
    d->previewWidget->setWhatsThis(i18n("This is the perspective transformation operation preview. "
                                        "You can use the mouse for dragging the corner to adjust the "
                                        "perspective transformation area."));
    setToolView(frame);

    // -------------------------------------------------------------

    QString temp;
    ImageIface iface;

    d->gboxSettings      = new EditorToolSettings(nullptr);
    d->gboxSettings->setTools(EditorToolSettings::ColorGuide);

    // -------------------------------------------------------------

    QLabel* const label1 = new QLabel(i18n("New width:"));
    d->newWidthLabel     = new QLabel(temp.setNum( iface.originalSize().width()) + i18n(" px"));
    d->newWidthLabel->setAlignment( Qt::AlignBottom | Qt::AlignRight );

    QLabel* const label2 = new QLabel(i18n("New height:"));
    d->newHeightLabel    = new QLabel(temp.setNum( iface.originalSize().height()) + i18n(" px"));
    d->newHeightLabel->setAlignment( Qt::AlignBottom | Qt::AlignRight );

    // -------------------------------------------------------------

    DLineWidget* const line  = new DLineWidget (Qt::Horizontal);
    QLabel* const angleLabel = new QLabel(i18n("Angles (in degrees):"));
    QLabel* const label3     = new QLabel(i18n("  Top left:"));
    d->topLeftAngleLabel     = new QLabel;
    QLabel* const label4     = new QLabel(i18n("  Top right:"));
    d->topRightAngleLabel    = new QLabel;
    QLabel* const label5     = new QLabel(i18n("  Bottom left:"));
    d->bottomLeftAngleLabel  = new QLabel;
    QLabel* const label6     = new QLabel(i18n("  Bottom right:"));
    d->bottomRightAngleLabel = new QLabel;

    // -------------------------------------------------------------

    DLineWidget* const line2   = new DLineWidget (Qt::Horizontal);
    d->drawWhileMovingCheckBox = new QCheckBox(i18n("Draw preview while moving"));
    d->drawGridCheckBox        = new QCheckBox(i18n("Draw grid"));
    d->inverseTransformation   = new QCheckBox(i18n("Inverse transformation"));

    // -------------------------------------------------------------

    const int spacing = d->gboxSettings->spacingHint();

    QGridLayout* const grid = new QGridLayout;
    grid->addWidget(label1,                       0, 0, 1, 1);
    grid->addWidget(d->newWidthLabel,             0, 1, 1, 2);
    grid->addWidget(label2,                       1, 0, 1, 1);
    grid->addWidget(d->newHeightLabel,            1, 1, 1, 2);
    grid->addWidget(line,                         2, 0, 1, 3);
    grid->addWidget(angleLabel,                   3, 0, 1, 3);
    grid->addWidget(label3,                       4, 0, 1, 1);
    grid->addWidget(d->topLeftAngleLabel,         4, 1, 1, 2);
    grid->addWidget(label4,                       5, 0, 1, 1);
    grid->addWidget(d->topRightAngleLabel,        5, 1, 1, 2);
    grid->addWidget(label5,                       6, 0, 1, 1);
    grid->addWidget(d->bottomLeftAngleLabel,      6, 1, 1, 2);
    grid->addWidget(label6,                       7, 0, 1, 1);
    grid->addWidget(d->bottomRightAngleLabel,     7, 1, 1, 2);
    grid->addWidget(line2,                        8, 0, 1, 3);
    grid->addWidget(d->drawWhileMovingCheckBox,   9, 0, 1, 3);
    grid->addWidget(d->drawGridCheckBox,         10, 0, 1, 3);
    grid->addWidget(d->inverseTransformation,    11, 0, 1, 3);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(12, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);
    d->gboxSettings->plainPage()->setLayout(grid);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(signalPerspectiveChanged(QRect,float,float,float,float,bool)),
            this, SLOT(slotUpdateInfo(QRect,float,float,float,float,bool)));

    connect(d->drawWhileMovingCheckBox, SIGNAL(toggled(bool)),
            d->previewWidget, SLOT(slotToggleDrawWhileMoving(bool)));

    connect(d->drawGridCheckBox, SIGNAL(toggled(bool)),
            d->previewWidget, SLOT(slotToggleDrawGrid(bool)));

    connect(d->inverseTransformation, SIGNAL(toggled(bool)),
            this, SLOT(slotInverseTransformationChanged(bool)));

    connect(d->gboxSettings, SIGNAL(signalColorGuideChanged()),
            this, SLOT(slotColorGuideChanged()));
}

PerspectiveTool::~PerspectiveTool()
{
    delete d;
}

void PerspectiveTool::slotColorGuideChanged()
{
    d->previewWidget->slotChangeGuideColor(d->gboxSettings->guideColor());
    d->previewWidget->slotChangeGuideSize(d->gboxSettings->guideSize());
}

void PerspectiveTool::readSettings()
{
    QColor defaultGuideColor(Qt::red);
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->drawWhileMovingCheckBox->setChecked(group.readEntry( d->configDrawWhileMovingEntry,       true));
    d->drawGridCheckBox->setChecked(group.readEntry(        d->configDrawGridEntry,              false));
    d->inverseTransformation->setChecked(group.readEntry(   d->configInverseTransformationEntry, false));

    d->previewWidget->slotToggleDrawWhileMoving(d->drawWhileMovingCheckBox->isChecked());
    d->previewWidget->slotToggleDrawGrid(d->drawGridCheckBox->isChecked());
}

void PerspectiveTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configDrawWhileMovingEntry,       d->drawWhileMovingCheckBox->isChecked());
    group.writeEntry(d->configDrawGridEntry,              d->drawGridCheckBox->isChecked());
    group.writeEntry(d->configInverseTransformationEntry, d->inverseTransformation->isChecked());
    config->sync();
}

void PerspectiveTool::slotResetSettings()
{
    d->previewWidget->reset();
}

void PerspectiveTool::finalRendering()
{
    qApp->setOverrideCursor(Qt::WaitCursor);
    d->previewWidget->applyPerspectiveAdjustment();
    qApp->restoreOverrideCursor();
}

void PerspectiveTool::slotUpdateInfo(const QRect& newSize, float topLeftAngle, float topRightAngle,
                                     float bottomLeftAngle, float bottomRightAngle, bool valid)
{
    QString temp;
    d->newWidthLabel->setText(temp.setNum(newSize.width()) + i18n(" px"));
    d->newHeightLabel->setText(temp.setNum(newSize.height()) + i18n(" px"));

    d->topLeftAngleLabel->setText(temp.setNum(topLeftAngle, 'f', 1));
    d->topRightAngleLabel->setText(temp.setNum(topRightAngle, 'f', 1));
    d->bottomLeftAngleLabel->setText(temp.setNum(bottomLeftAngle, 'f', 1));
    d->bottomRightAngleLabel->setText(temp.setNum(bottomRightAngle, 'f', 1));

    d->gboxSettings->button(EditorToolSettings::Ok)->setEnabled(valid);
}

void PerspectiveTool::slotInverseTransformationChanged(bool b)
{
    d->drawWhileMovingCheckBox->setEnabled(!b);
    d->drawGridCheckBox->setEnabled(!b);
    d->previewWidget->slotInverseTransformationChanged(b);
}

void PerspectiveTool::setBackgroundColor(const QColor& bg)
{
    d->previewWidget->setBackgroundColor(bg);
}

} // namespace DigikamEditorPerspectiveToolPlugin
