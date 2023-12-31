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

#include "MediaIO_p.h"

// Windows includes

#include <shcore.h>
#include <wrl.h>
#include <windows.foundation.h>
#include <Windows.ApplicationModel.activation.h>

// Qt includes

#include <QCoreApplication> // required by qfunctions_winrt.h
#include <qfunctions_winrt.h>

// Local includes

#include "QtAV_mkid.h"
#include "QtAV_factory.h"
#include "digikam_debug.h"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Storage;
using namespace ABI::Windows::Storage::Streams;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;

#define COM_LOG_COMPONENT  "WinRTIO"
#define COM_ENSURE(f, ...) COM_CHECK(f, return __VA_ARGS__;)
#define COM_WARN(f)        COM_CHECK(f)

#define COM_CHECK(f, ...) \
    do { \
        HRESULT hr = f; \
        if (FAILED(hr)) { \
            qCWarning(DIGIKAM_QTAV_LOG_WARN) << QString::fromLatin1(COM_LOG_COMPONENT " error@%1. " #f ": (0x%2) %3").arg(__LINE__).arg(hr, 0, 16).arg(qt_error_string(hr)); \
            __VA_ARGS__ \
        } \
    } while (0)

namespace QtAV
{

static const MediaIOId MediaIOId_WinRT = mkid::id32base36_5<'W','i','n','R','T'>::value;
static const char kName[]              = "WinRT";

class WinRTIOPrivate;

class Q_DECL_HIDDEN WinRTIO : public MediaIO
{
    DPTR_DECLARE_PRIVATE(WinRTIO)

public:

    WinRTIO();

    QString name()                                          const override
    {
        return QLatin1String(kName);
    }

    const QStringList& protocols()                          const override
    {
        static QStringList p = QStringList() << name().toLower();

        return p;
    }

    virtual bool isSeekable()                               const override;
    virtual bool isWritable()                               const override;
    virtual qint64 read(char* data, qint64 maxSize)               override;
    virtual qint64 write(const char* data, qint64 maxSize)        override;
    virtual bool seek(qint64 offset, int from)                    override;
    virtual qint64 position()                               const override;

    /*!
     * \brief size
     * \return <=0 if not support
     */
    virtual qint64 size()                                   const override;

protected:

    explicit WinRTIO(WinRTIOPrivate& d);
    void onUrlChanged()                                           override;

private:

    // item available until this function finished. If app is launch from association, addRef there

    void openFromStorage(IStorageItem* item);

    // only special locations is supported, but seems better than qfile if not use file picker
    // qfile can not open those paths in standard paths directly

    void openFromPath(const QString& path);
    void open(ComPtr<IStorageFile>& file);
};

typedef WinRTIO MediaIOWinRT;

FACTORY_REGISTER(MediaIO, WinRT, kName)

class Q_DECL_HIDDEN WinRTIOPrivate : public MediaIOPrivate
{
public:

    WinRTIOPrivate()
        : MediaIOPrivate(),
          pos(0)
    {
    }

    ComPtr<IStorageItem> item;
    ComPtr<IStream>      stream;
    qint64               pos;
};

WinRTIO::WinRTIO()
    : MediaIO(*new WinRTIOPrivate())
{
}

WinRTIO::WinRTIO(WinRTIOPrivate& d)
    : MediaIO(d)
{
}

bool WinRTIO::isSeekable() const
{
    DPTR_D(const WinRTIO); // cppcheck-suppress constVariable

    return !!d.stream;
}

bool WinRTIO::isWritable() const
{
    DPTR_D(const WinRTIO); // cppcheck-suppress constVariable

    return !!d.stream;
}

//http://www.codeproject.com/Tips/489450/Creating-Custom-FFmpeg-IO-Context      // krazy:exclude=insecurenet

qint64 WinRTIO::read(char* data, qint64 maxSize)
{
    DPTR_D(WinRTIO);

    if (!d.stream)
        return 0;

    ULONG bytesRead = 0;

    // TODO: -1 0r 0 if error?

    COM_ENSURE(d.stream->Read(data, maxSize, &bytesRead), -1);
    d.pos += bytesRead;

    return bytesRead;
}

qint64 WinRTIO::write(const char* data, qint64 maxSize)
{
    DPTR_D(WinRTIO);

    if (!d.stream)
        return 0;

    ULONG bytesWritten;

    // TODO: -1 0r 0 if error?

    COM_ENSURE(d.stream->Write(data, maxSize, &bytesWritten), -1);
    d.pos += bytesWritten;

    return bytesWritten;
}

bool WinRTIO::seek(qint64 offset, int from)
{
    DPTR_D(WinRTIO);

    if (!d.stream)
        return false;

    LARGE_INTEGER in;
    in.QuadPart        = offset;
    ULARGE_INTEGER out = { 0 };

    // dwOrigin: has the same value as ffmpeg. STREAM_SEEK_SET, STREAM_SEEK_CUR, STREAM_SEEK_END

    COM_ENSURE(d.stream->Seek(in, from, &out), false);
    d.pos = out.QuadPart;

    return true;
}

qint64 WinRTIO::position() const
{
    DPTR_D(const WinRTIO); // cppcheck-suppress constVariable

    return d.pos;
}

qint64 WinRTIO::size() const
{
    DPTR_D(const WinRTIO);

    if (!d.stream)
        return 0;

    STATSTG st;
    COM_ENSURE(d.stream->Stat(&st, STATFLAG_DEFAULT), 0);

    return st.cbSize.QuadPart;
}

void WinRTIO::onUrlChanged()
{
    d_func().pos = 0;
    qCDebug(DIGIKAM_QTAV_LOG) << "onUrlChanged: " << url();

    // winrt:@ptr_address:path
    // winrt:path

    if      (url().startsWith(name().append(QLatin1String(":@")), Qt::CaseInsensitive))
    {
        const int addr_begin = name().size() + 2;
        const int addr_end   = url().indexOf(QLatin1Char(':'), addr_begin);
        QString addr(url().mid(addr_begin, addr_end - addr_begin));
        openFromStorage((IStorageItem*)(qptrdiff)addr.toULongLong());
    }
    else if (url().startsWith(name().append(QLatin1String(":")), Qt::CaseInsensitive))
    {
        openFromPath(url().mid(name().size() + 1));
    }
    else
    {
        return;
    }
}

void WinRTIO::openFromStorage(IStorageItem *item)
{
    DPTR_D(WinRTIO);

    d.item                 = item;

    HString path;
    COM_ENSURE(item->get_Path(path.GetAddressOf()));
    quint32 pathLen;
    const wchar_t* pathStr = path.GetRawBuffer(&pathLen);
    const QString filePath = QString::fromWCharArray(pathStr, pathLen);

    qCDebug(DIGIKAM_QTAV_LOG) << "winrt.io from storage file: " << filePath;

    ComPtr<IStorageFile> file;
    COM_ENSURE(d.item.As(&file));

    open(file);     // krazy:exclude=syscalls
}

void WinRTIO::openFromPath(const QString &path)
{
    ComPtr<IStorageFileStatics> fileFactory;
    COM_ENSURE(RoGetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Storage_StorageFile).Get(),
                                                             IID_PPV_ARGS(&fileFactory)));

    // 2 bytes for utf-16, 4 bytes for ucs-4(unix)

    wchar_t* wp   = new wchar_t[path.size()*4];
    const int len = path.toWCharArray(wp);
    HString p;
    p.Set(wp, len);
    delete [] wp;

    ComPtr<IAsyncOperation<StorageFile*>> op;
    COM_ENSURE(fileFactory->GetFileFromPathAsync(p.Get(), &op));
    ComPtr<IStorageFile> file;
    COM_ENSURE(QWinRTFunctions::await(op, file.GetAddressOf()));

    open(file);      // krazy:exclude=syscalls
}

void WinRTIO::open(ComPtr<IStorageFile> &file)
{
    ComPtr<IAsyncOperation<IRandomAccessStream*>> op;
    COM_ENSURE(file->OpenAsync(accessMode() == Read ? FileAccessMode_Read : FileAccessMode_ReadWrite, &op));
    ComPtr<IRandomAccessStream> stream;
    COM_ENSURE(QWinRTFunctions::await(op, stream.GetAddressOf()));

    // Convert asynchronous IRandomAccessStream to synchronous IStream. This API requires shcore.h and shcore.lib

    COM_ENSURE(CreateStreamOverRandomAccessStream(reinterpret_cast<IUnknown*>(stream.Get()), IID_PPV_ARGS(&d_func().stream)));
}

} // namespace QtAV
