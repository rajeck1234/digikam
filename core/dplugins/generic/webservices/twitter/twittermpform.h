/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-06-29
 * Description : a tool to export images to Twitter social network
 *
 * SPDX-FileCopyrightText: 2018 by Tarek Talaat <tarektalaat93 at gmail dot com>
 * SPDX-FileCopyrightText: 2019 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TWITTER_MPFORM_H
#define DIGIKAM_TWITTER_MPFORM_H

// Qt includes

#include <QByteArray>
#include <QString>
#include <QList>

#define MAX_MEDIA_SIZE 1048576

namespace DigikamGenericTwitterPlugin
{

class TwMPForm
{

public:

    explicit TwMPForm();
    ~TwMPForm();

    void reset();
    QByteArray fileHeader(const QString& imgPath);
    bool addFile(const QString& imgPath, bool fragmented = false);
    QByteArray createPair(const QByteArray& name, const QByteArray& value);
    bool addPair(const QByteArray& pair);
    void finish();
    QByteArray border();

    int numberOfChunks()           const;
    QString contentType()          const;
    QByteArray formData()          const;
    QByteArray getChunk(int index) const;

private:

    void formChunks(const QByteArray& data);

private:

    QByteArray        m_buffer;
    QByteArray        m_boundary;
    QList<QByteArray> m_chunks;
};

} // namespace DigikamGenericTwitterPlugin

#endif // DIGIKAM_TWITTER_MPFORM_H
