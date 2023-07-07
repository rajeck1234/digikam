/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-03-20
 * Description : a tool to export images to iNaturalist web service
 *
 * SPDX-FileCopyrightText: 2021      by Joerg Lohse <joergmlpts at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_INAT_TAXON_EDIT_H
#define DIGIKAM_INAT_TAXON_EDIT_H

// Qt includes

#include <QWidget>
#include <QLineEdit>
#include <QFocusEvent>

namespace DigikamGenericINatPlugin
{

class TaxonEdit : public QLineEdit
{
    Q_OBJECT

public:

    explicit TaxonEdit(QWidget* const parent = nullptr);
    ~TaxonEdit();

Q_SIGNALS:

    void inFocus();

protected:

    virtual void focusInEvent(QFocusEvent* e);
};

} // namespace DigikamGenericINatPlugin

#endif // DIGIKAM_INAT_TAXON_EDIT_H
