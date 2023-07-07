/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_EXPO_BLENDING_ACTIONS_H
#define DIGIKAM_EXPO_BLENDING_ACTIONS_H

// Qt includes

#include <QString>
#include <QImage>
#include <QMetaType>
#include <QMap>

// Local includes

#include "enfusesettings.h"

namespace DigikamGenericExpoBlendingPlugin
{

enum ExpoBlendingAction
{
    EXPOBLENDING_NONE = 0,
    EXPOBLENDING_IDENTIFY,
    EXPOBLENDING_PREPROCESSING,
    EXPOBLENDING_ENFUSEPREVIEW,
    EXPOBLENDING_ENFUSEFINAL,
    EXPOBLENDING_LOAD
};

class ExpoBlendingItemPreprocessedUrls
{
public:

    ExpoBlendingItemPreprocessedUrls()          = default;

    ExpoBlendingItemPreprocessedUrls(const QUrl& preprocessed, const QUrl& preview)
        : preprocessedUrl(preprocessed),
          previewUrl     (preview)
    {
    };

    ~ExpoBlendingItemPreprocessedUrls()         = default;

    QUrl preprocessedUrl;              ///< Can be original file or aligned version, depending of user choice.
    QUrl previewUrl;                   ///< The JPEG preview version, accordingly of preprocessedUrl constant.
};

typedef QMap<QUrl, ExpoBlendingItemPreprocessedUrls> ExpoBlendingItemUrlsMap;   ///< Map between original Url and processed temp Urls.

class ExpoBlendingActionData
{
public:

    explicit ExpoBlendingActionData()
        : starting(false),
          success (false),
          action  (EXPOBLENDING_NONE)
    {
    }

    bool                    starting;
    bool                    success;

    QString                 message;

    QImage                  image;

    QList<QUrl>             inUrls;
    QList<QUrl>             outUrls;

    EnfuseSettings          enfuseSettings;

    ExpoBlendingItemUrlsMap preProcessedUrlsMap;

    ExpoBlendingAction      action;
};

} // namespace DigikamGenericExpoBlendingPlugin

Q_DECLARE_METATYPE(DigikamGenericExpoBlendingPlugin::ExpoBlendingActionData)
Q_DECLARE_METATYPE(DigikamGenericExpoBlendingPlugin::ExpoBlendingItemPreprocessedUrls)

#endif // DIGIKAM_EXPO_BLENDING_ACTIONS_H
