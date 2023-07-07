/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-01
 * Description : A simple backend to search OSM and Geonames.org.
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_SEARCH_BACKEND_H
#define DIGIKAM_SEARCH_BACKEND_H

// Qt includes

#include <QObject>
#include <QNetworkReply>

// Local includes

#include "geoifacetypes.h"
#include "rgwidget.h"

using namespace Digikam;

namespace DigikamGenericGeolocationEditPlugin
{

class SearchBackend : public QObject
{
    Q_OBJECT

public:

    class SearchResult
    {
    public:

        explicit SearchResult()
        {
        }

        typedef QList<SearchResult> List;
        GeoCoordinates              coordinates;
        QString                     name;
        GeoCoordinates::Pair        boundingBox;
        QString                     internalId;
    };

public:

    explicit SearchBackend(QObject* const parent);
    ~SearchBackend()                                      override;

    bool search(const QString& backendName, const QString& searchTerm);
    SearchResult::List getResults()                 const;
    QString getErrorMessage()                       const;
    QList<QPair<QString, QString> >  getBackends()  const;

Q_SIGNALS:

    void signalSearchCompleted();

private Q_SLOTS:

    void slotFinished(QNetworkReply* reply);

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericGeolocationEditPlugin

#endif // DIGIKAM_SEARCH_BACKEND_H
