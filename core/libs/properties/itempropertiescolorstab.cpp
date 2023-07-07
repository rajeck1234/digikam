/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : a tab to display item colors information
 *
 * SPDX-FileCopyrightText: 2004-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itempropertiescolorstab.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QButtonGroup>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QScrollArea>
#include <QToolButton>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dimg.h"
#include "imagehistogram.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "colorgradientwidget.h"
#include "sharedloadsavethread.h"
#include "iccprofilewidget.h"
#include "cietonguewidget.h"
#include "itempropertiestxtlabel.h"
#include "digikam_globals.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemPropertiesColorsTab::Private
{
public:

    enum MetadataTab
    {
        HISTOGRAM=0,
        ICCPROFILE
    };

public:

    explicit Private()
      : minInterv           (nullptr),
        maxInterv           (nullptr),
        labelMeanValue      (nullptr),
        labelPixelsValue    (nullptr),
        labelStdDevValue    (nullptr),
        labelCountValue     (nullptr),
        labelMedianValue    (nullptr),
        labelPercentileValue(nullptr),
        labelColorDepth     (nullptr),
        labelAlphaChannel   (nullptr),
        labelImageRegion    (nullptr),
        iccProfileWidget    (nullptr),
        imageLoaderThread   (nullptr),
        histogramBox        (nullptr),
        redHistogram        (nullptr),
        greenHistogram      (nullptr),
        blueHistogram       (nullptr)
    {
    }

public:

    QSpinBox*             minInterv;
    QSpinBox*             maxInterv;

    DTextLabelValue*      labelMeanValue;
    DTextLabelValue*      labelPixelsValue;
    DTextLabelValue*      labelStdDevValue;
    DTextLabelValue*      labelCountValue;
    DTextLabelValue*      labelMedianValue;
    DTextLabelValue*      labelPercentileValue;
    DTextLabelValue*      labelColorDepth;
    DTextLabelValue*      labelAlphaChannel;
    DTextLabelValue*      labelImageRegion;

    QString               currentFilePath;
    LoadingDescription    currentLoadingDescription;

    QRect                 selectionArea;

    IccProfile            embeddedProfile;

    DImg                  image;
    DImg                  imageSelection;

    ICCProfileWidget*     iccProfileWidget;
    SharedLoadSaveThread* imageLoaderThread;

    HistogramBox*         histogramBox;
    HistogramWidget*      redHistogram;
    HistogramWidget*      greenHistogram;
    HistogramWidget*      blueHistogram;
};

