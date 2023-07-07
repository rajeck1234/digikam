/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_INVISIBLE_BUTTON_GROUP_H
#define DIGIKAM_INVISIBLE_BUTTON_GROUP_H

// Qt includes

#include <QWidget>

class QAbstractButton;

namespace DigikamGenericHtmlGalleryPlugin
{

class InvisibleButtonGroup : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int current READ selected WRITE setSelected)

public:

    explicit InvisibleButtonGroup(QWidget* const parent);
    ~InvisibleButtonGroup() override;

    int selected() const;

    void addButton(QAbstractButton* button, int id);

public Q_SLOTS:

    void setSelected(int id);

Q_SIGNALS:

    void selectionChanged(int id);

private:

    // Disable
    InvisibleButtonGroup() = delete;

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericHtmlGalleryPlugin

#endif // DIGIKAM_INVISIBLE_BUTTON_GROUP_H
