/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-08-06
 * Description : Raw decoding settings for digiKam:
 *               standard RawEngine parameters plus
 *               few customized for post processing.
 *
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "drawdecoding.h"

// Qt includes

#include <QDomDocument>

// Local includes

#include "drawdecoder.h"
#include "filteraction.h"
#include "digikam_version.h"

namespace Digikam
{

class Q_DECL_HIDDEN DRawDecoderSettingsWriter
{
public:

    DRawDecoderSettingsWriter(const DRawDecoderSettings& settings, FilterAction& action, const QString& prefix = QString())
        : settings(settings),
          action(action),
          prefix(prefix)
    {
        timeOptimizedSettings.optimizeTimeLoading();
    }

    inline QString combinedKey(const QString& key)
    {
        return (prefix + key);
    }

    template <typename T>
    void addParameterIfNotDefault(const QString& key, const T& value, const T& defaultValue)
    {
        if (value != defaultValue)
        {
            action.addParameter(key, value);
        }
    }

#define AddParameterIfNotDefault(name) AddParameterIfNotDefaultWithValue(name, name)
#define AddParameterIfNotDefaultWithValue(name, value) \
        addParameterIfNotDefault(prefix + QLatin1String(#name), settings.value, defaultSettings.value)

#define AddParameterIfNotDefaultEnum(name) AddParameterIfNotDefaultEnumWithValue(name, name)
#define AddParameterIfNotDefaultEnumWithValue(name, value) \
        addParameterIfNotDefault<int>(prefix + QLatin1String(#name), settings.value, defaultSettings.value)

#define AddParameter(name) action.addParameter(prefix + QLatin1String(#name), settings.name)
#define AddParameterEnum(name) action.addParameter(prefix + QLatin1String(#name), static_cast<int>(settings.name))

    void write();

public:

    const DRawDecoderSettings& settings;
    FilterAction&              action;
    QString                    prefix;

    DRawDecoderSettings        defaultSettings;
    DRawDecoderSettings        timeOptimizedSettings;
};

void DRawDecoderSettingsWriter::write()
{
    action.addParameter(QLatin1String("RawDecoder"), QLatin1String(digikam_version_short));

    if (settings == defaultSettings)
    {
        action.addParameter(QLatin1String("RawDefaultSettings"), true);
        return;
    }

    if (settings == timeOptimizedSettings)
    {
        action.addParameter(QLatin1String("RawTimeOptimizedSettings"), true);
        return;
    }

    AddParameter(sixteenBitsImage);

    AddParameter(fixColorsHighlights);
    AddParameter(autoBrightness);

    AddParameterEnum(whiteBalance);

    if (settings.whiteBalance == DRawDecoderSettings::CUSTOM)
    {
        AddParameter(customWhiteBalance);
        AddParameter(customWhiteBalanceGreen);
    }

    AddParameterIfNotDefault(RGBInterpolate4Colors);
    AddParameterIfNotDefault(DontStretchPixels);
    AddParameterIfNotDefault(unclipColors);

    AddParameterEnum(RAWQuality);

    AddParameterIfNotDefault(medianFilterPasses);
    AddParameterIfNotDefaultEnumWithValue(noiseReductionType,              NRType);
    AddParameterIfNotDefaultWithValue(noiseReductionThreshold,             NRThreshold);
    AddParameterIfNotDefault(brightness);

    AddParameter(enableBlackPoint);

    if (settings.enableBlackPoint)
    {
        AddParameter(blackPoint);
    }

    AddParameter(enableWhitePoint);

    if (settings.enableWhitePoint)
    {
        AddParameter(whitePoint);
    }

    AddParameterEnum(inputColorSpace);

    if (settings.inputColorSpace == DRawDecoderSettings::CUSTOMINPUTCS)
    {
        AddParameter(inputProfile);
    }

    AddParameterEnum(outputColorSpace);

    if (settings.outputColorSpace == DRawDecoderSettings::CUSTOMOUTPUTCS)
    {
        AddParameter(outputProfile);
    }

    AddParameterIfNotDefault(deadPixelMap);

    if (settings.whiteBalance == DRawDecoderSettings::AERA /*sic*/)
    {
        if (!settings.whiteBalanceArea.isNull())
        {
            action.addParameter(prefix + QLatin1String("whiteBalanceAreaX"),      settings.whiteBalanceArea.x());
            action.addParameter(prefix + QLatin1String("whiteBalanceAreaY"),      settings.whiteBalanceArea.y());
            action.addParameter(prefix + QLatin1String("whiteBalanceAreaWidth"),  settings.whiteBalanceArea.width());
            action.addParameter(prefix + QLatin1String("whiteBalanceAreaHeight"), settings.whiteBalanceArea.height());
        }
    }

    AddParameterIfNotDefault(dcbIterations);
    AddParameterIfNotDefault(dcbEnhanceFl);
    AddParameterIfNotDefault(expoCorrection);
    AddParameterIfNotDefaultWithValue(exposureCorrectionShift,            expoCorrectionShift);
    AddParameterIfNotDefaultWithValue(exposureCorrectionHighlight,        expoCorrectionHighlight);
}

// --------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DRawDecoderSettingsReader
{
public:

    explicit DRawDecoderSettingsReader(const FilterAction& action, const QString& prefix = QString())
        : action(action),
          prefix(prefix)
    {
    }

    template <typename enumType, typename variantType>
    void readParameter(const QString& key, enumType& setting, const variantType& defaultValue)
    {
        setting = static_cast<enumType>(action.parameter(key, defaultValue));
    }

#define ReadParameter(name) ReadParameterWithValue(name, name)
#define ReadParameterWithValue(name, value) \
        settings.value = action.parameter(prefix + QLatin1String(#name), settings.value)

#define ReadParameterEnum(name) ReadParameterEnumWithValue(name, name)
#define ReadParameterEnumWithValue(name, value) \
        readParameter(prefix + QLatin1String(#name), settings.value, static_cast<int>(settings.value))

    void read();

public:

    const FilterAction& action;
    QString             prefix;
    DRawDecoderSettings settings;
};

void DRawDecoderSettingsReader::read()
{
    if (action.parameter(QLatin1String("RawDefaultSettings")).toBool())
    {
        return;
    }

    if (action.parameter(QLatin1String("RawTimeOptimizedSettings")).toBool())
    {
        settings.optimizeTimeLoading();
        return;
    }

    ReadParameter(sixteenBitsImage);

    ReadParameter(fixColorsHighlights);
    ReadParameter(autoBrightness);

    ReadParameterEnum(whiteBalance);

    if (settings.whiteBalance == DRawDecoderSettings::CUSTOM)
    {
        ReadParameter(customWhiteBalance);
        ReadParameter(customWhiteBalanceGreen);
    }

    ReadParameter(RGBInterpolate4Colors);
    ReadParameter(DontStretchPixels);
    ReadParameter(unclipColors);

    ReadParameterEnum(RAWQuality);

    ReadParameter(medianFilterPasses);
    ReadParameterEnumWithValue(noiseReductionType,              NRType);
    ReadParameterWithValue(noiseReductionThreshold,             NRThreshold);
    ReadParameter(brightness);

    ReadParameter(enableBlackPoint);

    if (settings.enableBlackPoint)
    {
        ReadParameter(blackPoint);
    }

    ReadParameter(enableWhitePoint);

    if (settings.enableWhitePoint)
    {
        ReadParameter(whitePoint);
    }

    ReadParameterEnum(inputColorSpace);

    if (settings.inputColorSpace == DRawDecoderSettings::CUSTOMINPUTCS)
    {
        ReadParameter(inputProfile);
    }

    ReadParameterEnum(outputColorSpace);

    if (settings.outputColorSpace == DRawDecoderSettings::CUSTOMOUTPUTCS)
    {
        ReadParameter(outputProfile);
    }

    ReadParameter(deadPixelMap);

    if (!action.hasParameter(QLatin1String("whiteBalanceAreaX")))
    {
        int x      = action.parameter(prefix + QLatin1String("whiteBalanceAreaX"),      0);
        int y      = action.parameter(prefix + QLatin1String("whiteBalanceAreaY"),      0);
        int width  = action.parameter(prefix + QLatin1String("whiteBalanceAreaWidth"),  0);
        int height = action.parameter(prefix + QLatin1String("whiteBalanceAreaHeight"), 0);
        QRect rect(x, y, width, height);

        if (rect.isValid())
        {
            settings.whiteBalanceArea = rect;
        }
    }

    ReadParameter(dcbIterations);
    ReadParameter(dcbEnhanceFl);
    ReadParameter(expoCorrection);
    ReadParameterWithValue(exposureCorrectionShift,     expoCorrectionShift);
    ReadParameterWithValue(exposureCorrectionHighlight, expoCorrectionHighlight);
}

// --------------------------------------------------------------------------------------------

DRawDecoding::DRawDecoding()
{
    resetPostProcessingSettings();
}

DRawDecoding::DRawDecoding(const DRawDecoderSettings& prm)
    : rawPrm(prm)
{
    resetPostProcessingSettings();
}

DRawDecoding::~DRawDecoding()
{
}

void DRawDecoding::optimizeTimeLoading()
{
    rawPrm.optimizeTimeLoading();
    resetPostProcessingSettings();
}

void DRawDecoding::resetPostProcessingSettings()
{
    bcg          = BCGContainer();
    wb           = WBContainer();
    curvesAdjust = CurvesContainer();
}

bool DRawDecoding::postProcessingSettingsIsDirty() const
{
    return !((bcg == BCGContainer()) &&
             (wb  == WBContainer())  &&
             curvesAdjust.isEmpty());
}

bool DRawDecoding::operator==(const DRawDecoding& other) const
{
    return ((rawPrm       == other.rawPrm)       &&
            (bcg          == other.bcg)          &&
            (wb           == other.wb)           &&
            (curvesAdjust == other.curvesAdjust));
}

DRawDecoding DRawDecoding::fromFilterAction(const FilterAction& action, const QString& prefix)
{
    DRawDecoding settings;

    DRawDecoderSettingsReader reader(action, prefix);
    reader.read();
    settings.rawPrm       = reader.settings;

    settings.bcg          = BCGContainer::fromFilterAction(action);
    settings.wb           = WBContainer::fromFilterAction(action);
    settings.curvesAdjust = CurvesContainer::fromFilterAction(action);

    return settings;
}

void DRawDecoding::writeToFilterAction(FilterAction& action, const QString& prefix) const
{
    DRawDecoderSettingsWriter writer(rawPrm, action, prefix);
    writer.write();

    if (!bcg.isDefault())
    {
        bcg.writeToFilterAction(action, prefix);
    }

    if (!wb.isDefault())
    {
        wb.writeToFilterAction(action, prefix);
    }

    if (!curvesAdjust.isEmpty())
    {
        curvesAdjust.writeToFilterAction(action, prefix);
    }
}

void DRawDecoding::decodingSettingsToXml(const DRawDecoderSettings& prm, QDomElement& elm)
{
    QDomDocument doc = elm.ownerDocument();
    QDomElement  data;

    data = doc.createElement(QLatin1String("autobrightness"));
    data.setAttribute(QLatin1String("value"), prm.autoBrightness);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("fixcolorshighlights"));
    data.setAttribute(QLatin1String("value"), prm.fixColorsHighlights);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("sixteenbitsimage"));
    data.setAttribute(QLatin1String("value"), prm.sixteenBitsImage);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("brightness"));
    data.setAttribute(QLatin1String("value"), prm.brightness);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("rawquality"));
    data.setAttribute(QLatin1String("value"), prm.RAWQuality);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("inputcolorspace"));
    data.setAttribute(QLatin1String("value"), prm.inputColorSpace);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("outputcolorspace"));
    data.setAttribute(QLatin1String("value"), prm.outputColorSpace);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("rgbinterpolate4colors"));
    data.setAttribute(QLatin1String("value"), prm.RGBInterpolate4Colors);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("dontstretchpixels"));
    data.setAttribute(QLatin1String("value"), prm.DontStretchPixels);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("unclipcolors"));
    data.setAttribute(QLatin1String("value"), prm.unclipColors);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("whitebalance"));
    data.setAttribute(QLatin1String("value"), prm.whiteBalance);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("customwhitebalance"));
    data.setAttribute(QLatin1String("value"), prm.customWhiteBalance);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("customwhitebalancegreen"));
    data.setAttribute(QLatin1String("value"), prm.customWhiteBalanceGreen);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("halfsizecolorimage"));
    data.setAttribute(QLatin1String("value"), prm.halfSizeColorImage);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("enableblackpoint"));
    data.setAttribute(QLatin1String("value"), prm.enableBlackPoint);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("blackpoint"));
    data.setAttribute(QLatin1String("value"), prm.blackPoint);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("enablewhitepoint"));
    data.setAttribute(QLatin1String("value"), prm.enableWhitePoint);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("whitepoint"));
    data.setAttribute(QLatin1String("value"), prm.whitePoint);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("noisereductiontype"));
    data.setAttribute(QLatin1String("value"), prm.NRType);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("noisereductionthreshold"));
    data.setAttribute(QLatin1String("value"), prm.NRThreshold);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("medianfilterpasses"));
    data.setAttribute(QLatin1String("value"), prm.medianFilterPasses);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("inputprofile"));
    data.setAttribute(QLatin1String("value"), prm.inputProfile);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("outputprofile"));
    data.setAttribute(QLatin1String("value"), prm.outputProfile);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("deadpixelmap"));
    data.setAttribute(QLatin1String("value"), prm.deadPixelMap);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("whitebalanceareax"));
    data.setAttribute(QLatin1String("value"), prm.whiteBalanceArea.x());
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("whitebalanceareay"));
    data.setAttribute(QLatin1String("value"), prm.whiteBalanceArea.y());
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("whitebalanceareawidth"));
    data.setAttribute(QLatin1String("value"), prm.whiteBalanceArea.width());
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("whitebalanceareaheight"));
    data.setAttribute(QLatin1String("value"), prm.whiteBalanceArea.height());
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("dcbiterations"));
    data.setAttribute(QLatin1String("value"), prm.dcbIterations);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("dcbenhancefl"));
    data.setAttribute(QLatin1String("value"), prm.dcbEnhanceFl);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("expocorrection"));
    data.setAttribute(QLatin1String("value"), prm.expoCorrection);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("expocorrectionshift"));
    data.setAttribute(QLatin1String("value"), prm.expoCorrectionShift);
    elm.appendChild(data);

    data = doc.createElement(QLatin1String("expocorrectionhighlight"));
    data.setAttribute(QLatin1String("value"), prm.expoCorrectionHighlight);
    elm.appendChild(data);
}

