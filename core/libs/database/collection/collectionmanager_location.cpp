/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-09
 * Description : Collection location management - location helpers.
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "collectionmanager_p.h"

namespace Digikam
{

CollectionLocation CollectionManager::addLocation(const QUrl& fileUrl, const QString& label)
{
    qCDebug(DIGIKAM_DATABASE_LOG) << "addLocation" << fileUrl;
    QString path = fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile();

    if (!locationForPath(path).isNull())
    {
        return CollectionLocation();
    }

    QList<SolidVolumeInfo> volumes = d->listVolumes();
    SolidVolumeInfo volume         = d->findVolumeForUrl(fileUrl, volumes);

    if (!volume.isNull())
    {
        // volume.path has a trailing slash. We want to split in front of this.

        QString specificPath = path.mid(volume.path.length() - 1);
        CollectionLocation::Type type;

        if (volume.isRemovable)
        {
            type = CollectionLocation::VolumeRemovable;
        }
        else
        {
            type = CollectionLocation::VolumeHardWired;
        }

        ChangingDB changing(d);
        CoreDbAccess().db()->addAlbumRoot(type, d->volumeIdentifier(volume), specificPath, label);
    }
    else
    {
        // Empty volumes indicates that Solid is not working correctly.

        if (volumes.isEmpty())
        {
            qCDebug(DIGIKAM_DATABASE_LOG) << "Solid did not return any storage volumes on your system.";
            qCDebug(DIGIKAM_DATABASE_LOG) << "This indicates a missing implementation or a problem with your installation";
            qCDebug(DIGIKAM_DATABASE_LOG) << "On Linux, check that Solid and HAL are working correctly. "
                                             "Problems with RAID partitions have been reported, "
                                             "if you have RAID this error may be normal.";
            qCDebug(DIGIKAM_DATABASE_LOG) << "On Windows, Solid may not be fully implemented, "
                                             "if you are running Windows this error may be normal.";
        }

        // fall back

        qCWarning(DIGIKAM_DATABASE_LOG) << "Unable to identify a path with Solid. Adding the location with path only.";

        ChangingDB changing(d);
        CoreDbAccess().db()->addAlbumRoot(CollectionLocation::VolumeHardWired,
                                          d->volumeIdentifier(path), QLatin1String("/"), label);
    }

    // Do not Q_EMIT the locationAdded signal here, it is done in updateLocations()

    updateLocations();

    return locationForPath(path);
}

CollectionLocation CollectionManager::addNetworkLocation(const QUrl& fileUrl, const QString& label)
{
    qCDebug(DIGIKAM_DATABASE_LOG) << "addLocation" << fileUrl;
    QString path = fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile();

    if (!locationForPath(path).isNull())
    {
        return CollectionLocation();
    }

    ChangingDB changing(d);
    CoreDbAccess().db()->addAlbumRoot(CollectionLocation::Network,
                                      d->networkShareIdentifier(QStringList() << path),
                                      QLatin1String("/"), label);

    // Do not Q_EMIT the locationAdded signal here, it is done in updateLocations()

    updateLocations();

    return locationForPath(path);
}

CollectionLocation CollectionManager::refreshLocation(const CollectionLocation& location, int newType,
                                                      const QStringList& pathList, const QString& label)
{
    QUrl fileUrl = QUrl::fromLocalFile(pathList.first());
    qCDebug(DIGIKAM_DATABASE_LOG) << "refreshLocation" << fileUrl;
    QString path = fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile();

    if (location.isNull())
    {
        return CollectionLocation();
    }

    QWriteLocker locker(&d->lock);

    AlbumRootLocation* const albumLoc = d->locations.value(location.id());

    if (!albumLoc)
    {
        return CollectionLocation();
    }

    QList<SolidVolumeInfo> volumes = d->listVolumes();
    SolidVolumeInfo volume         = d->findVolumeForUrl(fileUrl, volumes);

    if (!volume.isNull() || (newType == CollectionLocation::Network))
    {
        CollectionLocation::Type type;
        QString specificPath;
        QString identifier;

        if      (newType == CollectionLocation::VolumeRemovable)
        {
            // volume.path has a trailing slash. We want to split in front of this.

            type         = CollectionLocation::VolumeRemovable;
            identifier   = d->volumeIdentifier(volume);
            specificPath = path.mid(volume.path.length() - 1);
        }
        else if (newType == CollectionLocation::Network)
        {
            type         = CollectionLocation::Network;
            specificPath = QLatin1String("/");
            identifier   = d->networkShareIdentifier(pathList);
        }
        else
        {
            type         = CollectionLocation::VolumeHardWired;
            identifier   = d->volumeIdentifier(volume);
            specificPath = path.mid(volume.path.length() - 1);
        }

        CoreDbAccess access;
        ChangingDB changing(d);
        access.db()->setAlbumRootLabel(location.id(),           label);
        access.db()->setAlbumRootType(location.id(),            type);
        access.db()->migrateAlbumRoot(location.id(),            identifier);
        access.db()->setAlbumRootPath(location.id(),            specificPath);
        access.db()->setAlbumRootCaseSensitivity(location.id(), CollectionLocation::UnknownCaseSensitivity);

        albumLoc->setLabel(label);
        albumLoc->identifier   = identifier;
        albumLoc->specificPath = specificPath;
        albumLoc->setType((CollectionLocation::Type)type);
        albumLoc->setCaseSensitivity(CollectionLocation::UnknownCaseSensitivity);

        locker.unlock();

        Q_EMIT locationPropertiesChanged(*albumLoc);
    }
    else
    {
        // Empty volumes indicates that Solid is not working correctly.

        if (volumes.isEmpty())
        {
            qCDebug(DIGIKAM_DATABASE_LOG) << "Solid did not return any storage volumes on your system.";
            qCDebug(DIGIKAM_DATABASE_LOG) << "This indicates a missing implementation or a problem with your installation";
            qCDebug(DIGIKAM_DATABASE_LOG) << "On Linux, check that Solid and HAL are working correctly. "
                                             "Problems with RAID partitions have been reported, "
                                             "if you have RAID this error may be normal.";
            qCDebug(DIGIKAM_DATABASE_LOG) << "On Windows, Solid may not be fully implemented, "
                                             "if you are running Windows this error may be normal.";
        }

        // fall back

        qCWarning(DIGIKAM_DATABASE_LOG) << "Unable to identify a path with Solid. Update the location with path only.";

        CoreDbAccess access;
        ChangingDB changing(d);
        CollectionLocation::Type type = CollectionLocation::VolumeHardWired;
        access.db()->setAlbumRootLabel(location.id(),           label);
        access.db()->setAlbumRootType(location.id(),            type);
        access.db()->setAlbumRootPath(location.id(),            QLatin1String("/"));
        access.db()->migrateAlbumRoot(location.id(),            d->volumeIdentifier(path));
        access.db()->setAlbumRootCaseSensitivity(location.id(), CollectionLocation::UnknownCaseSensitivity);

        albumLoc->setLabel(label);
        albumLoc->specificPath = QLatin1String("/");
        albumLoc->setType((CollectionLocation::Type)type);
        albumLoc->identifier   = d->volumeIdentifier(path);
        albumLoc->setCaseSensitivity(CollectionLocation::UnknownCaseSensitivity);

        locker.unlock();

        Q_EMIT locationPropertiesChanged(*albumLoc);
    }

    // Do not Q_EMIT the locationAdded signal here, it is done in updateLocations()

    updateLocations();

    return locationForPath(path);
}

CollectionManager::LocationCheckResult CollectionManager::checkLocation(const QUrl& fileUrl,
                                                                        QList<CollectionLocation>& assumeDeleted,
                                                                        QString* message,
                                                                        QString* iconName)
{
    if (!fileUrl.isLocalFile())
    {
        if (message)
        {
            *message = i18n("Sorry, digiKam does not support remote URLs as collections.");
        }

        if (iconName)
        {
            *iconName = QLatin1String("dialog-error");
        }

        return LocationNotAllowed;
    }

    QString path = fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile();
    QDir dir(path);

    if (!dir.isReadable())
    {
        if (message)
        {
            *message = i18n("The selected folder does not exist or is not readable");
        }

        if (iconName)
        {
            *iconName = QLatin1String("dialog-error");
        }

        return LocationNotAllowed;
    }

    if (d->checkIfExists(path, assumeDeleted))
    {
        if (message)
        {
            *message = i18n("There is already a collection containing the folder \"%1\"", QDir::toNativeSeparators(path));
        }

        if (iconName)
        {
            *iconName = QLatin1String("dialog-error");
        }

        return LocationNotAllowed;
    }

    QList<SolidVolumeInfo> volumes = d->listVolumes();
    SolidVolumeInfo volume         = d->findVolumeForUrl(fileUrl, volumes);

    if (!volume.isNull())
    {
        if (!volume.uuid.isEmpty())
        {
            if (volume.isRemovable)
            {
                if (message)
                {
                    *message = i18n("The storage media can be uniquely identified.");
                }

                if (iconName)
                {
                    *iconName = QLatin1String("drive-removable-media");
                }
            }
            else
            {
                if (message)
                {
                    *message = i18n("The collection is located on your harddisk");
                }

                if (iconName)
                {
                    *iconName = QLatin1String("drive-harddisk");
                }
            }

            return LocationAllRight;
        }
        else if (!volume.label.isEmpty() && (volume.isOpticalDisc || volume.isRemovable))
        {
            if (volume.isOpticalDisc)
            {
                bool hasOtherLocation = false;

                Q_FOREACH (AlbumRootLocation* const otherLocation, d->locations)
                {
                    QUrl otherUrl(otherLocation->identifier);

                    if ((otherUrl.scheme() == QLatin1String("volumeid")) &&
                        (QUrlQuery(otherUrl).queryItemValue(QLatin1String("label")) == volume.label))
                    {
                        hasOtherLocation = true;
                        break;
                    }
                }

                if (iconName)
                {
                    *iconName = QLatin1String("media-optical");
                }

                if (hasOtherLocation)
                {
                    if (message)
                    {
                        *message = i18n("This is a CD/DVD, which is identified by the label "
                                        "that you can set in your CD burning application. "
                                        "There is already another entry with the same label. "
                                        "The two will be distinguished by the files in the top directory, "
                                        "so please do not append files to the CD, or it will not be recognized. "
                                        "In the future, please set a unique label on your CDs and DVDs "
                                        "if you intend to use them with digiKam.");
                    }

                    return LocationHasProblems;
                }
                else
                {
                    if (message)
                    {
                        *message = i18n("This is a CD/DVD. It will be identified by the label (\"%1\")"
                                        "that you have set in your CD burning application. "
                                        "If you create further CDs for use with digikam in the future, "
                                        "please remember to give them a unique label as well.",
                                        volume.label);
                    }

                    return LocationAllRight;
                }
            }
            else
            {
                // Which situation? HasProblems or AllRight?

                if (message)
                {
                    *message = i18n("This is a removable storage medium that will be identified by its label (\"%1\")",
                                    volume.label);
                }

                if (iconName)
                {
                    *iconName = QLatin1String("drive-removable-media");
                }

                return LocationAllRight;
            }
        }
        else
        {
            if (message)
            {
                *message = i18n("This entry will only be identified by the path where it is found on your system (\"%1\"). "
                                "No more specific means of identification (UUID, label) is available.",
                                QDir::toNativeSeparators(volume.path));
            }

            if (iconName)
            {
                *iconName = QLatin1String("drive-removale-media");
            }

            return LocationHasProblems;
        }
    }
    else
    {
        if (message)
        {
            *message = i18n("It is not possible on your system to identify the storage medium of this path. "
                            "It will be added using the file path as the only identifier. "
                            "This will work well for your local hard disk.");
        }

        if (iconName)
        {
            *iconName = QLatin1String("folder-important");
        }

        return LocationHasProblems;
    }
}

CollectionManager::LocationCheckResult CollectionManager::checkNetworkLocation(const QUrl& fileUrl,
                                                                               QList<CollectionLocation>& assumeDeleted,
                                                                               QString* message,
                                                                               QString* iconName)
{
    if (!fileUrl.isLocalFile())
    {
        if (message)
        {
            if (fileUrl.scheme() == QLatin1String("smb"))
            {
                *message = i18n("You need to locally mount your Samba share. "
                                "Sorry, digiKam does currently not support smb:// URLs. ");
            }
            else
            {
                *message = i18n("Your network storage must be set up to be accessible "
                                "as files and folders through the operating system. "
                                "digiKam does not support remote URLs.");
            }
        }

        if (iconName)
        {
            *iconName = QLatin1String("dialog-error");
        }

        return LocationNotAllowed;
    }

    QString path = fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile();

    QDir dir(path);

    if (!dir.isReadable())
    {
        if (message)
        {
            *message = i18n("The selected folder does not exist or is not readable");
        }

        if (iconName)
        {
            *iconName = QLatin1String("dialog-error");
        }

        return LocationNotAllowed;
    }

    if (d->checkIfExists(path, assumeDeleted))
    {
        if (message)
        {
            *message = i18n("There is already a collection for a network share with the same path.");
        }

        if (iconName)
        {
            *iconName = QLatin1String("dialog-error");
        }

        return LocationNotAllowed;
    }

    if (message)
    {
        *message = i18n("The network share will be identified by the path you selected. "
                        "If the path is empty, the share will be considered unavailable.");
    }

    if (iconName)
    {
        *iconName = QLatin1String("network-wired-activated");
    }

    return LocationAllRight;
}

void CollectionManager::removeLocation(const CollectionLocation& location)
{
    QWriteLocker locker(&d->lock);

    AlbumRootLocation* const albumLoc = d->locations.value(location.id());

    if (!albumLoc)
    {
        return;
    }

    // Ensure that all albums are set to orphan and no images will be permanently deleted,
    // as would do only calling deleteAlbumRoot by a Trigger

    CoreDbAccess access;
    QList<int> albumIds = access.db()->getAlbumsOnAlbumRoot(albumLoc->id());
    ChangingDB changing(d);
    CollectionScanner scanner;
    CoreDbTransaction transaction(&access);
    locker.unlock();
    scanner.safelyRemoveAlbums(albumIds);
    locker.relock();
    access.db()->deleteAlbumRoot(albumLoc->id());

    // Do not Q_EMIT the locationRemoved signal here, it is done in updateLocations()

    locker.unlock();

    updateLocations();
}

QList<CollectionLocation> CollectionManager::checkHardWiredLocations()
{
    QList<CollectionLocation> disappearedLocations;
    QList<SolidVolumeInfo> volumes = d->listVolumes();

    QReadLocker locker(&d->lock);

    Q_FOREACH (AlbumRootLocation* const location, d->locations)
    {
        // Hardwired and unavailable?

        if ((location->type()   == CollectionLocation::VolumeHardWired) &&
            (location->status() == CollectionLocation::LocationUnavailable))
        {
            disappearedLocations << *location;
        }
    }

    return disappearedLocations;
}

void CollectionManager::migrationCandidates(const CollectionLocation& location,
                                            QString* const description,
                                            QStringList* const candidateIdentifiers,
                                            QStringList* const candidateDescriptions)
{
    description->clear();
    candidateIdentifiers->clear();
    candidateDescriptions->clear();

    QList<SolidVolumeInfo> volumes = d->listVolumes();

    QReadLocker locker(&d->lock);

    AlbumRootLocation* const albumLoc = d->locations.value(location.id());

    if (!albumLoc)
    {
        return;
    }

    *description = d->technicalDescription(albumLoc);

    // Find possible new volumes where the specific path is found.

    Q_FOREACH (const SolidVolumeInfo& info, volumes)
    {
        if (info.isMounted && !info.path.isEmpty())
        {
            QDir dir(info.path + albumLoc->specificPath);

            if (dir.exists())
            {
                *candidateIdentifiers  << d->volumeIdentifier(info);
                *candidateDescriptions << dir.absolutePath();
            }
        }
    }
}

void CollectionManager::migrateToVolume(const CollectionLocation& location, const QString& identifier)
{
    QWriteLocker locker(&d->lock);

    AlbumRootLocation* const albumLoc = d->locations.value(location.id());

    if (!albumLoc)
    {
        return;
    }

    // update db

    ChangingDB db(d);
    CoreDbAccess().db()->migrateAlbumRoot(albumLoc->id(), identifier);

    // update local structure

    albumLoc->identifier = identifier;

    locker.unlock();

    updateLocations();
}

void CollectionManager::setLabel(const CollectionLocation& location, const QString& label)
{
    QWriteLocker locker(&d->lock);

    AlbumRootLocation* const albumLoc = d->locations.value(location.id());

    if (!albumLoc)
    {
        return;
    }

    // update db

    ChangingDB db(d);
    CoreDbAccess().db()->setAlbumRootLabel(albumLoc->id(), label);

    // update local structure

    albumLoc->setLabel(label);

    locker.unlock();

    Q_EMIT locationPropertiesChanged(*albumLoc);
}

void CollectionManager::changeType(const CollectionLocation& location, int type)
{
    QWriteLocker locker(&d->lock);

    AlbumRootLocation* const albumLoc = d->locations.value(location.id());

    if (!albumLoc)
    {
        return;
    }

    // update db

    ChangingDB db(d);
    CoreDbAccess().db()->setAlbumRootType(albumLoc->id(), (CollectionLocation::Type)type);

    // update local structure

    albumLoc->setType((CollectionLocation::Type)type);

    locker.unlock();

    Q_EMIT locationPropertiesChanged(*albumLoc);
}

QList<CollectionLocation> CollectionManager::allLocations()
{
    QReadLocker locker(&d->lock);

    QList<CollectionLocation> list;

    Q_FOREACH (AlbumRootLocation* const location, d->locations)
    {
        list << *location;
    }

    return list;
}

QList<CollectionLocation> CollectionManager::allAvailableLocations()
{
    QReadLocker locker(&d->lock);

    QList<CollectionLocation> list;

    Q_FOREACH (AlbumRootLocation* const location, d->locations)
    {
        if (location->status() == CollectionLocation::LocationAvailable)
        {
            list << *location;
        }
    }

    return list;
}

CollectionLocation CollectionManager::locationForAlbumRootId(int id)
{
    QReadLocker locker(&d->lock);

    AlbumRootLocation* const location = d->locations.value(id);

    if (location)
    {
        return *location;
    }

    return CollectionLocation();
}

CollectionLocation CollectionManager::locationForAlbumRoot(const QUrl& fileUrl)
{
    return locationForAlbumRootPath(fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile());
}

CollectionLocation CollectionManager::locationForAlbumRootPath(const QString& albumRootPath)
{
    // This function is used when an album is created or an external scan is
    // initiated by the AlbumWatcher. We check if there is an entry because
    // the mount path of a network share may not be available.

    if (!QDirIterator(albumRootPath, QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot).hasNext())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "Album root path not exist" << albumRootPath;
        qCWarning(DIGIKAM_DATABASE_LOG) << "Drive or network connection broken?";

        updateLocations();
    }