ItemPropertiesColorsTab::ItemPropertiesColorsTab(QWidget* const parent)
    : QTabWidget(parent),
      d         (new Private)
{
    // Histogram tab area -----------------------------------------------------

    QScrollArea* const sv        = new QScrollArea(this);
    sv->setFrameStyle(QFrame::NoFrame);
    sv->setWidgetResizable(true);

    QWidget* const histogramPage = new QWidget(sv->viewport());
    QGridLayout* const topLayout = new QGridLayout(histogramPage);
    sv->setWidget(histogramPage);

    // -------------------------------------------------------------

    DVBox* const histoBox = new DVBox(histogramPage);
    d->histogramBox       = new HistogramBox(histoBox, LRGBAC, true);
    d->histogramBox->setStatisticsVisible(false);

    QLabel* const space   = new QLabel(histoBox);
    space->setFixedHeight(1);

    // -------------------------------------------------------------

    QHBoxLayout* const hlay3 = new QHBoxLayout();
    QLabel* const label3     = new QLabel(i18nc("@label: color properties", "Range:"), histogramPage);
    label3->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    d->minInterv = new QSpinBox(histogramPage);
    d->minInterv->setRange(0, 255);
    d->minInterv->setSingleStep(1);
    d->minInterv->setValue(0);
    d->minInterv->setWhatsThis(i18nc("@info: color properties", "Select the minimal intensity "
                                     "value of the histogram selection here."));
    d->maxInterv = new QSpinBox(histogramPage);
    d->minInterv->setRange(0, 255);
    d->minInterv->setSingleStep(1);
    d->maxInterv->setValue(255);
    d->minInterv->setWhatsThis(i18nc("@info: color properties", "Select the maximal intensity value "
                                     "of the histogram selection here."));
    hlay3->addWidget(label3);
    hlay3->addWidget(d->minInterv);
    hlay3->addWidget(d->maxInterv);

    // -------------------------------------------------------------

    QGroupBox* const gbox         = new QGroupBox(i18nc("@title: color properties", "Statistics"), histogramPage);
    gbox->setWhatsThis(i18nc("@info: color properties", "Here you can see the statistical results calculated from the "
                             "selected histogram part. These values are available for all "
                             "channels."));
    QGridLayout* const grid       = new QGridLayout(gbox);

    DTextLabelName* const label5  = new DTextLabelName(i18nc("@label: color properties", "Pixels: "), gbox);
    d->labelPixelsValue           = new DTextLabelValue(QString(), gbox);

    DTextLabelName* const label7  = new DTextLabelName(i18nc("@label: color properties", "Count: "), gbox);
    d->labelCountValue            = new DTextLabelValue(QString(), gbox);

    DTextLabelName* const label4  = new DTextLabelName(i18nc("@label: color properties", "Mean: "), gbox);
    d->labelMeanValue             = new DTextLabelValue(QString(), gbox);

    DTextLabelName* const label6  = new DTextLabelName(i18nc("@label: color properties", "Std. deviation: "), gbox);
    d->labelStdDevValue           = new DTextLabelValue(QString(), gbox);

    DTextLabelName* const label8  = new DTextLabelName(i18nc("@label: color properties", "Median: "), gbox);
    d->labelMedianValue           = new DTextLabelValue(QString(), gbox);

    DTextLabelName* const label9  = new DTextLabelName(i18nc("@label: color properties", "Percentile: "), gbox);
    d->labelPercentileValue       = new DTextLabelValue(QString(), gbox);

    DTextLabelName* const label10 = new DTextLabelName(i18nc("@label: color properties", "Color depth: "), gbox);
    d->labelColorDepth            = new DTextLabelValue(QString(), gbox);

    DTextLabelName* const label11 = new DTextLabelName(i18nc("@label: color properties", "Alpha Channel: "), gbox);
    d->labelAlphaChannel          = new DTextLabelValue(QString(), gbox);

    DTextLabelName* const label12 = new DTextLabelName(i18nc("@label: color properties", "Source: "), gbox);
    d->labelImageRegion           = new DTextLabelValue(QString(), gbox);

    const int spacing             = qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                             QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));

    grid->addWidget(label5,                  0, 0, 1, 1);
    grid->addWidget(d->labelPixelsValue,     0, 1, 1, 1);
    grid->addWidget(label7,                  1, 0, 1, 1);
    grid->addWidget(d->labelCountValue,      1, 1, 1, 1);
    grid->addWidget(label4,                  2, 0, 1, 1);
    grid->addWidget(d->labelMeanValue,       2, 1, 1, 1);
    grid->addWidget(label6,                  3, 0, 1, 1);
    grid->addWidget(d->labelStdDevValue,     3, 1, 1, 1);
    grid->addWidget(label8,                  4, 0, 1, 1);
    grid->addWidget(d->labelMedianValue,     4, 1, 1, 1);
    grid->addWidget(label9,                  5, 0, 1, 1);
    grid->addWidget(d->labelPercentileValue, 5, 1, 1, 1);
    grid->addWidget(label10,                 6, 0, 1, 1);
    grid->addWidget(d->labelColorDepth,      6, 1, 1, 1);
    grid->addWidget(label11,                 7, 0, 1, 1);
    grid->addWidget(d->labelAlphaChannel,    7, 1, 1, 1);
    grid->addWidget(label12,                 8, 0, 1, 1);
    grid->addWidget(d->labelImageRegion,     8, 1, 1, 1);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(0);
    grid->setColumnStretch(0, 10);
    grid->setColumnStretch(1, 10);

    // -------------------------------------------------------------

    d->redHistogram   = new HistogramWidget(256, 100, histogramPage);
    d->greenHistogram = new HistogramWidget(256, 100, histogramPage);
    d->blueHistogram  = new HistogramWidget(256, 100, histogramPage);

    d->redHistogram->setChannelType(RedChannel);
    d->redHistogram->setStatisticsVisible(true);
    d->greenHistogram->setChannelType(GreenChannel);
    d->greenHistogram->setStatisticsVisible(true);
    d->blueHistogram->setChannelType(BlueChannel);
    d->blueHistogram->setStatisticsVisible(true);

    // -------------------------------------------------------------

    topLayout->addWidget(histoBox,          0, 0, 2, 4);
    topLayout->addLayout(hlay3,             2, 0, 1, 4);
    topLayout->addWidget(gbox,              3, 0, 1, 4);
    topLayout->addWidget(d->redHistogram,   4, 0, 1, 4);
    topLayout->addWidget(d->greenHistogram, 5, 0, 1, 4);
    topLayout->addWidget(d->blueHistogram,  6, 0, 1, 4);
    topLayout->setRowStretch(7, 10);
    topLayout->setColumnStretch(2, 10);
    topLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    topLayout->setSpacing(spacing);

    insertTab(Private::HISTOGRAM, sv, i18nc("@title: color properties", "Histogram"));

    // ICC Profiles tab area ---------------------------------------

    QScrollArea* const sv2 = new QScrollArea(this);
    sv2->setFrameStyle(QFrame::NoFrame);
    sv2->setWidgetResizable(true);

    d->iccProfileWidget    = new ICCProfileWidget(sv2->viewport());
    sv2->setWidget(d->iccProfileWidget);
    insertTab(Private::ICCPROFILE, sv2, i18nc("@title: color properties", "ICC profile"));

    // -------------------------------------------------------------
    // histogramBox connections

    connect(d->histogramBox->histogram(), SIGNAL(signalIntervalChanged(int,int)),
            this, SLOT(slotUpdateInterval(int,int)));

    connect(d->redHistogram, SIGNAL(signalIntervalChanged(int,int)),
            this, SLOT(slotUpdateIntervalFromRGB(int,int)));

    connect(d->greenHistogram, SIGNAL(signalIntervalChanged(int,int)),
            this, SLOT(slotUpdateIntervalFromRGB(int,int)));

    connect(d->blueHistogram, SIGNAL(signalIntervalChanged(int,int)),
            this, SLOT(slotUpdateIntervalFromRGB(int,int)));

    connect(d->histogramBox->histogram(), SIGNAL(signalMaximumValueChanged(int)),
            this, SLOT(slotUpdateIntervRange(int)));

    connect(d->histogramBox->histogram(), SIGNAL(signalHistogramComputationDone(bool)),
            this, SLOT(slotRefreshOptions()));

    connect(d->histogramBox->histogram(), SIGNAL(signalHistogramComputationFailed()),
            this, SLOT(slotHistogramComputationFailed()));

    connect(d->histogramBox, SIGNAL(signalChannelChanged(ChannelType)),
            this, SLOT(slotChannelChanged()));

    connect(d->histogramBox, SIGNAL(signalScaleChanged(HistogramScale)),
            this, SLOT(slotScaleChanged()));

    // -------------------------------------------------------------

    connect(d->minInterv, SIGNAL(valueChanged(int)),
            this, SLOT(slotMinValueChanged(int)));

    connect(d->maxInterv, SIGNAL(valueChanged(int)),
            this, SLOT(slotMaxValueChanged(int)));
}

