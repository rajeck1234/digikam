/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : a stack of widgets to set image file save
 *               options into image editor.
 *
 * SPDX-FileCopyrightText: 2007-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_FILE_SAVE_OPTIONS_BOX_H
#define DIGIKAM_FILE_SAVE_OPTIONS_BOX_H

// Qt includes

#include <QStackedWidget>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "digikam_config.h"

namespace Digikam
{

class DIGIKAM_EXPORT FileSaveOptionsBox : public QStackedWidget
{
    Q_OBJECT

public:

    enum FORMAT
    {
        /**
         * NOTE: Order is important here:
         * See filesaveoptionbox.cpp which use these values to fill a stack of widgets.
         */
        NONE = 0,
        JPEG,
        PNG,
        TIFF,
#ifdef HAVE_JASPER
        JP2K,
#endif
        PGF,
#ifdef HAVE_X265
        HEIF,
#endif
        JXL,
        WEBP,
        AVIF
    };

public:

    /**
     * Constructor. Don't forget to call setDialog after creation of the dialog.
     *
     * @param parent the parent for Qt's parent child mechanism
     */
    explicit FileSaveOptionsBox(QWidget* const parent = nullptr);

    /**
     * Destructor.
     */
    ~FileSaveOptionsBox() override;

    void applySettings();

    /**
     * Tries to discover a file format that has options to change based on a
     * filename.
     *
     * @param filename file name to discover the desired format from
     * @param fallback the fallback format to return if no format could be
     *                 discovered based on the filename
     * @return file format guessed from the file name or the given fallback
     *         format if no format could be guessed based on the file name
     */
    FORMAT discoverFormat(const QString& filename, FORMAT fallback = NONE);

    void setImageFileFormat(const QString&);

private:

    void readSettings();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_FILE_SAVE_OPTIONS_BOX_H
