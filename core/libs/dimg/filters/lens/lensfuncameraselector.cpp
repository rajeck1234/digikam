/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * SPDX-FileCopyrightText: 2008      by Adrian Schroeter <adrian at suse dot de>
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "lensfuncameraselector.h"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QWidget>
#include <QApplication>
#include <QStyle>
#include <QVariant>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "squeezedcombobox.h"
#include "dnuminput.h"
#include "dexpanderbox.h"

// Disable deprecated API from Lensfun.
#if defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace Digikam
{

class Q_DECL_HIDDEN LensFunCameraSelector::Private
{
public:

    explicit Private()
        : passiveMetadataUsage  (false),
          metadataUsage         (nullptr),
          metadataResult        (nullptr),
          makeLabel             (nullptr),
          modelLabel            (nullptr),
          lensLabel             (nullptr),
          focalLabel            (nullptr),
          aperLabel             (nullptr),
          distLabel             (nullptr),
          configUseMetadata     (QLatin1String("UseMetadata")),
          configCameraModel     (QLatin1String("CameraModel")),
          configCameraMake      (QLatin1String("CameraMake")),
          configLensModel       (QLatin1String("LensModel")),
          configSubjectDistance (QLatin1String("SubjectDistance")),
          configFocalLength     (QLatin1String("FocalLength")),
          configCropFactor      (QLatin1String("CropFactor")),
          configAperture        (QLatin1String("Aperture")),
          redStyle              (QLatin1String("QLabel {color: red;}")),
          orangeStyle           (QLatin1String("QLabel {color: orange;}")),
          greenStyle            (QLatin1String("QLabel {color: green;}")),
          lensDescription       (nullptr),
          makeDescription       (nullptr),
          modelDescription      (nullptr),
          make                  (nullptr),
          model                 (nullptr),
          lens                  (nullptr),
          focal                 (nullptr),
          aperture              (nullptr),
          distance              (nullptr),
          metadata              (nullptr),
          iface                 (nullptr)
    {
    }

    bool                passiveMetadataUsage;

    QCheckBox*          metadataUsage;
    QLabel*             metadataResult;
    QLabel*             makeLabel;
    QLabel*             modelLabel;
    QLabel*             lensLabel;
    QLabel*             focalLabel;
    QLabel*             aperLabel;
    QLabel*             distLabel;

    const QString       configUseMetadata;
    const QString       configCameraModel;
    const QString       configCameraMake;
    const QString       configLensModel;
    const QString       configSubjectDistance;
    const QString       configFocalLength;
    const QString       configCropFactor;
    const QString       configAperture;
    const QString       redStyle;
    const QString       orangeStyle;
    const QString       greenStyle;

    DAdjustableLabel*   lensDescription;
    DAdjustableLabel*   makeDescription;
    DAdjustableLabel*   modelDescription;

    SqueezedComboBox*   make;
    SqueezedComboBox*   model;
    SqueezedComboBox*   lens;

    DDoubleNumInput*    focal;
    DDoubleNumInput*    aperture;
    DDoubleNumInput*    distance;

    DMetadata*          metadata;

    LensFunIface*       iface;
};

LensFunCameraSelector::LensFunCameraSelector(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    d->iface                = new LensFunIface();

    QGridLayout* const grid = new QGridLayout(this);
    DHBox* const hbox       = new DHBox(this);
    d->metadataUsage        = new QCheckBox(i18n("Use Metadata"), hbox);
    QLabel* const space     = new QLabel(hbox);
    d->metadataResult       = new QLabel(hbox);
    hbox->setStretchFactor(space, 10);

    DHBox* const hbox1      = new DHBox(this);
    d->makeLabel            = new QLabel(i18nc("camera make",  "Make:"),  hbox1);
    QLabel* const space1    = new QLabel(hbox1);
    d->makeDescription      = new DAdjustableLabel(hbox1);
    hbox1->setStretchFactor(space1, 10);
    d->makeDescription->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    d->makeDescription->setWhatsThis(i18n("This is the camera maker description string found in image meta-data. "
                                          "This one is used to query and find relevant camera device information from Lensfun database."));

    d->make                 = new SqueezedComboBox(this);
    d->make->setCurrentIndex(0);

    DHBox* const hbox2      = new DHBox(this);
    d->modelLabel           = new QLabel(i18nc("camera model", "Model:"), hbox2);
    QLabel* const space2    = new QLabel(hbox2);
    d->modelDescription     = new DAdjustableLabel(hbox2);
    hbox2->setStretchFactor(space2, 10);
    d->modelDescription->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    d->modelDescription->setWhatsThis(i18n("This is the camera model description string found in image meta-data. "
                                           "This one is used to query and found relevant camera device information from Lensfun database."));

    d->model                = new SqueezedComboBox(this);
    d->model->setCurrentIndex(0);

    DHBox* const hbox3      = new DHBox(this);
    d->lensLabel            = new QLabel(i18nc("camera lens",  "Lens:"),  hbox3);
    QLabel* const space3    = new QLabel(hbox3);
    d->lensDescription      = new DAdjustableLabel(hbox3);

    // Workaround for layout problem with long lens names.

    d->lensDescription->setAdjustedText(QString(40, QLatin1Char(' ')));
    d->lensDescription->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    d->lensDescription->setWhatsThis(i18n("This is the lens description string found in image meta-data. "
                                          "This one is used to query and found relevant lens information from Lensfun database."));
    hbox3->setStretchFactor(space3, 10);

    d->lens                 = new SqueezedComboBox(this);
    d->lens->setCurrentIndex(0);

    d->metadataUsage->setEnabled(false);
    d->metadataUsage->setCheckState(Qt::Unchecked);
    d->metadataUsage->setWhatsThis(i18n("Set this option to try to guess the right camera/lens settings "
                                        "from the image metadata (as Exif or XMP)."));

    d->focalLabel           = new QLabel(i18n("Focal Length (mm):"), this);
    d->aperLabel            = new QLabel(i18n("Aperture:"), this);
    d->distLabel            = new QLabel(i18n("Subject Distance (m):"), this);

    d->focal                = new DDoubleNumInput(this);
    d->focal->setDecimals(1);
    d->focal->setRange(1.0, 10000.0, 0.01);
    d->focal->setDefaultValue(1.0);

    d->aperture             = new DDoubleNumInput(this);
    d->aperture->setDecimals(1);
    d->aperture->setRange(1.1, 256.0, 0.1);
    d->aperture->setDefaultValue(1.1);

    d->distance             = new DDoubleNumInput(this);
    d->distance->setDecimals(1);
    d->distance->setRange(0.0, 10000.0, 0.1);
    d->distance->setDefaultValue(0.0);

    grid->addWidget(hbox,          0, 0, 1, 3);
    grid->addWidget(hbox1,         1, 0, 1, 3);
    grid->addWidget(d->make,       2, 0, 1, 3);
    grid->addWidget(hbox2,         3, 0, 1, 3);
    grid->addWidget(d->model,      4, 0, 1, 3);
    grid->addWidget(hbox3,         5, 0, 1, 3);
    grid->addWidget(d->lens,       6, 0, 1, 3);
    grid->addWidget(d->focalLabel, 7, 0, 1, 1);
    grid->addWidget(d->focal,      7, 1, 1, 2);
    grid->addWidget(d->aperLabel,  8, 0, 1, 1);
    grid->addWidget(d->aperture,   8, 1, 1, 2);
    grid->addWidget(d->distLabel,  9, 0, 1, 1);
    grid->addWidget(d->distance,   9, 1, 1, 2);
    grid->setSpacing(qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                             QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing)));
    grid->setContentsMargins(QMargins());

    connect(d->metadataUsage, SIGNAL(toggled(bool)),
            this, SLOT(slotUseMetadata(bool)));

    connect(d->make, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotMakeSelected()));

    connect(d->model, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotModelChanged()));

    connect(d->lens, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotLensSelected()));

    connect(d->focal, SIGNAL(valueChanged(double)),
            this, SLOT(slotFocalChanged()));

    connect(d->aperture, SIGNAL(valueChanged(double)),
            this, SLOT(slotApertureChanged()));

    connect(d->distance, SIGNAL(valueChanged(double)),
            this, SLOT(slotDistanceChanged()));

    populateDeviceCombos();
    populateLensCombo();
}

