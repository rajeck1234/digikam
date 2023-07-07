/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-24
 * Description : MJPEG stream settings container.
 *
 * SPDX-FileCopyrightText: 2021-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021 by Quoc Hưng Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_MJPEG_STREAM_SETTINGS_H
#define DIGIKAM_MJPEG_STREAM_SETTINGS_H

// Qt includes

#include <QList>
#include <QString>
#include <QUrl>
#include <QFont>

// Local includes

#include "mjpegserver.h"
#include "dinfointerface.h"
#include "effectmngr.h"
#include "transitionmngr.h"

class KConfigGroup;

using namespace Digikam;

namespace DigikamGenericMjpegStreamPlugin
{

class MjpegStreamSettings
{

public:

    explicit MjpegStreamSettings();
    ~MjpegStreamSettings();

    /**
     * Helper method to compute urlsList from a set of albms selected by end-users from GUI.
     */
    void setCollectionMap(const MjpegServerMap& map);

    /**
     * Read and write settings in config file between sessions.
     */
    void  readSettings(KConfigGroup& group);
    void  writeSettings(KConfigGroup& group);

public:

    bool                        printTitle;              ///< Print image title while streaming
    bool                        printTags;               ///< Print tags title while streaming
    bool                        printCapIfNoTitle;       ///< Print image captions if no title available while streaming
    bool                        printComment;            ///< Print picture comment while streaming
    bool                        printMakeModel;          ///< Print camera Make and Model while streaming
    bool                        printLensModel;          ///< Print camera Lens model while streaming
    bool                        printExpoSensitivity;    ///< Print camera Exposure and Sensitivity while streaming
    bool                        printApertureFocal;      ///< Print camera Aperture and Focal while streaming
    bool                        printName;               ///< Print picture file name while streaming
    bool                        printDate;               ///< Print picture creation date while streaming
    bool                        printRating;             ///< Print rating while streaming
    QFont                       osdFont;                 ///< Font for the display of osd text
    int                         port;                    ///< IP port to use with MJPEG Server.
    bool                        loop;                    ///< Image stream as loop.
    int                         quality;                 ///< Jpeg compression [1...100].
    int                         delay;                   ///< Delay in seconds between inages.
    QList<QUrl>                 inputImages;             ///< Ordered list of images to stream.
    int                         outSize;                 ///< Output JPEG size ID.
    int                         rate;                    ///< Number of frames by seconds.
    TransitionMngr::TransType   transition;              ///< Transition type between images.
    EffectMngr::EffectType      effect;                  ///< Effect while displaying images.
    DInfoInterface*             iface;                   ///< Plugin host interface to handle item properties.
};

} // namespace DigikamGenericMjpegStreamPlugin

#endif // DIGIKAM_MJPEG_STREAM_SETTINGS_H