ItemPropertiesColorsTab::~ItemPropertiesColorsTab()
{
    // If there is a currently histogram computation when dialog is closed,
    // stop it before the d->image data are deleted automatically!

    d->histogramBox->histogram()->stopHistogramComputation();
    d->redHistogram->stopHistogramComputation();
    d->greenHistogram->stopHistogramComputation();
    d->blueHistogram->stopHistogramComputation();

    if (d->imageLoaderThread)
    {
        delete d->imageLoaderThread;
    }

    delete d;
}

void ItemPropertiesColorsTab::readSettings(const KConfigGroup& group)
{
    setCurrentIndex(group.readEntry("ImagePropertiesColors Tab",                  (int)Private::HISTOGRAM));
    d->iccProfileWidget->setMode(group.readEntry("ICC Level",                     (int)ICCProfileWidget::CUSTOM));
    d->iccProfileWidget->setCurrentItemByKey(group.readEntry("Current ICC Item",  QString()));
    d->histogramBox->setChannel((ChannelType)group.readEntry("Histogram Channel", (int)LuminosityChannel));
    d->histogramBox->setScale((HistogramScale)group.readEntry("Histogram Scale",  (int)LogScaleHistogram));
}

void ItemPropertiesColorsTab::writeSettings(KConfigGroup& group)
{
    group.writeEntry("ImagePropertiesColors Tab", currentIndex());
    group.writeEntry("Histogram Channel",         (int)d->histogramBox->channel());
    group.writeEntry("Histogram Scale",           (int)d->histogramBox->scale());
    group.writeEntry("ICC Level",                 d->iccProfileWidget->getMode());
    group.writeEntry("Current ICC Item",          d->iccProfileWidget->getCurrentItemKey());
}

