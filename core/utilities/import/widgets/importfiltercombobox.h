/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-16
 * Description : Import filter combobox
 *
 * SPDX-FileCopyrightText: 2010-2011 by Petri Damstén <petri dot damsten at iki dot fi>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_IMPORT_FILTER_COMBOBOX_H
#define DIGIKAM_IMPORT_FILTER_COMBOBOX_H

// Qt includes

#include <QComboBox>

// Local includes

#include "importfilter.h"

namespace Digikam
{

class CamItemInfo;

class ImportFilterComboBox : public QComboBox
{
    Q_OBJECT

public:

    explicit ImportFilterComboBox(QWidget* const parent);
    ~ImportFilterComboBox() override;

    Filter* currentFilter();
    void saveSettings();
    void updateFilter();

    static void  defaultFilters(FilterList* const filters);
    static const QString defaultIgnoreNames;
    static const QString defaultIgnoreExtensions;

Q_SIGNALS:

    void signalFilterChanged(Filter*);

protected:

    void  fillCombo();

protected Q_SLOTS:

    void indexChanged(int index);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_IMPORT_FILTER_COMBOBOX_H
