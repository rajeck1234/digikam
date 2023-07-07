/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * date        : 2006-09-13
 * Description : Raw Decoder settings widgets
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2007-2008 by Guillaume Castagnino <casta at xwing dot info>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#define OPTIONFIXCOLORSHIGHLIGHTSENTRY                 "FixColorsHighlights"
#define OPTIONDECODESIXTEENBITENTRY                    "SixteenBitsImage"
#define OPTIONWHITEBALANCEENTRY                        "White Balance"
#define OPTIONCUSTOMWHITEBALANCEENTRY                  "Custom White Balance"
#define OPTIONCUSTOMWBGREENENTRY                       "Custom White Balance Green"
#define OPTIONFOURCOLORRGBENTRY                        "Four Color RGB"
#define OPTIONUNCLIPCOLORSENTRY                        "Unclip Color"
#define OPTIONDONTSTRETCHPIXELSSENTRY                  "Dont Stretch Pixels"        // krazy:exclude=spelling
#define OPTIONMEDIANFILTERPASSESENTRY                  "Median Filter Passes"       // krazy:exclude=spelling
#define OPTIONNOISEREDUCTIONTYPEENTRY                  "Noise Reduction Type"
#define OPTIONNOISEREDUCTIONTHRESHOLDENTRY             "Noise Reduction Threshold"
#define OPTIONAUTOBRIGHTNESSENTRY                      "AutoBrightness"
#define OPTIONDECODINGQUALITYENTRY                     "Decoding Quality"
#define OPTIONINPUTCOLORSPACEENTRY                     "Input Color Space"
#define OPTIONOUTPUTCOLORSPACEENTRY                    "Output Color Space"
#define OPTIONINPUTCOLORPROFILEENTRY                   "Input Color Profile"
#define OPTIONOUTPUTCOLORPROFILEENTRY                  "Output Color Profile"
#define OPTIONBRIGHTNESSMULTIPLIERENTRY                "Brightness Multiplier"
#define OPTIONUSEBLACKPOINTENTRY                       "Use Black Point"
#define OPTIONBLACKPOINTENTRY                          "Black Point"
#define OPTIONUSEWHITEPOINTENTRY                       "Use White Point"
#define OPTIONWHITEPOINTENTRY                          "White Point"

//-- Extended demosaicing settings ----------------------------------------------------------

#define OPTIONDCBITERATIONSENTRY                       "Dcb Iterations"
#define OPTIONDCBENHANCEFLENTRY                        "Dcb Enhance Filter"
#define OPTIONEXPOCORRECTIONENTRY                      "Expo Correction"
#define OPTIONEXPOCORRECTIONSHIFTENTRY                 "Expo Correction Shift"
#define OPTIONEXPOCORRECTIONHIGHLIGHTENTRY             "Expo Correction Highlight"

#include "drawdecoderwidget.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QWhatsThis>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QIcon>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "drawdecoder.h"
#include "dnuminput.h"
#include "dcombobox.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN DRawDecoderWidget::Private
{
public:

    explicit Private()
      : demosaicingSettings             (nullptr),
        whiteBalanceSettings            (nullptr),
        correctionsSettings             (nullptr),
        colormanSettings                (nullptr),
        whiteBalanceLabel               (nullptr),
        customWhiteBalanceLabel         (nullptr),
        customWhiteBalanceGreenLabel    (nullptr),
        brightnessLabel                 (nullptr),
        RAWQualityLabel                 (nullptr),
        NRLabel1                        (nullptr),
        unclipColorLabel                (nullptr),
        reconstructLabel                (nullptr),
        inputColorSpaceLabel            (nullptr),
        outputColorSpaceLabel           (nullptr),
        medianFilterPassesLabel         (nullptr),
        noiseReductionLabel             (nullptr),
        expoCorrectionShiftLabel        (nullptr),
        expoCorrectionHighlightLabel    (nullptr),
        blackPointCheckBox              (nullptr),
        whitePointCheckBox              (nullptr),
        sixteenBitsImage                (nullptr),
        autoBrightnessBox               (nullptr),
        fourColorCheckBox               (nullptr),
        dontStretchPixelsCheckBox       (nullptr),
        fixColorsHighlightsBox          (nullptr),
        refineInterpolationBox          (nullptr),
        expoCorrectionBox               (nullptr),
        inIccUrlEdit                    (nullptr),
        outIccUrlEdit                   (nullptr),
        noiseReductionComboBox          (nullptr),
        whiteBalanceComboBox            (nullptr),
        RAWQualityComboBox              (nullptr),
        unclipColorComboBox             (nullptr),
        inputColorSpaceComboBox         (nullptr),
        outputColorSpaceComboBox        (nullptr),
        customWhiteBalanceSpinBox       (nullptr),
        reconstructSpinBox              (nullptr),
        blackPointSpinBox               (nullptr),
        whitePointSpinBox               (nullptr),
        NRSpinBox1                      (nullptr),
        medianFilterPassesSpinBox       (nullptr),
        customWhiteBalanceGreenSpinBox  (nullptr),
        brightnessSpinBox               (nullptr),
        expoCorrectionShiftSpinBox      (nullptr),
        expoCorrectionHighlightSpinBox  (nullptr)
    {
    }

    /** Convert Exposure correction shift E.V value from GUI to Linear value needs by libraw decoder.
     */
    double shiftExpoFromEvToLinear(double ev) const
    {
        // From GUI : -2.0EV => 0.25
        //            +3.0EV => 8.00
        return (1.55*ev + 3.35);
    }

    /** Convert Exposure correction shift Linear value from liraw decoder to E.V value needs by GUI.
     */
    double shiftExpoFromLinearToEv(double lin) const
    {
        // From GUI : 0.25 => -2.0EV
        //            8.00 => +3.0EV
        return ((lin-3.35) / 1.55);
    }

public:

    QWidget*         demosaicingSettings;
    QWidget*         whiteBalanceSettings;
    QWidget*         correctionsSettings;
    QWidget*         colormanSettings;

    QLabel*          whiteBalanceLabel;
    QLabel*          customWhiteBalanceLabel;
    QLabel*          customWhiteBalanceGreenLabel;
    QLabel*          brightnessLabel;
    QLabel*          RAWQualityLabel;
    QLabel*          NRLabel1;
    QLabel*          unclipColorLabel;
    QLabel*          reconstructLabel;
    QLabel*          inputColorSpaceLabel;
    QLabel*          outputColorSpaceLabel;
    QLabel*          medianFilterPassesLabel;
    QLabel*          noiseReductionLabel;
    QLabel*          expoCorrectionShiftLabel;
    QLabel*          expoCorrectionHighlightLabel;

    QCheckBox*       blackPointCheckBox;
    QCheckBox*       whitePointCheckBox;
    QCheckBox*       sixteenBitsImage;
    QCheckBox*       autoBrightnessBox;
    QCheckBox*       fourColorCheckBox;
    QCheckBox*       dontStretchPixelsCheckBox;
    QCheckBox*       fixColorsHighlightsBox;
    QCheckBox*       refineInterpolationBox;
    QCheckBox*       expoCorrectionBox;

    DFileSelector*   inIccUrlEdit;
    DFileSelector*   outIccUrlEdit;

    DComboBox*       noiseReductionComboBox;
    DComboBox*       whiteBalanceComboBox;
    DComboBox*       RAWQualityComboBox;
    DComboBox*       unclipColorComboBox;
    DComboBox*       inputColorSpaceComboBox;
    DComboBox*       outputColorSpaceComboBox;

    DIntNumInput*    customWhiteBalanceSpinBox;
    DIntNumInput*    reconstructSpinBox;
    DIntNumInput*    blackPointSpinBox;
    DIntNumInput*    whitePointSpinBox;
    DIntNumInput*    NRSpinBox1;
    DIntNumInput*    medianFilterPassesSpinBox;

    DDoubleNumInput* customWhiteBalanceGreenSpinBox;
    DDoubleNumInput* brightnessSpinBox;
    DDoubleNumInput* expoCorrectionShiftSpinBox;
    DDoubleNumInput* expoCorrectionHighlightSpinBox;
};