void ItemPropertiesColorsTab::setData(const QUrl& url, const QRect& selectionArea, DImg* const img)
{
    // We might be getting duplicate events from AlbumIconView,
    // which will cause all sorts of duplicate work.
    // More importantly, while the loading thread can handle this pretty well,
    // this will completely mess up the timing of progress info in the histogram widget.
    // So filter here, before the stopHistogramComputation!
    // But do not filter if current path is null, as it would not disable the widget on first run.

    if (!img && !d->currentFilePath.isNull() && (url.toLocalFile() == d->currentFilePath))
    {
        return;
    }

    // This is necessary to stop computation because d->image.bits() is currently used by
    // threaded histogram algorithm.

    d->histogramBox->histogram()->stopHistogramComputation();
    d->redHistogram->stopHistogramComputation();
    d->greenHistogram->stopHistogramComputation();
    d->blueHistogram->stopHistogramComputation();

    d->currentFilePath.clear();
    d->currentLoadingDescription = LoadingDescription();
    d->iccProfileWidget->loadFromURL(QUrl());

    // Clear information.

    d->labelMeanValue->setAdjustedText();
    d->labelPixelsValue->setAdjustedText();
    d->labelStdDevValue->setAdjustedText();
    d->labelCountValue->setAdjustedText();
    d->labelMedianValue->setAdjustedText();
    d->labelPercentileValue->setAdjustedText();
    d->labelColorDepth->setAdjustedText();
    d->labelAlphaChannel->setAdjustedText();

    if (url.isEmpty())
    {
        setEnabled(false);
        d->image.reset();
        return;
    }

    d->selectionArea = selectionArea;
    d->image.reset();
    setEnabled(true);

    if (!img)
    {
        loadImageFromUrl(url);
    }
    else
    {
        d->image = img->copy();

        if (!d->image.isNull())
        {
            getICCData();

            // If a selection area is done in Image Editor and if the current image is the same
            // in Image Editor, then compute too the histogram for this selection.

            if (d->selectionArea.isValid())
            {
                d->imageSelection = d->image.copy(d->selectionArea);
                d->histogramBox->histogram()->updateData(d->image, d->imageSelection);
                d->redHistogram->updateData(d->image, d->imageSelection);
                d->greenHistogram->updateData(d->image, d->imageSelection);
                d->blueHistogram->updateData(d->image, d->imageSelection);

                slotRenderingChanged(ImageSelectionHistogram);
                updateInformation();
            }
            else
            {
                d->histogramBox->histogram()->updateData(d->image);
                d->redHistogram->updateData(d->image);
                d->greenHistogram->updateData(d->image);
                d->blueHistogram->updateData(d->image);
                slotRenderingChanged(FullImageHistogram);
                updateInformation();
            }
        }
        else
        {
            d->histogramBox->histogram()->setLoadingFailed();
            d->redHistogram->setLoadingFailed();
            d->greenHistogram->setLoadingFailed();
            d->blueHistogram->setLoadingFailed();
            d->iccProfileWidget->setLoadingFailed();
            slotHistogramComputationFailed();
        }
    }
}

