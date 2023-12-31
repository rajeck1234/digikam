/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-10-31
 * Description : QtAV: Multimedia framework based on Qt and FFmpeg
 *
 * SPDX-FileCopyrightText: 2012-2022 Wang Bin <wbsecg1 at gmail dot com>
 * SPDX-FileCopyrightText:      2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef QTAV_MEDIA_IO_H
#define QTAV_MEDIA_IO_H

// C++ includes

#include <cstdio> // SEEK_SET

// Qt includes

#include <QStringList>
#include <QObject>

// Local includes

#include "QtAV_Global.h"

namespace QtAV
{

/*!
 * \brief MediaIO
 * Built-in io (use MediaIO::create(name), example: MediaIO *qio = MediaIO::create("QIODevice"))
 * "QIODevice":
 *   properties:
 *     device - read/write. parameter: QIODevice*. example: io->setDevice(mydev)
 * "QFile"
 *   properties:
 *     device - read only. example: io->device()
 *   protocols: "", "qrc"
 */
typedef int MediaIOId;

class MediaIOPrivate;

class DIGIKAM_EXPORT MediaIO : public QObject
{
    Q_OBJECT
    DPTR_DECLARE_PRIVATE(MediaIO)
    Q_DISABLE_COPY(MediaIO)

public:

    enum AccessMode
    {
        Read, ///< default
        Write
    };
    Q_ENUM(AccessMode)

    /// Registered MediaIO::name(): "QIODevice", "QFile"

    static QStringList builtInNames();

    /*!
     * \brief createForProtocol
     * If an MediaIO subclass SomeInput.protocols() contains the protocol, return it's instance.
     * "QFile" input has protocols "qrc"(and empty "" means "qrc")
     * \return Null if none of registered MediaIO supports the protocol
     */
    static MediaIO* createForProtocol(const QString& protocol);

    /*!
     * \brief createForUrl
     * Create a MediaIO and setUrl(url) if protocol of url is supported.
     * Example: MediaIO *qrc = MediaIO::createForUrl("qrc:/icon/test.mkv");
     * \return MediaIO instance with url set. Null if protocol is not supported.
     */
    static MediaIO* createForUrl(const QString& url);

    virtual ~MediaIO();
    virtual QString name() const = 0;

    /*!
     * \brief setUrl
     * onUrlChange() will be called if url is different.
     * onUrlChange() will close the old url and open the new url if it's not empty
     * \param url
     */
    void setUrl(const QString& url = QString());
    QString url() const;

    /*!
     * \brief setAccessMode
     * A MediaIO instance can be 1 mode, Read (default) or Write.
     * If !isWritable(), then set to Write will fail and mode does not change
     * Call it before any function!
     * \return false if set failed
     */
    bool setAccessMode(AccessMode value);
    AccessMode accessMode() const;

    /// supported protocols. default is empty

    virtual const QStringList& protocols() const;
    virtual bool isSeekable() const = 0;

    virtual bool isWritable() const
    {
        return false;
    }

    /*!
     * \brief read
     * read at most maxSize bytes to data, and return the bytes were actually read
     */
    virtual qint64 read(char* data, qint64 maxSize) = 0;

    /*!
     * \brief write
     * write at most maxSize bytes from data, and return the bytes were actually written
     */
    virtual qint64 write(const char* data, qint64 maxSize)
    {
        Q_UNUSED(data);
        Q_UNUSED(maxSize);

        return 0;
    }

    /*!
     * \brief seek
     * \param from SEEK_SET, SEEK_CUR and SEEK_END from stdio.h
     * \return true if success
     */
    virtual bool seek(qint64 offset, int from = SEEK_SET) = 0;

    /*!
     * \brief position
     * MUST implement this. Used in seek
     * TODO: implement internally by default
     */
    virtual qint64 position() const = 0;

    /*!
     * \brief size
     * \return <=0 if not support
     */
    virtual qint64 size() const = 0;

    /*!
     * \brief isVariableSize
     * Experiment: A hack for size() changes during playback.
     * If true, containers that estimate duration from pts(or bit rate)
     * will get an invalid duration. Thus no eof get
     * when the size of playback start reaches. So playback will not stop.
     * Demuxer seeking should work for this case.
     */
    virtual bool isVariableSize() const
    {
        return false;
    }

    /*!
     * \brief setBufferSize
     * \param value <0: use default value
     */
    void setBufferSize(int value = -1);
    int bufferSize() const;

    // The followings are for internal use. used by AVDemuxer, AVMuxer

    //struct AVIOContext; // anonymous struct in FFmpeg1.0.x

    void* avioContext();  // const?
    void release();       // TODO: how to remove it?

public:

    static void registerAll();

    template<class C>
    static bool Register(MediaIOId id, const char* name)
    {
        return Register(id, create<C>, name);
    }

    static MediaIO* create(MediaIOId id);
    static MediaIO* create(const char* name);

    /*!
     * \brief next
     * \param id nullptr to get the first id address
     * \return address of id or nullptr if not found/end
     */
    static MediaIOId* next(MediaIOId* id = nullptr);
    static const char* name(MediaIOId id);
    static MediaIOId id(const char* name);

private:

    template<class C>
    static MediaIO* create()
    {
        return new C();
    }

    typedef MediaIO* (*MediaIOCreator)();

    static bool Register(MediaIOId id, MediaIOCreator, const char* name);

protected:

    MediaIO(MediaIOPrivate& d, QObject* const parent = nullptr);

    /*!
     * \brief onUrlChanged
     * Here you can close old url, parse new url() and open it
     */
    virtual void onUrlChanged();
    DPTR_DECLARE(MediaIO)

    // base class, not direct create. only final class has public ctor is enough
    // FIXME: it's required by Q_DECLARE_METATYPE (also copy ctor)

    MediaIO(QObject* const parent = nullptr);
};

typedef MediaIO AVInput; // for source compatibility

} // namespace QtAV

#endif // QTAV_MEDIA_IO_H