DRawDecoderWidget::DRawDecoderWidget(QWidget* const parent, int advSettings)
    : DExpanderBox(parent),
      d           (new Private)
{
    setup(advSettings);
}

void DRawDecoderWidget::setup(int advSettings)
{
    setObjectName( QLatin1String("DCRawSettings Expander" ));

    // ---------------------------------------------------------------
    // DEMOSAICING Settings panel

    d->demosaicingSettings               = new QWidget(this);
    QGridLayout* const demosaicingLayout = new QGridLayout(d->demosaicingSettings);
    int line                             = 0;

    d->sixteenBitsImage = new QCheckBox(i18nc("@option:check", "16 bits color depth"), d->demosaicingSettings);
    d->sixteenBitsImage->setWhatsThis(xi18nc("@info:whatsthis", "<para>If enabled, all RAW files will "
                                "be decoded in 16-bit color depth using a linear gamma curve. To "
                                "prevent dark picture rendering in the editor, it is recommended to "
                                "use Color Management in this mode.</para>"
                                "<para>If disabled, all RAW files will be decoded in 8-bit color "
                                "depth with a BT.709 gamma curve and a 99th-percentile white point. "
                                "This mode is faster than 16-bit decoding.</para>"));
    demosaicingLayout->addWidget(d->sixteenBitsImage, 0, 0, 1, 2);

    if (advSettings & SIXTEENBITS)
    {
        d->sixteenBitsImage->show();
        line = 1;
    }
    else
    {
        d->sixteenBitsImage->hide();
    }

    d->fourColorCheckBox = new QCheckBox(i18nc("@option:check", "Interpolate RGB as four colors"), d->demosaicingSettings);
    d->fourColorCheckBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>Interpolate RGB as four "
                                "colors</title>"
                                "<para>The default is to assume that all green pixels are the same. "
                                "If even-row green pixels are more sensitive to ultraviolet light "
                                "than odd-row this difference causes a mesh pattern in the output; "
                                "using this option solves this problem with minimal loss of detail.</para>"
                                "<para>To resume, this option blurs the image a little, but it "
                                "eliminates false 2x2 mesh patterns with VNG quality method or "
                                "mazes with AHD quality method.</para>"));
    demosaicingLayout->addWidget(d->fourColorCheckBox, line, 0, 1, line == 0 ? 2 : 3);
    line++;

    QLabel* const dcrawVersion = new QLabel(d->demosaicingSettings);
    dcrawVersion->setAlignment(Qt::AlignRight);
    dcrawVersion->setToolTip(i18nc("@info:tooltip", "Visit LibRaw project website"));
    dcrawVersion->setOpenExternalLinks(true);
    dcrawVersion->setTextFormat(Qt::RichText);
    dcrawVersion->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    dcrawVersion->setText(QString::fromLatin1("<a href=\"%1\">%2</a>")
                          .arg(QLatin1String("https://www.libraw.org"))
                          .arg(QString::fromLatin1("libraw %1").arg(DRawDecoder::librawVersion())));

    demosaicingLayout->addWidget(dcrawVersion, 0, 2, 1, 1);

    d->dontStretchPixelsCheckBox  = new QCheckBox(i18nc("@option:check", "Do not stretch or rotate pixels"), d->demosaicingSettings);
    d->dontStretchPixelsCheckBox->setWhatsThis(xi18nc("@info:whatsthis",
                                "<title>Do not stretch or rotate pixels</title>"
                                "<para>For Fuji Super CCD cameras, show the image tilted 45 degrees. "
                                "For cameras with non-square pixels, do not stretch the image to "
                                "its correct aspect ratio. In any case, this option guarantees that "
                                "each output pixel corresponds to one RAW pixel.</para>"));
    demosaicingLayout->addWidget(d->dontStretchPixelsCheckBox, line, 0, 1, 3);
    line++;

    d->RAWQualityLabel    = new QLabel(i18nc("@label:listbox", "Quality:"), d->demosaicingSettings);
    d->RAWQualityComboBox = new DComboBox(d->demosaicingSettings);

    // Original Raw engine demosaicing methods
    d->RAWQualityComboBox->insertItem(0, i18nc("@item:inlistbox Quality", "Bilinear"), QVariant(DRawDecoderSettings::BILINEAR));
    d->RAWQualityComboBox->insertItem(1, i18nc("@item:inlistbox Quality", "VNG"),      QVariant(DRawDecoderSettings::VNG));
    d->RAWQualityComboBox->insertItem(2, i18nc("@item:inlistbox Quality", "PPG"),      QVariant(DRawDecoderSettings::PPG));
    d->RAWQualityComboBox->insertItem(3, i18nc("@item:inlistbox Quality", "AHD"),      QVariant(DRawDecoderSettings::AHD));
    d->RAWQualityComboBox->insertItem(4, i18nc("@item:inlistbox Quality", "DCB"),      QVariant(DRawDecoderSettings::DCB));
    d->RAWQualityComboBox->insertItem(5, i18nc("@item:inlistbox Quality", "DHT"),      QVariant(DRawDecoderSettings::DHT));
    d->RAWQualityComboBox->insertItem(6, i18nc("@item:inlistbox Quality", "AAHD"),     QVariant(DRawDecoderSettings::AAHD));

    d->RAWQualityComboBox->setDefaultIndex(0);
    d->RAWQualityComboBox->setCurrentIndex(0);
    d->RAWQualityComboBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>Quality (interpolation)</title>"
                                "<para>Select here the demosaicing method to use when decoding RAW "
                                "images. A demosaicing algorithm is a digital image process used to "
                                "interpolate a complete image from the partial raw data received "
                                "from the color-filtered image sensor, internal to many digital "
                                "cameras, in form of a matrix of colored pixels. Also known as CFA "
                                "interpolation or color reconstruction, another common spelling is "
                                "demosaicing. The following methods are available for demosaicing "
                                "RAW images:</para>"

                                "<para><list><item><emphasis strong='true'>Bilinear</emphasis>: use "
                                "high-speed but low-quality bilinear interpolation (default - for "
                                "slow computers). In this method, the red value of a non-red pixel "
                                "is computed as the average of the adjacent red pixels, and similarly "
                                "for blue and green.</item>"

                                "<item><emphasis strong='true'>VNG</emphasis>: use Variable Number "
                                "of Gradients interpolation. This method computes gradients near "
                                "the pixel of interest and uses the lower gradients (representing "
                                "smoother and more similar parts of the image) to make an estimate.</item>"

                                "<item><emphasis strong='true'>PPG</emphasis>: use Patterned-Pixel-"
                                "Grouping interpolation. Pixel Grouping uses assumptions about "
                                "natural scenery in making estimates. It has fewer color artifacts "
                                "on natural images than the Variable Number of Gradients method.</item>"

                                "<item><emphasis strong='true'>AHD</emphasis>: use Adaptive "
                                "Homogeneity-Directed interpolation. This method selects the "
                                "direction of interpolation so as to maximize a homogeneity metric, "
                                "thus typically minimizing color artifacts.</item>"

                                "<item><emphasis strong='true'>DCB</emphasis>: DCB interpolation from "
                                "linuxphoto.org project.</item>"

                                "<item><emphasis strong='true'>DHT</emphasis>: DHT interpolation."
                                "See https://www.libraw.org/node/2306 for details.</item>"

                                "<item><emphasis strong='true'>AAHD</emphasis>: modified AHD "
                                "interpolation.</item>"
                                "</list></para>"));

    demosaicingLayout->addWidget(d->RAWQualityLabel,    line, 0, 1, 1);
    demosaicingLayout->addWidget(d->RAWQualityComboBox, line, 1, 1, 2);
    line++;

    d->medianFilterPassesSpinBox = new DIntNumInput(d->demosaicingSettings);
    d->medianFilterPassesSpinBox->setRange(0, 10, 1);
    d->medianFilterPassesSpinBox->setDefaultValue(0);
    d->medianFilterPassesLabel   = new QLabel(i18nc("@label:slider", "Pass:"), d->whiteBalanceSettings);
    d->medianFilterPassesSpinBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>Pass</title>"
                                "<para>Set here the passes used by the median filter applied after "
                                "interpolation to Red-Green and Blue-Green channels.</para>"
                                "<para>This setting is only available for specific Quality options: "
                                "<emphasis strong='true'>Bilinear</emphasis>, <emphasis strong='true'>"
                                "VNG</emphasis>, <emphasis strong='true'>PPG</emphasis>, "
                                "<emphasis strong='true'>AHD</emphasis>, <emphasis strong='true'>"
                                "DCB</emphasis>, and <emphasis strong='true'>VCD & AHD</emphasis>.</para>"));
    demosaicingLayout->addWidget(d->medianFilterPassesLabel,   line, 0, 1, 1);
    demosaicingLayout->addWidget(d->medianFilterPassesSpinBox, line, 1, 1, 2);
    line++;

    d->refineInterpolationBox = new QCheckBox(i18nc("@option:check", "Refine interpolation"), d->demosaicingSettings);
    d->refineInterpolationBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>Refine interpolation</title>"
                                "<para>This setting is available only for few Quality options:</para>"
                                "<para><list><item><emphasis strong='true'>DCB</emphasis>: turn on "
                                "the enhance interpolated colors filter.</item>"
                                "<item><emphasis strong='true'>VCD & AHD</emphasis>: turn on the "
                                "enhanced effective color interpolation (EECI) refine to improve "
                                "sharpness.</item></list></para>"));
    demosaicingLayout->addWidget(d->refineInterpolationBox, line, 0, 1, 2);

    d->medianFilterPassesLabel->setEnabled(false);
    d->medianFilterPassesSpinBox->setEnabled(false);
    d->refineInterpolationBox->setEnabled(false);

    addItem(d->demosaicingSettings, QIcon::fromTheme(QLatin1String("image-x-adobe-dng")).pixmap(16, 16), i18nc("@label", "Demosaicing"), QLatin1String("demosaicing"), true);

    // ---------------------------------------------------------------
    // WHITE BALANCE Settings Panel

    d->whiteBalanceSettings               = new QWidget(this);
    QGridLayout* const whiteBalanceLayout = new QGridLayout(d->whiteBalanceSettings);

    d->whiteBalanceLabel    = new QLabel(i18nc("@label:listbox", "Method:"), d->whiteBalanceSettings);
    d->whiteBalanceComboBox = new DComboBox(d->whiteBalanceSettings);
    d->whiteBalanceComboBox->insertItem(DRawDecoderSettings::NONE,   i18nc("@item:inlistbox", "Default D65"));
    d->whiteBalanceComboBox->insertItem(DRawDecoderSettings::CAMERA, i18nc("@item:inlistbox", "Camera"));
    d->whiteBalanceComboBox->insertItem(DRawDecoderSettings::AUTO,   i18nc("@item:inlistbox set while balance automatically", "Automatic"));
    d->whiteBalanceComboBox->insertItem(DRawDecoderSettings::CUSTOM, i18nc("@item:inlistbox set white balance manually", "Manual"));
    d->whiteBalanceComboBox->setDefaultIndex(DRawDecoderSettings::CAMERA);
    d->whiteBalanceComboBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>White Balance</title>"
                                "<para>Configure the raw white balance:</para>"
                                "<para><list><item><emphasis strong='true'>Default D65</emphasis>: "
                                "Use a standard daylight D65 white balance.</item>"
                                "<item><emphasis strong='true'>Camera</emphasis>: Use the white "
                                "balance specified by the camera. If not available, reverts to "
                                "default neutral white balance.</item>"
                                "<item><emphasis strong='true'>Automatic</emphasis>: Calculates an "
                                "automatic white balance averaging the entire image.</item>"
                                "<item><emphasis strong='true'>Manual</emphasis>: Set a custom "
                                "temperature and green level values.</item></list></para>"));

    d->customWhiteBalanceSpinBox = new DIntNumInput(d->whiteBalanceSettings);
    d->customWhiteBalanceSpinBox->setRange(2000, 12000, 10);
    d->customWhiteBalanceSpinBox->setDefaultValue(6500);
    d->customWhiteBalanceLabel   = new QLabel(i18nc("@label:slider", "T(K):"), d->whiteBalanceSettings);
    d->customWhiteBalanceSpinBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>Temperature</title>"
                                "<para>Set here the color temperature in Kelvin.</para>"));

    d->customWhiteBalanceGreenSpinBox = new DDoubleNumInput(d->whiteBalanceSettings);
    d->customWhiteBalanceGreenSpinBox->setDecimals(2);
    d->customWhiteBalanceGreenSpinBox->setRange(0.2, 2.5, 0.01);
    d->customWhiteBalanceGreenSpinBox->setDefaultValue(1.0);
    d->customWhiteBalanceGreenLabel   = new QLabel(i18nc("@label:slider Green component", "Green:"), d->whiteBalanceSettings);
    d->customWhiteBalanceGreenSpinBox->setWhatsThis(xi18nc("@info:whatsthis", "<para>Set here the "
                                "green component to set magenta color cast removal level.</para>"));

    d->unclipColorLabel    = new QLabel(i18nc("@label:listbox", "Highlights:"), d->whiteBalanceSettings);
    d->unclipColorComboBox = new DComboBox(d->whiteBalanceSettings);
    d->unclipColorComboBox->insertItem(0, i18nc("@item:inlistbox", "Solid white"));
    d->unclipColorComboBox->insertItem(1, i18nc("@item:inlistbox", "Unclip"));
    d->unclipColorComboBox->insertItem(2, i18nc("@item:inlistbox", "Blend"));
    d->unclipColorComboBox->insertItem(3, i18nc("@item:inlistbox", "Rebuild"));
    d->unclipColorComboBox->setDefaultIndex(0);
    d->unclipColorComboBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>Highlights</title>"
                                "<para>Select here the highlight clipping method:</para>"
                                "<para><list><item><emphasis strong='true'>Solid white</emphasis>: "
                                "clip all highlights to solid white</item>"
                                "<item><emphasis strong='true'>Unclip</emphasis>: leave highlights "
                                "unclipped in various shades of pink</item>"
                                "<item><emphasis strong='true'>Blend</emphasis>:Blend clipped and "
                                "unclipped values together for a gradual fade to white</item>"
                                "<item><emphasis strong='true'>Rebuild</emphasis>: reconstruct "
                                "highlights using a level value</item></list></para>"));

    d->reconstructLabel   = new QLabel(i18nc("@label:slider Highlight reconstruct level", "Level:"), d->whiteBalanceSettings);
    d->reconstructSpinBox = new DIntNumInput(d->whiteBalanceSettings);
    d->reconstructSpinBox->setRange(0, 6, 1);
    d->reconstructSpinBox->setDefaultValue(0);
    d->reconstructSpinBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>Level</title>"
                                "<para>Specify the reconstruct highlight level. Low values favor "
                                "whites and high values favor colors.</para>"));

    d->expoCorrectionBox = new QCheckBox(i18nc("@option:check", "Exposure Correction (E.V)"), d->whiteBalanceSettings);
    d->expoCorrectionBox->setWhatsThis(xi18nc("@info:whatsthis", "<para>Turn on the exposure "
                                "correction before interpolation.</para>"));

    d->expoCorrectionShiftLabel   = new QLabel(i18nc("@label:slider", "Linear Shift:"), d->whiteBalanceSettings);
    d->expoCorrectionShiftSpinBox = new DDoubleNumInput(d->whiteBalanceSettings);
    d->expoCorrectionShiftSpinBox->setDecimals(2);
    d->expoCorrectionShiftSpinBox->setRange(-2.0, 3.0, 0.01);
    d->expoCorrectionShiftSpinBox->setDefaultValue(0.0);
    d->expoCorrectionShiftSpinBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>Shift</title>"
                                "<para>Linear Shift of exposure correction before interpolation in E.V</para>"));

    d->expoCorrectionHighlightLabel   = new QLabel(i18nc("@label:slider", "Highlight:"), d->whiteBalanceSettings);
    d->expoCorrectionHighlightSpinBox = new DDoubleNumInput(d->whiteBalanceSettings);
    d->expoCorrectionHighlightSpinBox->setDecimals(2);
    d->expoCorrectionHighlightSpinBox->setRange(0.0, 1.0, 0.01);
    d->expoCorrectionHighlightSpinBox->setDefaultValue(0.0);
    d->expoCorrectionHighlightSpinBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>Highlight</title>"
                                "<para>Amount of highlight preservation for exposure correction "
                                "before interpolation in E.V. Only take effect if Shift Correction is > 1.0 E.V</para>"));

    d->fixColorsHighlightsBox = new QCheckBox(i18nc("@option:check", "Correct false colors in highlights"), d->whiteBalanceSettings);
    d->fixColorsHighlightsBox->setWhatsThis(xi18nc("@info:whatsthis", "<para>If enabled, images with "
                                "overblown channels are processed much more accurately, without "
                                "'pink clouds' (and blue highlights under tungsten lamps).</para>"));

    d->autoBrightnessBox = new QCheckBox(i18nc("@option:check", "Auto Brightness"), d->whiteBalanceSettings);
    d->autoBrightnessBox->setWhatsThis(xi18nc("@info:whatsthis", "<para>If disable, use a fixed white level "
                                "and ignore the image histogram to adjust brightness.</para>"));

    d->brightnessLabel   = new QLabel(i18nc("@label:slider", "Brightness:"), d->whiteBalanceSettings);
    d->brightnessSpinBox = new DDoubleNumInput(d->whiteBalanceSettings);
    d->brightnessSpinBox->setDecimals(2);
    d->brightnessSpinBox->setRange(0.0, 10.0, 0.01);
    d->brightnessSpinBox->setDefaultValue(1.0);
    d->brightnessSpinBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>Brightness</title>"
                                "<para>Specify the brightness level of output image. The default "
                                "value is 1.0 (works in 8-bit mode only).</para>"));

    if (! (advSettings & POSTPROCESSING))
    {
        d->brightnessLabel->hide();
        d->brightnessSpinBox->hide();
    }

    d->blackPointCheckBox = new QCheckBox(i18nc("@option:check Black point", "Black:"), d->whiteBalanceSettings);
    d->blackPointCheckBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>Black point</title>"
                                "<para>Use a specific black point value to decode RAW pictures. If "
                                "you set this option to off, the Black Point value will be "
                                "automatically computed.</para>"));
    d->blackPointSpinBox = new DIntNumInput(d->whiteBalanceSettings);
    d->blackPointSpinBox->setRange(0, 1000, 1);
    d->blackPointSpinBox->setDefaultValue(0);
    d->blackPointSpinBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>Black point value</title>"
                                "<para>Specify specific black point value of the output image.</para>"));

    d->whitePointCheckBox = new QCheckBox(i18nc("@option:check White point", "White:"), d->whiteBalanceSettings);
    d->whitePointCheckBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>White point</title>"
                                "<para>Use a specific white point value to decode RAW pictures. If "
                                "you set this option to off, the White Point value will be "
                                "automatically computed.</para>"));
    d->whitePointSpinBox = new DIntNumInput(d->whiteBalanceSettings);
    d->whitePointSpinBox->setRange(0, 20000, 1);
    d->whitePointSpinBox->setDefaultValue(0);
    d->whitePointSpinBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>White point value</title>"
                                "<para>Specify specific white point value of the output image.</para>"));

    if (! (advSettings & BLACKWHITEPOINTS))
    {
        d->blackPointCheckBox->hide();
        d->blackPointSpinBox->hide();
        d->whitePointCheckBox->hide();
        d->whitePointSpinBox->hide();
    }

    const int spacing = qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                             QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));

    whiteBalanceLayout->addWidget(d->whiteBalanceLabel,              0,  0, 1, 1);
    whiteBalanceLayout->addWidget(d->whiteBalanceComboBox,           0,  1, 1, 2);
    whiteBalanceLayout->addWidget(d->customWhiteBalanceLabel,        1,  0, 1, 1);
    whiteBalanceLayout->addWidget(d->customWhiteBalanceSpinBox,      1,  1, 1, 2);
    whiteBalanceLayout->addWidget(d->customWhiteBalanceGreenLabel,   2,  0, 1, 1);
    whiteBalanceLayout->addWidget(d->customWhiteBalanceGreenSpinBox, 2,  1, 1, 2);
    whiteBalanceLayout->addWidget(d->unclipColorLabel,               3,  0, 1, 1);
    whiteBalanceLayout->addWidget(d->unclipColorComboBox,            3,  1, 1, 2);
    whiteBalanceLayout->addWidget(d->reconstructLabel,               4,  0, 1, 1);
    whiteBalanceLayout->addWidget(d->reconstructSpinBox,             4,  1, 1, 2);
    whiteBalanceLayout->addWidget(d->expoCorrectionBox,              5,  0, 1, 2);
    whiteBalanceLayout->addWidget(d->expoCorrectionShiftLabel,       6,  0, 1, 1);
    whiteBalanceLayout->addWidget(d->expoCorrectionShiftSpinBox,     6,  1, 1, 2);
    whiteBalanceLayout->addWidget(d->expoCorrectionHighlightLabel,   7,  0, 1, 1);
    whiteBalanceLayout->addWidget(d->expoCorrectionHighlightSpinBox, 7,  1, 1, 2);
    whiteBalanceLayout->addWidget(d->fixColorsHighlightsBox,         8,  0, 1, 3);
    whiteBalanceLayout->addWidget(d->autoBrightnessBox,              9,  0, 1, 3);
    whiteBalanceLayout->addWidget(d->brightnessLabel,                10, 0, 1, 1);
    whiteBalanceLayout->addWidget(d->brightnessSpinBox,              10, 1, 1, 2);
    whiteBalanceLayout->addWidget(d->blackPointCheckBox,             11, 0, 1, 1);
    whiteBalanceLayout->addWidget(d->blackPointSpinBox,              11, 1, 1, 2);
    whiteBalanceLayout->addWidget(d->whitePointCheckBox,             12, 0, 1, 1);
    whiteBalanceLayout->addWidget(d->whitePointSpinBox,              12, 1, 1, 2);
    whiteBalanceLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    whiteBalanceLayout->setSpacing(spacing);

    addItem(d->whiteBalanceSettings, QIcon::fromTheme(QLatin1String("bordertool")).pixmap(16, 16), i18nc("@label", "White Balance"), QLatin1String("whitebalance"), true);

    // ---------------------------------------------------------------
    // CORRECTIONS Settings panel

    d->correctionsSettings               = new QWidget(this);
    QGridLayout* const correctionsLayout = new QGridLayout(d->correctionsSettings);

    d->noiseReductionLabel    = new QLabel(i18nc("@label:listbox", "Noise reduction:"), d->correctionsSettings);
    d->noiseReductionComboBox = new DComboBox(d->colormanSettings);
    d->noiseReductionComboBox->insertItem(DRawDecoderSettings::NONR,       i18nc("@item:inlistbox Noise Reduction", "None"));
    d->noiseReductionComboBox->insertItem(DRawDecoderSettings::WAVELETSNR, i18nc("@item:inlistbox Noise Reduction", "Wavelets"));
    d->noiseReductionComboBox->insertItem(DRawDecoderSettings::FBDDNR,     i18nc("@item:inlistbox Noise Reduction", "FBDD"));
    d->noiseReductionComboBox->setDefaultIndex(DRawDecoderSettings::NONR);
    d->noiseReductionComboBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>Noise Reduction</title>"
                                "<para>Select here the noise reduction method to apply during RAW "
                                "decoding.</para>"
                                "<para><list><item><emphasis strong='true'>None</emphasis>: no "
                                "noise reduction.</item>"
                                "<item><emphasis strong='true'>Wavelets</emphasis>: wavelets "
                                "correction to erase noise while preserving real detail. It is "
                                "applied after interpolation.</item>"
                                "<item><emphasis strong='true'>FBDD</emphasis>: Fake Before "
                                "Demosaicing Denoising noise reduction. It is applied before "
                                "interpolation.</item></list></para>"));

    d->NRSpinBox1 = new DIntNumInput(d->correctionsSettings);
    d->NRSpinBox1->setRange(100, 1000, 1);
    d->NRSpinBox1->setDefaultValue(100);
    d->NRLabel1   = new QLabel(d->correctionsSettings);

    correctionsLayout->addWidget(d->noiseReductionLabel,    0, 0, 1, 1);
    correctionsLayout->addWidget(d->noiseReductionComboBox, 0, 1, 1, 2);
    correctionsLayout->addWidget(d->NRLabel1,               1, 0, 1, 1);
    correctionsLayout->addWidget(d->NRSpinBox1,             1, 1, 1, 2);
    correctionsLayout->setRowStretch(2, 10);
    correctionsLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    correctionsLayout->setSpacing(spacing);

    addItem(d->correctionsSettings, QIcon::fromTheme(QLatin1String("document-edit")).pixmap(16, 16), i18nc("@label", "Corrections"), QLatin1String("corrections"), false);

    // ---------------------------------------------------------------
    // COLOR MANAGEMENT Settings panel

    d->colormanSettings               = new QWidget(this);
    QGridLayout* const colormanLayout = new QGridLayout(d->colormanSettings);

    d->inputColorSpaceLabel     = new QLabel(i18nc("@label:listbox", "Camera Profile:"), d->colormanSettings);
    d->inputColorSpaceComboBox  = new DComboBox(d->colormanSettings);
    d->inputColorSpaceComboBox->insertItem(DRawDecoderSettings::NOINPUTCS,     i18nc("@item:inlistbox Camera Profile", "None"));
    d->inputColorSpaceComboBox->insertItem(DRawDecoderSettings::EMBEDDED,      i18nc("@item:inlistbox Camera Profile", "Embedded"));
    d->inputColorSpaceComboBox->insertItem(DRawDecoderSettings::CUSTOMINPUTCS, i18nc("@item:inlistbox Camera Profile", "Custom"));
    d->inputColorSpaceComboBox->setDefaultIndex(DRawDecoderSettings::NOINPUTCS);
    d->inputColorSpaceComboBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>Camera Profile</title>"
                                "<para>Select here the input color space used to decode RAW data.</para>"
                                "<para><list><item><emphasis strong='true'>None</emphasis>: no "
                                "input color profile is used during RAW decoding.</item>"
                                "<item><emphasis strong='true'>Embedded</emphasis>: use embedded "
                                "color profile from RAW file, if it exists.</item>"
                                "<item><emphasis strong='true'>Custom</emphasis>: use a custom "
                                "input color space profile.</item></list></para>"));

    d->inIccUrlEdit = new DFileSelector(d->colormanSettings);
    d->inIccUrlEdit->setFileDlgMode(QFileDialog::ExistingFile);
    d->inIccUrlEdit->setFileDlgFilter(i18nc("@info: open file filters", "ICC Files (*.icc *.icm)"));

    d->outputColorSpaceLabel    = new QLabel(i18nc("@label:listbox", "Workspace:"), d->colormanSettings);
    d->outputColorSpaceComboBox = new DComboBox( d->colormanSettings );
    d->outputColorSpaceComboBox->insertItem(DRawDecoderSettings::RAWCOLOR,       i18nc("@item:inlistbox Workspace", "Raw (no profile)"));
    d->outputColorSpaceComboBox->insertItem(DRawDecoderSettings::SRGB,           i18nc("@item:inlistbox Workspace", "sRGB"));
    d->outputColorSpaceComboBox->insertItem(DRawDecoderSettings::ADOBERGB,       i18nc("@item:inlistbox Workspace", "Adobe RGB"));
    d->outputColorSpaceComboBox->insertItem(DRawDecoderSettings::WIDEGAMMUT,     i18nc("@item:inlistbox Workspace", "Wide Gamut"));
    d->outputColorSpaceComboBox->insertItem(DRawDecoderSettings::PROPHOTO,       i18nc("@item:inlistbox Workspace", "Pro-Photo"));
    d->outputColorSpaceComboBox->insertItem(DRawDecoderSettings::CUSTOMOUTPUTCS, i18nc("@item:inlistbox Workspace", "Custom"));
    d->outputColorSpaceComboBox->setDefaultIndex(DRawDecoderSettings::SRGB);
    d->outputColorSpaceComboBox->setWhatsThis(xi18nc("@info:whatsthis", "<title>Workspace</title>"
                                "<para>Select here the output color space used to decode RAW data.</para>"
                                "<para><list><item><emphasis strong='true'>Raw (linear)</emphasis>: "
                                "in this mode, no output color space is used during RAW decoding.</item>"
                                "<item><emphasis strong='true'>sRGB</emphasis>: this is an RGB "
                                "color space, created cooperatively by Hewlett-Packard and "
                                "Microsoft. It is the best choice for images destined for the Web "
                                "and portrait photography.</item>"
                                "<item><emphasis strong='true'>Adobe RGB</emphasis>: this color "
                                "space is an extended RGB color space, developed by Adobe. It is "
                                "used for photography applications such as advertising and fine "
                                "art.</item>"
                                "<item><emphasis strong='true'>Wide Gamut</emphasis>: this color "
                                "space is an expanded version of the Adobe RGB color space.</item>"
                                "<item><emphasis strong='true'>Pro-Photo</emphasis>: this color "
                                "space is an RGB color space, developed by Kodak, that offers an "
                                "especially large gamut designed for use with photographic outputs "
                                "in mind.</item>"
                                "<item><emphasis strong='true'>Custom</emphasis>: use a custom "
                                "output color space profile.</item></list></para>"));

    d->outIccUrlEdit = new DFileSelector(d->colormanSettings);
    d->outIccUrlEdit->setFileDlgMode(QFileDialog::ExistingFile);
    d->outIccUrlEdit->setFileDlgFilter(i18nc("@info: open file filters", "ICC Files (*.icc *.icm)"));

    colormanLayout->addWidget(d->inputColorSpaceLabel,     0, 0, 1, 1);
    colormanLayout->addWidget(d->inputColorSpaceComboBox,  0, 1, 1, 2);
    colormanLayout->addWidget(d->inIccUrlEdit,             1, 0, 1, 3);
    colormanLayout->addWidget(d->outputColorSpaceLabel,    2, 0, 1, 1);
    colormanLayout->addWidget(d->outputColorSpaceComboBox, 2, 1, 1, 2);
    colormanLayout->addWidget(d->outIccUrlEdit,            3, 0, 1, 3);
    colormanLayout->setRowStretch(4, 10);
    colormanLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    colormanLayout->setSpacing(spacing);

    addItem(d->colormanSettings, QIcon::fromTheme(QLatin1String("preferences-desktop-display-color")).pixmap(16, 16), i18nc("@label", "Color Management"), QLatin1String("colormanagement"), false);

    if (! (advSettings & COLORSPACE))
    {
        removeItem(COLORMANAGEMENT);
    }

    addStretch();

    // ---------------------------------------------------------------

    connect(d->unclipColorComboBox, static_cast<void (DComboBox::*)(int)>(&DComboBox::activated),
            this, &DRawDecoderWidget::slotUnclipColorActivated);

    connect(d->whiteBalanceComboBox, static_cast<void (DComboBox::*)(int)>(&DComboBox::activated),
            this, &DRawDecoderWidget::slotWhiteBalanceToggled);

    connect(d->noiseReductionComboBox, static_cast<void (DComboBox::*)(int)>(&DComboBox::activated),
            this, &DRawDecoderWidget::slotNoiseReductionChanged);

    connect(d->blackPointCheckBox, SIGNAL(toggled(bool)),
            d->blackPointSpinBox, SLOT(setEnabled(bool)));

    connect(d->whitePointCheckBox, SIGNAL(toggled(bool)),
            d->whitePointSpinBox, SLOT(setEnabled(bool)));

    connect(d->sixteenBitsImage, &QCheckBox::toggled,
            this, &DRawDecoderWidget::slotsixteenBitsImageToggled);

    connect(d->inputColorSpaceComboBox, static_cast<void (DComboBox::*)(int)>(&DComboBox::activated),
            this, &DRawDecoderWidget::slotInputColorSpaceChanged);

    connect(d->outputColorSpaceComboBox, static_cast<void (DComboBox::*)(int)>(&DComboBox::activated),
            this, &DRawDecoderWidget::slotOutputColorSpaceChanged);

    connect(d->expoCorrectionBox, &QCheckBox::toggled,
            this, &DRawDecoderWidget::slotExposureCorrectionToggled);

    connect(d->expoCorrectionShiftSpinBox, &DDoubleNumInput::valueChanged,
            this, &DRawDecoderWidget::slotExpoCorrectionShiftChanged);

    // Wrapper to Q_EMIT signal when something is changed in settings.

    connect(d->inIccUrlEdit->lineEdit(), &QLineEdit::textChanged,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->outIccUrlEdit->lineEdit(), &QLineEdit::textChanged,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->whiteBalanceComboBox, static_cast<void (DComboBox::*)(int)>(&DComboBox::activated),
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->RAWQualityComboBox, static_cast<void (DComboBox::*)(int)>(&DComboBox::activated),
            this, &DRawDecoderWidget::slotRAWQualityChanged);

    connect(d->unclipColorComboBox, static_cast<void (DComboBox::*)(int)>(&DComboBox::activated),
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->inputColorSpaceComboBox, static_cast<void (DComboBox::*)(int)>(&DComboBox::activated),
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->outputColorSpaceComboBox, static_cast<void (DComboBox::*)(int)>(&DComboBox::activated),
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->blackPointCheckBox, &QCheckBox::toggled,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->whitePointCheckBox, &QCheckBox::toggled,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->sixteenBitsImage, &QCheckBox::toggled,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->fixColorsHighlightsBox, &QCheckBox::toggled,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->autoBrightnessBox, &QCheckBox::toggled,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->fourColorCheckBox, &QCheckBox::toggled,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->dontStretchPixelsCheckBox, &QCheckBox::toggled,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->refineInterpolationBox, &QCheckBox::toggled,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->customWhiteBalanceSpinBox, &DIntNumInput::valueChanged,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->reconstructSpinBox, &DIntNumInput::valueChanged,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->blackPointSpinBox, &DIntNumInput::valueChanged,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->whitePointSpinBox, &DIntNumInput::valueChanged,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->NRSpinBox1, &DIntNumInput::valueChanged,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->medianFilterPassesSpinBox, &DIntNumInput::valueChanged,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->customWhiteBalanceGreenSpinBox, &DDoubleNumInput::valueChanged,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->brightnessSpinBox, &DDoubleNumInput::valueChanged,
            this, &DRawDecoderWidget::signalSettingsChanged);

    connect(d->expoCorrectionHighlightSpinBox, &DDoubleNumInput::valueChanged,
            this, &DRawDecoderWidget::signalSettingsChanged);
}

