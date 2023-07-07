/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-13
 * Description : slide show settings container.
 *
 * SPDX-FileCopyrightText: 2007-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_SLIDESHOW_SETTINGS_PLUGIN_H
#define DIGIKAM_SLIDESHOW_SETTINGS_PLUGIN_H

// Qt includes

#include <QMap>
#include <QColor>
#include <QString>
#include <QFont>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "previewsettings.h"
#include "dinfointerface.h"
#include "dplugingeneric.h"

using namespace Digikam;

namespace DigikamGenericSlideShowPlugin
{

/**
 * This class contain all settings to perform a slide show of a group of pictures
 */
class SlideShowSettings
{

public:

    explicit SlideShowSettings();
    ~SlideShowSettings();

    void readFromConfig();
    void writeToConfig();

    int  indexOf(const QUrl&) const;
    int  count()              const;

    void suffleImages();

public:

    /// Global Slide Show Settings

    /**
     * Start Slide with current selected item
     */
    bool                         startWithCurrent;

    /**
     * Auto-rotate image accordingly with Exif Rotation tag
     */
    bool                         exifRotate;

    /**
     * Print picture file name while slide
     */
    bool                         printName;

    /**
     * Print picture creation date while slide
     */
    bool                         printDate;

    /**
     * Print camera Aperture and Focal while slide
     */
    bool                         printApertureFocal;

    /**
     * Print camera Make and Model while slide
     */
    bool                         printMakeModel;

    /**
     * Print camera Lens model while slide
     */
    bool                         printLensModel;

    /**
     * Print camera Exposure and Sensitivity while slide
     */
    bool                         printExpoSensitivity;

    /**
     * Print picture comment while slide
     */
    bool                         printComment;

    /**
     * Print image title while slide
     */
    bool                         printTitle;

    /**
     * Print image captions if no title available while slide
     */
    bool                         printCapIfNoTitle;

    /**
     * Print tag names while slide
     */
    bool                         printTags;

    /**
     * Print color label and pick label while slide
     */
    bool                         printLabels;

    /**
     * Print rating while slide
     */
    bool                         printRating;

    /**
     * Slide pictures in loop
     */
    bool                         loop;

    /**
     * Suffle pictures
     */
    bool                         suffle;

    /**
     * Delay in seconds
     */
    int                          delay;

    /**
     * Whether to enable the auto-move feature.
     */
    bool                         autoPlayEnabled;

    /**
     * Screen to use in case of multi-monitor computer.
     */
    int                          slideScreen;

    /**
     * Show progress indicator
     */
    bool                         showProgressIndicator;

    /**
     * Load images (previews) in full size, not reduced version
     */
    PreviewSettings              previewSettings;

    /**
     * List of pictures URL to slide
     */
    QList<QUrl>                  fileList;

    /**
     * Original list of pictures URL to slide
     */
    QList<QUrl>                  originalFileList;

    /**
     * URL of the first image to show if requested
     */
    QUrl                         imageUrl;

    /**
     * Font for the display of caption text
     */
    QFont                        captionFont;

    /**
     * Interface to access to host application data
     */
    DInfoInterface*              iface;

    /**
     * The plugin instance.
     */
    DPluginGeneric*              plugin;

private:

    static const QString         configGroupName;
    static const QString         configSlideShowStartCurrentEntry;
    static const QString         configSlideShowDelayEntry;
    static const QString         configSlideShowLoopEntry;
    static const QString         configSlideShowSuffleEntry;
    static const QString         configSlideShowPrintApertureFocalEntry;
    static const QString         configSlideShowPrintCommentEntry;
    static const QString         configSlideShowPrintTitleEntry;
    static const QString         configSlideShowPrintCapIfNoTitleEntry;
    static const QString         configSlideShowPrintDateEntry;
    static const QString         configSlideShowPrintExpoSensitivityEntry;
    static const QString         configSlideShowPrintMakeModelEntry;
    static const QString         configSlideShowPrintLensModelEntry;
    static const QString         configSlideShowPrintNameEntry;
    static const QString         configSlideShowPrintTagsEntry;
    static const QString         configSlideShowPrintLabelsEntry;
    static const QString         configSlideShowPrintRatingEntry;
    static const QString         configSlideShowProgressIndicatorEntry;
    static const QString         configSlideShowCaptionFontEntry;
    static const QString         configSlideScreenEntry;
};

} // namespace DigikamGenericSlideShowPlugin

#endif // DIGIKAM_SLIDESHOW_SETTINGS_PLUGIN_H
