/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2016-04-21
 * Description : video thumbnails extraction based on ffmpeg
 *
 * SPDX-FileCopyrightText: 2010      by Dirk Vanden Boer <dirk dot vdb at gmail dot com>
 * SPDX-FileCopyrightText: 2016-2018 by Maik Qualmann <metzpinguin at gmail dot com>
 * SPDX-FileCopyrightText: 2016-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_VIDEO_STRIP_FILTER_H
#define DIGIKAM_VIDEO_STRIP_FILTER_H

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class VideoFrame;

class DIGIKAM_EXPORT VideoStripFilter
{
public:

    explicit VideoStripFilter();
    ~VideoStripFilter();

    void process(VideoFrame& videoFrame);

private:

    // Disable
    VideoStripFilter(const VideoStripFilter&)            = delete;
    VideoStripFilter& operator=(const VideoStripFilter&) = delete;
};

} // namespace Digikam

#endif // DIGIKAM_VIDEO_STRIP_FILTER_H