DRawDecoderWidget::~DRawDecoderWidget()
{
    delete d;
}

void DRawDecoderWidget::updateMinimumWidth()
{
    int width = 0;

    for (int i = 0 ; i < count() ; ++i)
    {
        if (widget(i)->width() > width)
        {
            width = widget(i)->width();
        }
    }

    setMinimumWidth(width);
}

DFileSelector* DRawDecoderWidget::inputProfileUrlEdit() const
{
    return d->inIccUrlEdit;
}

DFileSelector* DRawDecoderWidget::outputProfileUrlEdit() const
{
    return d->outIccUrlEdit;
}

void DRawDecoderWidget::resetToDefault()
{
    setSettings(DRawDecoderSettings());
}

void DRawDecoderWidget::slotsixteenBitsImageToggled(bool b)
{
    setEnabledBrightnessSettings(!b);
    Q_EMIT signalSixteenBitsImageToggled(d->sixteenBitsImage->isChecked());
}

void DRawDecoderWidget::slotWhiteBalanceToggled(int v)
{
    if (v == 3)
    {
        d->customWhiteBalanceSpinBox->setEnabled(true);
        d->customWhiteBalanceGreenSpinBox->setEnabled(true);
        d->customWhiteBalanceLabel->setEnabled(true);
        d->customWhiteBalanceGreenLabel->setEnabled(true);
    }
    else
    {
        d->customWhiteBalanceSpinBox->setEnabled(false);
        d->customWhiteBalanceGreenSpinBox->setEnabled(false);
        d->customWhiteBalanceLabel->setEnabled(false);
        d->customWhiteBalanceGreenLabel->setEnabled(false);
    }
}