LensFunCameraSelector::~LensFunCameraSelector()
{
    delete d->metadata;
    delete d->iface;
    delete d;
}

LensFunIface* LensFunCameraSelector::iface() const
{
    return d->iface;
}

LensFunContainer LensFunCameraSelector::settings()
{
    // Update settings in LensFun interface

    blockSignals(true);

    slotModelSelected();
    slotLensSelected();
    slotFocalChanged();
    slotApertureChanged();
    slotDistanceChanged();

    blockSignals(false);

    return d->iface->settings();
}

void LensFunCameraSelector::setSettings(const LensFunContainer& settings)
{
    blockSignals(true);

    d->iface->setSettings(settings);
    refreshSettingsView();

    blockSignals(false);
}

void LensFunCameraSelector::resetToDefault()
{
    setUseMetadata(true);
}

void LensFunCameraSelector::readSettings(KConfigGroup& group)
{
    setUseMetadata(group.readEntry(d->configUseMetadata, true));

    if (!useMetadata())
    {
        LensFunContainer settings = d->iface->settings();
        settings.cameraModel      = group.readEntry(d->configCameraModel, QString());
        settings.cameraMake       = group.readEntry(d->configCameraMake,  QString());
        settings.lensModel        = group.readEntry(d->configLensModel,   QString());

        if (settings.subjectDistance <= 0.0)
        {
            settings.subjectDistance = group.readEntry(d->configSubjectDistance, -1.0);
        }

        if (settings.focalLength <= 0.0)
        {
            settings.focalLength = group.readEntry(d->configFocalLength, -1.0);
        }

        settings.cropFactor = group.readEntry(d->configCropFactor, -1.0);

        if (settings.aperture <= 0.0)
        {
            settings.aperture = group.readEntry(d->configAperture, -1.0);
        }

        setSettings(settings);
    }

    slotUseMetadata(useMetadata());
}

