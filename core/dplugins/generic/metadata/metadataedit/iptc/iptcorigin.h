/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-13
 * Description : IPTC origin settings page.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_IPTC_ORIGIN_H
#define DIGIKAM_IPTC_ORIGIN_H

// Qt includes

#include <QWidget>
#include <QDateTime>

// Local includes

#include "dmetadata.h"
#include "metadataeditpage.h"

using namespace Digikam;

namespace DigikamGenericMetadataEditPlugin
{

class IPTCOrigin : public MetadataEditPage
{
    Q_OBJECT

public:

    explicit IPTCOrigin(QWidget* const parent);
    ~IPTCOrigin() override;

    void applyMetadata(const DMetadata& meta);
    void readMetadata(const DMetadata& meta);

    bool syncEXIFDateIsChecked()            const;

    void setCheckedSyncEXIFDate(bool c);

    QDateTime getIPTCCreationDate()         const;

Q_SIGNALS:

    void signalModified();

private Q_SLOTS:

    void slotSetTodayCreated();
    void slotSetTodayDigitalized();
    void slotLineEditModified();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericMetadataEditPlugin

#endif // DIGIKAM_IPTC_ORIGIN_H
