/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Vincent Garcia <xavier dot vincent dot garcia at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_MEDIAWIKI_PARSE_H
#define DIGIKAM_MEDIAWIKI_PARSE_H

// Qt includes

#include <QString>
#include <QLocale>

// Local includes

#include "mediawiki_job.h"


namespace MediaWiki
{

class Iface;
class ParsePrivate;

class Parse : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Parse)

public:

    /**
     * @brief Indicates all possible error conditions found during the processing of the job.
     */
    enum
    {
         /**
         * @brief An internal error occurred.
         */
        InternalError= Job::UserDefinedError+1,

        /**
         * @brief The page parameter cannot be used together with the text and title parameters
         */
        TooManyParams,

        /**
         * @brief The page you specified doesn't exist
         */
        MissingPage
    };

    explicit Parse(Iface& MediaWiki, QObject* const parent = nullptr);
    ~Parse() override;

    void setText(const QString& param);

    void setTitle(const QString& param);

    void setPageName(const QString& param);

    void setUseLang(const QString& param);

    void start() override;

    Q_SIGNALS:

    void result(const QString& text);

private Q_SLOTS:

    void doWorkSendRequest();
    void doWorkProcessReply();
};

} // namespace MediaWiki

#endif // DIGIKAM_MEDIAWIKI_PARSE_H
