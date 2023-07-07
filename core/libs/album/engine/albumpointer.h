/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-06-15
 * Description : Albums manager interface.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ALBUM_POINTER_H
#define DIGIKAM_ALBUM_POINTER_H

// Qt includes

#include <QList>

// Local includes

#include "album.h"
#include "albummanager.h"

namespace Digikam
{

/**
 * You can use AlbumPointer to store a guarded pointer to Album
 * or one of the subclasses (use template parameter).
 * The pointer will be set to 0 when the album object is deleted.
 */
template <class T = Album>

class AlbumPointer
{
public:

    AlbumPointer()
        : album(nullptr)
    {
    }

    // cppcheck-suppress noExplicitConstructor
    AlbumPointer(T* const a)                    // krazy:exclude=explicit
        : album(a)
    {
        AlbumManager::instance()->addGuardedPointer(album, &album);
    }

    // cppcheck-suppress noExplicitConstructor
    AlbumPointer(const AlbumPointer<T>& p)      // krazy:exclude=explicit
        : album(p.album)
    {
        AlbumManager::instance()->addGuardedPointer(album, &album);
    }

    ~AlbumPointer()
    {
        AlbumManager::instance()->removeGuardedPointer(album, &album);
    }

    AlbumPointer<T>& operator=(T* const a)
    {
        Album* const oldAlbum = album;
        album                 = a;
        AlbumManager::instance()->changeGuardedPointer(oldAlbum, album, &album);

        return *this;
    }

    AlbumPointer<T>& operator=(const AlbumPointer<T>& p)
    {
        Album* const oldAlbum = album;
        album                 = p.album;
        AlbumManager::instance()->changeGuardedPointer(oldAlbum, album, &album);

        return *this;
    }

    T* operator->() const
    {
        return static_cast<T*>(const_cast<Album*>(album));
    }

    T& operator*() const
    {
        return *static_cast<T*>(const_cast<Album*>(album));
    }

    operator T* () const
    {
        return static_cast<T*>(const_cast<Album*>(album));
    }

    bool operator!() const
    {
        return !album;
    }

private:

    friend class AlbumManager;
    Album* album;
};

// ------------------------------------------------------------------------------------

template <class T = Album>

class AlbumPointerList : public QList<AlbumPointer<T> >
{
public:

    AlbumPointerList()
    {
    }

    explicit AlbumPointerList(const AlbumPointerList<T>& list)
        : QList<AlbumPointer<T> >(list)
    {
    }

    explicit AlbumPointerList(const QList<T*>& list)
    {
        operator=(list);
    }

    // cppcheck-suppress operatorEqRetRefThis
    AlbumPointerList<T>& operator=(const AlbumPointerList<T>& list)
    {
        return QList<AlbumPointer<T> >::operator=(list);
    }

    AlbumPointerList<T>& operator=(const QList<T*>& list)
    {
        Q_FOREACH (T* const t, list)
        {
            this->append(AlbumPointer<T>(t));
        }

        return *this;
    }
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::AlbumPointer<>)
Q_DECLARE_METATYPE(Digikam::AlbumPointer<Digikam::PAlbum>)
Q_DECLARE_METATYPE(Digikam::AlbumPointer<Digikam::TAlbum>)
Q_DECLARE_METATYPE(Digikam::AlbumPointer<Digikam::SAlbum>)
Q_DECLARE_METATYPE(Digikam::AlbumPointer<Digikam::DAlbum>)

#endif // DIGIKAM_ALBUM_POINTER_H