void LensFunCameraSelector::writeSettings(KConfigGroup& group)
{
    group.writeEntry(d->configUseMetadata,     useMetadata());
    group.writeEntry(d->configCameraModel,     d->iface->settings().cameraModel);
    group.writeEntry(d->configCameraMake,      d->iface->settings().cameraMake);
    group.writeEntry(d->configLensModel,       d->iface->settings().lensModel);
    group.writeEntry(d->configSubjectDistance, d->iface->settings().subjectDistance);
    group.writeEntry(d->configFocalLength,     d->iface->settings().focalLength);
    group.writeEntry(d->configCropFactor,      d->iface->settings().cropFactor);
    group.writeEntry(d->configAperture,        d->iface->settings().aperture);
}

void LensFunCameraSelector::setMetadata(const MetaEngineData& meta)
{
    if (d->metadata)
    {
        delete d->metadata;
    }

    d->metadata = new DMetadata(meta);

    if (d->metadata->isEmpty())
    {
        d->metadataUsage->setCheckState(Qt::Unchecked);
        setEnabledUseMetadata(false);
    }
    else
    {
        setEnabledUseMetadata(true);
        findFromMetadata();
    }
}

void LensFunCameraSelector::setEnabledUseMetadata(bool b)
{
    d->metadataUsage->setEnabled(b);
}

void LensFunCameraSelector::setUseMetadata(bool b)
{
    d->metadataUsage->setChecked(b);
}

bool LensFunCameraSelector::useMetadata() const
{
    return (d->metadataUsage->isChecked());
}

void LensFunCameraSelector::setPassiveMetadataUsage(bool b)
{
    d->passiveMetadataUsage = b;
}

