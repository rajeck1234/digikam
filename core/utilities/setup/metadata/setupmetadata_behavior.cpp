/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-08-03
 * Description : setup Metadata tab.
 *
 * SPDX-FileCopyrightText: 2003-2004 by Ralf Holzer <ralf at well dot com>
 * SPDX-FileCopyrightText: 2003-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupmetadata_p.h"

namespace Digikam
{

void SetupMetadata::appendBehaviorTab()
{
    QWidget* const panel            = new QWidget;
    QVBoxLayout* const mainLayout   = new QVBoxLayout;

    d->fieldsGroup                  = new QGroupBox;
    QGridLayout* const fieldsLayout = new QGridLayout;

    d->fieldsGroup->setWhatsThis(xi18nc("@info:whatsthis",
                                        "<para>In addition to the pixel content, image files usually "
                                        "contain a variety of metadata. A lot of the parameters you can use "
                                        "in digiKam to manage files, such as rating or comment, can be written "
                                        "to the files' metadata.</para> "
                                        "<para>Storing in metadata allows one to preserve this information "
                                        "when moving or sending the files to different systems.</para>"));

    QLabel* const fieldsIconLabel = new QLabel;
    fieldsIconLabel->setPixmap(QIcon::fromTheme(QLatin1String("format-list-unordered")).pixmap(32));

    QLabel* const fieldsLabel     = new QLabel(i18nc("@label", "Write This Information to the Metadata"));

    d->saveTagsBox       = new QCheckBox;
    d->saveTagsBox->setText(i18nc("@option:check", "Image tags"));
    d->saveTagsBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store the item tags "
                                       "in the XMP and IPTC tags."));

    d->saveCommentsBox   = new QCheckBox;
    d->saveCommentsBox->setText(i18nc("@option:check", "Captions and title"));
    d->saveCommentsBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store item caption and title "
                                           "in the JFIF Comment section, the Exif tag, the XMP tag, "
                                           "and the IPTC tag."));

    d->saveRatingBox     = new QCheckBox;
    d->saveRatingBox->setText(i18nc("@option:check", "Rating"));
    d->saveRatingBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store the item rating "
                                         "in the Exif tag and the XMP tags."));

    d->savePickLabelBox  = new QCheckBox;
    d->savePickLabelBox->setText(i18nc("@option:check", "Pick label"));
    d->savePickLabelBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store the item pick label "
                                            "in the XMP tags."));

    d->saveColorLabelBox = new QCheckBox;
    d->saveColorLabelBox->setText(i18nc("@option:check", "Color label"));
    d->saveColorLabelBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store the item color label "
                                             "in the XMP tags."));

    d->saveDateTimeBox   = new QCheckBox;
    d->saveDateTimeBox->setText(i18nc("@option:check", "Timestamps"));
    d->saveDateTimeBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store the item date and time "
                                           "in the EXIF, XMP, and IPTC tags."));

    d->saveTemplateBox   = new QCheckBox;
    d->saveTemplateBox->setText(i18nc("@option:check", "Metadata templates (Copyright etc.)"));
    d->saveTemplateBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store the metadata "
                                           "template in the XMP and the IPTC tags. "
                                           "You can set template values to Template setup page."));
    d->saveFaceTags      = new QCheckBox;
    d->saveFaceTags->setText(i18nc("@option:check", "Face Tags (including face areas)"));
    d->saveFaceTags->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store face tags "
                                        "with face rectangles in the XMP tags."));

    d->savePosition      = new QCheckBox;
    d->savePosition->setText(i18nc("@option:check", "Geolocation information (GPS)"));
    d->savePosition->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store Geolocation information "
                                        "in the Exif tag and the XMP tags."));

    fieldsLayout->addWidget(fieldsIconLabel,       0, 0, 2, 3);
    fieldsLayout->addWidget(fieldsLabel,           0, 1, 2, 3);
    fieldsLayout->addWidget(d->saveTagsBox,        2, 0, 1, 3);
    fieldsLayout->addWidget(d->saveCommentsBox,    3, 0, 1, 3);
    fieldsLayout->addWidget(d->saveRatingBox,      4, 0, 1, 3);
    fieldsLayout->addWidget(d->savePickLabelBox,   5, 0, 1, 3);
    fieldsLayout->addWidget(d->saveColorLabelBox,  6, 0, 1, 3);
    fieldsLayout->addWidget(d->saveDateTimeBox,    7, 0, 1, 3);
    fieldsLayout->addWidget(d->saveTemplateBox,    8, 0, 1, 3);
    fieldsLayout->addWidget(d->saveFaceTags,       9, 0, 1, 3);
    fieldsLayout->addWidget(d->savePosition,      10, 0, 1, 3);
    fieldsLayout->setColumnStretch(3, 10);
    d->fieldsGroup->setLayout(fieldsLayout);

    // --------------------------------------------------------

    d->readWriteGroup                  = new QGroupBox;
    QGridLayout* const readWriteLayout = new QGridLayout;

    QLabel* const readWriteIconLabel   = new QLabel;
    readWriteIconLabel->setPixmap(QIcon::fromTheme(QLatin1String("document-open")).pixmap(32));

    QLabel* const readWriteLabel       = new QLabel(i18nc("@label", "Reading and Writing Metadata"));

    d->writeWithExifToolBox            = new QCheckBox;
    d->writeWithExifToolBox->setText(i18nc("@option:check", "Delegate to ExifTool backend all operations to write metadata to files"));
    d->writeWithExifToolBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to write metadata to files "
                                                "with ExifTool backend instead Exiv2. This will slowdown a little bit the "
                                                "synchronization of files metadata with database."));

    d->writeDngFilesBox                = new QCheckBox;
    d->writeDngFilesBox->setText(i18nc("@option:check", "Write metadata to DNG files"));
    d->writeDngFilesBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to write metadata into DNG files."));

    d->writeRawFilesBox                = new QCheckBox;
    d->writeRawFilesBox->setText(i18nc("@option:check", "If possible write metadata to RAW files"));
    d->writeRawFilesBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to write metadata into RAW files. "
                                            "This feature is delegate to ExifTool backend and is disabled by default."));

    d->writeWithExifToolLabel          = new QLabel;
    d->writeWithExifToolLabel->setOpenExternalLinks(true);

    // ---

    d->useLazySync                     = new QCheckBox;
    d->useLazySync->setText(i18nc("@option:check", "Use lazy synchronization"));
    d->useLazySync->setWhatsThis(i18nc("@info:whatsthis",
                                       "Instead of synchronizing metadata, just schedule it for synchronization."
                                       "Synchronization can be done later by triggering the apply pending, or at digikam exit"));

    d->updateFileTimeStampBox          = new QCheckBox;
    d->updateFileTimeStampBox->setText(i18nc("@option:check", "&Update file modification timestamp when files are modified"));
    d->updateFileTimeStampBox->setWhatsThis(i18nc("@info:whatsthis",
                                                  "Turn off this option to not update file timestamps when files are changed as "
                                                  "when you update metadata or image data. Note: disabling this option can "
                                                  "introduce some dysfunctions with applications which use file timestamps "
                                                  "properties to detect file modifications automatically."));

    d->rescanImageIfModifiedBox        = new QCheckBox;
    d->rescanImageIfModifiedBox->setText(i18nc("@option:check", "&Rescan file when files are modified"));
    d->rescanImageIfModifiedBox->setWhatsThis(i18nc("@info:whatsthis",
                                                    "Turning this option on, will force digiKam to rescan files that has been "
                                                    "modified outside digiKam. If a file has changed it is file size or if "
                                                    "the last modified timestamp has changed, a rescan of that "
                                                    "file will be performed when digiKam starts."));

    readWriteLayout->addWidget(readWriteIconLabel,                    0,  0, 2, 3);
    readWriteLayout->addWidget(readWriteLabel,                        0,  1, 2, 3);
    readWriteLayout->addWidget(d->writeWithExifToolBox,               2,  0, 1, 3);
    readWriteLayout->addWidget(d->writeDngFilesBox,                   3,  1, 1, 3);
    readWriteLayout->addWidget(d->writeRawFilesBox,                   4,  1, 1, 3);
    readWriteLayout->addWidget(d->writeWithExifToolLabel,             5,  0, 1, 4);
    readWriteLayout->addWidget(new DLineWidget(Qt::Horizontal, this), 6,  0, 1, 4);
    readWriteLayout->addWidget(d->useLazySync,                        7,  0, 1, 3);
    readWriteLayout->addWidget(d->updateFileTimeStampBox,             8,  0, 1, 3);
    readWriteLayout->addWidget(d->rescanImageIfModifiedBox,           9,  0, 1, 3);
    readWriteLayout->setColumnStretch(0, 5);
    readWriteLayout->setColumnStretch(1, 100);
    d->readWriteGroup->setLayout(readWriteLayout);

    // --------------------------------------------------------

    QFrame* const infoBox              = new QFrame;
    QGridLayout* const infoBoxGrid     = new QGridLayout;
    infoBox->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    DActiveLabel* const exiv2LogoLabel = new DActiveLabel(QUrl(QLatin1String("https://github.com/Exiv2/exiv2")),
                                                          QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/logo-exiv2.png")),
                                                          infoBox);
    exiv2LogoLabel->setWhatsThis(i18nc("@info:whatsthis", "Visit Exiv2 project website"));

    QLabel* const explanation          = new QLabel(infoBox);
    explanation->setOpenExternalLinks(true);
    explanation->setWordWrap(true);
    QString txt;

    txt.append(QString::fromUtf8("<p><a href='https://en.wikipedia.org/wiki/Exif'>Exif</a> - %1</p>")
               .arg(i18nc("@info", "a standard used by most digital cameras today to store technical "
                          "information (like aperture and shutter speed) about an image.")));

    txt.append(QString::fromUtf8("<p><a href='https://en.wikipedia.org/wiki/IPTC_Information_Interchange_Model'>IPTC</a> - %1</p>")
               .arg(i18nc("@info", "an older standard used in digital photography to store "
                          "photographer information in images.")));

    if (MetaEngine::supportXmp())
    {
        txt.append(QString::fromUtf8("<p><a href='https://en.wikipedia.org/wiki/Extensible_Metadata_Platform'>XMP</a> - %1</p>")
                   .arg(i18nc("@info", "a new standard used in digital photography, designed to replace IPTC.")));
    }

    explanation->setText(txt);

    infoBoxGrid->addWidget(exiv2LogoLabel, 0, 0, 1, 1);
    infoBoxGrid->addWidget(explanation,    0, 1, 1, 2);
    infoBoxGrid->setColumnStretch(1, 10);
    infoBoxGrid->setRowStretch(1, 10);
    infoBoxGrid->setSpacing(0);
    infoBox->setLayout(infoBoxGrid);

    // --------------------------------------------------------

    mainLayout->addWidget(d->fieldsGroup);
    mainLayout->addWidget(d->readWriteGroup);
    mainLayout->addWidget(infoBox);
    panel->setLayout(mainLayout);

    d->tab->insertTab(Behavior, panel, i18nc("@title:tab", "Behavior"));
}

void SetupMetadata::slotExifToolSettingsChanged(bool available)
{
    if (available)
    {
        d->writeWithExifToolBox->setEnabled(true);
        d->writeDngFilesBox->setEnabled(true);
        d->writeRawFilesBox->setEnabled(true);
        d->writeWithExifToolLabel->setText(i18nc("@label", "Note: see %1 of ExifTool backend.",
                                                 QString::fromUtf8("<a href='https://exiftool.org/#limitations'>%1</a>")
                                                     .arg(i18nc("@label", "write limitations"))));
    }
    else
    {
        d->writeWithExifToolBox->setEnabled(false);
        d->writeDngFilesBox->setEnabled(false);
        d->writeRawFilesBox->setEnabled(false);
        d->writeWithExifToolLabel->setText(i18nc("@label", "Note: these options depends of %1 availability. "
                                                 "Check in the ExifTool tab for details.",
                                                 QString::fromUtf8("<a href='https://exiftool.org/'>%1</a>")
                                                    .arg(i18nc("@label", "ExifTool backend"))));
    }
}

} // namespace Digikam
