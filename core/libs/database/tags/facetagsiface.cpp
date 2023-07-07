/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-27
 * Description : Interface for info stored about a face tag in the database
 *
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "facetagsiface.h"

// Qt includes

#include <QStringList>

// Local includes

#include "digikam_debug.h"
#include "coredbconstants.h"
#include "tagscache.h"
#include "facetags.h"

namespace Digikam
{

FaceTagsIface::FaceTagsIface()
    : m_type(InvalidFace),
      m_imageId(0),
      m_tagId(0)
{
}

FaceTagsIface::FaceTagsIface(Type type, qlonglong imageId, int tagId, const TagRegion& region)
    : m_type(type),
      m_imageId(imageId),
      m_tagId(tagId),
      m_region(region)
{
}

FaceTagsIface::FaceTagsIface(const QString& attribute, qlonglong imageId, int tagId, const TagRegion& region)
    : m_imageId(imageId),
      m_tagId(tagId),
      m_region(region)
{
    m_type = typeForAttribute(attribute, tagId);
}

bool FaceTagsIface::isNull() const
{
    return (m_type == InvalidFace);
}

FaceTagsIface::Type FaceTagsIface::type() const
{
    return m_type;
}

qlonglong FaceTagsIface::imageId() const
{
    return m_imageId;
}

int FaceTagsIface::tagId() const
{
    return m_tagId;
}

TagRegion FaceTagsIface::region() const
{
    return m_region;
}

void FaceTagsIface::setType(Type type)
{
    m_type = type;
}

void FaceTagsIface::setTagId(int tagId)
{
    m_tagId = tagId;
}

void FaceTagsIface::setRegion(const TagRegion& region)
{
    m_region = region;
}

bool FaceTagsIface::operator==(const FaceTagsIface& other) const
{
    return (
            (m_tagId   == other.m_tagId)   &&
            (m_imageId == other.m_imageId) &&
            (m_type    == other.m_type)    &&
            (m_region  == other.m_region)
           );
}

QStringList FaceTagsIface::attributesForFlags(TypeFlags flags)
{
    QStringList attributes;

    for (int i = FaceTagsIface::TypeFirst ; i <= FaceTagsIface::TypeLast ; i <<= 1)
    {
        if (flags & FaceTagsIface::Type(i))
        {
            QString attribute = attributeForType(FaceTagsIface::Type(i));

            if (!attributes.contains(attribute))
            {
                attributes << attribute;
            }
        }
    }

    return attributes;
}

QString FaceTagsIface::attributeForType(Type type)
{
    if ((type == FaceTagsIface::UnknownName) || (type == FaceTagsIface::UnconfirmedName))
    {
        return ImageTagPropertyName::autodetectedFace();
    }

    if (type == FaceTagsIface::ConfirmedName)
    {
        return ImageTagPropertyName::tagRegion();
    }

    if (type == FaceTagsIface::IgnoredName)
    {
        return ImageTagPropertyName::ignoredFace();
    }

    if (type == FaceTagsIface::FaceForTraining)
    {
        return ImageTagPropertyName::faceToTrain();
    }

    return QString();
}

FaceTagsIface::Type FaceTagsIface::typeForId(int tagId)
{
        if (!FaceTags::isPerson(tagId))
        {
            return InvalidFace;
        }

        if      (FaceTags::isTheUnknownPerson(tagId))
        {
            return UnknownName;
        }
        else if (FaceTags::isTheUnconfirmedPerson(tagId))
        {
            return UnconfirmedName;
        }
        else if (FaceTags::isTheIgnoredPerson(tagId))
        {
            return IgnoredName;
        }

        return ConfirmedName;
}

FaceTagsIface::Type FaceTagsIface::typeForAttribute(const QString& attribute, int tagId)
{
    if      (attribute == ImageTagPropertyName::autodetectedFace())
    {
        if (tagId && TagsCache::instance()->hasProperty(tagId, TagPropertyName::unknownPerson()))
        {
            return FaceTagsIface::UnknownName;
        }
        else
        {
            return FaceTagsIface::UnconfirmedName;
        }
    }
    else if (attribute == ImageTagPropertyName::ignoredFace())
    {
        return FaceTagsIface::IgnoredName;
    }
    else if (attribute == ImageTagPropertyName::tagRegion())
    {
        return FaceTagsIface::ConfirmedName;
    }
    else if (attribute == ImageTagPropertyName::faceToTrain())
    {
        return FaceTagsIface::FaceForTraining;
    }

    return FaceTagsIface::InvalidFace;
}

FaceTagsIface FaceTagsIface::fromVariant(const QVariant& var)
{
#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))
    if (var.typeId() == QVariant::List)
#else
    if (var.type() == QVariant::List)
#endif
    {
        QList<QVariant> list(var.toList());

        if ((list.size() == 4) || (list.size() == 5))
        {
            return FaceTagsIface((Type)list.at(0).toInt(),
                                 list.at(1).toLongLong(),
                                 list.at(2).toInt(),
                                 TagRegion::fromVariant(list.at(3)));
        }
    }

    return FaceTagsIface();
}

QVariant FaceTagsIface::toVariant() const
{
    // this is still not perfect, with QList<QVariant> being inefficient
    // we must keep to native types, to make operator== work.

    QList<QVariant> list;
    list << (int)m_type;
    list << m_imageId;
    list << m_tagId;
    list << m_region.toVariant();

    return list;
}

FaceTagsIface FaceTagsIface::fromListing(qlonglong imageId, const QList<QVariant>& extraValues)
{
    if (extraValues.size() < 3)
    {
        return FaceTagsIface();
    }

    // See imagelister.cpp: value - property - tagId

    int tagId         = extraValues.at(2).toInt();
    QString attribute = extraValues.at(1).toString();
    QString value     = extraValues.at(0).toString();
/*
    qCDebug(DIGIKAM_DATABASE_LOG) << tagId << attribute << value;
*/
    return FaceTagsIface(attribute,
                        imageId, tagId,
                        TagRegion(value));
}

QString FaceTagsIface::getAutodetectedPersonString() const
{
    if (isUnconfirmedType())
    {
        return (QString::number(tagId())                 +
                QLatin1Char(',')                         +
                ImageTagPropertyName::autodetectedFace() +
                QLatin1Char(',')                         +
                (TagRegion(region().toRect())).toXml());
    }
    else
    {
        return QString();
    }
}

QDebug operator<<(QDebug dbg, const FaceTagsIface& f)
{
    dbg.nospace() << "FaceTagsIface(" << f.type()
                  << ", image "       << f.imageId()
                  << ", tag "         << f.tagId()
                  << ", region"       << f.region();
    return dbg;
}

} // namespace Digikam
