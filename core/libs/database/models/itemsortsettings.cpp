/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Filter values for use with ItemFilterModel
 *
 * SPDX-FileCopyrightText: 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2014 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemsortsettings.h"

// Qt includes

#include <QRectF>
#include <QDateTime>

// Local includes

#include "iteminfo.h"
#include "coredbfields.h"
#include "facetagseditor.h"

namespace Digikam
{

ItemSortSettings::ItemSortSettings()
    : categorizationMode            (NoCategories),
      categorizationSortOrder       (DefaultOrder),
      currentCategorizationSortOrder(Qt::AscendingOrder),
      categorizationCaseSensitivity (Qt::CaseSensitive),
      sortRole                      (SortByFileName),
      sortOrder                     (DefaultOrder),
      strTypeNatural                (true),
      currentSortOrder              (Qt::AscendingOrder),
      sortCaseSensitivity           (Qt::CaseSensitive)
{
}

bool ItemSortSettings::operator==(const ItemSortSettings& other) const
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

void ItemSortSettings::setCategorizationMode(CategorizationMode mode)
{
    categorizationMode = mode;

    if (categorizationSortOrder == DefaultOrder)
    {
        currentCategorizationSortOrder = defaultSortOrderForCategorizationMode(categorizationMode);
    }
}

void ItemSortSettings::setCategorizationSortOrder(SortOrder order)
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

void ItemSortSettings::setSortRole(SortRole role)
{
    sortRole = role;

    if (sortOrder == DefaultOrder)
    {
        currentSortOrder = defaultSortOrderForSortRole(sortRole);
    }
}

void ItemSortSettings::setSortOrder(SortOrder order)
{
    sortOrder = order;

    if (sortOrder == DefaultOrder)
    {
        currentSortOrder = defaultSortOrderForSortRole(sortRole);
    }
    else
    {
        currentSortOrder = (Qt::SortOrder)order;
    }
}

void ItemSortSettings::setStringTypeNatural(bool natural)
{
    strTypeNatural = natural;
}

Qt::SortOrder ItemSortSettings::defaultSortOrderForCategorizationMode(CategorizationMode mode)
{
    switch (mode)
    {
        case OneCategory:
        case NoCategories:
        case CategoryByAlbum:
        case CategoryByMonth:
        case CategoryByFormat:
        default:
        {
            return Qt::AscendingOrder;
        }
    }
}

Qt::SortOrder ItemSortSettings::defaultSortOrderForSortRole(SortRole role)
{
    switch (role)
    {
        case SortByFilePath:
        case SortByFileName:
        case SortByCreationDate:
        case SortByModificationDate:
        case SortByManualOrderAndName:
        case SortByManualOrderAndDate:
        case SortByFaces:
        {
            return Qt::AscendingOrder;
        }

        case SortByRating:
        case SortByFileSize:
        case SortByImageSize:
        case SortBySimilarity:
        case SortByAspectRatio:
        {
            return Qt::DescendingOrder;
        }

        default:
        {
            return Qt::AscendingOrder;
        }
    }
}

// Feel free to optimize. QString::number is 3x slower.
static inline QString fastNumberToString(int id)
{
    const int size = sizeof(int) * 2;
    int number     = id;
    char c[size + 1];
    c[size]        = '\0';

    for (int i = 0 ; i < size ; ++i)
    {
        c[i]     = 'a' + (number & 0xF);
        number >>= 4;
    }

    return QLatin1String(c);
}

int ItemSortSettings::compareCategories(const ItemInfo& left, const ItemInfo& right,
                                        const FaceTagsIface& leftFace, const FaceTagsIface& rightFace) const
{
    switch (categorizationMode)
    {
        case NoCategories:
        case OneCategory:
        {
            return 0;
        }

        case CategoryByAlbum:
        {
            int leftAlbum  = left.albumId();
            int rightAlbum = right.albumId();

            // return comparison result

            if      (leftAlbum == rightAlbum)
            {
                return 0;
            }
            else if (lessThanByOrder(leftAlbum, rightAlbum,
                                     currentCategorizationSortOrder))
            {
                return -1;
            }
            else
            {
                return 1;
            }
        }

        case CategoryByFormat:
        {
            return naturalCompare(left.format(), right.format(),
                                  currentCategorizationSortOrder,
                                  categorizationCaseSensitivity, strTypeNatural);
        }

        case CategoryByMonth:
        {
            return compareByOrder(left.dateTime().date(),
                                  right.dateTime().date(),
                                  currentCategorizationSortOrder);
        }

        case CategoryByFaces:
        {
            if (leftFace.isNull() && rightFace.isNull())
            {
                return compareByOrder(left.faceCount(),
                                      right.faceCount(),
                                      currentCategorizationSortOrder);
            }

            bool isLeftIgnored = (leftFace.type() == FaceTagsIface::IgnoredName);
            bool isLeftUnknown = (leftFace.type() == FaceTagsIface::UnknownName);

            if (isLeftIgnored != (rightFace.type() == FaceTagsIface::IgnoredName))
            {
                if (currentCategorizationSortOrder == Qt::AscendingOrder)
                {
                    return isLeftIgnored ? 1 : -1;
                }
                else
                {
                    return isLeftIgnored ? -1 : 1;
                }
            }

            if (isLeftUnknown != (rightFace.type() == FaceTagsIface::UnknownName))
            {
                if (currentCategorizationSortOrder == Qt::AscendingOrder)
                {
                    return isLeftUnknown ? 1 : -1;
                }
                else
                {
                    return isLeftUnknown ? -1 : 1;
                }
            }

            QString leftValue;
            QString rightValue;

            if      (leftFace.type() == FaceTagsIface::ConfirmedName)
            {
                leftValue = FaceTags::faceNameForTag(leftFace.tagId());
            }
            else if (leftFace.type() == FaceTagsIface::UnconfirmedName)
            {
                leftValue = left.getSuggestedNames().value(leftFace.region().toXml());
            }

            if      (rightFace.type() == FaceTagsIface::ConfirmedName)
            {
                rightValue = FaceTags::faceNameForTag(rightFace.tagId());
            }
            else if (rightFace.type() == FaceTagsIface::UnconfirmedName)
            {
                 rightValue = right.getSuggestedNames().value(rightFace.region().toXml());
            }

            leftValue  += fastNumberToString(leftFace.tagId()) +
                          fastNumberToString(leftFace.type());

            rightValue += fastNumberToString(rightFace.tagId()) +
                          fastNumberToString(rightFace.type());

            // Compare alphabetically based on the face name

            return naturalCompare(leftValue, rightValue,
                                  currentCategorizationSortOrder,
                                  categorizationCaseSensitivity, strTypeNatural);
        }

        default:
        {
            return 0;
        }
    }
}

bool ItemSortSettings::lessThan(const ItemInfo& left, const ItemInfo& right) const
{
    int result = compare(left, right, sortRole);

    if (result != 0)
    {
        return (result < 0);
    }

    // are they identical?

    if (left == right)
    {
        return false;
    }

    // If left and right equal for first sort order, use a hierarchy of all sort orders

    if ((result = compare(left, right, SortByFileName)) != 0)
    {
        return (result < 0);
    }

    if ((result = compare(left, right, SortByCreationDate)) != 0)
    {
        return (result < 0);
    }

    if ((result = compare(left, right, SortByModificationDate)) != 0)
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

    if ((result = compare(left, right, SortBySimilarity)) != 0)
    {
        return (result < 0);
    }

    if ((result = compare(left, right, SortByManualOrderAndName)) != 0)
    {
        return (result < 0);
    }

    if ((result = compare(left, right, SortByManualOrderAndDate)) != 0)
    {
        return (result < 0);
    }

    return false;
}

int ItemSortSettings::compare(const ItemInfo& left, const ItemInfo& right) const
{
    return compare(left, right, sortRole);
}

int ItemSortSettings::compare(const ItemInfo& left, const ItemInfo& right, SortRole role) const
{
    switch (role)
    {
        case SortByFileName:
        {
            return naturalCompare(left.name(), right.name(),
                                  currentSortOrder, sortCaseSensitivity, strTypeNatural);
        }

        case SortByFilePath:
        {
            return naturalCompare(left.filePath(), right.filePath(),
                                  currentSortOrder, sortCaseSensitivity, strTypeNatural);
        }

        case SortByFileSize:
        {
            return compareByOrder(left.fileSize(), right.fileSize(), currentSortOrder);
        }

        case SortByCreationDate:
        {
            return compareByOrder(left.dateTime(), right.dateTime(), currentSortOrder);
        }

        case SortByModificationDate:
        {
            return compareByOrder(left.modDateTime(), right.modDateTime(), currentSortOrder);
        }

        case SortByRating:
        {
            // I have the feeling that inverting the sort order for rating is the natural order

            return - compareByOrder(left.rating(), right.rating(), currentSortOrder);
        }

        case SortByImageSize:
        {
            QSize leftSize  = left.dimensions();
            QSize rightSize = right.dimensions();
            int leftPixels  = leftSize.width()  * leftSize.height();
            int rightPixels = rightSize.width() * rightSize.height();
            return compareByOrder(leftPixels, rightPixels, currentSortOrder);
        }

        case SortByAspectRatio:
        {
            QSize leftSize  = left.dimensions();
            QSize rightSize = right.dimensions();
            int leftAR      = (double(leftSize.width())  / double(leftSize.height()))  * 1000000;
            int rightAR     = (double(rightSize.width()) / double(rightSize.height())) * 1000000;
            return compareByOrder(leftAR, rightAR, currentSortOrder);
        }

        case SortBySimilarity:
        {
            qlonglong leftReferenceImageId  = left.currentReferenceImage();
            qlonglong rightReferenceImageId = right.currentReferenceImage();

            // make sure that the original image has always the highest similarity.

            double leftSimilarity           = left.id()  == leftReferenceImageId  ? 1.1 : left.currentSimilarity();
            double rightSimilarity          = right.id() == rightReferenceImageId ? 1.1 : right.currentSimilarity();
            return compareByOrder(leftSimilarity, rightSimilarity, currentSortOrder);
        }

        // Implementation to make Unconfirmed Faces of a tag appear before Confirmed faces.

        case SortByFaces:
        {
            return compareByOrder(right.unconfirmedFaceCount(), left.unconfirmedFaceCount(), currentSortOrder);
        }

        case SortByManualOrderAndName:
        case SortByManualOrderAndDate:
        {
            int result;

            if ((result = compareByOrder(left.manualOrder(), right.manualOrder(), currentSortOrder)) != 0)
            {
                return result;
            }

            if (role == SortByManualOrderAndDate)
            {
                return compareByOrder(left.dateTime(), right.dateTime(), currentSortOrder);
            }

            return naturalCompare(left.name(), right.name(),
                                  currentSortOrder, sortCaseSensitivity, strTypeNatural);
        }

        default:
        {
            return 1;
        }
    }
}

bool ItemSortSettings::lessThan(const QVariant& left, const QVariant& right) const
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

#if __GNUC__ >= 7       // krazy:exclude=cpp
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

DatabaseFields::Set ItemSortSettings::watchFlags() const
{
    DatabaseFields::Set set;

    switch (sortRole)
    {
        case SortByFileName:
        {
            set |= DatabaseFields::Name;
            break;
        }

        case SortByFilePath:
        {
            set |= DatabaseFields::Name;
            break;
        }

        case SortByFileSize:
        {
            set |= DatabaseFields::FileSize;
            break;
        }

        case SortByModificationDate:
        {
            set |= DatabaseFields::ModificationDate;
            break;
        }

        case SortByCreationDate:
        {
            set |= DatabaseFields::CreationDate;
            break;
        }

        case SortByRating:
        {
            set |= DatabaseFields::Rating;
            break;
        }

        case SortByImageSize:
        {
            set |= DatabaseFields::Width | DatabaseFields::Height;
            break;
        }

        case SortByAspectRatio:
        {
            set |= DatabaseFields::Width | DatabaseFields::Height;
            break;
        }

        case SortBySimilarity:
        {
            // TODO: Not sure what to do here....

            set |= DatabaseFields::Name;
            break;
        }

        case SortByFaces:
        {
            // Nothing needed for this.

            break;
        }

        case SortByManualOrderAndName:
        case SortByManualOrderAndDate:
        {
            set |= DatabaseFields::ManualOrder;
            break;
        }
    }

    switch (categorizationMode)
    {
        case OneCategory:
        case NoCategories:
        {
            break;
        }

        case CategoryByAlbum:
        {
            set |= DatabaseFields::Album;
            break;
        }

        case CategoryByFormat:
        {
            set |= DatabaseFields::Format;
            break;
        }

        case CategoryByMonth:
        {
            set |= DatabaseFields::CreationDate;
            break;
        }

        case CategoryByFaces:
        {
            // nothing needed here.

            break;
        }
    }

    return set;
}

} // namespace Digikam