void LensFunCameraSelector::slotUseMetadata(bool b)
{
    d->makeDescription->setAdjustedText();
    d->modelDescription->setAdjustedText();
    d->lensDescription->setAdjustedText();
    d->metadataResult->clear();
    d->makeLabel->setStyleSheet(qApp->styleSheet());
    d->modelLabel->setStyleSheet(qApp->styleSheet());
    d->lensLabel->setStyleSheet(qApp->styleSheet());
    d->focalLabel->setStyleSheet(qApp->styleSheet());
    d->aperLabel->setStyleSheet(qApp->styleSheet());
    d->distLabel->setStyleSheet(qApp->styleSheet());
    d->make->setEnabled(true);
    d->model->setEnabled(true);
    d->lens->setEnabled(true);
    d->focal->setEnabled(true);
    d->aperture->setEnabled(true);
    d->distance->setEnabled(true);

    if (b)
    {
        if (d->passiveMetadataUsage)
        {
            d->make->setEnabled(false);
            d->model->setEnabled(false);
            d->lens->setEnabled(false);
            d->focal->setEnabled(false);
            d->aperture->setEnabled(false);
            d->distance->setEnabled(false);
            Q_EMIT signalLensSettingsChanged();
        }
        else
        {
            LensFunIface::MetadataMatch ret = findFromMetadata();

            switch (ret)
            {
                case LensFunIface::MetadataUnavailable:
                {
                    d->metadataResult->setText(i18n("(no metadata available)"));
                    d->metadataResult->setStyleSheet(d->redStyle);
                    break;
                }

                case LensFunIface::MetadataNoMatch:
                {
                    d->metadataResult->setText(i18n("(no match found)"));
                    d->metadataResult->setStyleSheet(d->redStyle);
                    break;
                }

                case LensFunIface::MetadataPartialMatch:
                {
                    d->metadataResult->setText(i18n("(partial match found)"));
                    d->metadataResult->setStyleSheet(d->orangeStyle);
                    break;
                }

                default:
                {
                    d->metadataResult->setText(i18n("(exact match found)"));
                    d->metadataResult->setStyleSheet(d->greenStyle);
                    break;
                }
            }
        }
    }
}

LensFunIface::MetadataMatch LensFunCameraSelector::findFromMetadata()
{
    LensFunIface::MetadataMatch ret = d->iface->findFromMetadata(d->metadata);
    refreshSettingsView();
    slotModelSelected();
    slotLensSelected();

    return ret;
}

