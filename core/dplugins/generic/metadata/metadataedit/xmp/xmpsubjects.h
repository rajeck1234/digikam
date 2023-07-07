/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-15
 * Description : XMP subjects settings page.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_XMP_SUBJECTS_H
#define DIGIKAM_XMP_SUBJECTS_H

// Qt includes

#include <QWidget>

// Local includes

#include "dmetadata.h"
#include "subjectwidget.h"

using namespace Digikam;

namespace DigikamGenericMetadataEditPlugin
{

class XMPSubjects : public SubjectWidget
{
    Q_OBJECT

public:

    explicit XMPSubjects(QWidget* const parent);
    ~XMPSubjects() override;

    void applyMetadata(const DMetadata& meta);
    void readMetadata(const DMetadata& meta);
};

} // namespace DigikamGenericMetadataEditPlugin

#endif // DIGIKAM_XMP_SUBJECTS_H
