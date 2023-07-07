/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-10-09
 * Description : internal private container for DRawDecoder
 *
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "drawdecoder_p.h"

// Qt includes

#include <QString>
#include <QFile>
#include <QScopedPointer>

// DNG SDK includes

#ifdef USE_DNGSDK
#   include "dng_host.h"
#endif

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "metaengine.h"

namespace Digikam
{

int s_progressCallbackForLibRaw(void* context, enum LibRaw_progress p, int iteration, int expected)
{
    if (context)
    {
        DRawDecoder::Private* const d = static_cast<DRawDecoder::Private*>(context);

        return d->progressCallback(p, iteration, expected);
    }

    return 0;
}

void s_exifParserCallbackForLibRaw(void* context, int tag, int type, int len, unsigned int ord, void* ifp, INT64 base)
{
    if (context)
    {
        DRawDecoder::Private* const d = static_cast<DRawDecoder::Private*>(context);
        d->exifParserCallback(tag, type, len, ord, ifp, base);
    }
}

// --------------------------------------------------------------------------------------------------

DRawDecoder::Private::Private(DRawDecoder* const p)
    : m_progress(0.0),
      m_parent  (p)
{
}

DRawDecoder::Private::~Private()
{
}

void DRawDecoder::Private::createPPMHeader(QByteArray& imgData, libraw_processed_image_t* const img)
{
    QString header = QString::fromUtf8("P%1\n%2 %3\n%4\n").arg((img->colors == 3) ? QLatin1String("6") : QLatin1String("5"))
                                                          .arg(img->width)
                                                          .arg(img->height)
                                                          .arg((1 << img->bits)-1);
    imgData.append(header.toLatin1());
    imgData.append(QByteArray((const char*)img->data, (int)img->data_size));
}

int DRawDecoder::Private::progressCallback(enum LibRaw_progress p, int iteration, int expected)
{
    qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw progress:" << libraw_strprogress(p) << "pass"
                                   << iteration << "of" << expected;

    // post a little change in progress indicator to show raw processor activity.

    setProgress(progressValue() + 0.01);

    // Clean processing termination by user...

    if (m_parent->checkToCancelWaitingData())
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw process terminaison invoked...";
        m_parent->m_cancel = true;
        m_progress         = 0.0;

        return 1;
    }

    // Return 0 to continue processing...

    return 0;
}

void DRawDecoder::Private::exifParserCallback(int tag, int type, int len, unsigned int ord, void* ifp, INT64 base)
{
    // Note: see https://github.com/LibRaw/LibRaw/issues/323 for details

    qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw Exif Parser:"
                                   << "tag:"  << tag
                                   << "type:" << type
                                   << "len:"  << len
                                   << "ord:"  << ord
                                   << "ifp:"  << ifp
                                   << "base:" << base;
}

void DRawDecoder::Private::setProgress(double value)
{
    m_progress = value;
    m_parent->setWaitingDataProgress(m_progress);
}

double DRawDecoder::Private::progressValue() const
{
    return m_progress;
}

