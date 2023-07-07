/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-07
 * Description : a tool to print images
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ADV_PRINT_THREAD_H
#define DIGIKAM_ADV_PRINT_THREAD_H

// Local includes

#include "advprintsettings.h"
#include "advprinttask.h"
#include "actionthreadbase.h"

using namespace Digikam;

namespace DigikamGenericPrintCreatorPlugin
{

class AdvPrintThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit AdvPrintThread(QObject* const parent);
    ~AdvPrintThread() override;

    void preparePrint(AdvPrintSettings* const settings, int sizeIndex);
    void print(AdvPrintSettings* const settings);
    void preview(AdvPrintSettings* const settings, const QSize& size);

Q_SIGNALS:

    void signalProgress(int);
    void signalDone(bool);
    void signalMessage(const QString&, bool);
    void signalPreview(const QImage&);
};

} // namespace DigikamGenericPrintCreatorPlugin

#endif // DIGIKAM_ADV_PRINT_THREAD_H
