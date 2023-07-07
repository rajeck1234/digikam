/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-03-27
 * Description : file copy thread.
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_FC_THREAD_H
#define DIGIKAM_FC_THREAD_H

// Qt includes

#include <QUrl>

// Local includes

#include "actionthreadbase.h"
#include "fccontainer.h"

using namespace Digikam;

namespace DigikamGenericFileCopyPlugin
{

class FCThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit FCThread(QObject* const parent);
    ~FCThread() override;

    void createCopyJobs(const QList<QUrl>& itemsList,
                        const FCContainer& settings);
    void cancel();

Q_SIGNALS:

    void signalUrlProcessed(const QUrl& from, const QUrl& to);
    void signalCancelTask();
};

} // namespace DigikamGenericFileCopyPlugin

#endif // DIGIKAM_FC_THREAD_H
