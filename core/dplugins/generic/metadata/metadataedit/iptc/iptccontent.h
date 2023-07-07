/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-12
 * Description : IPTC caption settings page.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_IPTC_CONTENT_H
#define DIGIKAM_IPTC_CONTENT_H

// Qt includes

#include <QWidget>

// Local includes

#include "dmetadata.h"
#include "metadataeditpage.h"

using namespace Digikam;

namespace DigikamGenericMetadataEditPlugin
{

class IPTCContent : public MetadataEditPage
{
    Q_OBJECT

public:

    explicit IPTCContent(QWidget* const parent);
    ~IPTCContent() override;

    void applyMetadata(const DMetadata& meta);
    void readMetadata(const DMetadata& meta);

    bool syncJFIFCommentIsChecked() const;
    bool syncEXIFCommentIsChecked() const;

    void setCheckedSyncJFIFComment(bool c);
    void setCheckedSyncEXIFComment(bool c);

    QString getIPTCCaption() const;

Q_SIGNALS:

    void signalModified();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericMetadataEditPlugin

#endif // DIGIKAM_IPTC_CONTENT_H
