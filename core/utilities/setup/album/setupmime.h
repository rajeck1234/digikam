/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-05-03
 * Description : mime types setup tab.
 *
 * SPDX-FileCopyrightText: 2004-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_SETUP_MIME_H
#define DIGIKAM_SETUP_MIME_H

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupMime : public QScrollArea
{
    Q_OBJECT

public:

    explicit SetupMime(QWidget* const parent = nullptr);
    ~SetupMime() override;

    void applySettings();
    void readSettings();

private Q_SLOTS:

    void slotShowCurrentImageSettings();
    void slotShowCurrentMovieSettings();
    void slotShowCurrentAudioSettings();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_SETUP_MIME_H
