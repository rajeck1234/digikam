/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-23-06
 * Description : Sort settings for camera item infos
 *
 * SPDX-FileCopyrightText: 2012 by Islam Wazery <wazery at ubuntu dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "camitemsortsettings.h"

// Qt includes

#include <QRectF>

// Local includes

#include "camiteminfo.h"

namespace Digikam
{

CamItemSortSettings::CamItemSortSettings()
    : categorizationMode            (NoCategories),
      categorizationSortOrder       (DefaultOrder),
      currentCategorizationSortOrder(Qt::AscendingOrder),
      categorizationCaseSensitivity (Qt::CaseSensitive),
      sortOrder                     (DefaultOrder),
      sortRole                      (SortByFileName),
      strTypeNatural                (true),
      currentSortOrder              (Qt::AscendingOrder),
      sortCaseSensitivity           (Qt::CaseSensitive)
{
}

CamItemSortSettings::~CamItemSortSettings()
{
}

bool CamItemSortSettings::operator ==(const CamItemSortSettings& other) const
{
    return (
            (categorizationMode            == other.categorizationMode)            &&
            (categorizationSortOrder       == other.categorizationSortOrder)       &&
            (categorizationCaseSensitivity == other.categorizationCaseSensitivity) &&
            (sortRole                      == other.sortRole)                      &&
            (sortOrder                     == other.sortOrder)                     &&
            (sortCaseSensitivity           == other.sortCaseSensitivity)
           );
}

void CamItemSortSettings::setCategorizationMode(CategorizationMode mode)
{
    categorizationMode = mode;

    if (categorizationSortOrder == DefaultOrder)
    {
        currentCategorizationSortOrder = defaultSortOrderForCategorizationMode(categorizationMode);
    }
}

void CamItemSortSettings::setCategorizationSortOrder(SortOrder order)
{
    categorizationSortOrder = order;

    if (categorizationSortOrder == DefaultOrder)
    {
        currentCategorizationSortOrder = defaultSortOrderForCategorizationMode(categorizationMode);
    }
    else
    {
        currentCategorizationSortOrder = (Qt::SortOrder)categorizationSortOrder;
    }
}

void CamItemSortSettings::setSortRole(SortRole role)
{
    sortRole = role;

    if (sortOrder == DefaultOrder)
    {
        currentSortOrder = defaultSortOrderForSortRole(sortRole);
    }
}

void CamItemSortSettings::setSortOrder(SortOrder order)
{
    sortOrder = order;
    setCategorizationSortOrder(order);

    if (sortOrder == DefaultOrder)
    {
        currentSortOrder = defaultSortOrderForSortRole(sortRole);
    }
    else
    {
        currentSortOrder = (Qt::SortOrder)order;
    }
}

void CamItemSortSettings::setStringTypeNatural(bool natural)
{
    strTypeNatural = natural;
}

Qt::SortOrder CamItemSortSettings::defaultSortOrderForCategorizationMode(CategorizationMode mode)
{
    switch (mode)
    {
        case NoCategories:
        case CategoryByFolder:
        case CategoryByFormat:
        case CategoryByDate:
        default:
        {
            return Qt::AscendingOrder;
        }
    }
}

Qt::SortOrder CamItemSortSettings::defaultSortOrderForSortRole(SortRole role)
{
    switch (role)
    {
        case SortByFileName:
        case SortByFilePath:
        {
            return Qt::AscendingOrder;
        }

        case SortByFileSize:
        {
            return Qt::DescendingOrder;
        }

        case SortByCreationDate:
        {
            return Qt::AscendingOrder;
        }

        case SortByDownloadState:
        {
            return Qt::AscendingOrder;
        }

        case SortByRating:
        {
            return Qt::DescendingOrder;
        }

        default:
        {
            return Qt::AscendingOrder;
        }
    }
}

int CamItemSortSettings::compareCategories(const CamItemInfo& left, const CamItemInfo& right) const
{
    switch (categorizationMode)
    {
        case NoCategories:
        case CategoryByFolder:
        {
            return naturalCompare(left.folder, right.folder,
                                  currentCategorizationSortOrder,
                                  categorizationCaseSensitivity, strTypeNatural);
        }

        case CategoryByFormat:
        {
            return naturalCompare(left.mime, right.mime,
                                  currentCategorizationSortOrder,
                                  categorizationCaseSensitivity, strTypeNatural);
        }

        case CategoryByDate:
        {
            return compareByOrder(left.ctime.date(), right.ctime.date(),
                                  currentCategorizationSortOrder);
        }

        default:
        {
            return 0;
        }
    }
}

bool CamItemSortSettings::lessThan(const CamItemInfo& left, const CamItemInfo& right) const
{
    int result = compare(left, right, sortRole);

    if (result != 0)
    {
        return (result < 0);
    }

    if (left == right)
    {
        return false;
    }

    if ((result = compare(left, right, SortByFileName)) != 0)
    {
        return (result < 0);
    }

    if ((result = compare(left, right, SortByCreationDate)) != 0)
    {
        return (result < 0);
    }

    if ((result = compare(left, right, SortByFilePath)) != 0)
    {
        return (result < 0);
    }

    if ((result = compare(left, right, SortByFileSize)) != 0)
    {
        return (result < 0);
    }

    if ((result = compare(left, right, SortByRating)) != 0)
    {
        return (result < 0);
    }

    if ((result = compare(left, right, SortByDownloadState)) != 0)
    {
        return (result < 0);
    }

    return false;
}

int CamItemSortSettings::compare(const CamItemInfo& left, const CamItemInfo& right) const
{
    return compare(left, right, sortRole);
}

int CamItemSortSettings::compare(const CamItemInfo& left, const CamItemInfo& right, SortRole role) const
{
    switch (role)
    {
        case SortByFileName:
        {
            return naturalCompare(left.name, right.name,
                                  currentSortOrder, sortCaseSensitivity, strTypeNatural);
        }

        case SortByFilePath:
        {
            return naturalCompare(left.url().toLocalFile(), right.url().toLocalFile(),
                                  currentSortOrder, sortCaseSensitivity, strTypeNatural);
        }

        case SortByFileSize:
        {
            return compareByOrder(left.size, right.size, currentSortOrder);
            //FIXME: Change it to creation date instead of modification date.
        }

        case SortByCreationDate:
        {
            return compareByOrder(left.ctime, right.ctime, currentSortOrder);
        }

        case SortByRating:
        {
            return compareByOrder(left.rating, right.rating, currentSortOrder);
        }

        case SortByDownloadState:
        {
            return compareByOrder(left.downloaded, right.downloaded, currentSortOrder);
        }

        default:
        {
            return 1;
        }
    }
}

bool CamItemSortSettings::lessThan(const QVariant& left, const QVariant& right) const
{

#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))
    if (left.typeId() != right.typeId())
#else
    if (left.type() != right.type())
#endif
    {
        return false;
    }

#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))
    switch (left.typeId())
#else
    switch (left.type())
#endif
    {
        case QVariant::Int:
        {
            return compareByOrder(left.toInt(), right.toInt(), currentSortOrder);
        }

        case QVariant::UInt:
        {
            return compareByOrder(left.toUInt(), right.toUInt(), currentSortOrder);
        }

        case QVariant::LongLong:
        {
            return compareByOrder(left.toLongLong(), right.toLongLong(), currentSortOrder);
        }

        case QVariant::ULongLong:
        {
            return compareByOrder(left.toULongLong(), right.toULongLong(), currentSortOrder);
        }

        case QVariant::Double:
        {
            return compareByOrder(left.toDouble(), right.toDouble(), currentSortOrder);
        }

        case QVariant::Date:
        {
            return compareByOrder(left.toDate(), right.toDate(), currentSortOrder);
        }

        case QVariant::DateTime:
        {
            return compareByOrder(left.toDateTime(), right.toDateTime(), currentSortOrder);
        }

        case QVariant::Time:
        {
            return compareByOrder(left.toTime(), right.toTime(), currentSortOrder);
        }

        case QVariant::Rect:
        case QVariant::RectF:
        {
            QRectF rectLeft  = left.toRectF();
            QRectF rectRight = right.toRectF();
            int result;

            if ((result = compareByOrder(rectLeft.top(), rectRight.top(), currentSortOrder)) != 0)
            {
                return (result < 0);
            }

            if ((result = compareByOrder(rectLeft.left(), rectRight.left(), currentSortOrder)) != 0)
            {
                return (result < 0);
            }

            QSizeF sizeLeft  = rectLeft.size();
            QSizeF sizeRight = rectRight.size();

            if ((result = compareByOrder(sizeLeft.width()*sizeLeft.height(), sizeRight.width()*sizeRight.height(), currentSortOrder)) != 0)
            {
                return (result < 0);
            }

#if __GNUC__ >= 7   // krazy:exclude=cpp
            [[fallthrough]];
#endif
        }

        default:
        {
            return naturalCompare(left.toString(), right.toString(),
                                  currentSortOrder, sortCaseSensitivity, strTypeNatural);
        }
    }
}

} // namespace Digikam