    QReadLocker locker(&d->lock);

    Q_FOREACH (AlbumRootLocation* const location, d->locations)
    {
        if (location->albumRootPath() == albumRootPath)
        {   // cppcheck-suppress useStlAlgorithm
            return *location;
        }
    }

    return CollectionLocation();
}

CollectionLocation CollectionManager::locationForUrl(const QUrl& fileUrl)
{
    return locationForPath(fileUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile());
}

CollectionLocation CollectionManager::locationForPath(const QString& givenPath)
{
    QReadLocker locker(&d->lock);

    Q_FOREACH (AlbumRootLocation* const location, d->locations)
    {
        QString rootPath = location->albumRootPath();
        QString filePath = QDir::fromNativeSeparators(givenPath);

        if (!rootPath.isEmpty() && filePath.startsWith(rootPath))
        {
            // see also bug #221155 for extra checks

            if ((filePath == rootPath) || filePath.startsWith(rootPath + QLatin1Char('/')))
            {
                return *location;
            }
        }
    }

    return CollectionLocation();
}

void CollectionManager::updateLocations()
{
    QList<SolidVolumeInfo> volumes;

    // get information from Solid

    volumes = d->listVolumes();

    QWriteLocker locker(&d->lock);

    // read information from database

    QList<AlbumRootInfo> infos = CoreDbAccess().db()->getAlbumRoots();

    // synchronize map with database

    QMap<int, AlbumRootLocation*> locs = d->locations;
    d->locations.clear();

    Q_FOREACH (const AlbumRootInfo& info, infos)
    {
        if (locs.contains(info.id))
        {
            d->locations[info.id] = locs.value(info.id);
            locs.remove(info.id);
        }
        else
        {
            d->locations[info.id] = new AlbumRootLocation(info);
        }
    }

    // delete old locations

    Q_FOREACH (AlbumRootLocation* const location, locs)
    {
        CollectionLocation::Status oldStatus = location->status();
        location->setStatus(CollectionLocation::LocationDeleted);
        locker.unlock();
        Q_EMIT locationStatusChanged(*location, oldStatus);
        locker.relock();
        delete location;
    }

    // update status with current access state, store old status in list

    QList<CollectionLocation::Status> oldStatus;

    Q_FOREACH (AlbumRootLocation* const location, d->locations)
    {
        oldStatus << location->status();
        bool available = false;
        QString absolutePath;

        if (location->type() == CollectionLocation::Network)
        {
            Q_FOREACH (const QString& path, d->networkShareMountPathsFromIdentifier(location))
            {
                QFileInfo fileInfo(path);
                available    = (fileInfo.isReadable() &&
                                QDirIterator(path, QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot).hasNext());
                absolutePath = path;

                if (available)
                {
                    break;
                }
            }
        }
        else
        {
            SolidVolumeInfo info = d->findVolumeForLocation(location, volumes);

            if (!info.isNull())
            {
                QString volumePath = info.path;

                // volume.path has a trailing slash (and this is good)
                // but specific path has a leading slash, so remove it

                volumePath.chop(1);

                // volumePath is the mount point of the volume;
                // specific path is the path on the file system of the volume.

                absolutePath = volumePath + location->specificPath;
                available    = (info.isMounted && QFileInfo::exists(absolutePath));
            }
            else
            {
                QString path = d->pathFromIdentifier(location);

                if (!path.isNull())
                {
                    available    = true;

                    // Here we have the absolute path as definition of the volume.
                    // specificPath is "/" as per convention, but ignored,
                    // absolute path shall not have a trailing slash.

                    absolutePath = path;
                }
            }
        }

        // set values in location
        // Don't touch location->status, do not interfere with "hidden" setting

        location->available = available;
        location->setAbsolutePath(absolutePath);

        if (available && (location->caseSensitivity() == CollectionLocation::UnknownCaseSensitivity))
        {
            QFileInfo writeInfo(absolutePath);

            if (writeInfo.isWritable())
            {
                SafeTemporaryFile* const temp = new SafeTemporaryFile(absolutePath +
                                                                      QLatin1String("/CaseSensitivity-XXXXXX-Test"));
                temp->setAutoRemove(false);
                temp->open();
                QFileInfo tempInfo(temp->safeFilePath());
                QFileInfo testInfo(tempInfo.path()  +
                                   QLatin1Char('/') +
                                   tempInfo.fileName().toLower());
                bool testCaseSensitivity      = testInfo.exists();
                delete temp;
                QFile::remove(tempInfo.filePath());

                if (testCaseSensitivity)
                {
                    location->setCaseSensitivity(CollectionLocation::CaseInsensitive);
                }
                else
                {
                    location->setCaseSensitivity(CollectionLocation::CaseSensitive);
                }

                CoreDbAccess().db()->setAlbumRootCaseSensitivity(location->id(),
                                                                 location->caseSensitivity());
            }
        }

        qCDebug(DIGIKAM_DATABASE_LOG) << "location for" << absolutePath
                                      << "is available:" << available
                                      << "=>" << "case sensitivity:"
                                      << location->caseSensitivity();

        // set the status depending on "hidden" and "available"

        location->setStatusFromFlags();
    }

    // Q_EMIT status changes (and new locations)

    int i = 0;

    Q_FOREACH (AlbumRootLocation* const location, d->locations)
    {
        if (oldStatus.at(i) != location->status())
        {
            locker.unlock();
            Q_EMIT locationStatusChanged(*location, oldStatus.at(i));
            locker.relock();
        }

        ++i;
    }
}

} // namespace Digikam
