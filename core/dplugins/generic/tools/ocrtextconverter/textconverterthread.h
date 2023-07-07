/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-26
 * Description : Text Converter threads manager
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TEXT_CONVERTER_THREAD_H
#define DIGIKAM_TEXT_CONVERTER_THREAD_H

// Qt includes

#include <QUrl>

// Local includes

#include "ocroptions.h"
#include "actionthreadbase.h"
#include "textconverteraction.h"

using namespace Digikam;

namespace DigikamGenericTextConverterPlugin
{

class TextConverterActionThread : public ActionThreadBase
{
    Q_OBJECT

public:

    explicit TextConverterActionThread(QObject* const parent);
    ~TextConverterActionThread();

    OcrOptions ocrOptions() const;
    void setOcrOptions(const OcrOptions& opt);

    void ocrProcessFile(const QUrl& url);
    void ocrProcessFiles(const QList<QUrl>& urlList);

    void cancel();

Q_SIGNALS:

    void signalStarting(const DigikamGenericTextConverterPlugin::TextConverterActionData& ad);
    void signalFinished(const DigikamGenericTextConverterPlugin::TextConverterActionData& ad);

    /**
     * Signal emitted to sub-tasks for cancel processing.
     */
    void signalCancelTextConverterTask();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericTextConverterPlugin

#endif // DIGIKAM_TEXT_CONVERTER_THREAD_H