void DRawDecoderWidget::slotUnclipColorActivated(int v)
{
    if (v == 3)     // Reconstruct Highlight method
    {
        d->reconstructLabel->setEnabled(true);
        d->reconstructSpinBox->setEnabled(true);
    }
    else
    {
        d->reconstructLabel->setEnabled(false);
        d->reconstructSpinBox->setEnabled(false);
    }
}

void DRawDecoderWidget::slotNoiseReductionChanged(int item)
{
    d->NRSpinBox1->setEnabled(true);
    d->NRLabel1->setEnabled(true);
    d->NRLabel1->setText(i18nc("@label", "Threshold:"));
    d->NRSpinBox1->setWhatsThis(xi18nc("@info:whatsthis", "<title>Threshold</title>"
                                "<para>Set here the noise reduction threshold value to use.</para>"));

    switch (item)
    {
        case DRawDecoderSettings::WAVELETSNR:
        case DRawDecoderSettings::FBDDNR:
        {
            // NOTE : no ops
            break;
        }

        default:
        {
            d->NRSpinBox1->setEnabled(false);
            d->NRLabel1->setEnabled(false);
            break;
        }
    }

    Q_EMIT signalSettingsChanged();
}

void DRawDecoderWidget::slotExposureCorrectionToggled(bool b)
{
    d->expoCorrectionShiftLabel->setEnabled(b);
    d->expoCorrectionShiftSpinBox->setEnabled(b);
    d->expoCorrectionHighlightLabel->setEnabled(b);
    d->expoCorrectionHighlightSpinBox->setEnabled(b);

    slotExpoCorrectionShiftChanged(d->expoCorrectionShiftSpinBox->value());
}

