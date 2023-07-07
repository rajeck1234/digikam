/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-10
 * Description : flip image batch tool.
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_FLIP_H
#define DIGIKAM_BQM_FLIP_H

// Local includes

#include "batchtool.h"

class QComboBox;

using namespace Digikam;

namespace DigikamBqmFlipPlugin
{

class Flip : public BatchTool
{
    Q_OBJECT

public:

    explicit Flip(QObject* const parent = nullptr);
    ~Flip()                                                 override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

private:

    QComboBox* m_comboBox;
};

} // namespace DigikamBqmFlipPlugin

#endif // DIGIKAM_BQM_FLIP_H