void DRawDecoder::Private::fillIndentifyInfo(LibRaw* const raw, DRawInfo& identify)
{
    identify.dateTime.setSecsSinceEpoch(raw->imgdata.other.timestamp);
    identify.make                  = QString::fromUtf8(raw->imgdata.idata.make);
    identify.model                 = QString::fromUtf8(raw->imgdata.idata.model);
    identify.owner                 = QString::fromUtf8(raw->imgdata.other.artist);
    identify.software              = QString::fromUtf8(raw->imgdata.idata.software);
    identify.firmware              = QString::fromUtf8(raw->imgdata.color.model2);
    identify.description           = QString::fromUtf8(raw->imgdata.other.desc);
    identify.serialNumber          = raw->imgdata.other.shot_order;
    identify.DNGVersion            = QString::number(raw->imgdata.idata.dng_version);
    identify.uniqueCameraModel     = QString::fromUtf8(raw->imgdata.color.UniqueCameraModel);
    identify.localizedCameraModel  = QString::fromUtf8(raw->imgdata.color.LocalizedCameraModel);
    identify.imageID               = QString::fromUtf8(raw->imgdata.color.ImageUniqueID);
    identify.rawDataUniqueID       = QString::fromUtf8(raw->imgdata.color.RawDataUniqueID);
    identify.originalRawFileName   = QString::fromUtf8(raw->imgdata.color.RawDataUniqueID);
    identify.model                 = QString::fromUtf8(raw->imgdata.idata.model);
    identify.sensitivity           = raw->imgdata.other.iso_speed;
    identify.exposureTime          = raw->imgdata.other.shutter;
    identify.aperture              = raw->imgdata.other.aperture;
    identify.focalLength           = raw->imgdata.other.focal_len;

    identify.hasGpsInfo            = (raw->imgdata.other.parsed_gps.gpsparsed == 1) ? true : false;
    identify.latitude              = MetaEngine::convertDegreeAngleToDouble(raw->imgdata.other.parsed_gps.latitude[0],
                                                                            raw->imgdata.other.parsed_gps.latitude[1],
                                                                            raw->imgdata.other.parsed_gps.latitude[2]);
    identify.longitude             = MetaEngine::convertDegreeAngleToDouble(raw->imgdata.other.parsed_gps.longitude[0],
                                                                            raw->imgdata.other.parsed_gps.longitude[1],
                                                                            raw->imgdata.other.parsed_gps.longitude[2]);
    identify.altitude              = raw->imgdata.other.parsed_gps.altitude;
    identify.imageSize             = QSize(raw->imgdata.sizes.width,      raw->imgdata.sizes.height);
    identify.fullSize              = QSize(raw->imgdata.sizes.raw_width,  raw->imgdata.sizes.raw_height);
    identify.outputSize            = QSize(raw->imgdata.sizes.iwidth,     raw->imgdata.sizes.iheight);
    identify.thumbSize             = QSize(raw->imgdata.thumbnail.twidth, raw->imgdata.thumbnail.theight);
    identify.topMargin             = raw->imgdata.sizes.top_margin;
    identify.leftMargin            = raw->imgdata.sizes.left_margin;
    identify.hasIccProfile         = raw->imgdata.color.profile ? true : false;
    identify.isDecodable           = true;
    identify.pixelAspectRatio      = raw->imgdata.sizes.pixel_aspect;
    identify.baselineExposure      = raw->imgdata.color.dng_levels.baseline_exposure;

    identify.ambientTemperature    = raw->imgdata.makernotes.common.exifAmbientTemperature;
    identify.ambientHumidity       = raw->imgdata.makernotes.common.exifHumidity;
    identify.ambientPressure       = raw->imgdata.makernotes.common.exifPressure;
    identify.ambientWaterDepth     = raw->imgdata.makernotes.common.exifWaterDepth;
    identify.ambientAcceleration   = raw->imgdata.makernotes.common.exifAcceleration;
    identify.ambientElevationAngle = raw->imgdata.makernotes.common.exifCameraElevationAngle;

    identify.exposureIndex         = raw->imgdata.makernotes.common.exifExposureIndex;
    identify.flashUsed             = (int)raw->imgdata.color.flash_used;
    identify.meteringMode          = raw->imgdata.shootinginfo.MeteringMode;
    identify.exposureProgram       = raw->imgdata.shootinginfo.ExposureProgram;

    identify.rawColors             = raw->imgdata.idata.colors;
    identify.rawImages             = raw->imgdata.idata.raw_count;
    identify.blackPoint            = raw->imgdata.color.black;

    for (int ch = 0 ; ch < 4 ; ++ch)
    {
        identify.blackPointCh[ch]  = raw->imgdata.color.cblack[ch];
    }

    identify.whitePoint            = raw->imgdata.color.maximum;
    identify.orientation           = (DRawInfo::ImageOrientation)raw->imgdata.sizes.flip;

    memcpy(&identify.cameraColorMatrix1, &raw->imgdata.color.cmatrix, sizeof(raw->imgdata.color.cmatrix));
    memcpy(&identify.cameraColorMatrix2, &raw->imgdata.color.rgb_cam, sizeof(raw->imgdata.color.rgb_cam));
    memcpy(&identify.cameraXYZMatrix,    &raw->imgdata.color.cam_xyz, sizeof(raw->imgdata.color.cam_xyz));

    if (raw->imgdata.idata.filters)
    {
        if (!raw->imgdata.idata.cdesc[3])
        {
            raw->imgdata.idata.cdesc[3] = 'G';
        }

        for (int i = 0 ; i < 16 ; ++i)
        {
            identify.filterPattern.append(QLatin1Char(raw->imgdata.idata.cdesc[raw->COLOR(i >> 1, i & 1)]));
        }

        identify.colorKeys         = QLatin1String(raw->imgdata.idata.cdesc);
    }

    for (int c = 0 ; c < raw->imgdata.idata.colors ; ++c)
    {
        identify.daylightMult[c]   = raw->imgdata.color.pre_mul[c];
    }

    if (raw->imgdata.color.cam_mul[0] > 0)
    {
        for (int c = 0 ; c < 4 ; ++c)
        {
            identify.cameraMult[c] = raw->imgdata.color.cam_mul[c];
        }
    }

    identify.xmpData               = QByteArray(raw->imgdata.idata.xmpdata, raw->imgdata.idata.xmplen);

    if (identify.hasIccProfile)
    {
        identify.iccData           = QByteArray((char*)raw->imgdata.color.profile, raw->imgdata.color.profile_length);
    }

    if (raw->imgdata.thumbnail.tformat != LIBRAW_THUMBNAIL_UNKNOWN)
    {
        identify.thumbnail         = QByteArray((char*)raw->imgdata.thumbnail.thumb, raw->imgdata.thumbnail.tlength);
    }

    identify.lensModel             = QString::fromUtf8(raw->imgdata.lens.Lens);
    identify.lensMake              = QString::fromUtf8(raw->imgdata.lens.LensMake);
    identify.lensSerial            = QString::fromUtf8(raw->imgdata.lens.LensSerial);
    identify.focalLengthIn35mmFilm = raw->imgdata.lens.FocalLengthIn35mmFormat;
    identify.maxAperture           = raw->imgdata.lens.EXIF_MaxAp;
}

