/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-08
 * Description : A tab to display import item information
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "importitempropertiestab.h"

// Qt includes

#include <QStyle>
#include <QFile>
#include <QGridLayout>
#include <QApplication>
#include <QIcon>
#include <QMimeType>
#include <QMimeDatabase>
#include <QLocale>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "itempropertiestxtlabel.h"
#include "dlayoutbox.h"
#include "itempropertiestab.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImportItemPropertiesTab::Private
{
public:

    explicit Private()
      : file                        (nullptr),
        folder                      (nullptr),
        date                        (nullptr),
        size                        (nullptr),
        isReadable                  (nullptr),
        isWritable                  (nullptr),
        mime                        (nullptr),
        dimensions                  (nullptr),
        ratio                       (nullptr),
        newFileName                 (nullptr),
        downloaded                  (nullptr),
        make                        (nullptr),
        model                       (nullptr),
        photoDate                   (nullptr),
        lens                        (nullptr),
        aperture                    (nullptr),
        focalLength                 (nullptr),
        exposureTime                (nullptr),
        sensitivity                 (nullptr),
        exposureMode                (nullptr),
        flash                       (nullptr),
        whiteBalance                (nullptr),
        labelFile                   (nullptr),
        labelFolder                 (nullptr),
        labelFileIsReadable         (nullptr),
        labelFileIsWritable         (nullptr),
        labelFileDate               (nullptr),
        labelFileSize               (nullptr),
        labelImageMime              (nullptr),
        labelImageDimensions        (nullptr),
        labelImageRatio             (nullptr),
        labelNewFileName            (nullptr),
        labelAlreadyDownloaded      (nullptr),
        labelPhotoMake              (nullptr),
        labelPhotoModel             (nullptr),
        labelPhotoDateTime          (nullptr),
        labelPhotoLens              (nullptr),
        labelPhotoAperture          (nullptr),
        labelPhotoFocalLength       (nullptr),
        labelPhotoExposureTime      (nullptr),
        labelPhotoSensitivity       (nullptr),
        labelPhotoExposureMode      (nullptr),
        labelPhotoFlash             (nullptr),
        labelPhotoWhiteBalance      (nullptr),
        aspectRatio                 (nullptr),
        duration                    (nullptr),
        frameRate                   (nullptr),
        videoCodec                  (nullptr),
        audioBitRate                (nullptr),
        audioChannelType            (nullptr),
        audioCodec                  (nullptr),
        labelVideoAspectRatio       (nullptr),
        labelVideoDuration          (nullptr),
        labelVideoFrameRate         (nullptr),
        labelVideoVideoCodec        (nullptr),
        labelVideoAudioBitRate      (nullptr),
        labelVideoAudioChannelType  (nullptr),
        labelVideoAudioCodec        (nullptr)
    {
    }

    DTextLabelName*  file;
    DTextLabelName*  folder;
    DTextLabelName*  date;
    DTextLabelName*  size;
    DTextLabelName*  isReadable;
    DTextLabelName*  isWritable;
    DTextLabelName*  mime;
    DTextLabelName*  dimensions;
    DTextLabelName*  ratio;
    DTextLabelName*  newFileName;
    DTextLabelName*  downloaded;

    DTextLabelName*  make;
    DTextLabelName*  model;
    DTextLabelName*  photoDate;
    DTextLabelName*  lens;
    DTextLabelName*  aperture;
    DTextLabelName*  focalLength;
    DTextLabelName*  exposureTime;
    DTextLabelName*  sensitivity;
    DTextLabelName*  exposureMode;
    DTextLabelName*  flash;
    DTextLabelName*  whiteBalance;

    DTextLabelValue* labelFile;
    DTextLabelValue* labelFolder;
    DTextLabelValue* labelFileIsReadable;
    DTextLabelValue* labelFileIsWritable;
    DTextLabelValue* labelFileDate;
    DTextLabelValue* labelFileSize;
    DTextLabelValue* labelImageMime;
    DTextLabelValue* labelImageDimensions;
    DTextLabelValue* labelImageRatio;
    DTextLabelValue* labelNewFileName;
    DTextLabelValue* labelAlreadyDownloaded;

    DTextLabelValue* labelPhotoMake;
    DTextLabelValue* labelPhotoModel;
    DTextLabelValue* labelPhotoDateTime;
    DTextLabelValue* labelPhotoLens;
    DTextLabelValue* labelPhotoAperture;
    DTextLabelValue* labelPhotoFocalLength;
    DTextLabelValue* labelPhotoExposureTime;
    DTextLabelValue* labelPhotoSensitivity;
    DTextLabelValue* labelPhotoExposureMode;
    DTextLabelValue* labelPhotoFlash;
    DTextLabelValue* labelPhotoWhiteBalance;

    DTextLabelName*  aspectRatio;
    DTextLabelName*  duration;
    DTextLabelName*  frameRate;
    DTextLabelName*  videoCodec;
    DTextLabelName*  audioBitRate;
    DTextLabelName*  audioChannelType;
    DTextLabelName*  audioCodec;

    DTextLabelValue* labelVideoAspectRatio;
    DTextLabelValue* labelVideoDuration;
    DTextLabelValue* labelVideoFrameRate;
    DTextLabelValue* labelVideoVideoCodec;
    DTextLabelValue* labelVideoAudioBitRate;
    DTextLabelValue* labelVideoAudioChannelType;
    DTextLabelValue* labelVideoAudioCodec;
};