void ItemPropertiesColorsTab::loadImageFromUrl(const QUrl& url)
{
    // create thread on demand

    if (!d->imageLoaderThread)
    {
        d->imageLoaderThread = new SharedLoadSaveThread();

        connect(d->imageLoaderThread, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
                this, SLOT(slotLoadImageFromUrlComplete(LoadingDescription,DImg)));

        connect(d->imageLoaderThread, SIGNAL(signalMoreCompleteLoadingAvailable(LoadingDescription,LoadingDescription)),
                this, SLOT(slotMoreCompleteLoadingAvailable(LoadingDescription,LoadingDescription)));
    }

    LoadingDescription desc = LoadingDescription(url.toLocalFile());

    if (DImg::fileFormat(desc.filePath) == DImg::RAW)
    {
        // use raw settings optimized for speed

        DRawDecoding rawDecodingSettings = DRawDecoding();
        rawDecodingSettings.optimizeTimeLoading();
        desc                             = LoadingDescription(desc.filePath,
                                                              rawDecodingSettings,
                                                              LoadingDescription::RawDecodingTimeOptimized);
    }

    if (d->currentLoadingDescription.equalsOrBetterThan(desc))
    {
        return;
    }

    d->currentFilePath           = desc.filePath;
    d->currentLoadingDescription = desc;

    d->imageLoaderThread->load(d->currentLoadingDescription,
                               SharedLoadSaveThread::AccessModeRead,
                               SharedLoadSaveThread::LoadingPolicyFirstRemovePrevious);

    d->histogramBox->histogram()->setDataLoading();
    d->redHistogram->setDataLoading();
    d->greenHistogram->setDataLoading();
    d->blueHistogram->setDataLoading();
    d->iccProfileWidget->setDataLoading();
}

void ItemPropertiesColorsTab::slotLoadImageFromUrlComplete(const LoadingDescription& loadingDescription,
                                                           const DImg& img)
{
    // Discard any leftover messages from previous, possibly aborted loads

    if (!loadingDescription.equalsOrBetterThan(d->currentLoadingDescription))
    {
        return;
    }

    if (!img.isNull())
    {
        d->histogramBox->histogram()->updateData(img);
        d->redHistogram->updateData(img);
        d->greenHistogram->updateData(img);
        d->blueHistogram->updateData(img);

        // As a safety precaution, this must be changed only after updateData is called,
        // which stops computation because d->image.bits() is currently used by threaded histogram algorithm.

        d->image = img;
        updateInformation();
        getICCData();
    }
    else
    {
        d->histogramBox->histogram()->setLoadingFailed();
        d->redHistogram->setLoadingFailed();
        d->greenHistogram->setLoadingFailed();
        d->blueHistogram->setLoadingFailed();
        d->iccProfileWidget->setLoadingFailed();
        slotHistogramComputationFailed();
    }
}

void ItemPropertiesColorsTab::slotMoreCompleteLoadingAvailable(const LoadingDescription& oldLoadingDescription,
                                                               const LoadingDescription& newLoadingDescription)
{
    if ((oldLoadingDescription == d->currentLoadingDescription) &&
        newLoadingDescription.equalsOrBetterThan(d->currentLoadingDescription))
    {
        // Yes, we do want to stop our old time-optimized loading and chain to the current, more complete loading.
        // Even the time-optimized raw loading takes significant time, and we must avoid two Raw engine instances running
        // at a time.

        d->currentLoadingDescription = newLoadingDescription;
        d->imageLoaderThread->load(newLoadingDescription,
                                   SharedLoadSaveThread::AccessModeRead,
                                   SharedLoadSaveThread::LoadingPolicyFirstRemovePrevious);
    }
}

