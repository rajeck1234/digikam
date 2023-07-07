/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-04-18
 * Description : ExifTool error view.
 *
 * SPDX-FileCopyrightText: 2021-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_EXIF_TOOL_ERROR_VIEW_H
#define DIGIKAM_EXIF_TOOL_ERROR_VIEW_H

// Qt includes

#include <QWidget>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ExifToolErrorView : public QWidget
{
    Q_OBJECT

public:

    explicit ExifToolErrorView(QWidget* const parent);
    ~ExifToolErrorView() override;

    void setErrorText(const QString& err);

Q_SIGNALS:

    void signalSetupExifTool();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_EXIF_TOOL_ERROR_VIEW_H