ImportItemPropertiesTab::ImportItemPropertiesTab(QWidget* const parent)
    : DExpanderBox(parent),
      d           (new Private)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(style()->pixelMetric(QStyle::PM_DefaultFrameWidth));

    // --------------------------------------------------

    const int spacing         = qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                             QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));

    QWidget* w1               = new QWidget(this);
    QGridLayout* glay1        = new QGridLayout(w1);

    d->file                   = new DTextLabelName(i18nc("@label: item properties", "File: "),         w1);
    d->folder                 = new DTextLabelName(i18nc("@label: item properties", "Folder: "),       w1);
    d->date                   = new DTextLabelName(i18nc("@label: item properties", "Date: "),         w1);
    d->size                   = new DTextLabelName(i18nc("@label: item properties", "Size: "),         w1);
    d->isReadable             = new DTextLabelName(i18nc("@label: item properties", "Readable: "),     w1);
    d->isWritable             = new DTextLabelName(i18nc("@label: item properties", "Writable: "),     w1);
    d->mime                   = new DTextLabelName(i18nc("@label: item properties", "Type: "),         w1);
    d->dimensions             = new DTextLabelName(i18nc("@label: item properties", "Dimensions: "),   w1);
    d->ratio                  = new DTextLabelName(i18nc("@label: item properties", "Aspect Ratio: "), w1);
    d->newFileName            = new DTextLabelName(i18nc("@label: item properties", "New Name: "),     w1);
    d->downloaded             = new DTextLabelName(i18nc("@label: item properties", "Downloaded: "),   w1);

    d->labelFile              = new DTextLabelValue(QString(), w1);
    d->labelFolder            = new DTextLabelValue(QString(), w1);
    d->labelFileDate          = new DTextLabelValue(QString(), w1);
    d->labelFileSize          = new DTextLabelValue(QString(), w1);
    d->labelFileIsReadable    = new DTextLabelValue(QString(), w1);
    d->labelFileIsWritable    = new DTextLabelValue(QString(), w1);
    d->labelImageMime         = new DTextLabelValue(QString(), w1);
    d->labelImageDimensions   = new DTextLabelValue(QString(), w1);
    d->labelImageRatio        = new DTextLabelValue(QString(), w1);
    d->labelNewFileName       = new DTextLabelValue(QString(), w1);
    d->labelAlreadyDownloaded = new DTextLabelValue(QString(), w1);

    glay1->addWidget(d->file,                   0,  0, 1, 1);
    glay1->addWidget(d->labelFile,              0,  1, 1, 1);
    glay1->addWidget(d->folder,                 1,  0, 1, 1);
    glay1->addWidget(d->labelFolder,            1,  1, 1, 1);
    glay1->addWidget(d->date,                   2,  0, 1, 1);
    glay1->addWidget(d->labelFileDate,          2,  1, 1, 1);
    glay1->addWidget(d->size,                   3,  0, 1, 1);
    glay1->addWidget(d->labelFileSize,          3,  1, 1, 1);
    glay1->addWidget(d->isReadable,             4,  0, 1, 1);
    glay1->addWidget(d->labelFileIsReadable,    4,  1, 1, 1);
    glay1->addWidget(d->isWritable,             5,  0, 1, 1);
    glay1->addWidget(d->labelFileIsWritable,    5,  1, 1, 1);
    glay1->addWidget(d->mime,                   6,  0, 1, 1);
    glay1->addWidget(d->labelImageMime,         6,  1, 1, 1);
    glay1->addWidget(d->dimensions,             7,  0, 1, 1);
    glay1->addWidget(d->labelImageDimensions,   7,  1, 1, 1);
    glay1->addWidget(d->ratio,                  8,  0, 1, 1);
    glay1->addWidget(d->labelImageRatio,        8,  1, 1, 1);
    glay1->addWidget(d->newFileName,            9,  0, 1, 1);
    glay1->addWidget(d->labelNewFileName,       9,  1, 1, 1);
    glay1->addWidget(d->downloaded,             10, 0, 1, 1);
    glay1->addWidget(d->labelAlreadyDownloaded, 10, 1, 1, 1);
    glay1->setColumnStretch(1, 10);
    glay1->setContentsMargins(spacing, spacing, spacing, spacing);
    glay1->setSpacing(0);

    addItem(w1, QIcon::fromTheme(QLatin1String("dialog-information")),
            i18nc("@title", "Camera File Properties"), QLatin1String("FileProperties"), true);

    // --------------------------------------------------

    QWidget* w2               = new QWidget(this);
    QGridLayout* glay2        = new QGridLayout(w2);

    d->make                   = new DTextLabelName(i18nc("@label: item properties", "Make: "),          w2);
    d->model                  = new DTextLabelName(i18nc("@label: item properties", "Model: "),         w2);
    d->photoDate              = new DTextLabelName(i18nc("@label: item properties", "Created: "),       w2);
    d->lens                   = new DTextLabelName(i18nc("@label: item properties", "Lens: "),          w2);
    d->aperture               = new DTextLabelName(i18nc("@label: item properties", "Aperture: "),      w2);
    d->focalLength            = new DTextLabelName(i18nc("@label: item properties", "Focal: "),         w2);
    d->exposureTime           = new DTextLabelName(i18nc("@label: item properties", "Exposure: "),      w2);
    d->sensitivity            = new DTextLabelName(i18nc("@label: item properties", "Sensitivity: "),   w2);
    d->exposureMode           = new DTextLabelName(i18nc("@label: item properties", "Mode/Program: "),  w2);
    d->flash                  = new DTextLabelName(i18nc("@label: item properties", "Flash: "),         w2);
    d->whiteBalance           = new DTextLabelName(i18nc("@label: item properties", "White balance: "), w2);

    d->labelPhotoMake         = new DTextLabelValue(QString(), w2);
    d->labelPhotoModel        = new DTextLabelValue(QString(), w2);
    d->labelPhotoDateTime     = new DTextLabelValue(QString(), w2);
    d->labelPhotoLens         = new DTextLabelValue(QString(), w2);
    d->labelPhotoAperture     = new DTextLabelValue(QString(), w2);
    d->labelPhotoFocalLength  = new DTextLabelValue(QString(), w2);
    d->labelPhotoExposureTime = new DTextLabelValue(QString(), w2);
    d->labelPhotoSensitivity  = new DTextLabelValue(QString(), w2);
    d->labelPhotoExposureMode = new DTextLabelValue(QString(), w2);
    d->labelPhotoFlash        = new DTextLabelValue(QString(), w2);
    d->labelPhotoWhiteBalance = new DTextLabelValue(QString(), w2);

    glay2->addWidget(d->make,                    0, 0, 1, 1);
    glay2->addWidget(d->labelPhotoMake,          0, 1, 1, 1);
    glay2->addWidget(d->model,                   1, 0, 1, 1);
    glay2->addWidget(d->labelPhotoModel,         1, 1, 1, 1);
    glay2->addWidget(d->photoDate,               2, 0, 1, 1);
    glay2->addWidget(d->labelPhotoDateTime,      2, 1, 1, 1);
    glay2->addWidget(d->lens,                    3, 0, 1, 1);
    glay2->addWidget(d->labelPhotoLens,          3, 1, 1, 1);
    glay2->addWidget(d->aperture,                4, 0, 1, 1);
    glay2->addWidget(d->labelPhotoAperture,      4, 1, 1, 1);
    glay2->addWidget(d->focalLength,             5, 0, 1, 1);
    glay2->addWidget(d->labelPhotoFocalLength,   5, 1, 1, 1);
    glay2->addWidget(d->exposureTime,            6, 0, 1, 1);
    glay2->addWidget(d->labelPhotoExposureTime,  6, 1, 1, 1);
    glay2->addWidget(d->sensitivity,             7, 0, 1, 1);
    glay2->addWidget(d->labelPhotoSensitivity,   7, 1, 1, 1);
    glay2->addWidget(d->exposureMode,            8, 0, 1, 1);
    glay2->addWidget(d->labelPhotoExposureMode,  8, 1, 1, 1);
    glay2->addWidget(d->flash,                   9, 0, 1, 1);
    glay2->addWidget(d->labelPhotoFlash,         9, 1, 1, 1);
    glay2->addWidget(d->whiteBalance,           10, 0, 1, 1);
    glay2->addWidget(d->labelPhotoWhiteBalance, 10, 1, 1, 1);
    glay2->setColumnStretch(1, 10);
    glay2->setContentsMargins(spacing, spacing, spacing, spacing);
    glay2->setSpacing(0);

    addItem(w2, QIcon::fromTheme(QLatin1String("camera-photo")),
            i18nc("@title", "Photograph Properties"), QLatin1String("PhotographProperties"), true);

    // --------------------------------------------------

    QWidget* const w3             = new QWidget(this);
    QGridLayout* const glay3      = new QGridLayout(w3);

    d->aspectRatio                = new DTextLabelName(i18nc("@label: item properties", "Aspect Ratio: "),       w3);
    d->audioBitRate               = new DTextLabelName(i18nc("@label: item properties", "Audio Bit Rate: "),     w3);
    d->audioChannelType           = new DTextLabelName(i18nc("@label: item properties", "Audio Channel Type: "), w3);
    d->audioCodec                 = new DTextLabelName(i18nc("@label: item properties", "Audio Codec: "),        w3);
    d->duration                   = new DTextLabelName(i18nc("@label: item properties", "Duration: "),           w3);
    d->frameRate                  = new DTextLabelName(i18nc("@label: item properties", "Frame Rate: "),         w3);
    d->videoCodec                 = new DTextLabelName(i18nc("@label: item properties", "Video Codec: "),        w3);

    d->labelVideoAspectRatio      = new DTextLabelValue(QString(), w3);
    d->labelVideoAudioBitRate     = new DTextLabelValue(QString(), w3);
    d->labelVideoAudioChannelType = new DTextLabelValue(QString(), w3);
    d->labelVideoAudioCodec       = new DTextLabelValue(QString(), w3);
    d->labelVideoDuration         = new DTextLabelValue(QString(), w3);
    d->labelVideoFrameRate        = new DTextLabelValue(QString(), w3);
    d->labelVideoVideoCodec       = new DTextLabelValue(QString(), w3);

    glay3->addWidget(d->aspectRatio,                0, 0, 1, 1);
    glay3->addWidget(d->labelVideoAspectRatio,      0, 1, 1, 1);
    glay3->addWidget(d->audioBitRate,               1, 0, 1, 1);
    glay3->addWidget(d->labelVideoAudioBitRate,     1, 1, 1, 1);
    glay3->addWidget(d->audioChannelType,           2, 0, 1, 1);
    glay3->addWidget(d->labelVideoAudioChannelType, 2, 1, 1, 1);
    glay3->addWidget(d->audioCodec,                 3, 0, 1, 1);
    glay3->addWidget(d->labelVideoAudioCodec,       3, 1, 1, 1);
    glay3->addWidget(d->duration,                   4, 0, 1, 1);
    glay3->addWidget(d->labelVideoDuration,         4, 1, 1, 1);
    glay3->addWidget(d->frameRate,                  5, 0, 1, 1);
    glay3->addWidget(d->labelVideoFrameRate,        5, 1, 1, 1);
    glay3->addWidget(d->videoCodec,                 6, 0, 1, 1);
    glay3->addWidget(d->labelVideoVideoCodec,       6, 1, 1, 1);
    glay3->setContentsMargins(spacing, spacing, spacing, spacing);
    glay3->setSpacing(0);
    glay3->setColumnStretch(1, 10);

    addItem(w3, QIcon::fromTheme(QLatin1String("video-x-generic")),
            i18nc("@title", "Audio/Video Properties"), QLatin1String("VideoProperties"), true);

    // --------------------------------------------------

    addStretch();
}

