/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-04
 * Description : A label to show video frame effect preview
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_EFFECT_PREVIEW_H
#define DIGIKAM_EFFECT_PREVIEW_H

// Qt includes

#include <QLabel>
#include <QString>
#include <QList>
#include <QUrl>

// Local includes

#include "effectmngr.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT EffectPreview : public QLabel
{
    Q_OBJECT

public:

    explicit EffectPreview(QWidget* const parent = nullptr);
    ~EffectPreview() override;

    void setImagesList(const QList<QUrl>& images);

    void startPreview(EffectMngr::EffectType eff);
    void stopPreview();

private Q_SLOTS:

    void slotProgressEffect();
    void slotRestart();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_EFFECT_PREVIEW_H
