/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-12-26
 * Description : Multithreaded loader for previews
 *
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_PREVIEW_TASK_H
#define DIGIKAM_PREVIEW_TASK_H

// Qt includes

#include <QImage>

// Local includes

#include "loadsavetask.h"
#include "metaengine_previews.h"

namespace Digikam
{

class PreviewLoadingTask : public SharedLoadingTask
{
public:

    explicit PreviewLoadingTask(LoadSaveThread* const thread,
                                const LoadingDescription& description);
    ~PreviewLoadingTask() override;

    void execute()        override;

private:

    bool loadExiv2Preview(MetaEnginePreviews& previews, int sizeLimit = -1);
    bool loadLibRawPreview(int sizeLimit = -1);
    bool loadHalfSizeRaw();
    bool loadFullSizeRaw();
    bool needToScale();
    bool loadImagePreview(int sizeLimit = -1);
    void convertQImageToDImg();

private:

    QImage m_qimage;
    bool   m_fromRawEmbeddedPreview;

private:

    // Disable
    PreviewLoadingTask(const PreviewLoadingTask&)            = delete;
    PreviewLoadingTask& operator=(const PreviewLoadingTask&) = delete;
};

} // namespace Digikam

#endif // DIGIKAM_PREVIEW_TASK_H