ImportItemPropertiesTab::~ImportItemPropertiesTab()
{
    delete d;
}

void ImportItemPropertiesTab::setCurrentItem(const CamItemInfo& itemInfo, DMetadata* const meta)
{
    if (itemInfo.isNull())
    {
        d->labelFile->setAdjustedText(QString());
        d->labelFolder->setAdjustedText(QString());
        d->labelFileIsReadable->setAdjustedText(QString());
        d->labelFileIsWritable->setAdjustedText(QString());
        d->labelFileDate->setAdjustedText(QString());
        d->labelFileSize->setAdjustedText(QString());
        d->labelImageMime->setAdjustedText(QString());
        d->labelImageDimensions->setAdjustedText(QString());
        d->labelImageRatio->setAdjustedText(QString());
        d->labelNewFileName->setAdjustedText(QString());
        d->labelAlreadyDownloaded->setAdjustedText(QString());

        d->labelPhotoMake->setAdjustedText(QString());
        d->labelPhotoModel->setAdjustedText(QString());
        d->labelPhotoDateTime->setAdjustedText(QString());
        d->labelPhotoLens->setAdjustedText(QString());
        d->labelPhotoAperture->setAdjustedText(QString());
        d->labelPhotoFocalLength->setAdjustedText(QString());
        d->labelPhotoExposureTime->setAdjustedText(QString());
        d->labelPhotoSensitivity->setAdjustedText(QString());
        d->labelPhotoExposureMode->setAdjustedText(QString());
        d->labelPhotoFlash->setAdjustedText(QString());
        d->labelPhotoWhiteBalance->setAdjustedText(QString());

        d->labelVideoAspectRatio->setAdjustedText(QString());
        d->labelVideoAudioBitRate->setAdjustedText(QString());
        d->labelVideoAudioChannelType->setAdjustedText(QString());
        d->labelVideoAudioCodec->setAdjustedText(QString());
        d->labelVideoDuration->setAdjustedText(QString());
        d->labelVideoFrameRate->setAdjustedText(QString());
        d->labelVideoVideoCodec->setAdjustedText(QString());

        setEnabled(false);
        return;
    }

    setEnabled(true);

    QString str;
    QString unknown(QString::fromUtf8("<i>%1</i>").arg(i18nc("@info: item properties", "unknown")));
    QString unchanged(QString::fromUtf8("<i>%1</i>").arg(i18nc("@info: item properties", "unchanged")));
    QString unavailable(QString::fromUtf8("<i>%1</i>").arg(i18nc("@info: item properties", "unavailable")));

    // -- Camera file system information ------------------------------------------

    d->labelFile->setAdjustedText(itemInfo.name);
    d->labelFolder->setAdjustedText(itemInfo.folder);

    if      (itemInfo.readPermissions < 0)
    {
        str = unknown;
    }
    else if (itemInfo.readPermissions == 0)
    {
        str = i18nc("@info: item properties", "No");
    }
    else
    {
        str = i18nc("@info: item properties", "Yes");
    }

    d->labelFileIsReadable->setAdjustedText(str);

    if      (itemInfo.writePermissions < 0)
    {
        str = unknown;
    }
    else if (itemInfo.writePermissions == 0)
    {
        str = i18nc("@info: item properties", "No");
    }
    else
    {
        str = i18nc("@info: item properties", "Yes");
    }

    d->labelFileIsWritable->setAdjustedText(str);

    if (itemInfo.ctime.isValid())
    {
        d->labelFileDate->setAdjustedText(QLocale().toString(itemInfo.ctime, QLocale::ShortFormat));
    }
    else
    {
        d->labelFileDate->setAdjustedText(unknown);
    }

    str = i18nc("@info: item size", "%1 (%2)", ItemPropertiesTab::humanReadableBytesCount(itemInfo.size), QLocale().toString(itemInfo.size));
    d->labelFileSize->setAdjustedText(str);

    // -- Image Properties --------------------------------------------------

    if (itemInfo.mime == QLatin1String("image/x-raw"))
    {
        d->labelImageMime->setAdjustedText(i18nc("@label", "RAW Image"));
    }
    else
    {
        QMimeType mimeType = QMimeDatabase().mimeTypeForName(itemInfo.mime);

        if (mimeType.isValid())
        {
            d->labelImageMime->setAdjustedText(mimeType.comment());
        }
        else
        {
            d->labelImageMime->setAdjustedText(itemInfo.mime);    // last fallback
        }
    }

    QString mpixels;
    QSize dims;

    if ((itemInfo.width == -1) && (itemInfo.height == -1))
    {
        // delayed loading to list faster from UMSCamera

        if (itemInfo.mime == QLatin1String("image/x-raw"))
        {
            dims = meta->getItemDimensions();
        }
        else
        {
            dims = meta->getPixelSize();
        }
    }
    else
    {
        // if available (GPCamera), take dimensions directly from itemInfo

        dims = QSize(itemInfo.width, itemInfo.height);
    }

    mpixels = QLocale().toString(dims.width()*dims.height()/1000000.0, 'f', 1);
    str = (!dims.isValid()) ? unknown : i18nc("@info: item resolution", "%1x%2 (%3Mpx)",
            dims.width(), dims.height(), mpixels);
    d->labelImageDimensions->setAdjustedText(str);

    if (!dims.isValid()) str = unknown;
    else ItemPropertiesTab::aspectRatioToString(dims.width(), dims.height(), str);

    d->labelImageRatio->setAdjustedText(str);

    // -- Download information ------------------------------------------

    d->labelNewFileName->setAdjustedText(itemInfo.downloadName.isEmpty() ? unchanged : itemInfo.downloadName);

    if (itemInfo.downloaded == CamItemInfo::DownloadUnknown)
    {
        str = unknown;
    }
    else if (itemInfo.downloaded == CamItemInfo::DownloadedYes)
    {
        str = i18nc("@info: item properties", "Yes");
    }
    else
    {
        str = i18nc("@info: item properties", "No");
    }

    d->labelAlreadyDownloaded->setAdjustedText(str);

    // -- Photograph information ------------------------------------------
    // Note: If something is changed here, please updated albumfiletip section too.

    PhotoInfoContainer photoInfo = meta->getPhotographInformation();

    if (photoInfo.isEmpty())
    {
        widget(1)->hide();
    }
    else
    {
        widget(1)->show();
    }

    ItemPropertiesTab::shortenedMakeInfo(photoInfo.make);
    ItemPropertiesTab::shortenedModelInfo(photoInfo.model);
    d->labelPhotoMake->setAdjustedText(photoInfo.make.isEmpty()   ? unavailable : photoInfo.make);
    d->labelPhotoModel->setAdjustedText(photoInfo.model.isEmpty() ? unavailable : photoInfo.model);

    if (photoInfo.dateTime.isValid())
    {
        str = QLocale().toString(photoInfo.dateTime, QLocale::ShortFormat);
        d->labelPhotoDateTime->setAdjustedText(str);
    }
    else
    {
        d->labelPhotoDateTime->setAdjustedText(unavailable);
    }

    d->labelPhotoLens->setAdjustedText(photoInfo.lens.isEmpty()         ? unavailable : photoInfo.lens);
    d->labelPhotoAperture->setAdjustedText(photoInfo.aperture.isEmpty() ? unavailable : photoInfo.aperture);

    if (photoInfo.focalLength35mm.isEmpty())
    {
        d->labelPhotoFocalLength->setAdjustedText(photoInfo.focalLength.isEmpty() ? unavailable : photoInfo.focalLength);
    }
    else
    {
        str = i18nc("@info: focal length", "%1 (%2)", photoInfo.focalLength, photoInfo.focalLength35mm);
        d->labelPhotoFocalLength->setAdjustedText(str);
    }

    d->labelPhotoExposureTime->setAdjustedText(photoInfo.exposureTime.isEmpty() ? unavailable : photoInfo.exposureTime);
    d->labelPhotoSensitivity->setAdjustedText(photoInfo.sensitivity.isEmpty()   ? unavailable : i18nc("@label", "%1 ISO", photoInfo.sensitivity));

    if (photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
    {
        d->labelPhotoExposureMode->setAdjustedText(unavailable);
    }
    else if (!photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
    {
        d->labelPhotoExposureMode->setAdjustedText(photoInfo.exposureMode);
    }
    else if (photoInfo.exposureMode.isEmpty() && !photoInfo.exposureProgram.isEmpty())
    {
        d->labelPhotoExposureMode->setAdjustedText(photoInfo.exposureProgram);
    }
    else
    {
        str = QString::fromUtf8("%1 / %2").arg(photoInfo.exposureMode).arg(photoInfo.exposureProgram);
        d->labelPhotoExposureMode->setAdjustedText(str);
    }

    d->labelPhotoFlash->setAdjustedText(photoInfo.flash.isEmpty()               ? unavailable : photoInfo.flash);
    d->labelPhotoWhiteBalance->setAdjustedText(photoInfo.whiteBalance.isEmpty() ? unavailable : photoInfo.whiteBalance);

    // -- Audio/Video information ------------------------------------------

    VideoInfoContainer videoInfo = meta->getVideoInformation();

    if (videoInfo.isEmpty())
    {
        widget(2)->hide();
    }
    else
    {
        widget(2)->show();
    }

    d->labelVideoAspectRatio->setAdjustedText(videoInfo.aspectRatio.isEmpty()           ? unavailable : videoInfo.aspectRatio);
    d->labelVideoAudioBitRate->setAdjustedText(videoInfo.audioBitRate.isEmpty()         ? unavailable : videoInfo.audioBitRate);
    d->labelVideoAudioChannelType->setAdjustedText(videoInfo.audioChannelType.isEmpty() ? unavailable : videoInfo.audioChannelType);
    d->labelVideoAudioCodec->setAdjustedText(videoInfo.audioCodec.isEmpty()             ? unavailable : videoInfo.audioCodec);
    d->labelVideoDuration->setAdjustedText(videoInfo.duration.isEmpty()                 ? unavailable : videoInfo.duration);
    d->labelVideoFrameRate->setAdjustedText(videoInfo.frameRate.isEmpty()               ? unavailable : videoInfo.frameRate);
    d->labelVideoVideoCodec->setAdjustedText(videoInfo.videoCodec.isEmpty()             ? unavailable : videoInfo.videoCodec);
}

} // namespace Digikam
