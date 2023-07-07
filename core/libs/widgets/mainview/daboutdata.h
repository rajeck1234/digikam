/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-07-30
 * Description : digiKam about data.
 *
 * SPDX-FileCopyrightText: 2008-2023 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DABOUT_DATA_H
#define DIGIKAM_DABOUT_DATA_H

// Qt includes

#include <QObject>
#include <QUrl>
#include <QString>

// Local includes

#include "digikam_export.h"

class KAboutData;

namespace Digikam
{

class DXmlGuiWindow;

class DIGIKAM_EXPORT DAboutData : public QObject
{
    Q_OBJECT

public:

    explicit DAboutData(DXmlGuiWindow* const parent);
    ~DAboutData() override;

    static const QString digiKamSloganFormated();
    static const QString digiKamSlogan();
    static const QString digiKamFamily();
    static const QString copyright();
    static const QUrl    webProjectUrl();
    static void          authorsRegistration(KAboutData& aboutData);

private:

    // Disable
    explicit DAboutData(QObject*);
};

} // namespace Digikam

#endif // DIGIKAM_DABOUT_DATA_H