bool DRawDecoder::Private::loadFromLibraw(const QString& filePath, QByteArray& imageData,
                                          int& width, int& height, int& rgbmax)
{
    m_parent->m_cancel       = false;
    LibRaw* const raw        = new LibRaw;

    // Set progress call back function.

    raw->set_progress_handler(s_progressCallbackForLibRaw, this);
    raw->set_exifparser_handler(s_exifParserCallbackForLibRaw, this);

    QByteArray deadpixelPath = QFile::encodeName(m_parent->m_decoderSettings.deadPixelMap);
    QByteArray cameraProfile = QFile::encodeName(m_parent->m_decoderSettings.inputProfile);
    QByteArray outputProfile = QFile::encodeName(m_parent->m_decoderSettings.outputProfile);

    if (!m_parent->m_decoderSettings.autoBrightness)
    {
        // Use a fixed white level, ignoring the image histogram.

        raw->imgdata.params.no_auto_bright = 1;
    }

    if (m_parent->m_decoderSettings.sixteenBitsImage)
    {
        // (-4) 16bit ppm output

        raw->imgdata.params.output_bps = 16;
    }

    if (m_parent->m_decoderSettings.halfSizeColorImage)
    {
        // (-h) Half-size color image (3x faster than -q).

        raw->imgdata.params.half_size = 1;
    }

    if (m_parent->m_decoderSettings.RGBInterpolate4Colors)
    {
        // (-f) Interpolate RGB as four colors.

        raw->imgdata.params.four_color_rgb = 1;
    }

    if (m_parent->m_decoderSettings.DontStretchPixels)
    {
        // (-j) Do not stretch the image to its correct aspect ratio.

        raw->imgdata.params.use_fuji_rotate = 1;
    }

    // (-H) Unclip highlight color.

    raw->imgdata.params.highlight = m_parent->m_decoderSettings.unclipColors;

    if (m_parent->m_decoderSettings.brightness != 1.0)
    {
        // (-b) Set Brightness value.

        raw->imgdata.params.bright = m_parent->m_decoderSettings.brightness;
    }

    if (m_parent->m_decoderSettings.enableBlackPoint)
    {
        // (-k) Set Black Point value.

        raw->imgdata.params.user_black = m_parent->m_decoderSettings.blackPoint;
    }

    if (m_parent->m_decoderSettings.enableWhitePoint)
    {
        // (-S) Set White Point value (saturation).

        raw->imgdata.params.user_sat = m_parent->m_decoderSettings.whitePoint;
    }

    if (m_parent->m_decoderSettings.medianFilterPasses > 0)
    {
        // (-m) After interpolation, clean up color artifacts by repeatedly applying a 3x3 median filter to the R-G and B-G channels.

        raw->imgdata.params.med_passes = m_parent->m_decoderSettings.medianFilterPasses;
    }

    if (!m_parent->m_decoderSettings.deadPixelMap.isEmpty())
    {
        // (-P) Read the dead pixel list from this file.

        raw->imgdata.params.bad_pixels = deadpixelPath.data();
    }

    switch (m_parent->m_decoderSettings.whiteBalance)
    {
        case DRawDecoderSettings::NONE:
        {
            break;
        }

        case DRawDecoderSettings::CAMERA:
        {
            // (-w) Use camera white balance, if possible.

            raw->imgdata.params.use_camera_wb = 1;
            break;
        }

        case DRawDecoderSettings::AUTO:
        {
            // (-a) Use automatic white balance.

            raw->imgdata.params.use_auto_wb = 1;
            break;
        }

        case DRawDecoderSettings::CUSTOM:
        {
            /*
             * Convert between Temperature and RGB.
             */
            double RGB[3]            = { 0.0 };
            double xD                = 0.0;
            double yD                = 0.0;
            double X                 = 0.0;
            double Y                 = 0.0;
            double Z                 = 0.0;
            double T                 = m_parent->m_decoderSettings.customWhiteBalance;
            QScopedPointer<DRawInfo> identify(new DRawInfo);

            // -----------------------------------------------------------------------
            // Here starts the code picked and adapted from ufraw (0.12.1)
            // to convert Temperature + green multiplier to RGB multipliers

            /*
             * Convert between Temperature and RGB.
             * Base on information from www.brucelindbloom.com/
             * The fit for D-illuminant between 4000K and 12000K are from CIE
             * The generalization to 2000K < T < 4000K and the blackbody fits
             * are my own and should be taken with a grain of salt.
             */
            const double XYZ_to_RGB[3][3] = {
                                                { 3.24071,  -0.969258,  0.0556352 },
                                                {-1.53726,  1.87599,    -0.203996 },
                                                {-0.498571, 0.0415557,  1.05707   }
                                            };

            // Fit for CIE Daylight illuminant

            if      (T <= 4000)
            {
                xD = 0.27475e9/(T*T*T) - 0.98598e6/(T*T) + 1.17444e3/T + 0.145986;
            }
            else if (T <= 7000)
            {
                xD = -4.6070e9/(T*T*T) + 2.9678e6/(T*T) + 0.09911e3/T + 0.244063;
            }
            else
            {
                xD = -2.0064e9/(T*T*T) + 1.9018e6/(T*T) + 0.24748e3/T + 0.237040;
            }

            yD     = -3 * xD * xD + 2.87 * xD - 0.275;
            X      = xD / yD;
            Y      = 1;
            Z      = (1 - xD - yD) / yD;
            RGB[0] = X*XYZ_to_RGB[0][0] + Y*XYZ_to_RGB[1][0] + Z*XYZ_to_RGB[2][0];
            RGB[1] = X*XYZ_to_RGB[0][1] + Y*XYZ_to_RGB[1][1] + Z*XYZ_to_RGB[2][1];
            RGB[2] = X*XYZ_to_RGB[0][2] + Y*XYZ_to_RGB[1][2] + Z*XYZ_to_RGB[2][2];

            // End of the code picked to ufraw
            // -----------------------------------------------------------------------

            RGB[1] = RGB[1] / m_parent->m_decoderSettings.customWhiteBalanceGreen;

            /*
             * By default, decraw override his default D65 WB
             * We need to keep it as a basis : if not, colors with some
             * DSLR will have a high dominant of color that will lead to
             * a completely wrong WB
             */
            if (rawFileIdentify(*identify, filePath))
            {
                RGB[0] = identify->daylightMult[0] / RGB[0];
                RGB[1] = identify->daylightMult[1] / RGB[1];
                RGB[2] = identify->daylightMult[2] / RGB[2];
            }
            else
            {
                RGB[0] = 1.0 / RGB[0];
                RGB[1] = 1.0 / RGB[1];
                RGB[2] = 1.0 / RGB[2];
                qCDebug(DIGIKAM_RAWENGINE_LOG) << "Warning: cannot get daylight multipliers";
            }

            // (-r) set Raw Color Balance Multipliers.

            raw->imgdata.params.user_mul[0] = RGB[0];
            raw->imgdata.params.user_mul[1] = RGB[1];
            raw->imgdata.params.user_mul[2] = RGB[2];
            raw->imgdata.params.user_mul[3] = RGB[1];
            break;
        }

        case DRawDecoderSettings::AERA:
        {
            // (-A) Calculate the white balance by averaging a rectangular area from image.

            raw->imgdata.params.greybox[0] = m_parent->m_decoderSettings.whiteBalanceArea.left();
            raw->imgdata.params.greybox[1] = m_parent->m_decoderSettings.whiteBalanceArea.top();
            raw->imgdata.params.greybox[2] = m_parent->m_decoderSettings.whiteBalanceArea.width();
            raw->imgdata.params.greybox[3] = m_parent->m_decoderSettings.whiteBalanceArea.height();
            break;
        }
    }

    // (-q) Use an interpolation method.

    raw->imgdata.params.user_qual = m_parent->m_decoderSettings.RAWQuality;

    switch (m_parent->m_decoderSettings.NRType)
    {
        case DRawDecoderSettings::WAVELETSNR:
        {
            // (-n) Use wavelets to erase noise while preserving real detail.

            raw->imgdata.params.threshold    = m_parent->m_decoderSettings.NRThreshold;
            break;
        }

        case DRawDecoderSettings::FBDDNR:
        {
            // (100 - 1000) => (1 - 10) conversion

            raw->imgdata.params.fbdd_noiserd = lround(m_parent->m_decoderSettings.NRThreshold / 100.0);
            break;
        }

        default:   // No Noise Reduction
        {
            raw->imgdata.params.threshold    = 0;
            raw->imgdata.params.fbdd_noiserd = 0;
            break;
        }
    }

    // Exposure Correction before interpolation.

    raw->imgdata.params.exp_correc = m_parent->m_decoderSettings.expoCorrection;
    raw->imgdata.params.exp_shift  = m_parent->m_decoderSettings.expoCorrectionShift;
    raw->imgdata.params.exp_preser = m_parent->m_decoderSettings.expoCorrectionHighlight;

    switch (m_parent->m_decoderSettings.inputColorSpace)
    {
        case DRawDecoderSettings::EMBEDDED:
        {
            // (-p embed) Use input profile from RAW file to define the camera's raw colorspace.

            raw->imgdata.params.camera_profile = (char*)"embed";
            break;
        }

        case DRawDecoderSettings::CUSTOMINPUTCS:
        {
            if (!m_parent->m_decoderSettings.inputProfile.isEmpty())
            {
                // (-p) Use input profile file to define the camera's raw colorspace.

                raw->imgdata.params.camera_profile = cameraProfile.data();
            }

            break;
        }

        default:
        {
            // No input profile

            break;
        }
    }

    switch (m_parent->m_decoderSettings.outputColorSpace)
    {
        case DRawDecoderSettings::CUSTOMOUTPUTCS:
        {
            if (!m_parent->m_decoderSettings.outputProfile.isEmpty())
            {
                // (-o) Use ICC profile file to define the output colorspace.

                raw->imgdata.params.output_profile = outputProfile.data();
            }

            break;
        }

        default:
        {
            // (-o) Define the output colorspace.

            raw->imgdata.params.output_color = m_parent->m_decoderSettings.outputColorSpace;
            break;
        }
    }

    //-- Extended demosaicing settings ----------------------------------------------------------

    raw->imgdata.params.dcb_iterations = m_parent->m_decoderSettings.dcbIterations;
    raw->imgdata.params.dcb_enhance_fl = m_parent->m_decoderSettings.dcbEnhanceFl;

#ifdef USE_DNGSDK

    qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: setup internal DNG SDK";

    raw->imgdata.rawparams.use_dngsdk = LIBRAW_DNG_ALL;
    dng_host* const dnghost           = new dng_host;
    raw->set_dng_host(dnghost);

#endif

    //-------------------------------------------------------------------------------------------

    setProgress(0.1);

    qCDebug(DIGIKAM_RAWENGINE_LOG) << filePath;
    qCDebug(DIGIKAM_RAWENGINE_LOG) << m_parent->m_decoderSettings;

#ifdef Q_OS_WIN

    int ret = raw->open_file((const wchar_t*)filePath.utf16());

#else

    int ret = raw->open_file(filePath.toUtf8().constData());

#endif

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run open_file: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

#ifdef USE_DNGSDK

        delete dnghost;

#endif

        return false;
    }

    if (m_parent->m_cancel)
    {
        raw->recycle();
        delete raw;

#ifdef USE_DNGSDK

        delete dnghost;

#endif

        return false;
    }

    setProgress(0.2);

    ret = raw->unpack();

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run unpack: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

