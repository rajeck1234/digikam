/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-24
 * Description : file list view and items.
 *
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DNG_CONVERTER_LIST_H
#define DIGIKAM_DNG_CONVERTER_LIST_H

// Qt includes

#include <QUrl>

// Local includes

#include "ditemslist.h"

using namespace Digikam;

namespace DigikamGenericDNGConverterPlugin
{

class DNGConverterList : public DItemsList
{
    Q_OBJECT

public:

    /**
     * List the different columns in a list.
     */
    enum FieldType
    {
        TARGETFILENAME = DItemsListView::User1,
        IDENTIFICATION = DItemsListView::User2,
        STATUS         = DItemsListView::User3
    };

public:

    explicit DNGConverterList(QWidget* const parent = nullptr);
    ~DNGConverterList();

public Q_SLOTS:

    void slotAddImages(const QList<QUrl>& list) override;

protected Q_SLOTS:

    void slotRemoveItems()                      override;
};

// -------------------------------------------------------------------------

class DNGConverterListViewItem : public DItemsListViewItem
{

public:

    DNGConverterListViewItem(DItemsListView* const view, const QUrl& url);
    ~DNGConverterListViewItem();

    void setDestFileName(const QString& str);
    QString destFileName()  const;

    void setIdentity(const QString& str);
    QString identity()      const;

    void setStatus(const QString& str);

    QString destPath()      const;

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericDNGConverterPlugin

#endif // DIGIKAM_DNG_CONVERTER_LIST_H
