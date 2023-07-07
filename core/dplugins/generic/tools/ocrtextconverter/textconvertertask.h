/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-26
 * Description : Text Converter thread
 *
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2022      by Quoc Hung Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TEXT_CONVERTER_TASK_H
#define DIGIKAM_TEXT_CONVERTER_TASK_H

// Qt includes

#include <QUrl>

// Local includes

#include "ocroptions.h"
#include "actionthreadbase.h"
#include "textconverteraction.h"

using namespace Digikam;

namespace DigikamGenericTextConverterPlugin
{

class TextConverterTask : public ActionJob
{
    Q_OBJECT

public:

    explicit TextConverterTask(QObject* const parent,
                              const QUrl& url,
                              const TextConverterAction& action);
    ~TextConverterTask();

    OcrOptions ocrOptions() const;
    void setOcrOptions(const OcrOptions& opt);

Q_SIGNALS:

    void signalStarting(const DigikamGenericTextConverterPlugin::TextConverterActionData& ad);
    void signalFinished(const DigikamGenericTextConverterPlugin::TextConverterActionData& ad);

public Q_SLOTS:

    void slotCancel();

protected:

    void run();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericTextConverterPlugin

#endif // DIGIKAM_Text_CONVERTER_TASK_H