void DRawDecoderWidget::slotExpoCorrectionShiftChanged(double ev)
{
    // Only enable Highlight exposure correction if Shift correction is >= 1.0, else this settings do not take effect.

    bool b = (ev >= 1.0);

    d->expoCorrectionHighlightLabel->setEnabled(b);
    d->expoCorrectionHighlightSpinBox->setEnabled(b);

    Q_EMIT signalSettingsChanged();
}

void DRawDecoderWidget::slotInputColorSpaceChanged(int item)
{
    d->inIccUrlEdit->setEnabled(item == DRawDecoderSettings::CUSTOMINPUTCS);
}

void DRawDecoderWidget::slotOutputColorSpaceChanged(int item)
{
    d->outIccUrlEdit->setEnabled(item == DRawDecoderSettings::CUSTOMOUTPUTCS);
}

void DRawDecoderWidget::slotRAWQualityChanged(int quality)
{
    switch (quality)
    {
        case DRawDecoderSettings::DCB:
        {
            d->medianFilterPassesLabel->setEnabled(true);
            d->medianFilterPassesSpinBox->setEnabled(true);
            d->refineInterpolationBox->setEnabled(true);
            break;
        }

        default: // BILINEAR, VNG, PPG, AHD
        {
            d->medianFilterPassesLabel->setEnabled(true);
            d->medianFilterPassesSpinBox->setEnabled(true);
            d->refineInterpolationBox->setEnabled(false);
            break;
        }
    }

    Q_EMIT signalSettingsChanged();
}