void LensFunCameraSelector::refreshSettingsView()
{
    d->make->blockSignals(true);
    d->model->blockSignals(true);
    d->lens->blockSignals(true);

    d->makeLabel->setStyleSheet(qApp->styleSheet());
    d->modelLabel->setStyleSheet(qApp->styleSheet());
    d->lensLabel->setStyleSheet(qApp->styleSheet());
    d->focalLabel->setStyleSheet(qApp->styleSheet());
    d->aperLabel->setStyleSheet(qApp->styleSheet());
    d->distLabel->setStyleSheet(qApp->styleSheet());

    if (!d->passiveMetadataUsage)
    {
        d->makeDescription->setAdjustedText(QString::fromLatin1("<i>%1</i>").arg(d->iface->makeDescription()));
    }

    int makerIdx = -1;

    if (d->iface->usedCamera())
    {
        makerIdx = d->make->findOriginalText(d->iface->settings().cameraMake);
        qCDebug(DIGIKAM_DIMG_LOG) << "makerIdx: " << makerIdx << " (" << d->iface->settings().cameraMake << ")";
    }
    else
    {
        int i = d->make->findOriginalText(d->iface->makeDescription());

        if (i == -1)
        {
            i = d->make->findOriginalText(QLatin1String("Generic"));
        }

        if (i >= 0)
        {
            d->make->setCurrentIndex(i);
            populateDeviceCombos();
        }

        if (!d->passiveMetadataUsage)
        {
            d->makeLabel->setStyleSheet(d->orangeStyle);
        }
    }

    if (makerIdx >= 0)
    {
        d->make->setCurrentIndex(makerIdx);
        d->make->setEnabled(d->passiveMetadataUsage);

        if (!d->passiveMetadataUsage)
        {
            d->makeLabel->setStyleSheet(d->greenStyle);
        }

        populateDeviceCombos();
    }

    // ------------------------------------------------------------------------------------------------

    if (!d->passiveMetadataUsage)
    {
        d->modelDescription->setAdjustedText(QString::fromLatin1("<i>%1</i>").arg(d->iface->modelDescription()));
    }

    int modelIdx = -1;

    if (d->iface->usedCamera())
    {
        modelIdx = d->model->findOriginalText(d->iface->settings().cameraModel);
        qCDebug(DIGIKAM_DIMG_LOG) << "modelIdx: " << modelIdx << " (" << d->iface->settings().cameraModel << ")";
    }

    if (modelIdx >= 0)
    {
        d->model->setCurrentIndex(modelIdx);
        d->model->setEnabled(d->passiveMetadataUsage);

        if (!d->passiveMetadataUsage)
        {
            d->modelLabel->setStyleSheet(d->greenStyle);
        }

        populateLensCombo();
    }
    else
    {
        if (!d->passiveMetadataUsage)
        {
            d->modelLabel->setStyleSheet(d->orangeStyle);
        }
    }

    // ------------------------------------------------------------------------------------------------

    if (!d->passiveMetadataUsage)
    {
        d->lensDescription->setAdjustedText(QString::fromLatin1("<i>%1</i>").arg(d->iface->lensDescription()));
    }

    int lensIdx = -1;

    if (d->iface->usedLens())
    {
        lensIdx = d->lens->findOriginalText(d->iface->settings().lensModel);
        qCDebug(DIGIKAM_DIMG_LOG) << "lensIdx: " << lensIdx << " (" << d->iface->settings().lensModel << ")";
    }

    if (lensIdx >= 0)
    {
        // found lens model directly, best case :)

        d->lens->setCurrentIndex(lensIdx);
        d->lens->setEnabled(d->passiveMetadataUsage);

        if (!d->passiveMetadataUsage)
        {
            d->lensLabel->setStyleSheet(d->greenStyle);
        }
    }
    else
    {
        if (!d->passiveMetadataUsage)
        {
            d->lensLabel->setStyleSheet(d->orangeStyle);
        }
    }

    // ------------------------------------------------------------------------------------------------

    if (d->iface->settings().focalLength != -1.0)
    {
        d->focal->setValue(d->iface->settings().focalLength);
        d->focal->setEnabled(d->passiveMetadataUsage);

        if (!d->passiveMetadataUsage)
        {
            d->focalLabel->setStyleSheet(d->greenStyle);
        }
    }
    else
    {
        if (!d->passiveMetadataUsage)
        {
            d->focalLabel->setStyleSheet(d->orangeStyle);
        }
    }

    if (d->iface->settings().aperture != -1.0)
    {
        d->aperture->setValue(d->iface->settings().aperture);
        d->aperture->setEnabled(d->passiveMetadataUsage);

        if (!d->passiveMetadataUsage)
        {
            d->aperLabel->setStyleSheet(d->greenStyle);
        }
    }
    else
    {
        if (!d->passiveMetadataUsage)
        {
            d->aperLabel->setStyleSheet(d->orangeStyle);
        }
    }

    if (d->iface->settings().subjectDistance != -1.0)
    {
        d->distance->setValue(d->iface->settings().subjectDistance);
        d->distance->setEnabled(d->passiveMetadataUsage);

        if (!d->passiveMetadataUsage)
        {
            d->distLabel->setStyleSheet(d->greenStyle);
        }
    }
    else
    {
        if (!d->passiveMetadataUsage)
        {
            d->distLabel->setStyleSheet(d->orangeStyle);
        }
    }

    d->make->blockSignals(false);
    d->model->blockSignals(false);
    d->lens->blockSignals(false);
}

void LensFunCameraSelector::populateDeviceCombos()
{
    d->make->blockSignals(true);
    d->model->blockSignals(true);

    const lfCamera* const* it = d->iface->lensFunCameras();

    // reset box

    d->model->clear();

    bool firstRun = false;

    if (d->make->count() == 0)
    {
        firstRun = true;
    }

    while (*it)
    {
        if (firstRun)
        {
            // Maker DB does not change, so we fill it only once.

            if ((*it)->Maker)
            {
                QString t = QLatin1String((*it)->Maker);

                if (d->make->findOriginalText(t) < 0)
                {
                    d->make->addSqueezedItem(t);
                }
            }
        }

        // Fill models for current selected maker

        if ((*it)->Model &&
            (QLatin1String((*it)->Maker) == d->make->itemHighlighted()))
        {
            LensFunIface::DevicePtr dev = *it;
            QVariant b                  = QVariant::fromValue(dev);

            d->model->addSqueezedItem(QLatin1String(dev->Model), b);
        }

        ++it;
    }

    d->make->blockSignals(false);
    d->model->blockSignals(false);
}

