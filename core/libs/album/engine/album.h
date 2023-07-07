/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : digiKam album types
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ALBUM_H
#define DIGIKAM_ALBUM_H

// Qt includes

#include <QMap>
#include <QUrl>
#include <QList>
#include <QString>
#include <QVector>
#include <QObject>
#include <QMetaType>
#include <QReadWriteLock>

// Local includes

#include "coredbalbuminfo.h"
#include "digikam_export.h"

namespace Digikam
{

/**
 * Album list type definition.
 */
class Album;
typedef QList<Album*> AlbumList;

class CoreDbUrl;

/**
 * \class Album
 * \brief Abstract base class for all album types
 *
 * A class which provides an abstraction for a type Album. This class is meant to
 * be derived and every time a new Album Type is defined add a enum corresponding
 * to that to Album::Type
 *
 * This class provides a means of building a tree representation for
 * Albums @see Album::setParent().
 */
class DIGIKAM_GUI_EXPORT Album
{
public:

    enum Type
    {
        PHYSICAL=0, ///< A physical album type @see PAlbum
        TAG,        ///< A tag      album type @see TAlbum
        DATE,       ///< A date     album type @see DAlbum
        SEARCH,     ///< A search   album type @see SAlbum
        FACE        ///< A faces    album type @see FAlbum
    };

    /**
     * @return the parent album for this album
     */
    Album* parent()                         const;

    /**
     * @return the first child of this album or 0 if no children
     */
    Album* firstChild()                     const;

    /**
     * @return the last child of this album or 0 if no children
     */
    Album* lastChild()                      const;

    /**
     * @return the next sibling of this album of this album or 0
     * if no next sibling
     * @see AlbumIterator
     */
    Album* next()                           const;

    /**
     * @return the previous sibling of this album of this album or 0 if no
     * previous sibling
     * @see AlbumIterator
     */
    Album* prev()                           const;

    /**
     * @return the child of this album at row
     */
    Album* childAtRow(int row)              const;

    /**
     * @return a list of all child Albums
     */
    AlbumList childAlbums(bool recursive = false);

    /**
     * @return a list of all child Albums
     */
    QList<int> childAlbumIds(bool recursive = false);

    /**
     * @return the type of album
     * @see Type
     */
    Type type()                             const;

    /**
     * Each album has a @p ID uniquely identifying it in the set of Albums of
     * a Type
     *
     * \note The @p ID for a root Album is always 0
     *
     * @return the @p ID of the album
     * @see globalID()
     */
    int id()                                const;

    /**
     * An album ID is only unique among the set of all Albums of its Type.
     * This is a global Identifier which will uniquely identifying the Album
     * among all Albums
     *
     * \note If you are adding a new Album Type make sure to update
     * this implementation.
     *
     * You can always get the @p ID of the album using something like
     *
     * \code
     * int albumID = rootAlbum->globalID() - album->globalID();
     * \endcode
     *
     * @return the @p globalID of the album
     * @see id()
     */
    int globalID()                          const;

    /**
     * @return the @p childCount of the album
     */
    int childCount()                        const;

    /**
     * @return the @p rowFromAlbum of the album
     */
    int rowFromAlbum()                      const;

    /**
     * @return the @p title aka name of the album
     */
    QString title()                         const;

    /**
     * @return the kde url of the album
     */
    virtual CoreDbUrl databaseUrl()         const = 0;

    /**
     * @return true is the album is a Root Album
     */
    bool isRoot()                           const;

    /**
     * @return true if the @p album is in the parent hierarchy
     *
     * @param album the album to check whether it belongs in the child
     * hierarchy
     */
    bool isAncestorOf(Album* const album)   const;

    /**
     * @return true if the Album was created by Labels Tree
     *
     */
    bool isUsedByLabelsTree()               const;

    /**
     * @return true if the album was created to be a trash
     *         virtual album
     */
    bool isTrashAlbum()                     const;

    /**
     * @return count of trash content
     */
    int countTrashAlbum()                   const;

    /**
     * This allows to associate some "extra" data to a Album. As one
     * Album can be used by several objects (often views) which all need
     * to add some data, you have to use a key to reference your extra data
     * within the Album.
     *
     * That way a Album can hold and provide access to all those views
     * separately.
     *
     * for eg,
     *
     * \code
     * album->setExtraData( this, searchFolderItem );
     * \endcode
     *
     * and can later access the searchFolderItem by doing
     *
     * \code
     * SearchFolderItem *item = static_cast<SearchFolderItem*>(album->extraData(this));
     * \endcode
     *
     * Note: you have to remove and destroy the data you associated yourself
     * when you don't need it anymore!
     *
     * @param key the key of the extra data
     * @param value the value of the extra data
     * @see extraData
     * @see removeExtraData
     */
    void setExtraData(const void* const key, void* const value);

    /**
     * Remove the associated extra data associated with @p key
     *
     * @param key the key of the extra data
     * @see setExtraData
     * @see extraData
     */
    void removeExtraData(const void* const key);

    /**
     * Retrieve the associated extra data associated with @p key
     *
     * @param key the key of the extra data
     * @see setExtraData
     * @see extraData
     */
    void* extraData(const void* const key)  const;

    /**
     * Sets the property m_usedByLabelsTree to true if the search album
     * was created using the Colors and labels tree view
     *
     * @param isUsed => the status of the usage
     */
    void setUsedByLabelsTree(bool isUsed);

    /**
     * @brief Produces the global id
     * @param type The type of the album
     * @param id the (type-specific) id of the album
     * @return the global id
     */
    static int globalID(Type type, int id);

protected:

    /**
     * Constructor
     */
    Album(Album::Type type, int id, bool root);

    /**
     * Destructor
     *
     * this will also recursively delete all child Albums
     */
    virtual ~Album();

    /**
     * Delete all child albums and also remove any associated extra data
     */
    void clear();

    /**
     * @internal use only
     *
     * Set a new title for the album
     *
     * @param title new title for the album
     */
    void setTitle(const QString& title);

    /**
     * @internal use only
     *
     * Set the parent of the album
     *
     * @param parent set the parent album of album to @p parent
     */
    void setParent(Album* const parent);

    /**
     * @internal use only
     *
     * Insert an Album as a child for this album
     *
     * @param child the Album to add as child
     */
    void insertChild(Album* const child);

    /**
     * @internal use only
     *
     * Remove a Album from the children list for this album
     *
     * @param child the Album to remove
     */
    void removeChild(Album* const child);

private:

     // Disable
    Album()                         = delete;
    Album& operator==(const Album&) = delete;
    Q_DISABLE_COPY(Album)

private:

    bool                     m_root;
    bool                     m_usedByLabelsTree;

    int                      m_id;

    QString                  m_name;
    QString                  m_title;

    QMap<const void*, void*> m_extraMap;

    QVector<Album*>          m_childCache;
    mutable QReadWriteLock   m_cacheLock;

    Type                     m_type;

    Album*                   m_parent;

    friend class AlbumManager;
};

/**
 * \class PAlbum
 *
 * A Physical Album representation
 */
class DIGIKAM_GUI_EXPORT PAlbum : public Album
{
public:

    /// Constructor for root album
    explicit PAlbum(const QString& title);

    /// Constructor for album root albums
    PAlbum(int albumRoot, const QString& label);

    /// Constructor for normal albums
    PAlbum(int albumRoot, const QString& parentPath, const QString& title, int id);

    /// Constructor for Trash album
    PAlbum(const QString& parentPath, int albumRoot);
    ~PAlbum() override;

    void setCaption(const QString& caption);
    void setCategory(const QString& category);
    void setDate(const QDate& date);

    QString     albumRootPath()  const;
    QString     albumRootLabel() const;
    int         albumRootId()    const;
    QString     caption()        const;
    QString     category()       const;
    QDate       date()           const;
    QString     albumPath()      const;
    QString     prettyUrl()      const;
    QString     folderPath()     const;
    CoreDbUrl   databaseUrl()    const override;
    QUrl        fileUrl()        const;
    qlonglong   iconId()         const;
    bool        isAlbumRoot()    const;

private:

    /**
     * A special integer for Trash virtual folders Ids;
     * That gets decremented not incremented
     */
    static int m_uniqueTrashId;

    bool       m_isAlbumRootAlbum;

    int        m_albumRootId;

    QString    m_path;
    QString    m_parentPath;
    QString    m_category;
    QString    m_caption;
    qlonglong  m_iconId;

    QDate      m_date;

    friend class AlbumManager;
};

/**
 * \class TAlbum
 *
 * A Tag Album representation
 */
class DIGIKAM_GUI_EXPORT TAlbum : public Album
{
public:

    TAlbum(const QString& title, int id, bool root=false);
    ~TAlbum() override;

    /**
     * @return The tag path, e.g. "/People/Friend/John" if leadingSlash is true,
     *         "People/Friend/John" if leadingSlash if false.
     *         The root TAlbum returns "/" resp. "".
     */
    QString                tagPath(bool leadingSlash = true) const;
    QString                standardIconName()                const;
    CoreDbUrl              databaseUrl()                     const override;
    QString                prettyUrl()                       const;
    QString                icon()                            const;
    qlonglong              iconId()                          const;
    QList<int>             tagIDs()                          const;

    bool                   isInternalTag()                   const;
    bool                   hasProperty(const QString& key)   const;
    QString                property(const QString& key)      const;
    QMap<QString, QString> properties()                      const;

private:

    int       m_pid;

    QString   m_icon;
    qlonglong m_iconId;

    friend class AlbumManager;
};

/**
 * \class DAlbum
 *
 * A Date Album representation
 */
class DIGIKAM_GUI_EXPORT DAlbum : public Album
{
public:

    enum Range
    {
        Month = 0,
        Year
    };

public:

    explicit DAlbum(const QDate& date, bool root=false, Range range=Month);
    ~DAlbum() override;

    QDate     date()        const;
    Range     range()       const;
    CoreDbUrl databaseUrl() const override;

private:

    static int m_uniqueID;
    QDate      m_date;
    Range      m_range;

    friend class AlbumManager;
};

/**
 * \class SAlbum
 *
 * A Search Album representation
 */
class DIGIKAM_GUI_EXPORT SAlbum : public Album
{
public:

    SAlbum(const QString& title, int id, bool root=false);
    ~SAlbum() override;

    CoreDbUrl            databaseUrl()        const override;
    QString              query()              const;
    DatabaseSearch::Type searchType()         const;
    bool                 isNormalSearch()     const;
    bool                 isAdvancedSearch()   const;
    bool                 isKeywordSearch()    const;
    bool                 isTimelineSearch()   const;
    bool                 isHaarSearch()       const;
    bool                 isMapSearch()        const;
    bool                 isDuplicatesSearch() const;

    /**
     * Indicates whether this album is a temporary search or not.
     *
     * @return true if this is a temporary search album, else false
     */
    bool                 isTemporarySearch()  const;

    QString              displayTitle()       const;

    /**
     * Returns the title of search albums that is used to mark them as a
     * temporary search that isn't saved officially yet and is only used for
     * viewing purposes.
     *
     * @param type the type of the search to get the temporary title for
     * @param haarType there are several haar searches, so that this search type
     *                 needs a special handling
     * @return string that identifies this album uniquely as an unsaved search
     */
    static QString getTemporaryTitle(DatabaseSearch::Type type,
                                     DatabaseSearch::HaarSearchType haarType = DatabaseSearch::HaarImageSearch);

    /**
     * Returns the title for a temporary haar search depending on the sub-type
     * used for this search
     *
     * @param haarType type of the haar search to get the name for
     * @return string that identifies this album uniquely as an unsaved search
     */
    static QString getTemporaryHaarTitle(DatabaseSearch::HaarSearchType haarType);

private:

    void setSearch(DatabaseSearch::Type type, const QString& query);

private:

    QString              m_query;
    DatabaseSearch::Type m_searchType;

    friend class AlbumManager;
};

/**
 *  \class AlbumIterator
 *
 *  Iterate over all children of this Album.
 *  \note It will not include the specified album
 *
 *  Example usage:
 *  \code
 *  AlbumIterator it(album);
 *  while ( it.current() )
 *  {
 *     qCDebug(DIGIKAM_GENERAL_LOG) << "Album: " << it.current()->title();
 *     ++it;
 *  }
 * \endcode
 *
 *  \warning Do not delete albums using this iterator.
 */
class DIGIKAM_GUI_EXPORT AlbumIterator
{
public:

    explicit AlbumIterator(Album* const album);
    ~AlbumIterator();

    AlbumIterator& operator++();
    Album*         operator*();
    Album*         current() const;

private:

    // Disable
    AlbumIterator() = delete;
    Q_DISABLE_COPY(AlbumIterator)

private:

    Album* m_current;
    Album* m_root;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::Album*)
Q_DECLARE_METATYPE(QList<Digikam::TAlbum*>)

#endif // DIGIKAM_ALBUM_H
