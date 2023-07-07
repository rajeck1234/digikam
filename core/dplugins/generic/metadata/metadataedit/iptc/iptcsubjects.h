/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-15
 * Description : IPTC subjects settings page.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_IPTC_SUBJECTS_H
#define DIGIKAM_IPTC_SUBJECTS_H

// Qt includes

#include <QWidget>
#include <QByteArray>

// Local includes

#include "subjectwidget.h"
#include "dmetadata.h"

using namespace Digikam;

namespace DigikamGenericMetadataEditPlugin
{

class IPTCSubjects : public SubjectWidget
{
    Q_OBJECT

public:

    explicit IPTCSubjects(QWidget* const parent);
    ~IPTCSubjects() override;

    void applyMetadata(const DMetadata& meta);
    void readMetadata(const DMetadata& meta);

private Q_SLOTS:

    void slotLineEditModified();
};

} // namespace DigikamGenericMetadataEditPlugin

#endif // DIGIKAM_IPTC_SUBJECTS_H
