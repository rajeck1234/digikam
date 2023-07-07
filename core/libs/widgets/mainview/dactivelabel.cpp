/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-12
 * Description : A label with an active url
 *
 * SPDX-FileCopyrightText: 2014-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dactivelabel.h"

// Qt includes

#include <QByteArray>
#include <QBuffer>

namespace Digikam
{

DActiveLabel::DActiveLabel(const QUrl& url, const QString& imgPath, QWidget* const parent)
    : QLabel(parent)
{
    setContentsMargins(QMargins());
    setScaledContents(false);
    setOpenExternalLinks(true);
    setTextFormat(Qt::RichText);
    setFocusPolicy(Qt::NoFocus);
    setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));

    if (imgPath.isEmpty())
    {
        updateData(url, QImage());
    }
    else
    {
        updateData(url, QImage(imgPath));
    }
}

DActiveLabel::~DActiveLabel()
{
}

void DActiveLabel::updateData(const QUrl& url, const QImage& img)
{
    QByteArray byteArray;
    QBuffer    buffer(&byteArray);
    img.save(&buffer, "PNG");
    setText(QString::fromLatin1("<a href=\"%1\">%2</a>")
            .arg(url.url())
            .arg(QString::fromLatin1("<img src=\"data:image/png;base64,%1\">")
            .arg(QString::fromLatin1(byteArray.toBase64().data()))));
}

} // namespace Digikam
