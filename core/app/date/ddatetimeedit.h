/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : a widget to edit time stamp.
 *
 * SPDX-FileCopyrightText: 2005      by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DDATE_TIME_EDIT_H
#define DIGIKAM_DDATE_TIME_EDIT_H

// Qt includes

#include <QDateTime>

// Local includes

#include "dlayoutbox.h"
#include "digikam_export.h"

namespace Digikam
{

/**
 * This class is basically the same as the KDE Date Time widget
 * with the exception that a QTimeEdit is placed directly
 * besides it.
 */
class DIGIKAM_GUI_EXPORT DDateTimeEdit : public DHBox
{
    Q_OBJECT

public:

    /**
     * constructor
     * @param parent the parent widget
     * @param name the name of the widget
     */
    explicit DDateTimeEdit(QWidget* const parent, const QString& name);

    /**
     * destructor
     */
    ~DDateTimeEdit() override;

    /**
     * returns the date and time
     * @return a QDateTime with the currently chosen date and time
     */
    QDateTime dateTime() const;

    /**
     * Sets the date and the time of this widget.
     */
    void setDateTime(const QDateTime& dateTime);

Q_SIGNALS:

    /**
     * This signal is emitted whenever the user modifies the date or time.
     * The passed date and time can be invalid.
     */
    void dateTimeChanged(const QDateTime& dateTime);

private Q_SLOTS:

    void slotDateTimeChanged();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_DDATE_TIME_EDIT_H