void DRawDecoderWidget::setEnabledBrightnessSettings(bool b)
{
    d->brightnessLabel->setEnabled(b);
    d->brightnessSpinBox->setEnabled(b);
}

bool DRawDecoderWidget::brightnessSettingsIsEnabled() const
{
    return d->brightnessSpinBox->isEnabled();
}

void DRawDecoderWidget::setSettings(const DRawDecoderSettings& settings)
{
    d->sixteenBitsImage->setChecked(settings.sixteenBitsImage);

    switch (settings.whiteBalance)
    {
        case DRawDecoderSettings::CAMERA:
        {
            d->whiteBalanceComboBox->setCurrentIndex(1);
            break;
        }

        case DRawDecoderSettings::AUTO:
        {
            d->whiteBalanceComboBox->setCurrentIndex(2);
            break;
        }

        case DRawDecoderSettings::CUSTOM:
        {
            d->whiteBalanceComboBox->setCurrentIndex(3);
            break;
        }

        default:
        {
            d->whiteBalanceComboBox->setCurrentIndex(0);
            break;
        }
    }

    slotWhiteBalanceToggled(d->whiteBalanceComboBox->currentIndex());

    d->customWhiteBalanceSpinBox->setValue(settings.customWhiteBalance);
    d->customWhiteBalanceGreenSpinBox->setValue(settings.customWhiteBalanceGreen);
    d->fourColorCheckBox->setChecked(settings.RGBInterpolate4Colors);
    d->autoBrightnessBox->setChecked(settings.autoBrightness);
    d->fixColorsHighlightsBox->setChecked(settings.fixColorsHighlights);

    switch (settings.unclipColors)
    {
        case 0:
        {
            d->unclipColorComboBox->setCurrentIndex(0);
            break;
        }

        case 1:
        {
            d->unclipColorComboBox->setCurrentIndex(1);
            break;
        }

        case 2:
        {
            d->unclipColorComboBox->setCurrentIndex(2);
            break;
        }

        default:         // Reconstruct Highlight method
        {
            d->unclipColorComboBox->setCurrentIndex(3);
            d->reconstructSpinBox->setValue(settings.unclipColors-3);
            break;
        }
    }

    slotUnclipColorActivated(d->unclipColorComboBox->currentIndex());

    d->dontStretchPixelsCheckBox->setChecked(settings.DontStretchPixels);
    d->brightnessSpinBox->setValue(settings.brightness);
    d->blackPointCheckBox->setChecked(settings.enableBlackPoint);
    d->blackPointSpinBox->setEnabled(settings.enableBlackPoint);
    d->blackPointSpinBox->setValue(settings.blackPoint);
    d->whitePointCheckBox->setChecked(settings.enableWhitePoint);
    d->whitePointSpinBox->setEnabled(settings.enableWhitePoint);
    d->whitePointSpinBox->setValue(settings.whitePoint);

    int q = d->RAWQualityComboBox->combo()->findData(settings.RAWQuality);

    if (q == -1)
    {
        q = 0;      // Fail back to bilinear
    }

    d->RAWQualityComboBox->setCurrentIndex(q);

    switch (q)
    {
        case DRawDecoderSettings::DCB:
        {
            d->medianFilterPassesSpinBox->setValue(settings.dcbIterations);
            d->refineInterpolationBox->setChecked(settings.dcbEnhanceFl);
            break;
        }

        default:
        {
            d->medianFilterPassesSpinBox->setValue(settings.medianFilterPasses);
            d->refineInterpolationBox->setChecked(false); // option not used.
            break;
        }
    }

    slotRAWQualityChanged(q);

    d->inputColorSpaceComboBox->setCurrentIndex((int)settings.inputColorSpace);
    slotInputColorSpaceChanged((int)settings.inputColorSpace);
    d->outputColorSpaceComboBox->setCurrentIndex((int)settings.outputColorSpace);
    slotOutputColorSpaceChanged((int)settings.outputColorSpace);

    d->noiseReductionComboBox->setCurrentIndex(settings.NRType);
    slotNoiseReductionChanged(settings.NRType);
    d->NRSpinBox1->setValue(settings.NRThreshold);

    d->expoCorrectionBox->setChecked(settings.expoCorrection);
    slotExposureCorrectionToggled(settings.expoCorrection);
    d->expoCorrectionShiftSpinBox->setValue(d->shiftExpoFromLinearToEv(settings.expoCorrectionShift));
    d->expoCorrectionHighlightSpinBox->setValue(settings.expoCorrectionHighlight);

    d->inIccUrlEdit->setFileDlgPath(settings.inputProfile);
    d->outIccUrlEdit->setFileDlgPath(settings.outputProfile);
}

