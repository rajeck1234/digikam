/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Filter values for use with ItemFilterModel
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2010 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText:      2011 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText:      2014 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ITEM_FILTER_SETTINGS_H
#define DIGIKAM_ITEM_FILTER_SETTINGS_H

// Qt includes

#include <QHash>
#include <QList>
#include <QMap>
#include <QString>
#include <QSet>
#include <QUrl>
#include <QDateTime>

// Local includes

#include "searchtextbar.h"
#include "mimefilter.h"
#include "digikam_export.h"

namespace Digikam
{

class ItemInfo;
class VersionManagerSettings;

namespace DatabaseFields
{
    class Set;
}

// ---------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT SearchTextFilterSettings : public SearchTextSettings
{

public:

    enum TextFilterFields
    {
        None             = 0x00,
        ImageName        = 0x01,
        ImageTitle       = 0x02,
        ImageComment     = 0x04,
        TagName          = 0x08,
        AlbumName        = 0x10,
        ImageAspectRatio = 0x20,
        ImagePixelSize   = 0x40,
        All              = ImageName | ImageTitle | ImageComment | TagName | AlbumName | ImageAspectRatio | ImagePixelSize
    };

public:

    SearchTextFilterSettings()
    {
        textFields = None;
    }

    explicit SearchTextFilterSettings(const SearchTextSettings& settings)
    {
        caseSensitive = settings.caseSensitive;
        text          = settings.text;
        textFields    = None;
    }

    TextFilterFields textFields;
};

// ---------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemFilterSettings
{
public:

    /// Possible logical matching condition used to sort tags id.
    enum MatchingCondition
    {
        OrCondition,
        AndCondition
    };

    /// Possible conditions used to filter rating: >=, =, <=
    enum RatingCondition
    {
        GreaterEqualCondition,
        EqualCondition,
        LessEqualCondition
    };

    /// Possible logical matching condition used to sort geolocation.
    enum GeolocationCondition
    {
        GeolocationNoFilter       = 0,
        GeolocationNoCoordinates  = 1 << 1,
        GeolocationHasCoordinates = 1 << 2
    };

public:

    explicit ItemFilterSettings();

    /**
     *  Returns true if the given ItemInfo matches the filter criteria.
     *  Optionally, foundText is set to true if it matched by text search.
     */
    bool matches(const ItemInfo& info, bool* const foundText = nullptr) const;

public:

    /// --- Tags filter ---

    void setTagFilter(const QList<int>& includedTags,
                      const QList<int>& excludedTags,
                      MatchingCondition matchingCond,
                      bool              showUnTagged,
                      const QList<int>& clTagIds,
                      const QList<int>& plTagIds);

public:

    /// --- Rating filter ---
    void setRatingFilter(int rating, RatingCondition ratingCond, bool isUnratedExcluded);

public:

    /// --- Date filter ---
    void setDayFilter(const QList<QDateTime>& days);

public:

    /// --- Text filter ---
    void setTextFilter(const SearchTextFilterSettings& settings);
    void setTagNames(const QHash<int, QString>& tagNameHash);
    void setAlbumNames(const QHash<int, QString>& albumNameHash);

public:

    /// --- Mime filter ---
    void setMimeTypeFilter(int mimeTypeFilter);

public:

    /// --- Geolocation filter
    void setGeolocationFilter(const GeolocationCondition& condition);

public:

    /// Returns if the day is a filter criteria
    bool isFilteringByDay()                                 const;

    /// Returns if the type mime is a filter criteria
    bool isFilteringByTypeMime()                            const;

    /// Returns whether geolocation is a filter criteria
    bool isFilteringByGeolocation()                         const;

    /// Returns if the rating is a filter criteria
    bool isFilteringByRating()                              const;

    /// Returns if the pick labels is a filter criteria
    bool isFilteringByPickLabels()                          const;

    /// Returns if the color labels is a filter criteria
    bool isFilteringByColorLabels()                         const;

    /// Returns if the tag is a filter criteria
    bool isFilteringByTags()                                const;

    /// Returns if the text (including comment) is a filter criteria
    bool isFilteringByText()                                const;

    /// Returns if images will be filtered by these criteria at all
    bool isFiltering()                                      const;

public:

    /// --- URL whitelist filter
    void setUrlWhitelist(const QList<QUrl>& urlList, const QString& id);

public:

    /// --- ID whitelist filter
    void setIdWhitelist(const QList<qlonglong>& idList, const QString& id);

public:

    /// --- Change notification ---

    /** Returns database fields a change in which would affect the current filtering.
     *  To find out if an image tag change affects filtering, test isFilteringByTags().
     *  The text filter will also be affected by changes in tags and album names.
     */
    DatabaseFields::Set watchFlags()                        const;

private:

    /**
     * @brief Returns whether some internal filtering (whitelist by id or URL) or normal filtering is going on
     */
    bool isFilteringInternally()                            const;

private:

    /// --- Tags filter ---
    bool                              m_untaggedFilter;
    QList<int>                        m_includeTagFilter;
    QList<int>                        m_excludeTagFilter;
    MatchingCondition                 m_matchingCond;
    QList<int>                        m_colorLabelTagFilter;
    QList<int>                        m_pickLabelTagFilter;

    /// --- Rating filter ---
    int                               m_ratingFilter;
    RatingCondition                   m_ratingCond;
    bool                              m_isUnratedExcluded;

    /// --- Date filter ---
    QHash<QDateTime, bool>            m_dayFilter;

    /// --- Text filter ---
    SearchTextFilterSettings          m_textFilterSettings;

    /// Helpers for text search: Set these if you want to search album or tag names with text search
    QHash<int, QString>               m_tagNameHash;
    QHash<int, QString>               m_albumNameHash;

    /// --- Mime filter ---
    MimeFilter::TypeMimeFilter        m_mimeTypeFilter;

    /// --- Geolocation filter
    GeolocationCondition              m_geolocationCondition;

    /// --- URL whitelist filter
    QHash<QString,QList<QUrl>>        m_urlWhitelists;

    /// --- ID whitelist filter
    QHash<QString, QList<qlonglong> > m_idWhitelists;
};

// ---------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT VersionItemFilterSettings
{
public:

    VersionItemFilterSettings();
    explicit VersionItemFilterSettings(const VersionManagerSettings& settings);

    bool operator==(const VersionItemFilterSettings& other) const;

    /**
     *  Returns true if the given ItemInfo matches the filter criteria.
     */
    bool matches(const ItemInfo& info)                      const;

    bool isHiddenBySettings(const ItemInfo& info)           const;
    bool isExemptedBySettings(const ItemInfo& info)         const;

    /// --- Tags filter ---

    void setVersionManagerSettings(const VersionManagerSettings& settings);

    /**
     * Add list with exceptions: These images will be exempted from filtering by this filter
     */
    void setExceptionList(const QList<qlonglong>& idlist, const QString& id);

    /// Returns if images will be filtered by these criteria at all
    bool isFiltering()                                      const;

    /// Returns if the tag is a filter criteria
    bool isFilteringByTags()                                const;

    /// DatabaseFields::Set watchFlags() const: Would return 0

protected:

    QList<int>                        m_excludeTagFilter;
    int                               m_includeTagFilter;
    int                               m_exceptionTagFilter;
    QHash<QString, QList<qlonglong> > m_exceptionLists;
};

// ---------------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT GroupItemFilterSettings
{
public:

    explicit GroupItemFilterSettings();

    bool operator==(const GroupItemFilterSettings& other)   const;

    /**
     *  Returns true if the given ItemInfo matches the filter criteria.
     */
    bool matches(const ItemInfo& info)                      const;

    /**
     * Open or close a group.
     */
    void setOpen(qlonglong group, bool open);
    bool isOpen(qlonglong group)                            const;

    /**
     * Open all groups
     */
    void setAllOpen(bool open);
    bool isAllOpen()                                        const;

    /// Returns if images will be filtered by these criteria at all
    bool isFiltering()                                      const;

    DatabaseFields::Set watchFlags()                        const;

protected:

    bool            m_allOpen;
    QSet<qlonglong> m_openGroups;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ItemFilterSettings::GeolocationCondition)

#endif // DIGIKAM_ITEM_FILTER_SETTINGS_H
