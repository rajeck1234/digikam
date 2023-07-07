/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-06-05
 * Description : Multithreaded loader for thumbnails
 *
 * SPDX-FileCopyrightText: 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_THUMB_NAIL_TASK_H
#define DIGIKAM_THUMB_NAIL_TASK_H

// Qt includes

#include <QImage>

// Local includes

#include "loadsavetask.h"

namespace Digikam
{

class ThumbnailCreator;

class ThumbnailLoadingTask : public SharedLoadingTask
{
public:

    explicit ThumbnailLoadingTask(LoadSaveThread* const thread,
                                  const LoadingDescription& description);

    void execute()     override;
    void postProcess() override;
    void setThumbResult(const LoadingDescription& loadingDescription,
                        const QImage& qimage);

private:

    void setupCreator();

private:

    QImage            m_qimage;
    ThumbnailCreator* m_creator;

private:

    // Disable
    ThumbnailLoadingTask(const ThumbnailLoadingTask&)            = delete;
    ThumbnailLoadingTask& operator=(const ThumbnailLoadingTask&) = delete;
};

} // namespace Digikam

#endif // DIGIKAM_THUMB_NAIL_TASK_H
