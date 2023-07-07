/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-21
 * Description : a generic list view item widget to
 *               display metadata key like a title
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_MD_KEY_LIST_VIEW_ITEM_H
#define DIGIKAM_MD_KEY_LIST_VIEW_ITEM_H

// Qt includes

#include <QObject>
#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT MdKeyListViewItem : public QObject,
                                         public QTreeWidgetItem
{
    Q_OBJECT

public:

    MdKeyListViewItem(QTreeWidget* const parent, const QString& key);
    ~MdKeyListViewItem()              override;

    QString getKey()            const;
    QString getDecryptedKey()   const;

private Q_SLOTS:

    void slotThemeChanged();

private:

    QString m_key;
    QString m_decryptedKey;
};

} // namespace Digikam

#endif // DIGIKAM_MD_KEY_LIST_VIEW_ITEM_H
