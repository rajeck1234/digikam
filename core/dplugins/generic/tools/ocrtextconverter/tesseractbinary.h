/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect Tesseract binary program
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TESSERACT_BINARY_H
#define DIGIKAM_TESSERACT_BINARY_H

// Qt includes

#include <QStringList>

// Local includes

#include "dbinaryiface.h"

using namespace Digikam;

namespace DigikamGenericTextConverterPlugin
{

class TesseractBinary : public DBinaryIface
{
    Q_OBJECT

public:

    explicit TesseractBinary(QObject* const parent = nullptr);
    ~TesseractBinary()                      override;

    QStringList tesseractLanguages() const;
};

} // namespace DigikamGenericTextConverterPlugin

#endif // DIGIKAM_TESSERACT_BINARY_H
