/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-09
 * Description : Collection location management
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "collectionmanager_p.h"

/**
 * NOTE: This is because of the CollectionManager private slot.
 */
#include "moc_collectionmanager.cpp"

namespace Digikam
{

CollectionManager* CollectionManager::m_instance = nullptr;

CollectionManager* CollectionManager::instance()
{
    if (!m_instance)
    {
        m_instance = new CollectionManager;
    }

    return m_instance;
}

void CollectionManager::cleanUp()
{
    delete m_instance;
    m_instance = nullptr;
}

CollectionManager::CollectionManager()
    : d(new Private(this))
{
    qRegisterMetaType<CollectionLocation>("CollectionLocation");

    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)),
            this, SLOT(deviceAdded(QString)));

    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)),
            this, SLOT(deviceRemoved(QString)));

    // CoreDbWatch slot is connected at construction of CoreDbWatch, which may be later.
}

CollectionManager::~CollectionManager()
{
    qDeleteAll(d->locations);
    delete d;
}

void CollectionManager::refresh()
{
    clearLocations();
    updateLocations();
}

void CollectionManager::setWatchDisabled()
{
    d->watchEnabled = false;
}

void CollectionManager::deviceAdded(const QString& udi)
{
    if (!d->watchEnabled)
    {
        return;
    }

    Solid::Device device(udi);

    if (device.is<Solid::StorageAccess>())
    {
        updateLocations();
    }
}

void CollectionManager::deviceRemoved(const QString& udi)
{
    if (!d->watchEnabled)
    {
        return;
    }

    // we can't access the Solid::Device to check because it is removed
    {
        QReadLocker locker(&d->lock);

        if (!d->udisToWatch.contains(udi))
        {
            return;
        }
    }

    updateLocations();
}

void CollectionManager::accessibilityChanged(bool accessible, const QString& udi)
{
    Q_UNUSED(accessible);
    Q_UNUSED(udi);
    updateLocations();
}

void CollectionManager::clearLocations()
{
    QWriteLocker locker(&d->lock);

    // Internal method: Called with write lock
    // Cave: Difficult recursions with CoreDbAccess constructor and setParameters

    Q_FOREACH (AlbumRootLocation* const location, d->locations)
    {
        CollectionLocation::Status oldStatus = location->status();
        location->setStatus(CollectionLocation::LocationDeleted);
        locker.unlock();
        Q_EMIT locationStatusChanged(*location, oldStatus);
        locker.relock();
        delete location;
    }

    d->locations.clear();
}

} // namespace Digikam
