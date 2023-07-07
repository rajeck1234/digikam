/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Internal merge helper.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_META_ENGINE_MERGE_HELPER_H
#define DIGIKAM_META_ENGINE_MERGE_HELPER_H

#include "metaengine_p.h"

namespace Digikam
{

template <class Data, class Key, class KeyString, class KeyStringList = QList<KeyString> >

class Q_DECL_HIDDEN MetaEngineMergeHelper
{
public:

    KeyStringList keys;

public:

    MetaEngineMergeHelper& operator<<(const KeyString& key)
    {
        keys << key;

        return *this;
    }

    /**
     * Merge two (Exif,IPTC,Xmp) Data packages, where the result is stored in dest
     * and fields from src take precedence over existing data from dest.
     */
    void mergeAll(const Data& src, Data& dest)
    {
        for (typename Data::const_iterator it = src.begin() ; it != src.end() ; ++it)
        {
            typename Data::iterator destIt = dest.findKey(Key(it->key()));

            if (destIt == dest.end())
            {
                dest.add(*it);
            }
            else
            {
                *destIt = *it;
            }
        }
    }

    /**
     * Merge two (Exif,IPTC,Xmp) Data packages, the result is stored in dest.
     * Only keys in keys are considered for merging.
     * Fields from src take precedence over existing data from dest.
     */
    void mergeFields(const Data& src, Data& dest)
    {
        Q_FOREACH (const KeyString& keyString, keys)
        {
            Key key(keyString.latin1());
            typename Data::const_iterator it = src.findKey(key);

            if (it == src.end())
            {
                continue;
            }

            typename Data::iterator destIt = dest.findKey(key);

            if (destIt == dest.end())
            {
                dest.add(*it);
            }
            else
            {
                *destIt = *it;
            }
        }
    }

    /**
     * Merge two (Exif,IPTC,Xmp) Data packages, the result is stored in dest.
     * The following steps apply only to keys in "keys":
     * The result is determined by src.
     * Keys must exist in src to kept in dest.
     * Fields from src take precedence over existing data from dest.
     */
    void exclusiveMerge(const Data& src, Data& dest)
    {
        Q_FOREACH (const KeyString& keyString, keys)
        {
            Key key(keyString.latin1());
            typename Data::const_iterator it = src.findKey(key);
            typename Data::iterator destIt   = dest.findKey(key);

            if (destIt == dest.end())
            {
                if (it != src.end())
                {
                    dest.add(*it);
                }
            }
            else
            {
                if (it == src.end())
                {
                    dest.erase(destIt);
                }
                else
                {
                    *destIt = *it;
                }
            }
        }
    }
};

class Q_DECL_HIDDEN ExifMetaEngineMergeHelper
    : public MetaEngineMergeHelper<Exiv2::ExifData, Exiv2::ExifKey, QLatin1String>
{
};

class Q_DECL_HIDDEN IptcMetaEngineMergeHelper
    : public MetaEngineMergeHelper<Exiv2::IptcData, Exiv2::IptcKey, QLatin1String>
{
};

#ifdef _XMP_SUPPORT_

class Q_DECL_HIDDEN XmpMetaEngineMergeHelper
    : public MetaEngineMergeHelper<Exiv2::XmpData, Exiv2::XmpKey, QLatin1String>
{
};

#endif

} // namespace Digikam

#endif // DIGIKAM_META_ENGINE_MERGE_HELPER_H
