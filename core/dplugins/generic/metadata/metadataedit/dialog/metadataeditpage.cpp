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

#include "metadataeditpage.h"

namespace DigikamGenericMetadataEditPlugin
{

MetadataEditPage::MetadataEditPage(QWidget* const parent)
    : QScrollArea(parent)
{
    QWidget* const panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);
}

MetadataEditPage::~MetadataEditPage()
{
}

} // namespace DigikamGenericMetadataEditPlugin