DRawDecoderSettings DRawDecoderWidget::settings() const
{
    DRawDecoderSettings prm;
    prm.sixteenBitsImage = d->sixteenBitsImage->isChecked();

    switch (d->whiteBalanceComboBox->currentIndex())
    {
        case 1:
        {
            prm.whiteBalance = DRawDecoderSettings::CAMERA;
            break;
        }

        case 2:
        {
            prm.whiteBalance = DRawDecoderSettings::AUTO;
            break;
        }

        case 3:
        {
            prm.whiteBalance = DRawDecoderSettings::CUSTOM;
            break;
        }

        default:
        {
            prm.whiteBalance = DRawDecoderSettings::NONE;
            break;
        }
    }

    prm.customWhiteBalance      = d->customWhiteBalanceSpinBox->value();
    prm.customWhiteBalanceGreen = d->customWhiteBalanceGreenSpinBox->value();
    prm.RGBInterpolate4Colors   = d->fourColorCheckBox->isChecked();
    prm.autoBrightness          = d->autoBrightnessBox->isChecked();
    prm.fixColorsHighlights     = d->fixColorsHighlightsBox->isChecked();

    switch (d->unclipColorComboBox->currentIndex())
    {
        case 0:
        {
            prm.unclipColors = 0;
            break;
        }

        case 1:
        {
            prm.unclipColors = 1;
            break;
        }

        case 2:
        {
            prm.unclipColors = 2;
            break;
        }

        default:         // Reconstruct Highlight method
        {
            prm.unclipColors =  d->reconstructSpinBox->value()+3;
            break;
        }
    }

    prm.DontStretchPixels    = d->dontStretchPixelsCheckBox->isChecked();
    prm.brightness           = d->brightnessSpinBox->value();
    prm.enableBlackPoint     = d->blackPointCheckBox->isChecked();
    prm.blackPoint           = d->blackPointSpinBox->value();
    prm.enableWhitePoint     = d->whitePointCheckBox->isChecked();
    prm.whitePoint           = d->whitePointSpinBox->value();

    prm.RAWQuality           = (DRawDecoderSettings::DecodingQuality)d->RAWQualityComboBox->combo()->currentData().toInt();

    switch (prm.RAWQuality)
    {
        case DRawDecoderSettings::DCB:
        {
            prm.dcbIterations      = d->medianFilterPassesSpinBox->value();
            prm.dcbEnhanceFl       = d->refineInterpolationBox->isChecked();
            break;
        }

        default:
        {
            prm.medianFilterPasses = d->medianFilterPassesSpinBox->value();
            break;
        }
    }

    prm.NRType = (DRawDecoderSettings::NoiseReduction)d->noiseReductionComboBox->currentIndex();

    switch (prm.NRType)
    {
        case DRawDecoderSettings::NONR:
        {
            prm.NRThreshold     = 0;
            break;
        }

        default:    // WAVELETSNR and FBDDNR
        {
            prm.NRThreshold     = d->NRSpinBox1->value();
            break;
        }
    }

    prm.expoCorrection          = d->expoCorrectionBox->isChecked();
    prm.expoCorrectionShift     = d->shiftExpoFromEvToLinear(d->expoCorrectionShiftSpinBox->value());
    prm.expoCorrectionHighlight = d->expoCorrectionHighlightSpinBox->value();

    prm.inputColorSpace         = (DRawDecoderSettings::InputColorSpace)(d->inputColorSpaceComboBox->currentIndex());
    prm.outputColorSpace        = (DRawDecoderSettings::OutputColorSpace)(d->outputColorSpaceComboBox->currentIndex());
    prm.inputProfile            = d->inIccUrlEdit->fileDlgPath();
    prm.outputProfile           = d->outIccUrlEdit->fileDlgPath();

    return prm;
}