void ItemPropertiesColorsTab::setSelection(const QRect& selectionArea)
{
    if (selectionArea == d->selectionArea)
    {
        return;
    }

    // This is necessary to stop computation because d->image.bits() is currently used by
    // threaded histogram algorithm.

    d->histogramBox->histogram()->stopHistogramComputation();
    d->redHistogram->stopHistogramComputation();
    d->greenHistogram->stopHistogramComputation();
    d->blueHistogram->stopHistogramComputation();
    d->selectionArea = selectionArea;

    if (d->selectionArea.isValid())
    {
        d->imageSelection = d->image.copy(d->selectionArea);
        d->histogramBox->histogram()->updateSelectionData(d->imageSelection);
        d->redHistogram->updateSelectionData(d->imageSelection);
        d->greenHistogram->updateSelectionData(d->imageSelection);
        d->blueHistogram->updateSelectionData(d->imageSelection);
        slotRenderingChanged(ImageSelectionHistogram);
    }
    else
    {
        d->imageSelection.reset();
        slotRenderingChanged(FullImageHistogram);
    }
}

void ItemPropertiesColorsTab::slotRefreshOptions()
{
    slotChannelChanged();
    slotScaleChanged();

    if (d->selectionArea.isValid())
    {
        slotRenderingChanged(ImageSelectionHistogram);
    }
}

void ItemPropertiesColorsTab::slotHistogramComputationFailed()
{
    d->imageSelection.reset();
    d->image.reset();
}

void ItemPropertiesColorsTab::slotChannelChanged()
{
    updateStatistics();
}

void ItemPropertiesColorsTab::slotScaleChanged()
{
    HistogramScale scale = d->histogramBox->histogram()->scaleType();
    d->redHistogram->setScaleType(scale);
    d->greenHistogram->setScaleType(scale);
    d->blueHistogram->setScaleType(scale);
    updateStatistics();
}

void ItemPropertiesColorsTab::slotRenderingChanged(int rendering)
{
    d->histogramBox->histogram()->setRenderingType((HistogramRenderingType)rendering);
    d->redHistogram->setRenderingType((HistogramRenderingType)rendering);
    d->greenHistogram->setRenderingType((HistogramRenderingType)rendering);
    d->blueHistogram->setRenderingType((HistogramRenderingType)rendering);
    updateStatistics();
}

void ItemPropertiesColorsTab::slotMinValueChanged(int min)
{
    // Called when user changes values of spin box.
    // Communicate the change to histogram widget.

    // make the one control "push" the other

    if (min == (d->maxInterv->value() + 1))
    {
        d->maxInterv->setValue(min);
    }

    d->maxInterv->setMinimum(min-1);
    d->histogramBox->histogram()->slotMinValueChanged(min);
    d->redHistogram->slotMinValueChanged(min);
    d->greenHistogram->slotMinValueChanged(min);
    d->blueHistogram->slotMinValueChanged(min);
    updateStatistics();
}

void ItemPropertiesColorsTab::slotMaxValueChanged(int max)
{
    if (max == (d->minInterv->value() - 1))
    {
        d->minInterv->setValue(max);
    }

    d->minInterv->setMaximum(max+1);
    d->histogramBox->histogram()->slotMaxValueChanged(max);
    d->redHistogram->slotMaxValueChanged(max);
    d->greenHistogram->slotMaxValueChanged(max);
    d->blueHistogram->slotMaxValueChanged(max);
    updateStatistics();
}

