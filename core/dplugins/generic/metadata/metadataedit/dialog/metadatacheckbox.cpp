/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-18
 * Description : a checkbox with a boolean valid parameter.
 *               The boolean statement is used to check if
 *               a metadata value from a picture have a know
 *               value registered by EXIF/IPTC spec.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metadatacheckbox.h"

namespace DigikamGenericMetadataEditPlugin
{

MetadataCheckBox::MetadataCheckBox(const QString& text, QWidget* const parent)
    : QCheckBox(text, parent),
      m_valid  (true)
{
    connect(this, SIGNAL(toggled(bool)),
            this, SLOT(slotValid()));
}

MetadataCheckBox::~MetadataCheckBox()
{
}

void MetadataCheckBox::setValid(bool v)
{
    m_valid = v;
}

bool MetadataCheckBox::isValid() const
{
    return m_valid;
}

void MetadataCheckBox::slotValid()
{
    setValid(true);
}

} // namespace DigikamGenericMetadataEditPlugin