void DRawDecoderWidget::readSettings(KConfigGroup& group)
{
    DRawDecoderSettings prm;
    readSettings(prm, group);

    setSettings(prm);
    DExpanderBox::readSettings(group);
}

void DRawDecoderWidget::writeSettings(KConfigGroup& group)
{
    DRawDecoderSettings prm = settings();
    writeSettings(prm, group);

    DExpanderBox::writeSettings(group);
}

void DRawDecoderWidget::readSettings(DRawDecoderSettings& prm, KConfigGroup& group)
{
    DRawDecoderSettings defaultPrm;

    prm.fixColorsHighlights     = group.readEntry(OPTIONFIXCOLORSHIGHLIGHTSENTRY,                                     defaultPrm.fixColorsHighlights);
    prm.sixteenBitsImage        = group.readEntry(OPTIONDECODESIXTEENBITENTRY,                                        defaultPrm.sixteenBitsImage);
    prm.whiteBalance            = (DRawDecoderSettings::WhiteBalance)group.readEntry(OPTIONWHITEBALANCEENTRY,         (int)defaultPrm.whiteBalance);
    prm.customWhiteBalance      = group.readEntry(OPTIONCUSTOMWHITEBALANCEENTRY,                                      defaultPrm.customWhiteBalance);
    prm.customWhiteBalanceGreen = group.readEntry(OPTIONCUSTOMWBGREENENTRY,                                           defaultPrm.customWhiteBalanceGreen);
    prm.RGBInterpolate4Colors   = group.readEntry(OPTIONFOURCOLORRGBENTRY,                                            defaultPrm.RGBInterpolate4Colors);
    prm.unclipColors            = group.readEntry(OPTIONUNCLIPCOLORSENTRY,                                            defaultPrm.unclipColors);
    prm.DontStretchPixels       = group.readEntry(OPTIONDONTSTRETCHPIXELSSENTRY,                                       defaultPrm.DontStretchPixels);
    prm.NRType                  = (DRawDecoderSettings::NoiseReduction)group.readEntry(OPTIONNOISEREDUCTIONTYPEENTRY, (int)defaultPrm.NRType);
    prm.brightness              = group.readEntry(OPTIONBRIGHTNESSMULTIPLIERENTRY,                                    defaultPrm.brightness);
    prm.enableBlackPoint        = group.readEntry(OPTIONUSEBLACKPOINTENTRY,                                           defaultPrm.enableBlackPoint);
    prm.blackPoint              = group.readEntry(OPTIONBLACKPOINTENTRY,                                              defaultPrm.blackPoint);
    prm.enableWhitePoint        = group.readEntry(OPTIONUSEWHITEPOINTENTRY,                                           defaultPrm.enableWhitePoint);
    prm.whitePoint              = group.readEntry(OPTIONWHITEPOINTENTRY,                                              defaultPrm.whitePoint);
    prm.medianFilterPasses      = group.readEntry(OPTIONMEDIANFILTERPASSESENTRY,                                      defaultPrm.medianFilterPasses);
    prm.NRThreshold             = group.readEntry(OPTIONNOISEREDUCTIONTHRESHOLDENTRY,                                 defaultPrm.NRThreshold);
    prm.RAWQuality              = (DRawDecoderSettings::DecodingQuality)group.readEntry(OPTIONDECODINGQUALITYENTRY,   (int)defaultPrm.RAWQuality);
    prm.outputColorSpace        = (DRawDecoderSettings::OutputColorSpace)group.readEntry(OPTIONOUTPUTCOLORSPACEENTRY, (int)defaultPrm.outputColorSpace);
    prm.autoBrightness          = group.readEntry(OPTIONAUTOBRIGHTNESSENTRY,                                          defaultPrm.autoBrightness);

    //-- Extended demosaicing settings ----------------------------------------------------------

    prm.dcbIterations           = group.readEntry(OPTIONDCBITERATIONSENTRY,                                           defaultPrm.dcbIterations);
    prm.dcbEnhanceFl            = group.readEntry(OPTIONDCBENHANCEFLENTRY,                                            defaultPrm.dcbEnhanceFl);
    prm.expoCorrection          = group.readEntry(OPTIONEXPOCORRECTIONENTRY,                                          defaultPrm.expoCorrection);
    prm.expoCorrectionShift     = group.readEntry(OPTIONEXPOCORRECTIONSHIFTENTRY,                                     defaultPrm.expoCorrectionShift);
    prm.expoCorrectionHighlight = group.readEntry(OPTIONEXPOCORRECTIONHIGHLIGHTENTRY,                                 defaultPrm.expoCorrectionHighlight);
}

void DRawDecoderWidget::writeSettings(const DRawDecoderSettings& prm, KConfigGroup& group)
{
    group.writeEntry(OPTIONFIXCOLORSHIGHLIGHTSENTRY,     prm.fixColorsHighlights);
    group.writeEntry(OPTIONDECODESIXTEENBITENTRY,        prm.sixteenBitsImage);
    group.writeEntry(OPTIONWHITEBALANCEENTRY,            (int)prm.whiteBalance);
    group.writeEntry(OPTIONCUSTOMWHITEBALANCEENTRY,      prm.customWhiteBalance);
    group.writeEntry(OPTIONCUSTOMWBGREENENTRY,           prm.customWhiteBalanceGreen);
    group.writeEntry(OPTIONFOURCOLORRGBENTRY,            prm.RGBInterpolate4Colors);
    group.writeEntry(OPTIONUNCLIPCOLORSENTRY,            prm.unclipColors);
    group.writeEntry(OPTIONDONTSTRETCHPIXELSSENTRY,       prm.DontStretchPixels);
    group.writeEntry(OPTIONNOISEREDUCTIONTYPEENTRY,      (int)prm.NRType);
    group.writeEntry(OPTIONBRIGHTNESSMULTIPLIERENTRY,    prm.brightness);
    group.writeEntry(OPTIONUSEBLACKPOINTENTRY,           prm.enableBlackPoint);
    group.writeEntry(OPTIONBLACKPOINTENTRY,              prm.blackPoint);
    group.writeEntry(OPTIONUSEWHITEPOINTENTRY,           prm.enableWhitePoint);
    group.writeEntry(OPTIONWHITEPOINTENTRY,              prm.whitePoint);
    group.writeEntry(OPTIONMEDIANFILTERPASSESENTRY,      prm.medianFilterPasses);
    group.writeEntry(OPTIONNOISEREDUCTIONTHRESHOLDENTRY, prm.NRThreshold);
    group.writeEntry(OPTIONDECODINGQUALITYENTRY,         (int)prm.RAWQuality);
    group.writeEntry(OPTIONOUTPUTCOLORSPACEENTRY,        (int)prm.outputColorSpace);
    group.writeEntry(OPTIONAUTOBRIGHTNESSENTRY,          prm.autoBrightness);

    //-- Extended demosaicing settings ----------------------------------------------------------

    group.writeEntry(OPTIONDCBITERATIONSENTRY,           prm.dcbIterations);
    group.writeEntry(OPTIONDCBENHANCEFLENTRY,            prm.dcbEnhanceFl);
    group.writeEntry(OPTIONEXPOCORRECTIONENTRY,          prm.expoCorrection);
    group.writeEntry(OPTIONEXPOCORRECTIONSHIFTENTRY,     prm.expoCorrectionShift);
    group.writeEntry(OPTIONEXPOCORRECTIONHIGHLIGHTENTRY, prm.expoCorrectionHighlight);
}

} // NameSpace Digikam