void ItemPropertiesColorsTab::slotUpdateIntervalFromRGB(int min, int max)
{
    d->histogramBox->histogram()->slotMinValueChanged(min);
    d->histogramBox->histogram()->slotMaxValueChanged(max);
    slotUpdateInterval(min, max);
}

void ItemPropertiesColorsTab::slotUpdateInterval(int min, int max)
{
    // Called when value is set from within histogram widget.
    // Block signals to prevent slotMinValueChanged and
    // slotMaxValueChanged being called.

    d->minInterv->blockSignals(true);
    d->minInterv->setMaximum(max+1);
    d->minInterv->setValue(min);
    d->minInterv->blockSignals(false);

    d->maxInterv->blockSignals(true);
    d->maxInterv->setMinimum(min-1);
    d->maxInterv->setValue(max);
    d->maxInterv->blockSignals(false);

    d->redHistogram->slotMinValueChanged(min);
    d->redHistogram->slotMaxValueChanged(max);
    d->greenHistogram->slotMinValueChanged(min);
    d->greenHistogram->slotMaxValueChanged(max);
    d->blueHistogram->slotMinValueChanged(min);
    d->blueHistogram->slotMaxValueChanged(max);

    updateStatistics();
}

void ItemPropertiesColorsTab::slotUpdateIntervRange(int range)
{
    d->maxInterv->setMaximum(range);
}

void ItemPropertiesColorsTab::updateInformation()
{
    d->labelColorDepth->setAdjustedText(d->image.sixteenBit() ? i18nc("@label: color properties", "16 bits")
                                                              : i18nc("@label: color properties", "8 bits"));
    d->labelAlphaChannel->setAdjustedText(d->image.hasAlpha() ? i18nc("@label: color properties", "Yes")
                                                              : i18nc("@label: color properties", "No"));
}

void ItemPropertiesColorsTab::updateStatistics()
{
    ImageHistogram* const renderedHistogram = d->histogramBox->histogram()->currentHistogram();

    if (!renderedHistogram)
    {
        return;
    }

    QString value;
    int min                     = d->minInterv->value();
    int max                     = d->maxInterv->value();
    int channel                 = d->histogramBox->channel();
    HistogramRenderingType type = d->histogramBox->histogram()->renderingType();

    if (channel == ColorChannels)
    {
        channel = LuminosityChannel;
    }

    double mean       = renderedHistogram->getMean(channel, min, max);
    d->labelMeanValue->setAdjustedText(value.setNum(mean, 'f', 1));

    double pixels     = renderedHistogram->getPixels();
    d->labelPixelsValue->setAdjustedText(value.setNum((float)pixels, 'f', 0));

    double stddev     = renderedHistogram->getStdDev(channel, min, max);
    d->labelStdDevValue->setAdjustedText(value.setNum(stddev, 'f', 1));

    double counts     = renderedHistogram->getCount(channel, min, max);
    d->labelCountValue->setAdjustedText(value.setNum((float)counts, 'f', 0));

    double median     = renderedHistogram->getMedian(channel, min, max);
    d->labelMedianValue->setAdjustedText(value.setNum(median, 'f', 1));

    double percentile = (pixels > 0 ? (100.0 * counts / pixels) : 0.0);
    d->labelPercentileValue->setAdjustedText(value.setNum(percentile, 'f', 1));

    d->labelImageRegion->setAdjustedText((type == FullImageHistogram) ? i18nc("@label: color properties", "Full Image")
                                                                      : i18nc("@label: color properties", "Image Region"));
}

void ItemPropertiesColorsTab::getICCData()
{
    if      (DImg::fileFormat(d->currentFilePath) == DImg::RAW)
    {
        d->iccProfileWidget->setUncalibratedColor();
    }
    else if (!d->image.getIccProfile().isNull())
    {
        d->embeddedProfile = d->image.getIccProfile();
        d->iccProfileWidget->loadProfile(d->currentFilePath, d->embeddedProfile);
    }
    else
    {
        d->iccProfileWidget->setLoadingFailed();
    }
}

} // namespace Digikam
