/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-10
 * Description : Progress observer for duplicate scanning
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "duplicatesprogressobserver.h"

// Local includes

#include "dbjob.h"

namespace Digikam
{

DuplicatesProgressObserver::DuplicatesProgressObserver(SearchesJob* const thread)
    : HaarProgressObserver(),
      m_job(thread)
{
}

DuplicatesProgressObserver::~DuplicatesProgressObserver()
{
    m_job = nullptr;
}

void DuplicatesProgressObserver::imageProcessed()
{
    Q_EMIT m_job->signalImageProcessed();
}

bool DuplicatesProgressObserver::isCanceled()
{
    return m_job->isCanceled();
}

} // namespace Digikam