void LensFunCameraSelector::populateLensCombo()
{
    d->lens->blockSignals(true);
    d->lens->clear();
    d->lens->blockSignals(false);

    QVariant v = d->model->itemData(d->model->currentIndex());

    if (!v.isValid() || v.isNull())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Invalid variant value for device!";
        return;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "variant: " << v;

    LensFunIface::DevicePtr dev = v.value<LensFunIface::DevicePtr>();

    if (!dev)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Device is null!";
        return;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "dev: " << dev->Maker << " :: " << dev->Model << " :: " << dev->CropFactor;

    d->lens->blockSignals(true);
    const lfLens** lenses     = d->iface->lensFunDataBase()->FindLenses(dev, nullptr, nullptr);
    LensFunContainer settings = d->iface->settings();
    settings.cropFactor       = dev ? dev->CropFactor : -1.0;
    d->iface->setSettings(settings);

    QMultiMap<QString, QVariant> lensMap;

    while (lenses && *lenses)
    {
        LensFunIface::LensPtr lens = *lenses;
        QVariant b                 = QVariant::fromValue(lens);

        lensMap.insert(QLatin1String(lens->Model), b);
        ++lenses;
    }

    QMultiMap<QString, QVariant>::ConstIterator it = lensMap.constBegin();

    for ( ; it != lensMap.constEnd() ; ++it)
    {
        d->lens->addSqueezedItem(it.key(), it.value());
    }

    d->lens->blockSignals(false);
}

void LensFunCameraSelector::slotMakeSelected()
{
    populateDeviceCombos();
    slotModelSelected();

    // Fill Lens list for current Maker & Model and fire signalLensSettingsChanged().

    populateLensCombo();
    slotLensSelected();
}

void LensFunCameraSelector::slotModelChanged()
{
    populateLensCombo();
    slotModelSelected();
}

void LensFunCameraSelector::slotModelSelected()
{
    QVariant v = d->model->itemData(d->model->currentIndex());
    d->iface->setUsedCamera((d->metadataUsage->isChecked() && d->passiveMetadataUsage) ? nullptr
                                                                                       : v.value<LensFunIface::DevicePtr>());
    Q_EMIT signalLensSettingsChanged();
}

void LensFunCameraSelector::slotLensSelected()
{
    QVariant v = d->lens->itemData(d->lens->currentIndex());
    d->iface->setUsedLens((d->metadataUsage->isChecked() && d->passiveMetadataUsage) ? nullptr
                                                                                     : v.value<LensFunIface::LensPtr>());

    LensFunContainer settings = d->iface->settings();

    if (d->iface->usedLens() &&
        (settings.cropFactor <= 0.0)) // this should not happen
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "No crop factor is set for camera, using lens calibration data: "
                                  << d->iface->usedLens()->CropFactor;
        settings.cropFactor = d->iface->usedLens()->CropFactor;
    }

    d->iface->setSettings(settings);
    Q_EMIT signalLensSettingsChanged();
}

void LensFunCameraSelector::slotFocalChanged()
{
    LensFunContainer settings = d->iface->settings();
    settings.focalLength      = d->metadataUsage->isChecked() && d->passiveMetadataUsage ? -1.0
                                                                                         : d->focal->value();
    d->iface->setSettings(settings);
    Q_EMIT signalLensSettingsChanged();
}

void LensFunCameraSelector::slotApertureChanged()
{
    LensFunContainer settings = d->iface->settings();
    settings.aperture         = d->metadataUsage->isChecked() && d->passiveMetadataUsage ? -1.0
                                                                                         : d->aperture->value();
    d->iface->setSettings(settings);
    Q_EMIT signalLensSettingsChanged();
}

void LensFunCameraSelector::slotDistanceChanged()
{
    LensFunContainer settings = d->iface->settings();
    settings.subjectDistance  = d->metadataUsage->isChecked() && d->passiveMetadataUsage ? -1.0
                                                                                         : d->distance->value();
    d->iface->setSettings(settings);
    Q_EMIT signalLensSettingsChanged();
}

} // namespace Digikam

// Restore warnings

#if defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif
