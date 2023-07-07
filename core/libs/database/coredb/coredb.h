/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-18
 * Description : Core database interface.
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_CORE_DB_H
#define DIGIKAM_CORE_DB_H

// Qt includes

#include <QStringList>
#include <QDateTime>
#include <QVariant>
#include <QString>
#include <QList>
#include <QPair>
#include <QUuid>
#include <QMap>

// Local includes

#include "digikam_export.h"
#include "collectionlocation.h"
#include "coredbalbuminfo.h"
#include "coredbfields.h"
#include "coredbaccess.h"
#include "coredbconstants.h"
#include "dbenginesqlquery.h"
#include "album.h"

namespace Digikam
{

class CoreDbBackend;

class DIGIKAM_DATABASE_EXPORT CoreDB
{
public:

    /**
     * This adds a keyword-value combination to the database Settings table
     * if the keyword already exists, the value will be replaced with the new
     * value.
     * @param keyword The keyword
     * @param value The value
     */
    void setSetting(const QString& keyword, const QString& value);

    /**
     * This function returns the value which is stored in the database
     * (table Settings).
     * @param keyword The keyword for which the value has to be returned.
     * @return The values which belongs to the keyword, or a null string if
     *         no value is set.
     */
    QString getSetting(const QString& keyword)                                                                      const;

    /**
     * Get the settings for the file name filters of this database.
     * Returns a list with lowercase suffixes only, no wildcards added ("png", not "*.png")
     * Returned is a joint result of main and user settings.
     * If you are not interested in a specific value, pass 0.
     */
    void getFilterSettings(QStringList* imageFilter, QStringList* videoFilter, QStringList* audioFilter);

    /**
     * Returns the user-configurable filter settings.
     * If you are not interested in a specific value, pass 0.
     */
    void getUserFilterSettings(QString* imageFilterString, QString* videoFilterString, QString* audioFilterString);

    /**
     * Sets the main filter settings of the database. Should only be called at schema update.
     */
    void setFilterSettings(const QStringList& imageFilter, const QStringList& videoFilter, const QStringList& audioFilter);

    /**
     * Sets the user-configurable filter settings. The lists shall be as specified for getFilterSettings.
     * They may include entries starting with "-", which indicates that this format shall be removed from
     * the list, if it is included in the main settings list.
     */
    void setUserFilterSettings(const QStringList& imageFilter, const QStringList& videoFilter, const QStringList& audioFilter);

    void setIgnoreDirectoryFilterSettings(const QStringList& ignoreDirectoryFilter);

    void setUserIgnoreDirectoryFilterSettings(const QStringList& ignoreDirectoryFilters);

    void getUserIgnoreDirectoryFilterSettings(QString* ignoreDirectoryFilterString);

    void getIgnoreDirectoryFilterSettings(QStringList* ignoreDirectoryFilter);

    /**
     * Returns a UUID for the database file.
     * This UUID is kept stable over schema updates.
     */
    QUuid databaseUuid();

    /**
     * Returns database encoding.
     * For SQLite should UTF-8.
     * For MySQL like UTF8MB4.
     */
    QString getDatabaseEncoding()                                                                                   const;

    /**
     * Returns the version used for the unique hash in this database.
     * The value is cached.
     */
    int getUniqueHashVersion()                                                                                      const;

    void setUniqueHashVersion(int version);

    bool isUniqueHashV2()                                                                                           const;

    // ----------- AlbumRoot operations -----------

    /**
     * Returns all albums and their attributes in the database
     * @return a list of albums and their attributes
     */
    QList<AlbumRootInfo> getAlbumRoots()                                                                            const;

    /**
     * Add a new album to the database with the given attributes
     * @param type The type of the album root
     * @param specificPath The path specific to volume
     * @param label        An (optional) user-visible label
     * @returns the album root id of the newly created root
     */
    int addAlbumRoot(CollectionLocation::Type type, const QString& identifier,
                     const QString& specificPath, const QString& label)                                             const;

    /**
     * Deletes an album  root from the database.
     * @param rootId the id of the album root
     */
    void deleteAlbumRoot(int rootId);

    /**
     * Migrates a given album root to a new disk location. This only changes the values
     * in the AlbumRoots table. It is expected that this merely reflects underlying partition changes,
     * still pointing to the same data.
     */
    void migrateAlbumRoot(int rootId, const QString& identifier);

    /**
     * Changes the label of the specified album root
     * @param rootId the id of the album root
     * @param newLabel new label for the album root
     */
    void setAlbumRootLabel(int rootId, const QString& newLabel);

    /**
     * Changes the specificPath of the specified album root
     * @param rootId the id of the album root
     * @param newPath new path for the album root
     */
    void setAlbumRootPath(int rootId, const QString& newPath);

    /**
     * Sets the type of the specified album root to a new value.
     */
    void setAlbumRootType(int rootId, CollectionLocation::Type newType);

    /**
     * Sets the case sensitivity of the specified album root to a new value.
     */
    void setAlbumRootCaseSensitivity(int rootId, CollectionLocation::CaseSensitivity caseSensitivity);

    // ----------- Album Listing operations -----------
    /**
     * Returns all albums and their attributes in the database
     * @return a list of albums and their attributes
     */
    AlbumInfo::List scanAlbums()                                                                                    const;

    /**
     * Returns all tags and their attributes in the database
     * @return a list of tags and their attributes
     */
    TagInfo::List scanTags()                                                                                        const;
    TagInfo getTagInfo(int tagId)                                                                                   const;

    /**
     * Returns all searches from the database
     * @return a list of searches from the database
     */
    SearchInfo::List scanSearches()                                                                                 const;

    /**
     * Returns all albums in the database with their albumRoot and ID,
     * ordered by id.
     */
    QList<AlbumShortInfo> getAlbumShortInfos()                                                                      const;

    /**
     * Returns all tags in the database with their parent id and name,
     * ordered by id.
     */
    QList<TagShortInfo> getTagShortInfos()                                                                          const;

    // ----------- Operations on PAlbums -----------

    /**
     * Add a new album to the database with the given attributes
     * @param albumRootId   id of the album root of the new album
     * @param relativePath  url of the album
     * @param caption       the album caption
     * @param date          the date for the album
     * @param collection    the album collection
     * @return the id of the album added or -1 if it failed
     */
    int addAlbum(int albumRootId, const QString& relativePath,
                 const QString& caption,
                 const QDate& date, const QString& collection)                                                      const;

    /**
     * Find out the album for a given folder.
     * @param albumRootId  id of the album root of the album
     * @param relativePath The relative path for which you want the albumID relative to the album root
     * @param create       If true, an album is newly created if it does not yet exist.
     *                     If false, -1 is returned if no album exists.
     * @return The albumID for that folder,
     *         or -1 if it does not exist and create is false.
     */
    int  getAlbumForPath(int albumRootId, const QString& relativePath, bool create = true)                          const;

    /**
     * Find out the album ids for a given relative path, including the subalbums.
     * @param albumRootId  id of the album root of the album
     * @param relativePath The path for which you want the albumIDs relative to the album root
     * @return a list of album ids. The list is empty if no albums are found.
     */
    QList<int> getAlbumAndSubalbumsForPath(int albumRootId, const QString& relativePath)                            const;

    /**
     * Find out all album ids of a given album root
     * @return a list of album ids.
     */
    QList<int> getAlbumsOnAlbumRoot(int albumRootId)                                                                const;

    /**
     * Deletes an album from the database. This will not delete the
     * subalbums of the album.
     * @param albumID the id of the album
     */
    void deleteAlbum(int albumID);

    /** Makes the album a stale entry by setting the albumRoot to 0.
     *  Emits the same changeset as deleteAlbum()
     */
    void makeStaleAlbum(int albumID);

    /**
     * Deletes albums from the database that were previously removed
     * with makeStaleAlbum()
     */
    void deleteStaleAlbums();

    /**
     * Copy the properties of the given srcAlbum to the dstAlbum.
     * Both albums must exist.
     * @return true if the operations succeeds
     */
    bool copyAlbumProperties(int srcAlbumID, int dstAlbumID)                                                        const;

    /**
     * Give an existing album a new relativePath and a newAlbumRootId
     */
    void renameAlbum(int albumID, int newAlbumRootId, const QString& newRelativePath);

    /**
     * Set a caption for the album.
     * @param albumID the id of the album
     * @param caption the new caption for the album
     */
    void setAlbumCaption(int albumID, const QString& caption);

    /**
     * Set a category for the album.
     * @param albumID  the id of the album
     * @param category the new category for the album
     */
    void setAlbumCategory(int albumID, const QString& category);

    /**
     * Set a date for the album.
     * @param albumID  the id of the album
     * @param date     the date for the album
     */
    void setAlbumDate(int albumID, const QDate& date);

    /**
     * Set the modification date time for the album.
     * @param albumID  the id of the album
     * @param dateTime the modification date time for the album
     */
    void setAlbumModificationDate(int albumID, const QDateTime& modificationDate);

    /**
     * Set the icon for the album.
     * @param albumID the id of the album
     * @param iconID  the id of the icon file
     */
    void setAlbumIcon(int albumID, qlonglong iconID);

    /**
     * Given an albumid, this returns the album root id for that album
     * @param albumID the id of the albumdb
     * @return the id of the album root of this album
     */
    int getAlbumRootId(int albumID)                                                                                 const;

    /**
     * Given an albumid, this returns the relative path for that album
     * (the path below the album root, starting with a slash)
     * @param albumID the id of the album
     * @return the url of the album
     */
    QString getAlbumRelativePath(int albumID)                                                                       const;

    /**
     * Returns the lowest/oldest date of all images for that album.
     * @param albumID the id of the album to calculate
     * @return the date.
     */
    QDate getAlbumLowestDate(int albumID)                                                                           const;

    /**
     * Returns the highest/newest date of all images for that album.
     * @param albumID the id of the album to calculate
     * @return the date.
     */
    QDate getAlbumHighestDate(int albumID)                                                                          const;

    /**
     * Returns the average date of all images for that album.
     * @param albumID the id of the album to calculate
     * @return the date.
     */
    QDate getAlbumAverageDate(int albumID)                                                                          const;

    /**
     * Returns the QPair<int, int> of all items (first)
     * and albums (second) as a counter in the album.
     * @param albumID the id of the album
     */
    QPair<int, int> getNumberOfAllItemsAndAlbums(int albumID)                                                       const;

    /**
     * Returns the number of items in the album.
     * @param albumID the id of the album
     */
    int getNumberOfItemsInAlbum(int albumID)                                                                        const;

    /**
     * Returns the QDateTime of the album modification date.
     * @param albumID the id of the album
     */
    QDateTime getAlbumModificationDate(int albumID)                                                                 const;

    /**
     * Returns a QMap with relative path and the album modification date.
     * @param albumRootID id of the album root of the album
     */
    QMap<QString, QDateTime> getAlbumModificationMap(int albumRootId)                                               const;

    /**
     * Returns a QHash<int, int> of album id -> count of items
     * in the album
     */
    QHash<int, int> getNumberOfImagesInAlbums()                                                                      const;

    // ----------- Operations on TAlbums -----------

    /**
     * Adds a new tag to the database with given name, icon and parent id.
     * @param parentTagID the id of the tag which will become the new tags parent
     * @param name        the name of the tag
     * @param iconKDE     the name of the icon file (this is filename which kde
     * iconloader can load up)
     * @param iconID      the id of the icon file
     * Note: if the iconKDE parameter is empty, then the iconID parameter is used
     * @return the id of the tag added or -1 if it failed
     */
    int addTag(int parentTagID, const QString& name,
               const QString& iconKDE, qlonglong iconID)                                                            const;

    /**
     * Get a list of recently assigned tags (only last 6 tags are listed)
     * @return the list of recently assigned tags
     */
    // TODO move to other place (AlbumManager)
    QList<int> getRecentlyAssignedTags()                                                                            const;
      
    /**
     * Deletes a tag from the database. This will not delete the
     * subtags of the tag.
     * @param tagID the id of the tag
     */
    void deleteTag(int tagID);

    /**
     * Set a new name for the tag.
     * @param tagID the id of the tag
     * @param name  the new name for the tag
     */
    void setTagName(int tagID, const QString& name);

    /**
     * Set the icon for the tag.
     * @param tagID the id of the tag
     * @param iconKDE the filename for the kde icon file
     * @param iconID the id of the icon file
     * Note: Only one of the iconKDE or iconID parameters is used.
     * if the iconKDE parameter is empty, then the iconID parameter is used
     */
    void setTagIcon(int tagID, const QString& iconKDE, qlonglong iconID);

    /**
     * Set the parent tagid for the tag. This is equivalent to reparenting
     * the tag
     * @param tagID          the id of the tag
     * @param newParentTagID the new parentid for the tag
     */
    void setTagParentID(int tagID, int newParentTagID);

    /**
     * Returns the list of all tag properties (ordered by tag id, then property).
     */
    QList<TagProperty> getTagProperties()                                                                           const;

    /**
     * Returns the list of tag properties of the given tag.
     */
    QList<TagProperty> getTagProperties(int tagID)                                                                  const;

    /**
     * Returns the list of tag properties with the given attribute.
     */
    QList<TagProperty> getTagProperties(const QString& property)                                                    const;

    /**
     * Adds a tag property. Note that this never replaces existing entries.
     * It is also all right to add multiple entries for a tag with the same property.
     * To replace an existing entry, remove the entry before.
     */
    void addTagProperty(int tagId, const QString& property, const QString& value);
    void addTagProperty(const TagProperty& property);

    /**
     * Removes properties for the given tag. If the value is given, removes only
     * the entries with the given property/value pair. If only property is given,
     * removes all properties with the given name. If property is null,
     * removes all properties for the given tag.
     */
    void removeTagProperties(int tagId, const QString& property = QString(), const QString& value = QString());

    /**
     * Returns a list of tag ids with the specified property.
     * FIXME: Not tested, might not work at all.
     */
    QList<int> getTagsWithProperty(const QString& property)                                                         const;

    // ----------- Operations on SAlbums -----------

    /**
     * Add a new search to the database with the given attributes
     * @param type       search type
     * @param name       name of the search
     * @param query      search query to use
     * @return the id of the album added or -1 if it failed
     */
    int addSearch(DatabaseSearch::Type type, const QString& name, const QString& query)                             const;

    /**
     * Updates Search with new attributes
     * @param searchID   the id of the search
     * @param type       type of the search
     * @param query      database query of the search
     */
    void updateSearch(int searchID, DatabaseSearch::Type type,
                      const QString& name, const QString& query);

    /**
     * Delete a search from the database.
     * @param searchID the id of the search
     */
    void deleteSearch(int searchID);

    /**
     * Delete all search with the given type
     */
    void deleteSearches(DatabaseSearch::Type type);

    /**
     * Get information about the specified search
     */
    SearchInfo getSearchInfo(int searchId)                                                                          const;

    /**
     * Get the query for the search specified by its id
     */
    QString getSearchQuery(int searchId)                                                                            const;

    // ----------- Adding and deleting Items -----------
    /**
     * Put a new item in the database or replace an existing one.
     * @return the id of item added or -1 if it fails
     */
    qlonglong addItem(int albumID, const QString& name,
                      DatabaseItem::Status status,
                      DatabaseItem::Category category,
                      const QDateTime& modificationDate,
                      qlonglong fileSize,
                      const QString& uniqueHash)                                                                    const;

    /**
     * Deletes an item from the database.
     * @param albumID The id of the album.
     * @param file The filename of the file to delete.
     */
    void deleteItem(int albumID, const QString& file);

    /**
     * Deletes an item from the database if it does not belong to an album.
     * This method can only be used if the album of the image is NULL!
     * @param imageId The id of the image.
     */
    void deleteItem(qlonglong imageId);

    /**
     * Deletes an item from the database without checking the album.
     * @param imageId The id of the image.
     */
    void deleteObsoleteItem(qlonglong imageId);

    /**
     * Marks all items in the specified album as removed,
     * resets their dirids.
     * The album can be deleted afterwards without removing
     * the entries for the items, which
     * can later be removed by deleteRemovedItems().
     * @param albumID The id of the album
     * @param ids Fully optional: The image ids in the album, if you know them anyway.
     *            This parameter is only used for distributing the change notification.
     */
    void removeItemsFromAlbum(int albumID, const QList<qlonglong>& ids_forInformation = QList<qlonglong>());

    /**
     * Marks all items in the list as removed,
     * resets their dirids.
     * The items can later be removed by deleteRemovedItems().
     * @param itemIDs a list of item IDs to be marked
     * @param albumIDs this parameter is purely informational.
     *                 it shall contain the albums that the items are removed from.
     */
    void removeItems(QList<qlonglong> itemIDs, const QList<int>& albumIDs = QList<int>());

    /**
     * Marks all items in the list as obsolete,
     * resets their dirids.
     * The items can later be removed by deleteRemovedItems().
     * @param itemIDs a list of item IDs to be marked
     * @param albumIDs this parameter is purely informational.
     *                 it shall contain the albums that the items are removed from.
     */
    void removeItemsPermanently(QList<qlonglong> itemIDs, const QList<int>& albumIDs = QList<int>());

    /**
     * Delete all items from the database that are marked as removed.
     * Use with care!
     */
    void deleteRemovedItems();

    // ----------- Finding items -----------

    /**
     * Get the imageId of the item
     * @param albumID the albumID of the item
     * @param name the name of the item
     * @return the ImageId for the item, or -1 if it does not exist
     */
    qlonglong getImageId(int albumID, const QString& name)                                                          const;

    /**
     * Get the imageId fitting to the information given for the item
     * @param albumID the albumID of the item (-1 means NULL)
     * @param name the name of the item
     * @param status the status of the item
     * @return the ImageIds for the item, or an empty list if there are no matching entries.
     */
    QList<qlonglong> getImageIds(int albumID, const QString& name,
                                 DatabaseItem::Status status)                                                       const;

    /**
     * Returns all image ids with the given status.
     * @param status The status.
     * @return The ids of the images that have the given status.
     */
    QList<qlonglong> getImageIds(DatabaseItem::Status status)                                                       const;

    /**
     * Returns all image ids with the given status and category.
     * @param status The status.
     * @param category The category.
     * @return The ids of the images that have the given status.
     */
    QList<qlonglong> getImageIds(DatabaseItem::Status status,
                                 DatabaseItem::Category category)                                                   const;

    /**
     * Find the imageId fitting to the information given for the item
     * @param albumID the albumID of the item (-1 means NULL)
     * @param name the name of the item
     * @param status the status of the item
     * @param category the category of the item
     * @param fileSize the file size
     * @param uniqueHash the unique hash
     * @return the ImageId for the item, or -1 if no matching or more than one infos were found.
     */
    qlonglong findImageId(int albumID, const QString& name,
                          DatabaseItem::Status status,
                          DatabaseItem::Category category,
                          qlonglong fileSize,
                          const QString& uniqueHash)                                                                const;

    enum ItemSortOrder
    {
        NoItemSorting,
        ByItemName,
        ByItemPath,
        ByItemDate,
        ByItemRating
    };

    /**
     * Returns all items for a given albumid. This is used to
     * verify if all items on disk are consistent with the database
     * in the CollectionScanner class.
     * @param albumID The albumID for which you want all items.
     * @param recursive perform a recursive folder hierarchy parsing
     * @return It returns a QStringList with the filenames.
     */
    QStringList getItemNamesInAlbum(int albumID, bool recursive = false)                                            const;

    /**
     * Returns all ids of items in images table.
     */
    QList<qlonglong> getAllItems()                                                                                  const;

    /**
     * Returns all ids of items with album ids in images table.
     * QPair.first  == albumRootID
     * QPair.second == albumID
     */
    QHash<qlonglong, QPair<int, int> > getAllItemsWithAlbum()                                                       const;

    /**
     * Returns the id of the item with the given filename in
     * the album with the given id.
     * @param albumId The albumId in which we search the item.
     * @param fileName The name of the item file.
     * @return The item id or -1 if not existent.
     */
    qlonglong getItemFromAlbum(int albumID, const QString& fileName)                                                const;

    /**
     * Returns an ItemScanInfo object for each item in the album
     * with the specified album id.
     */
    QList<ItemScanInfo> getItemScanInfos(int albumID)                                                               const;

    /**
     * Given a albumID, get a list of the url of all items in the album
     * NOTE: Uses the CollectionManager
     * @param  albumID the id of the album
     * @param  order   order for the returned items to use
     * @return a list of urls for the items in the album. The urls are the
     * absolute path of the items
     */
    QStringList getItemURLsInAlbum(int albumID, ItemSortOrder order = NoItemSorting)                                const;

    /**
     * Given a albumID, get a list of Ids of all items in the album
     * @param  albumID the id of the album
     * @return a list of Ids for the items in the album.
     */
    QList<qlonglong> getItemIDsInAlbum(int albumID)                                                                 const;

    /**
     * Given a albumID, get a map of Ids and urls of all items in the album
     * NOTE: Uses the CollectionManager
     * @param  albumID the id of the album
     * @return a map of Ids and urls for the items in the album. The urls are the
     * absolute path of the items
     */
    QMap<qlonglong, QString> getItemIDsAndURLsInAlbum(int albumID)                                                  const;

    /**
     * Given a tagid, get a list of the url of all items in the tag
     * NOTE: Uses the CollectionManager
     * @param  tagID the id of the tag
     * @param  recursive perform a recursive folder hierarchy parsing
     * @return a list of urls for the items in the tag. The urls are the
     * absolute path of the items
     */
    QStringList getItemURLsInTag(int tagID, bool recursive = false)                                                 const;

    /**
     * Given a tagID, get a list of Ids of all items in the tag
     * @param  tagID the id of the tag
     * @param  recursive perform a recursive folder hierarchy parsing
     * @return a list of Ids for the items in the tag.
     */
    QList<qlonglong> getItemIDsInTag(int tagID, bool recursive = false)                                             const;

    /**
     * Returns a QVariantList of creationDate of all items
     */
    QVariantList getAllCreationDates()                                                                              const;

    /**
     * Get obsolete item Ids.
     */
    QList<qlonglong> getObsoleteItemIds()                                                                           const;

    // ----------- Item properties -----------

    /**
     * Find the album of an item
     * @param imageID The ID of the item
     * @return The ID of the PAlbum of the item, or -1 if not found
     */
    int getItemAlbum(qlonglong imageID)                                                                             const;

    /**
     * Retrieve the name of the item
     * @param imageID The ID of the item
     * @return The name of the item, or a null string if not found
     */
    QString getItemName(qlonglong imageID)                                                                          const;

    /**
     * Get item and album info from the image ID
     */
    ItemShortInfo getItemShortInfo(qlonglong imageID)                                                               const;

    /**
     * Get item and album if from albumRootId, album path and file name.
     */
    ItemShortInfo getItemShortInfo(int albumRootId, const QString& relativePath,
                                   const QString& name)                                                             const;

    /**
     * Get scan info from the image ID
     */
    ItemScanInfo getItemScanInfo(qlonglong imageID)                                                                 const;

    /**
     * Update the fields of the Images table that have changed when
     * the file has been modified on disk.
     * @param imageID the image that has been modified
     */
    void updateItem(qlonglong imageID,
                    DatabaseItem::Category category,
                    const QDateTime& modificationDate,
                    qlonglong fileSize,
                    const QString& uniqueHash);

    /**
     * Updates the status field for the item.
     * Note: Do not use this to set to the Removed status, see removeItems().
     */
    void setItemStatus(qlonglong imageID, DatabaseItem::Status status);

    /**
     * Updates the album field for the item.
     * Note: Do not use this to move the item. This function only has the purpose to
     * reuse image infos for restored images from trash.
     */
    void setItemAlbum(qlonglong imageID, qlonglong albumId);

    /**
     * Updates the manualOrder field for the item.
     */
    void setItemManualOrder(qlonglong imageID, qlonglong value);

    /**
     * Updates the modification date field for the item.
     */
    void setItemModificationDate(qlonglong imageID, const QDateTime& modificationDate);

    /**
     * Rename the item.
     * Note: we not use here ImageChangeset.
     */
    void renameItem(qlonglong imageID, const QString& newName);

    /**
     * Returns the requested fields from the Images table.
     * Choose the fields with the mask.
     * The fields will be returned in the following order and type:
     * 0) Int       Album
     * 1) String    Name
     * 2) Int       Status
     * 3) Int       Category
     * 4) DateTime  ModificationDate
     * 5) int       FileSize
     * 6) String    uniqueHash
     */
    QVariantList getImagesFields(qlonglong imageID,
                                 DatabaseFields::Images imagesFields)                                               const;

    /**
     * Add (or replace) the ItemInformation of the specified item.
     * If there is already an entry, it will be discarded.
     * The QVariantList shall have 9 entries, of types in this order:
     * 0) Int       rating
     * 1) DateTime* creationDate
     * 2) DateTime* digitizationDate
     * 3) Int       orientation
     * 4) Int       width
     * 5) Int       height
     * 6) String    format
     * 7) Int       colorDepth
     * 8) Int       colorModel
     * ( (*) You can provide the date also as a string in the format Qt::IsoDate)
     * You can leave out entries from this list, which will then be filled with null values.
     * Indicate the values that you have passed in the ItemInformation flag in the third parameters.
     */
    void addItemInformation(qlonglong imageID, const QVariantList& infos,
                            DatabaseFields::ItemInformation fields = DatabaseFields::ItemInformationAll);

    /**
     * Change the indicated fields of the image information for the specified item.
     * Fields not indicated by the fields parameter will not be touched.
     * This method does nothing if the item does not yet have an entry in the ItemInformation table.
     * The parameters are as for the method above.
     */
    void changeItemInformation(qlonglong imageID, const QVariantList& infos,
                               DatabaseFields::ItemInformation fields = DatabaseFields::ItemInformationAll);

    /**
     * Read image information. Parameters as above.
     */
    QVariantList getItemInformation(qlonglong imageID,
                                    DatabaseFields::ItemInformation infoFields
                                        = DatabaseFields::ItemInformationAll)                                       const;

    /**
     * Add (or replace) the ImageMetadata of the specified item.
     * If there is already an entry, it will be discarded.
     * The QVariantList shall have at most 16 entries, of types as defined
     * in the DBSCHEMA and in metadatainfo.h, in this order:
     *  0) String    make
     *  1) String    model
     *  2) String    lens
     *  3) Double    aperture
     *  4) Double    focalLength
     *  5) Double    focalLength35
     *  6) Double    exposureTime
     *  7) Int       exposureProgram
     *  8) Int       exposureMode
     *  9) Int       sensitivity
     * 10) Int       flash
     * 11) Int       WhiteBalance
     * 12) Int       WhiteBalanceColorTemperature
     * 13) Int       meteringMode
     * 14) Double    subjectDistance
     * 15) Double    subjectDistanceCategory
     * You can leave out entries from this list. Indicate the values that you have
     * passed in the ImageMetadata flag in the third parameters.
     */
    void addImageMetadata(qlonglong imageID, const QVariantList& infos,
                          DatabaseFields::ImageMetadata fields = DatabaseFields::ImageMetadataAll);

    /**
     * Change the indicated fields of the image information for the specified item.
     * This method does nothing if the item does not yet have an entry in the ItemInformation table.
     * The parameters are as for the method above.
     */
    void changeImageMetadata(qlonglong imageID, const QVariantList& infos,
                             DatabaseFields::ImageMetadata fields = DatabaseFields::ImageMetadataAll);

    /**
     * Read image metadata. Parameters as above.
     */
    QVariantList getImageMetadata(qlonglong imageID,
                                  DatabaseFields::ImageMetadata metadataFields = DatabaseFields::ImageMetadataAll) const;

    /**
     * Add (or replace) the VideoMetadata of the specified item.
     * If there is already an entry, it will be discarded.
     * The QVariantList shall have 8 entries, of types in this order:
     * 0) String    AspectRatio
     * 1) String    AudioBitRate
     * 2) String    AudioChannelType
     * 3) String    AudioCodec
     * 4) String    Duration
     * 5) String    FrameRate
     * 6) String    VideoCodec
     * You can leave out entries from this list, which will then be filled with null values.
     * Indicate the values that you have passed in the VideoMetadata flag in the third parameters.
     */
    void addVideoMetadata(qlonglong imageID, const QVariantList& infos,
                          DatabaseFields::VideoMetadata fields = DatabaseFields::VideoMetadataAll);

    /**
     * Change the indicated fields of the video information for the specified item.
     * This method does nothing if the item does not yet have an entry in the ItemInformation table.
     * The parameters are as for the method above.
     */
    void changeVideoMetadata(qlonglong imageID, const QVariantList& infos,
                             DatabaseFields::VideoMetadata fields = DatabaseFields::VideoMetadataAll);

    /**
     * Read video metadata. Parameters as above.
     */
    QVariantList getVideoMetadata(qlonglong imageID,
                                  DatabaseFields::VideoMetadata metadataFields = DatabaseFields::VideoMetadataAll)  const;

    /**
     * Add (or replace) the ItemPosition of the specified item.
     * If there is already an entry, it will be discarded.
     * The QVariantList shall have at most 10 entries, of types in this order:
     * 0) String    Latitude
     * 1) Double    LatitudeNumber
     * 2) String    Longitude
     * 3) Double    LongitudeNumber
     * 4) Double    Altitude
     * 5) Double    Orientation
     * 6) Double    Tilt
     * 7) Double    Roll
     * 8) Double    Accuracy
     * 9) String    Description
     * You can leave out entries from this list. Indicate the values that you have
     * passed in the ItemInfo flag in the third parameters.
     */
    void addItemPosition(qlonglong imageID, const QVariantList& infos,
                          DatabaseFields::ItemPositions fields = DatabaseFields::ItemPositionsAll);

    /**
     * Change the indicated fields of the image information for the specified item.
     * This method does nothing if the item does not yet have an entry in the ItemInformation table.
     * The parameters are as for the method above.
     */
    void changeItemPosition(qlonglong imageID, const QVariantList& infos,
                            DatabaseFields::ItemPositions fields = DatabaseFields::ItemPositionsAll);

    /**
     * Read image metadata. Parameters as above.
     */
    QVariantList getItemPosition(qlonglong imageID,
                                 DatabaseFields::ItemPositions positionFields = DatabaseFields::ItemPositionsAll)   const;

    QVariantList getItemPositions(QList<qlonglong> imageIDs, DatabaseFields::ItemPositions fields)                  const;

    /**
     * Remove the entry in ItemPositions for the given image
     */
    void removeItemPosition(qlonglong imageid);

    /**
     * Remove the altitude in ItemPositions for the given image
     */
    void removeItemPositionAltitude(qlonglong imageid);

    /**
     * Retrieves all available comments for the specified item.
     */
    QList<CommentInfo> getItemComments(qlonglong imageID)                                                           const;

    /**
     * Sets the comments for the image. A comment for the image with the same
     * source, language and author will be overwritten.
     * @param imageID  The imageID of the image
     * @param comment  The comment string
     * @param type     The type of the comment
     * @param language Information about the language of the comment. A null string shall be used
     *                 if language information is not available from the source, or if
     *                 the comment is in the default language.
     * @param author   Optional information about the author who wrote the comment.
     *                 If not supported by the source, pass a null string.
     * @param date     Optional information about the date when the comment was written
     *                 If not supported by the source, pass a null string.
     * @returns the comment ID of the comment
     */
    int setImageComment(qlonglong imageID, const QString& comment, DatabaseComment::Type type,
                        const QString& language = QString(), const QString& author = QString(),
                        const QDateTime& date = QDateTime())                                                        const;

    /**
     * Changes the properties of a comment.
     * The QVariantList shall have at most 5 entries, of types in this order:
     * 0) Int       Type
     * 1) String    Language
     * 2) String    Author
     * 3) DateTime  Date
     * 4) String    Comment
     */
    void changeImageComment(int commentId, qlonglong imageID, const QVariantList& infos,
                            DatabaseFields::ItemComments fields = DatabaseFields::ItemCommentsAll);

    /**
     * Remove the specified entry in ItemComments
     */
    void removeImageComment(int commentId, qlonglong imageID);

    /**
     * Remove all ItemComments
     */
    void removeAllImageComments(qlonglong imageID);

    /**
     * Returns the property with the specified name for the specified image
     */
    QString getImageProperty(qlonglong imageID, const QString& property)                                            const;

    /**
     * Sets the property with the given name for the given image to the specified value
     */
    void setImageProperty(qlonglong imageID, const QString& property, const QString& value);
    void removeImageProperty(qlonglong imageID, const QString& property);
    void removeImagePropertyByName(const QString& property);
    void removeAllImageProperties(qlonglong imageID);

    QStringList getAllImagePropertiesByName(const QString& property)                                                const;

    /**
     * Returns the copyright properties of the specified image.
     * If property is not null, only the given property is returned.
     */
    QList<CopyrightInfo> getItemCopyright(qlonglong imageID, const QString& property = QString())                   const;

    enum CopyrightPropertyUnique
    {
        PropertyUnique,
        PropertyExtraValueUnique,
        PropertyNoConstraint
    };

    /**
     * Sets the property with the given name for the given image to the specified value and extraValue
     */
    void setItemCopyrightProperty(qlonglong imageID, const QString& property,
                                   const QString& value, const QString& extraValue = QString(),
                                   CopyrightPropertyUnique uniqueness = PropertyUnique);

    /**
     * Removes copyright properties for the given image id. All values after the first null value,
     * in order of parameters, are treated as wild cards (you can give value as wildcard; value and
     * extraValue; or property, extraValue and value).
     * Note that extraValue is ordered before value in this method!
     */
    void removeItemCopyrightProperties(qlonglong imageID, const QString& property = QString(),
                                        const QString& extraValue = QString(),
                                        const QString& value = QString() /* NOTE parameter order */);

    /**
     * Removes all copyright properties for the given image id.
     */
    void removeAllItemCopyrightProperties(qlonglong imageID);

    /**
     * Returns all items with the given file name and creation date.
     */
    QList<qlonglong> findByNameAndCreationDate(const QString& fileName, const QDateTime& creationDate)              const;

    /**
     * Retrieves the history entry for the given image.
     */
    ImageHistoryEntry getItemHistory(qlonglong imageId)                                                             const;

    /**
     * Retrieves the image UUID
     */
    QString getImageUuid(qlonglong imageId)                                                                         const;

    /**
     * Retrieves the images with the given UUID
     */
    QList<qlonglong> getItemsForUuid(const QString& uuid)                                                           const;

    /**
     * Changes (adds or updates) the image history
     */
    void setItemHistory(qlonglong imageId, const QString& history);
    void setImageUuid(qlonglong imageId, const QString& uuid);

    /**
     * Returns true if the image has a history stored in DB
     * If not, it returns false
     */
    bool hasImageHistory(qlonglong imageId)                                                                         const;

    /**
     * Adds an image relation entry.
     */
    void addImageRelation(qlonglong subjectId, qlonglong objectId, DatabaseRelation::Type type);
    void addImageRelation(const ImageRelation& relation);

    /**
     * This method requires two lists of same size and will add list1[0]->list2[0],...,list1[n]->list2[n]
     */
    void addImageRelations(const QList<qlonglong>& subjectIds, const QList<qlonglong>& objectIds,
                           DatabaseRelation::Type type);

    /**
     * Removes image relations.
     * The batch methods return all removed partners.
     */
    void removeImageRelation(qlonglong subjectId, qlonglong objectId, DatabaseRelation::Type type);
    void removeImageRelation(const ImageRelation& relation);

    QList<qlonglong> removeAllImageRelationsTo(qlonglong objectId, DatabaseRelation::Type type)                     const;
    QList<qlonglong> removeAllImageRelationsFrom(qlonglong subjectId, DatabaseRelation::Type type)                  const;

    /**
     * Retrieves all images that the given image is related to (retrieves objects, given image is subject)
     * If type is given, filters by type, otherwise returns all types.
     * "Get images related to from this"
     */
    QList<qlonglong> getImagesRelatedFrom(qlonglong subjectId,
                                          DatabaseRelation::Type type = DatabaseRelation::UndefinedType)            const;
    bool hasImagesRelatedFrom(qlonglong subjectId,
                              DatabaseRelation::Type type = DatabaseRelation::UndefinedType)                        const;
    QVector<QList<qlonglong> > getImagesRelatedFrom(const QList<qlonglong>& subjectIds,
                                                    DatabaseRelation::Type type = DatabaseRelation::UndefinedType)  const;
    /**
     * Retrieves all images that relate to the given image (retrieves subject, given image is object)
     * If type is given, filters by type, otherwise returns all types.
     * "Get images this image is relating to"
     */
    QList<qlonglong> getImagesRelatingTo(qlonglong objectId,
                                         DatabaseRelation::Type type = DatabaseRelation::UndefinedType)             const;
    bool hasImagesRelatingTo(qlonglong objectId,
                             DatabaseRelation::Type type = DatabaseRelation::UndefinedType)                         const;
    QVector<QList<qlonglong> > getImagesRelatingTo(const QList<qlonglong>& objectIds,
                                                   DatabaseRelation::Type type = DatabaseRelation::UndefinedType)   const;

    /**
     * For the given image id, retrieves all relations of all related images:
     * Each pair (a,b) means "a is related to b".
     * Each a and b in the list will have a direct or indirect relation to the initial imageId.
     * If type is given, filters by type, otherwise returns all types.
     */
    QList<QPair<qlonglong, qlonglong> > getRelationCloud(qlonglong imageId,
                                                         DatabaseRelation::Type type
                                                             = DatabaseRelation::UndefinedType)                     const;

    /**
     * For each of the given ids, find one single related image (direction does not matter).
     * Ids are unique in the returned list, and do not correspond by index to the given list.
     */
    QList<qlonglong> getOneRelatedImageEach(const QList<qlonglong>& ids,
                                            DatabaseRelation::Type type = DatabaseRelation::UndefinedType)          const;

    /**
     * Retrieves all images that related to (retrieves objects) by given type.
     */
    QList<qlonglong> getRelatedImagesToByType(DatabaseRelation::Type type)                                          const;

    /**
     * Returns a list of all images where the Faces have either not been detected
     * yet, or is outdated because the file is identified as changed since
     * the generation of the fingerprint.
     * Return image ids or item URLs.
     */
    QStringList getDirtyOrMissingFaceImageUrls()                                                                    const;

    /**
     * Find items that are, with reasonable certainty, identical
     * to the file pointed to by id.
     * Criteria: Unique Hash, file size and album non-null.
     * The first variant will not return an ItemScanInfo for id.
     * The second allows to pass one id as source id for exclusion from the list.
     * If this is -1, no id is excluded.
     */
    QList<ItemScanInfo> getIdenticalFiles(qlonglong id)                                                             const;
    QList<ItemScanInfo> getIdenticalFiles(const QString& uniqueHash, qlonglong fileSize, qlonglong sourceId = -1)   const;

    /**
     * Returns a list of all images where tagId is assigned
     * Return item URLs.
     */
    QStringList getItemsURLsWithTag(int tagId)                                                                      const;

    /**
     * Returns the first item that has a confirmed face with the tag.
     */
    qlonglong getFirstItemWithFaceTag(int tagId)                                                                    const;

    // ----------- Items and their tags -----------

    /**
     * Add a tag for the item
     * @param imageID the ID of the item
     * @param tagID   the tagID for the tag
     * @param newTag  add to last assigned tag list
     */
    void addItemTag(qlonglong imageID, int tagID, bool newTag = false);

    /**
     * Add a tag for the item
     * @param albumID the albumID of the item
     * @param name    the name of the item
     * @param tagID   the tagID for the tag
     */
    void addItemTag(int albumID, const QString& name, int tagID);

    /**
     * Add each tag of a list of tags
     * to each member of a list of items.
     */
    void addTagsToItems(QList<qlonglong> imageIDs, QList<int> tagIDs);

    /**
     * Remove a specific tag for the item
     * @param imageID the ID of the item
     * @param tagID   the tagID for the tag
     */
    void removeItemTag(qlonglong imageID, int tagID);

    /**
     * Remove all tags for the item
     * @param imageID the ID of the item
     * @param currentTagIds the current tags ids assigned to the item
     */
    void removeItemAllTags(qlonglong imageID, const QList<int>& currentTagIds);

    /**
     * Remove each tag from a list of tags
     * from a each member of a list of items.
     */
    void removeTagsFromItems(QList<qlonglong> imageIDs, const QList<int>& tagIDs);

    /**
     * Get a list of names of all the tags for the item
     * @param imageID the ID of the item
     * @return the list of names of all tags for the item
     */
    QStringList getItemTagNames(qlonglong imageID)                                                                  const;

    /**
     * Get a list of IDs of all the tags for the item
     * @param imageID the ID of the item
     * @return the list of IDs of all tags for the item
     */
    QList<int> getItemTagIDs(qlonglong imageID)                                                                     const;

    /**
     * For a list of items, return the tag ids associated with the item.
     * Amounts to calling getItemTagIDs for each id in imageIds, but is optimized.
     */
    QVector<QList<int> > getItemsTagIDs(const QList<qlonglong>& imageIds)                                           const;

    /**
     * Get the properties for the given image/tag pair.
     * If the tagID is -1, returns the ImageTagProperties for all tagIds of the given image.
     */
    QList<ImageTagProperty> getImageTagProperties(qlonglong imageId, int tagId = -1)                                const;

    /**
     * Get all tagIds for which ImageTagProperties exist for the given image.
     */
    QList<int> getTagIdsWithProperties(qlonglong imageId)                                                           const;

    /**
     * Adds a tag property. Note that this never replaces existing entries.
     * It is also all right to add multiple entries for a tag with the same property.
     * To replace an existing entry, remove the entry before.
     */
    void addImageTagProperty(qlonglong imageId, int tagId, const QString& property, const QString& value);
    void addImageTagProperty(const ImageTagProperty& property);

    /**
     * Removes properties for the given tag. If the value is given, removes only
     * the entries with the given property/value pair. If only property is given,
     * removes all properties with the given name. If property is null,
     * removes all properties for the given tag.
     * If tagId is -1, removes all image tag properties for the given image.
     * Note: After the first parameter you give as a wildcard, the following will be ignored and taken as wildcard as well.
     */
    void removeImageTagProperties(qlonglong imageId, int tagId = -1, const QString& property = QString(),
                                  const QString& value = QString());

    /**
     * Given a set of items (identified by their IDs),
     * this will see if any of the items has a tag.
     * @param imageIDList a list of IDs of the items
     * @return true if at least one of the items has a tag
     */
    bool hasTags(const QList<qlonglong>& imageIDList)                                                               const;

    /**
     * Given a set of items (identified by their IDs),
     * get a list of ID of all common tags
     * @param imageIDList a list of IDs of the items
     * @return the list of common IDs of the given items
     */
    QList<int> getItemCommonTagIDs(const QList<qlonglong>& imageIDList)                                             const;

    /**
     * Returns a QHash<int, int> of tag id -> count of items
     * with the tag
     */
    QHash<int, int> getNumberOfImagesInTags()                                                                        const;

    /**
     * Returns a QHash<int, int> of tag id -> count of items
     * with the given tag property
     */
    QHash<int, int> getNumberOfImagesInTagProperties(const QString& property)                                        const;

    /**
     * Returns the count of images that have a tag property for the given tag.
     */
    int getNumberOfImagesInTagProperties(int tagId, const QString& property)                                        const;

    /**
     * Returns all image ids that are associated to the tag with the given property.
     */
    QList<qlonglong> getImagesWithImageTagProperty(int tagId, const QString& property)                              const;

    /**
     * Returns all image ids that are associated to the given property.
     */
    QList<qlonglong> getImagesWithProperty(const QString& property)                                                 const;

    /**
     * Returns a QMap<QString,int> of ItemInformation.format
     * -> count of items with that format.
     */
    QMap<QString, int> getFormatStatistics()                                                                        const;
    QMap<QString, int> getFormatStatistics(DatabaseItem::Category category)                                         const;

    /**
     * Return a list from a field from imageMetadata
     */
    QStringList getListFromImageMetadata(DatabaseFields::ImageMetadata field)                                       const;

    // ----------- Moving and Copying Items -----------

    /**
     * Move the attributes of an item to a different item. Useful when
     * say a file is renamed
     * @param  srcAlbumID the id of the source album
     * @param  dstAlbumID the id of the destination album
     * @param  srcName    the name of the source file
     * @param  dstName    the name of the destination file
     */
    void moveItem(int srcAlbumID, const QString& srcName,
                  int dstAlbumID, const QString& dstName);

    /**
     * Copy the attributes of an item to a different item. Useful when
     * say a file is copied.
     * The operation fails (returns -1) of src and dest are identical.
     * @param  srcAlbumID the id of the source album
     * @param  dstAlbumID the id of the destination album
     * @param  srcName    the name of the source file
     * @param  dstName    the name of the destination file
     * @return the id of item added or -1 if it fails
     */
    qlonglong copyItem(int srcAlbumID, const QString& srcName,
                       int dstAlbumID, const QString& dstName);

    /**
     * Copies all image-specific information, in all tables, from image srcId to destId.
     */
    void copyImageAttributes(qlonglong srcId, qlonglong destId);

    /**
     * Copies all entries in the ImageProperties table.
     */
    void copyImageProperties(qlonglong srcId, qlonglong dstId);

    /**
     * Copies all entries in the ImageTags table.
     */
    void copyImageTags(qlonglong srcId, qlonglong dstId);

    // ----------- Download history methods -----------

    /**
     * Search for the specified fingerprint in the download history table.
     * Returns the id of the entry, or -1 if not found.
     */
    int findInDownloadHistory(const QString& identifier, const QString& name,
                              qlonglong fileSize, const QDateTime& date)                                            const;

    /**
     * Add the specified fingerprint to the download history table.
     * Returns the id of the entry.
     */
    int addToDownloadHistory(const QString& identifier, const QString& name,
                             qlonglong fileSize, const QDateTime& date)                                             const;

    QList<QVariant> getImageIdsFromArea(qreal lat1, qreal lat2, qreal lng1, qreal lng2,
                                        int sortMode, const QString& sortBy)                                        const;

    // ----------- Database shrinking methods ----------

    /**
     * Returns true if the integrity of the database is preserved.
     */
    bool integrityCheck()                                                                                           const;

    /**
     * Shrinks the database.
     */
    void vacuum();

    // ----------- Static helper methods for constructing SQL queries -----------

    static QStringList imagesFieldList(DatabaseFields::Images fields);
    static QStringList imageInformationFieldList(DatabaseFields::ItemInformation fields);
    static QStringList videoMetadataFieldList(DatabaseFields::VideoMetadata fields);
    static QStringList imageMetadataFieldList(DatabaseFields::ImageMetadata fields);
    static QStringList imagePositionsFieldList(DatabaseFields::ItemPositions fields);
    static QStringList imageCommentsFieldList(DatabaseFields::ItemComments fields);
    static void addBoundValuePlaceholders(QString& query, int count);

public:

    friend class Digikam::CoreDbAccess;

    /**
     * Constructor
     */
    explicit CoreDB(CoreDbBackend* const backend);

    /**
     * Destructor
     */
    ~CoreDB();

protected:

    QList<qlonglong> getRelatedImages(qlonglong id, bool fromOrTo,
                                      DatabaseRelation::Type type, bool boolean)                                    const;
    QVector<QList<qlonglong> > getRelatedImages(QList<qlonglong> ids, bool fromOrTo,
                                                DatabaseRelation::Type type, bool boolean)                          const;

private:

    // Disable
    CoreDB(const CoreDB&)            = delete;
    CoreDB& operator=(const CoreDB&) = delete;

    void readSettings();
    void writeSettings();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_CORE_DB_H