#ifdef USE_DNGSDK

        delete dnghost;

#endif

        return false;
    }

    if (m_parent->m_cancel)
    {
        raw->recycle();
        delete raw;

#ifdef USE_DNGSDK

        delete dnghost;

#endif

        return false;
    }

    setProgress(0.25);

    if (m_parent->m_decoderSettings.fixColorsHighlights)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "Applying LibRaw highlights adjustments";

        // 1.0 is fallback to default value

        raw->imgdata.params.adjust_maximum_thr = 1.0;
    }
    else
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "Disabling LibRaw highlights adjustments";

        // 0.0 disables this feature

        raw->imgdata.params.adjust_maximum_thr = 0.0;
    }

    ret = raw->dcraw_process();

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run dcraw_process: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

#ifdef USE_DNGSDK

        delete dnghost;

#endif

        return false;
    }

    if (m_parent->m_cancel)
    {
        raw->recycle();
        delete raw;

#ifdef USE_DNGSDK

        delete dnghost;

#endif

        return false;
    }

    setProgress(0.3);

    libraw_processed_image_t* img = raw->dcraw_make_mem_image(&ret);

    if (!img)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run dcraw_make_mem_image: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

#ifdef USE_DNGSDK

        delete dnghost;

#endif

        return false;
    }

    if (m_parent->m_cancel)
    {
        // Clear memory allocation. Introduced with LibRaw 0.11.0

        raw->dcraw_clear_mem(img);
        raw->recycle();
        delete raw;

#ifdef USE_DNGSDK

        delete dnghost;

#endif

        return false;
    }

    setProgress(0.35);

    width  = img->width;
    height = img->height;
    rgbmax = (1 << img->bits)-1;

    if (img->colors == 3)
    {
        imageData = QByteArray((const char*)img->data, (int)img->data_size);
    }
    else
    {
        // img->colors == 1 (Grayscale) : convert to RGB

        imageData = QByteArray();

        for (int i = 0 ; i < (int)img->data_size ; ++i)
        {
            for (int j = 0 ; j < 3 ; ++j)
            {
                imageData.append(img->data[i]);
            }
        }
    }

    // Clear memory allocation. Introduced with LibRaw 0.11.0

    raw->dcraw_clear_mem(img);
    raw->recycle();
    delete raw;