void DRawDecoding::decodingSettingsFromXml(const QDomElement& elm, DRawDecoderSettings& prm)
{
    bool ok = false;

    for (QDomNode node = elm.firstChild(); !node.isNull(); node = node.nextSibling())
    {
        QDomElement echild = node.toElement();

        if (echild.isNull())
        {
            continue;
        }

        QString key        = echild.tagName();
        QString val        = echild.attribute(QLatin1String("value"));

        if      (key == QLatin1String("autobrightness"))
        {
            prm.autoBrightness = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("fixcolorshighlights"))
        {
            prm.fixColorsHighlights = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("sixteenbitsimage"))
        {
            prm.sixteenBitsImage = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("brightness"))
        {
            prm.brightness = val.toDouble(&ok);
        }
        else if (key == QLatin1String("rawquality"))
        {
            prm.RAWQuality = (DRawDecoderSettings::DecodingQuality)val.toInt(&ok);
        }
        else if (key == QLatin1String("inputcolorspace"))
        {
            prm.inputColorSpace = (DRawDecoderSettings::InputColorSpace)val.toInt(&ok);
        }
        else if (key == QLatin1String("outputcolorspace"))
        {
            prm.outputColorSpace = (DRawDecoderSettings::OutputColorSpace)val.toInt(&ok);
        }
        else if (key == QLatin1String("rgbinterpolate4colors"))
        {
            prm.RGBInterpolate4Colors = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("dontstretchpixels"))
        {
            prm.DontStretchPixels = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("unclipcolors"))
        {
            prm.unclipColors = (int)val.toInt(&ok);
        }
        else if (key == QLatin1String("whitebalance"))
        {
            prm.whiteBalance = (DRawDecoderSettings::WhiteBalance)val.toInt(&ok);
        }
        else if (key == QLatin1String("customwhitebalance"))
        {
            prm.customWhiteBalance = val.toInt(&ok);
        }
        else if (key == QLatin1String("customwhitebalancegreen"))
        {
            prm.customWhiteBalanceGreen = val.toDouble(&ok);
        }
        else if (key == QLatin1String("halfsizecolorimage"))
        {
            prm.halfSizeColorImage = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("enableblackpoint"))
        {
            prm.enableBlackPoint = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("blackpoint"))
        {
            prm.blackPoint = val.toInt(&ok);
        }
        else if (key == QLatin1String("enablewhitepoint"))
        {
            prm.enableWhitePoint = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("whitepoint"))
        {
            prm.whitePoint = val.toInt(&ok);
        }
        else if (key == QLatin1String("noisereductiontype"))
        {
            prm.NRType = (DRawDecoderSettings::NoiseReduction)val.toInt(&ok);
        }
        else if (key == QLatin1String("noisereductionthreshold"))
        {
            prm.NRThreshold = val.toInt(&ok);
        }
        else if (key == QLatin1String("medianfilterpasses"))
        {
            prm.medianFilterPasses = val.toInt(&ok);
        }
        else if (key == QLatin1String("inputprofile"))
        {
            prm.inputProfile = val;
        }
        else if (key == QLatin1String("outputprofile"))
        {
            prm.outputProfile = val;
        }
        else if (key == QLatin1String("deadpixelmap"))
        {
            prm.deadPixelMap = val;
        }
        else if (key == QLatin1String("whitebalanceareax"))
        {
            prm.whiteBalanceArea.setX(val.toInt(&ok));
        }
        else if (key == QLatin1String("whitebalanceareay"))
        {
            prm.whiteBalanceArea.setY(val.toInt(&ok));
        }
        else if (key == QLatin1String("whitebalanceareawidth"))
        {
            prm.whiteBalanceArea.setWidth(val.toInt(&ok));
        }
        else if (key == QLatin1String("whitebalanceareaheight"))
        {
            prm.whiteBalanceArea.setHeight(val.toInt(&ok));
        }
        else if (key == QLatin1String("dcbiterations"))
        {
            prm.dcbIterations = val.toInt(&ok);
        }
        else if (key == QLatin1String("dcbenhancefl"))
        {
            prm.dcbEnhanceFl = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("expocorrection"))
        {
            prm.expoCorrection = (bool)val.toInt(&ok);
        }
        else if (key == QLatin1String("expocorrectionshift"))
        {
            prm.expoCorrectionShift = val.toDouble(&ok);
        }
        else if (key == QLatin1String("expocorrectionhighlight"))
        {
            prm.expoCorrectionHighlight = val.toDouble(&ok);
        }
    }
}

} // namespace Digikam