#ifdef USE_DNGSDK

    delete dnghost;

#endif

    if (m_parent->m_cancel)
    {
        return false;
    }

    setProgress(0.4);

    qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: data info: width=" << width
                                   << " height=" << height
                                   << " rgbmax=" << rgbmax;

    return true;
}

bool DRawDecoder::Private::loadEmbeddedPreview(QByteArray& imgData, LibRaw* const raw)
{
    int ret = raw->unpack_thumb();

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run unpack_thumb: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

        return false;
    }

    libraw_processed_image_t* const thumb = raw->dcraw_make_mem_thumb(&ret);

    if (!thumb)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run dcraw_make_mem_thumb: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

        return false;
    }

    if (thumb->type == LIBRAW_IMAGE_BITMAP)
    {
        createPPMHeader(imgData, thumb);
    }
    else
    {
        imgData = QByteArray((const char*)thumb->data, (int)thumb->data_size);
    }

    // Clear memory allocation. Introduced with LibRaw 0.11.0

    raw->dcraw_clear_mem(thumb);
    raw->recycle();
    delete raw;

    if (imgData.isEmpty())
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "Failed to load JPEG thumb from LibRaw!";
        return false;
    }

    return true;
}

bool DRawDecoder::Private::loadHalfPreview(QImage& image, LibRaw* const raw, bool rotate)
{
    raw->imgdata.params.use_auto_wb   = 1;         // Use automatic white balance.
    raw->imgdata.params.use_camera_wb = 1;         // Use camera white balance, if possible.
    raw->imgdata.params.half_size     = 1;         // Half-size color image (3x faster than -q).

    if (!rotate)
    {
        raw->imgdata.params.user_flip = 0;
    }

#ifdef USE_DNGSDK

    qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: setup internal DNG SDK";

    raw->imgdata.rawparams.use_dngsdk = LIBRAW_DNG_ALL;
    dng_host* const dnghost           = new dng_host;
    raw->set_dng_host(dnghost);

#endif

    QByteArray imgData;

    int ret = raw->unpack();

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run unpack: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

#ifdef USE_DNGSDK

        delete dnghost;

#endif

        return false;
    }

    ret = raw->dcraw_process();

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run dcraw_process: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

#ifdef USE_DNGSDK

        delete dnghost;

#endif

        return false;
    }

    libraw_processed_image_t* halfImg = raw->dcraw_make_mem_image(&ret);

    if (!halfImg)
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "LibRaw: failed to run dcraw_make_mem_image: " << libraw_strerror(ret);
        raw->recycle();
        delete raw;

#ifdef USE_DNGSDK

        delete dnghost;

#endif

        return false;
    }

    Private::createPPMHeader(imgData, halfImg);

    // Clear memory allocation. Introduced with LibRaw 0.11.0

    raw->dcraw_clear_mem(halfImg);
    raw->recycle();
    delete raw;

#ifdef USE_DNGSDK

    delete dnghost;

#endif

    if (imgData.isEmpty())
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "Failed to load half preview from LibRaw!";
        return false;
    }

    if (!image.loadFromData(imgData))
    {
        qCDebug(DIGIKAM_RAWENGINE_LOG) << "Failed to load PPM data from LibRaw!";
        return false;
    }

    return true;
}

} // namespace Digikam
