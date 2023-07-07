/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Internal private container.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metaengine_p.h"

// C ANSI includes

extern "C"
{
#include <sys/stat.h>

#ifndef Q_CC_MSVC
#   include <utime.h>
#else
#   include <sys/utime.h>
#endif
}

// Qt includes

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#   include <QTextCodec>
#endif

#include <QFile>
#include <qplatformdefs.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "metaengine_data_p.h"
#include "drawfiles.h"
#include "exiftoolparser.h"
#include "filereadwritelock.h"

// Pragma directives to reduce warnings from Exiv2.

#if defined(Q_CC_GNU) && !defined(Q_CC_CLANG)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace Digikam
{

/**
 * This mutex is used to protect all Exiv2 API calls when MetaEngine is used with multi-threads.
 */
QRecursiveMutex s_metaEngineMutex;

/**
 * Boolean value about Bmff based file support (CR3, HEIF, HEIC, and AVIF).
 * Initialized at run time by initializeExiv2().
 */
bool s_metaEngineSupportBmff = false;

/**
 * Boolean value about Exiv2 warning or error in the Exiv2::LogMsg.
 * Changed in MetaEngine::Private::printExiv2MessageHandler.
 */
bool s_metaEngineWarnOrError = false;

MetaEngine::Private::Private(MetaEngine* const q)
    : writeWithExifTool     (false),
      writeRawFiles         (false),
      writeDngFiles         (false),
      updateFileTimeStamp   (false),
      useXMPSidecar4Reading (false),
      useCompatibleFileName (false),
      metadataWritingMode   (WRITE_TO_FILE_ONLY),
      loadedFromSidecar     (false),
      parent                (q),
      data                  (new MetaEngineData::Private)
{
    QMutexLocker lock(&s_metaEngineMutex);
    Exiv2::LogMsg::setHandler(MetaEngine::Private::printExiv2MessageHandler);
}

MetaEngine::Private::~Private()
{
}

const Exiv2::ExifData& MetaEngine::Private::exifMetadata() const
{
    return data.constData()->exifMetadata;
}

const Exiv2::IptcData& MetaEngine::Private::iptcMetadata() const
{
    return data.constData()->iptcMetadata;
}

const std::string& MetaEngine::Private::itemComments() const
{
    return data.constData()->imageComments;
}

Exiv2::ExifData& MetaEngine::Private::exifMetadata()
{
    return data.data()->exifMetadata;
}

Exiv2::IptcData& MetaEngine::Private::iptcMetadata()
{
    return data.data()->iptcMetadata;
}

std::string& MetaEngine::Private::itemComments()
{
    return data.data()->imageComments;
}

#ifdef _XMP_SUPPORT_

const Exiv2::XmpData& MetaEngine::Private::xmpMetadata() const
{
    return data.constData()->xmpMetadata;
}

Exiv2::XmpData& MetaEngine::Private::xmpMetadata()
{
    return data.data()->xmpMetadata;
}

#endif

void MetaEngine::Private::copyPrivateData(const Private* const other)
{
    QMutexLocker lock(&s_metaEngineMutex);

    data                  = other->data;
    filePath              = other->filePath;
    writeRawFiles         = other->writeRawFiles;
    writeDngFiles         = other->writeDngFiles;
    updateFileTimeStamp   = other->updateFileTimeStamp;
    useXMPSidecar4Reading = other->useXMPSidecar4Reading;
    useCompatibleFileName = other->useCompatibleFileName;
    metadataWritingMode   = other->metadataWritingMode;
}

bool MetaEngine::Private::saveToXMPSidecar(const QFileInfo& finfo) const
{
    QString xmpFile = MetaEngine::sidecarFilePathForFile(finfo.filePath());

    if (xmpFile.isEmpty())
    {
        return false;
    }

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::Image::AutoPtr image;

#if defined Q_OS_WIN && defined EXV_UNICODE_PATH

        image = Exiv2::ImageFactory::create(Exiv2::ImageType::xmp,
                                            (const wchar_t*)xmpFile.utf16());

#elif defined Q_OS_WIN

        image = Exiv2::ImageFactory::create(Exiv2::ImageType::xmp,
                                            QFile::encodeName(xmpFile).constData());

#else

        image = Exiv2::ImageFactory::create(Exiv2::ImageType::xmp,
                                            xmpFile.toUtf8().constData());

#endif

#if EXIV2_TEST_VERSION(0,27,99)

        return saveUsingExiv2(finfo, std::move(image));

#else

        return saveUsingExiv2(finfo, image);

#endif

    }
    catch (Exiv2::AnyError& e)
    {
        printExiv2ExceptionError(QLatin1String("Cannot save metadata to XMP sidecar with Exiv2 backend:"), e);

        return false;
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";

        return false;
    }
}

bool MetaEngine::Private::saveToFile(const QFileInfo& finfo) const
{
    if (!finfo.isWritable())
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "File" << finfo.fileName()
                                        << "is read only. Metadata not written.";
        return false;
    }

    QMutexLocker lock(&s_metaEngineMutex);

    if (writeWithExifTool)
    {
        return saveUsingExifTool(finfo);
    }
    else
    {
        try
        {
            Exiv2::Image::AutoPtr image;

#if defined Q_OS_WIN && defined EXV_UNICODE_PATH

            image = Exiv2::ImageFactory::open((const wchar_t*)finfo.filePath().utf16());

#elif defined Q_OS_WIN

            image = Exiv2::ImageFactory::open(QFile::encodeName(finfo.filePath()).constData());

#else

            image = Exiv2::ImageFactory::open(finfo.filePath().toUtf8().constData());

#endif

#if EXIV2_TEST_VERSION(0,27,99)

            return saveUsingExiv2(finfo, std::move(image));

#else

            return saveUsingExiv2(finfo, image);

#endif

        }
        catch (Exiv2::AnyError& e)
        {
            printExiv2ExceptionError(QLatin1String("Cannot save metadata to image with Exiv2 backend:"), e);

            return false;
        }
        catch (...)
        {
            qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";

            return false;
        }
    }
}

bool MetaEngine::Private::saveUsingExiv2(const QFileInfo& finfo, Exiv2::Image::AutoPtr image) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    QString ext = finfo.suffix().toLower();

    if (!ext.isEmpty())
    {
        if (s_rawFileExtensions().contains(ext) && (image->imageType() != Exiv2::ImageType::xmp))
        {
            // NOTE: never touch RAW files with Exiv2 as it's not safe. Use ExifTool backend instead.

            return false;
        }
    }

    try
    {
        Exiv2::AccessMode mode;
        bool wroteComment = false;
        bool wroteEXIF    = false;
        bool wroteIPTC    = false;
        bool wroteXMP     = false;

        // We need to load target file metadata to merge with new one. It's mandatory with TIFF format:
        // like all tiff file structure is based on Exif.

        image->readMetadata();

        // Image Comments ---------------------------------

        mode = image->checkMode(Exiv2::mdComment);

        if ((mode == Exiv2::amWrite) || (mode == Exiv2::amReadWrite))
        {
            image->setComment(itemComments());
            wroteComment = true;
        }

        qCDebug(DIGIKAM_METAENGINE_LOG) << "wroteComment: " << wroteComment;

        // Exif metadata ----------------------------------

        mode = image->checkMode(Exiv2::mdExif);

        if ((mode == Exiv2::amWrite) || (mode == Exiv2::amReadWrite))
        {
            if (image->mimeType() == "image/tiff")
            {
                Exiv2::ExifData orgExif = image->exifData();
                Exiv2::ExifData newExif;
                QStringList     untouchedTags;

                // With tiff image we cannot overwrite whole Exif data as well, because
                // image data are stored in Exif container. We need to take a care about
                // to not lost image data.

                untouchedTags << QLatin1String("Exif.Image.ImageWidth");
                untouchedTags << QLatin1String("Exif.Image.ImageLength");
                untouchedTags << QLatin1String("Exif.Image.BitsPerSample");
                untouchedTags << QLatin1String("Exif.Image.Compression");
                untouchedTags << QLatin1String("Exif.Image.PhotometricInterpretation");
                untouchedTags << QLatin1String("Exif.Image.FillOrder");
                untouchedTags << QLatin1String("Exif.Image.SamplesPerPixel");
                untouchedTags << QLatin1String("Exif.Image.StripOffsets");
                untouchedTags << QLatin1String("Exif.Image.RowsPerStrip");
                untouchedTags << QLatin1String("Exif.Image.StripByteCounts");
                untouchedTags << QLatin1String("Exif.Image.XResolution");
                untouchedTags << QLatin1String("Exif.Image.YResolution");
                untouchedTags << QLatin1String("Exif.Image.PlanarConfiguration");
                untouchedTags << QLatin1String("Exif.Image.ResolutionUnit");

                for (Exiv2::ExifData::const_iterator it = orgExif.begin() ; it != orgExif.end() ; ++it)
                {
                    if (untouchedTags.contains(QLatin1String(it->key().c_str())))
                    {
                        newExif[it->key().c_str()] = orgExif[it->key().c_str()];
                    }
                }

                Exiv2::ExifData readedExif = exifMetadata();

                for (Exiv2::ExifData::const_iterator it = readedExif.begin() ; it != readedExif.end() ; ++it)
                {
                    if (!untouchedTags.contains(QLatin1String(it->key().c_str())))
                    {
                        newExif[it->key().c_str()] = readedExif[it->key().c_str()];
                    }
                }

                image->setExifData(newExif);
            }
            else
            {
                image->setExifData(exifMetadata());
            }

            wroteEXIF = true;
        }

        qCDebug(DIGIKAM_METAENGINE_LOG) << "wroteEXIF: " << wroteEXIF;

        // Iptc metadata ----------------------------------

        mode = image->checkMode(Exiv2::mdIptc);

        if ((mode == Exiv2::amWrite) || (mode == Exiv2::amReadWrite))
        {
            image->setIptcData(iptcMetadata());
            wroteIPTC = true;
        }

        qCDebug(DIGIKAM_METAENGINE_LOG) << "wroteIPTC: " << wroteIPTC;

        // Xmp metadata -----------------------------------

        mode = image->checkMode(Exiv2::mdXmp);

        if ((mode == Exiv2::amWrite) || (mode == Exiv2::amReadWrite))
        {

#ifdef _XMP_SUPPORT_

            image->setXmpData(xmpMetadata());

            if (image->imageType() == Exiv2::ImageType::xmp)
            {
                QList<QPair<QString, QString> > pairedTags;
                typedef QPair<QString, QString> StringPair;

                // We have XMP tags interacting with IPTC tags when writing to sidecar files.
                // If an XMP tag does not exist remove the corresponding IPTC tag as well.
                // See: https://exiv2.org/conversion.html
                // See: BUG 462071

                pairedTags << qMakePair(QLatin1String("Xmp.dc.subject"),
                                        QLatin1String("Iptc.Application2.Keywords"));

                Q_FOREACH (const StringPair& pair, pairedTags)
                {
                    Exiv2::XmpKey xmpKey(pair.first.toLatin1().constData());
                    Exiv2::XmpData::const_iterator it1 = image->xmpData().findKey(xmpKey);

                    if (it1 == image->xmpData().end())
                    {
                        Exiv2::IptcKey iptcKey(pair.second.toLatin1().constData());
                        Exiv2::IptcData::iterator it2 = image->iptcData().findKey(iptcKey);

                        if (it2 != image->iptcData().end())
                        {
                            image->iptcData().erase(it2);
                        }
                    }
                }
            }

            wroteXMP = true;

#endif

        }

        qCDebug(DIGIKAM_METAENGINE_LOG) << "wroteXMP: " << wroteXMP;

        // cppcheck-suppress knownConditionTrueFalse
        if      (!wroteComment && !wroteEXIF && !wroteIPTC && !wroteXMP)
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Writing metadata is not supported for file" << finfo.fileName();

            return false;
        }
        else if (!wroteEXIF || !wroteIPTC || !wroteXMP)
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "Support for writing metadata is limited for file" << finfo.fileName();
        }

#ifdef _XMP_SUPPORT_

        if (!updateFileTimeStamp && (image->imageType() != Exiv2::ImageType::xmp))
#else

        if (!updateFileTimeStamp)

#endif

        {
            // Don't touch access and modification timestamp of file.

#ifdef Q_OS_WIN64

            struct __utimbuf64 ut;
            struct __stat64    st;
            int ret = _wstat64((const wchar_t*)finfo.filePath().utf16(), &st);

#elif defined Q_OS_WIN

            struct _utimbuf    ut;
            struct _stat       st;
            int ret = _wstat((const wchar_t*)finfo.filePath().utf16(), &st);

#else

            struct utimbuf     ut;
            QT_STATBUF         st;
            int ret = QT_STAT(finfo.filePath().toUtf8().constData(), &st);

#endif

            if (ret == 0)
            {
                ut.modtime = st.st_mtime;
                ut.actime  = st.st_atime;
            }

            image->writeMetadata();

            if (ret == 0)
            {

#ifdef Q_OS_WIN64

                _wutime64((const wchar_t*)finfo.filePath().utf16(), &ut);

#elif defined Q_OS_WIN

                _wutime((const wchar_t*)finfo.filePath().utf16(), &ut);

#else

                ::utime(finfo.filePath().toUtf8().constData(), &ut);

#endif

            }

            qCDebug(DIGIKAM_METAENGINE_LOG) << "File time stamp restored";
        }
        else
        {
            image->writeMetadata();
        }

        return true;
    }
    catch (Exiv2::AnyError& e)
    {
        printExiv2ExceptionError(QLatin1String("Cannot save metadata with Exiv2 backend:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return false;
}

bool MetaEngine::Private::saveUsingExifTool(const QFileInfo& finfo) const
{
    QString ext = finfo.suffix().toLower();

    if (!writeDngFiles && (ext == QLatin1String("dng")))
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << finfo.fileName()
                                        << "is a DNG file, "
                                        << "writing to such a file is disabled by current settings.";
        return false;
    }

    if (!writeRawFiles && s_rawFileExtensions().remove(QLatin1String("dng")).contains(ext))
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << finfo.fileName()
                                        << "is a RAW file, "
                                        << "writing to such a file is disabled by current settings.";
        return false;
    }

    QScopedPointer<ExifToolParser> const parser(new ExifToolParser(nullptr));

    if (!parser->exifToolAvailable())
    {
        qCWarning(DIGIKAM_METAENGINE_LOG) << "ExifTool is not available to save metadata...";

        return false;
    }

    QString dir                   = QDir::temp().path();
    SafeTemporaryFile* const temp = new SafeTemporaryFile(dir + QLatin1String("/MetaEngine-XXXXXX.exv"));
    temp->setAutoRemove(false);
    temp->open();
    QString exvPath = temp->safeFilePath();

    // Crash fix: a QTemporaryFile is not properly closed until its destructor is called.

    delete temp;

    // ---

    parent->exportChanges(exvPath);

    if (!updateFileTimeStamp)
    {
        // Don't touch access and modification timestamp of file.

#ifdef Q_OS_WIN64

        struct __utimbuf64 ut;
        struct __stat64    st;
        int ret = _wstat64((const wchar_t*)finfo.filePath().utf16(), &st);

#elif defined Q_OS_WIN

        struct _utimbuf    ut;
        struct _stat       st;
        int ret = _wstat((const wchar_t*)finfo.filePath().utf16(), &st);

#else

        struct utimbuf     ut;
        QT_STATBUF         st;
        int ret = QT_STAT(finfo.filePath().toUtf8().constData(), &st);

#endif

        if (ret == 0)
        {
            ut.modtime = st.st_mtime;
            ut.actime  = st.st_atime;
        }

        bool hasIptcCSet = (parent->getIptcTagData("Iptc.Envelope.CharacterSet") == "\33%G");

        if (!parser->applyChanges(finfo.filePath(), exvPath, parent->hasExif(), hasIptcCSet))
        {
            qCWarning(DIGIKAM_METAENGINE_LOG) << "Cannot apply changes with ExifTool on" << finfo.filePath();
            QFile::remove(exvPath);

            return false;
        }

        if (ret == 0)
        {

#ifdef Q_OS_WIN64

            _wutime64((const wchar_t*)finfo.filePath().utf16(), &ut);

#elif defined Q_OS_WIN

            _wutime((const wchar_t*)finfo.filePath().utf16(), &ut);

#else

            ::utime(finfo.filePath().toUtf8().constData(), &ut);

#endif

        }

        qCDebug(DIGIKAM_METAENGINE_LOG) << "File time stamp restored";
    }
    else
    {
        bool hasIptcCSet = (parent->getIptcTagData("Iptc.Envelope.CharacterSet") == "\33%G");

        if (!parser->applyChanges(finfo.filePath(), exvPath, parent->hasExif(), hasIptcCSet))
        {
            qCWarning(DIGIKAM_METAENGINE_LOG) << "Cannot apply changes with ExifTool on" << finfo.filePath();
            QFile::remove(exvPath);

            return false;
        }
    }

    QFile::remove(exvPath);

    return true;
}

void MetaEngine::Private::printExiv2ExceptionError(const QString& msg, Exiv2::AnyError& e)
{
    qCCritical(DIGIKAM_METAENGINE_LOG) << msg.toLatin1().constData()
                                       << " (Error #" << (int)e.code() << ": " << QString::fromStdString(e.what());
}

void MetaEngine::Private::printExiv2MessageHandler(int lvl, const char* msg)
{
    qCDebug(DIGIKAM_METAENGINE_LOG) << "Exiv2 (" << lvl << ") : " << msg;

    s_metaEngineWarnOrError |= ((lvl == Exiv2::LogMsg::warn) || (lvl == Exiv2::LogMsg::error));
}

QString MetaEngine::Private::convertCommentValue(const Exiv2::Exifdatum& exifDatum) const
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        std::string comment;
        std::string charset;

        comment = exifDatum.toString();

        // libexiv2 will prepend "charset=\"SomeCharset\" " if charset is specified
        // Before conversion to QString, we must know the charset, so we stay with std::string for a while

        if ((comment.length() > 8) && (comment.substr(0, 8) == "charset="))
        {
            // the prepended charset specification is followed by a blank

            std::string::size_type pos = comment.find_first_of(' ');

            if (pos != std::string::npos)
            {
                // extract string between the = and the blank

                charset = comment.substr(8, pos-8);

                // get the rest of the string after the charset specification

                comment = comment.substr(pos+1);
            }
        }

        if      (charset == "\"Unicode\"")
        {
            return QString::fromUtf8(comment.data());
        }
        else if (charset == "\"Jis\"")
        {

#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))

            if (Exiv2::convertStringCharset(comment, charset.c_str(), "UTF-8"))
            {
                return QString::fromUtf8(comment);
            }
            else
            {
                return QLatin1String("");
            }

#else

            QTextCodec* const codec = QTextCodec::codecForName("JIS7");

            if (codec)
            {
                const char* tmp = comment.c_str();

                if (tmp)
                {
                    return codec->toUnicode(tmp);
                }
            }

            return QLatin1String("");

#endif

        }
        else if (charset == "\"Ascii\"")
        {
            return QString::fromStdString(comment);
        }
        else
        {
            return detectEncodingAndDecode(comment);
        }
    }
    catch (Exiv2::AnyError& e)
    {
        printExiv2ExceptionError(QLatin1String("Cannot convert Comment with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return QString();
}

QString MetaEngine::Private::detectEncodingAndDecode(const std::string& value) const
{
    // For charset autodetection, we could use sophisticated code
    // (Mozilla chardet, KHTML's autodetection, QTextCodec::codecForContent),
    // but that is probably too much.
    // We check for UTF8, Local encoding and ASCII.
    // Look like KEncodingDetector class can provide a full implementation for encoding detection.

    if (value.empty())
    {
        return QString();
    }

    if (isUtf8(value.c_str()))
    {
        return QString::fromStdString(value);
    }

    // Utf8 has a pretty unique byte pattern.
    // That's not true for ASCII, it is not possible
    // to reliably autodetect different ISO-8859 charsets.
    // So we can use either local encoding, or latin1.

    QString localString = QString::fromLocal8Bit(value.c_str());

    // To fix broken JFIF comment, replace non printable
    // characters from the string. See bug 39617

    for (int i = 0 ; i < localString.size() ; ++i)
    {
        if (!localString.at(i).isPrint()             &&
            (localString.at(i) != QLatin1Char('\n')) &&
            (localString.at(i) != QLatin1Char('\r')))
        {
            localString[i] = QLatin1Char('_');
        }
    }

    return localString;
}

bool MetaEngine::Private::isUtf8(const char* const buffer) const
{
    int i, n;
    unsigned char c;
    bool gotone = false;

    if (!buffer)
    {
        return true;
    }

// character never appears in text

#define F 0

// character appears in plain ASCII text

#define T 1

// character appears in ISO-8859 text

#define I 2

// character appears in non-ISO extended ASCII (Mac, IBM PC)

#define X 3

    static const unsigned char text_chars[256] =
    {
        //                  BEL BS HT LF    FF CR
        F, F, F, F, F, F, F, T, T, T, T, F, T, T, F, F,  // 0x0X
        //                              ESC
        F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F,  // 0x1X
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  // 0x2X
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  // 0x3X
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  // 0x4X
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  // 0x5X
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  // 0x6X
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, F,  // 0x7X
        //            NEL
        X, X, X, X, X, T, X, X, X, X, X, X, X, X, X, X,  // 0x8X
        X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,  // 0x9X
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  // 0xaX
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  // 0xbX
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  // 0xcX
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  // 0xdX
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  // 0xeX
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I   // 0xfX
    };

    for (i = 0 ; (c = buffer[i]) ; ++i)
    {
        if      ((c & 0x80) == 0)
        {
            // 0xxxxxxx is plain ASCII

            // Even if the whole file is valid UTF-8 sequences,
            // still reject it if it uses weird control characters.

            if (text_chars[c] != T)
            {
                return false;
            }
        }
        else if ((c & 0x40) == 0)
        {
            // 10xxxxxx never 1st byte

            return false;
        }
        else
        {
            // 11xxxxxx begins UTF-8

            int following = 0;

            if      ((c & 0x20) == 0)
            {
                // 110xxxxx

                following = 1;
            }
            else if ((c & 0x10) == 0)
            {
                // 1110xxxx

                following = 2;
            }
            else if ((c & 0x08) == 0)
            {
                // 11110xxx

                following = 3;
            }
            else if ((c & 0x04) == 0)
            {
                // 111110xx

                following = 4;
            }
            else if ((c & 0x02) == 0)
            {
                // 1111110x

                following = 5;
            }
            else
            {
                return false;
            }

            for (n = 0 ; n < following ; ++n)
            {
                i++;

                if (!(c = buffer[i]))
                {
                    goto done;
                }

                if (((c & 0x80) == 0) || (c & 0x40))
                {
                    return false;
                }
            }

            gotone = true;
        }
    }

done:

    return gotone;   // don't claim it's UTF-8 if it's all 7-bit.
}

#undef F
#undef T
#undef I
#undef X

int MetaEngine::Private::getXMPTagsListFromPrefix(const QString& pf, MetaEngine::TagsMap& tagsMap) const
{
    int i = 0;

#ifdef _XMP_SUPPORT_

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        QList<const Exiv2::XmpPropertyInfo*> tags;
        tags << Exiv2::XmpProperties::propertyList(pf.toLatin1().data());

        for (QList<const Exiv2::XmpPropertyInfo*>::iterator it = tags.begin() ; it != tags.end() ; ++it)
        {
            while ((*it) && !QString::fromLatin1((*it)->name_).isNull())
            {
                QString     key = QLatin1String(Exiv2::XmpKey(pf.toLatin1().data(), (*it)->name_).key().c_str());
                QStringList values;
                values << QLatin1String((*it)->name_)
                       << QLatin1String((*it)->title_)
                       << QLatin1String((*it)->desc_);
                tagsMap.insert(key, values);
                ++(*it);
                i++;
            }
        }
    }
    catch (Exiv2::AnyError& e)
    {
        printExiv2ExceptionError(QLatin1String("Cannot get Xmp tags list using Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

#else

    Q_UNUSED(pf);
    Q_UNUSED(tagsMap);

#endif // _XMP_SUPPORT_

    return i;
}

#ifdef _XMP_SUPPORT_

void MetaEngine::Private::loadSidecarData(Exiv2::Image::AutoPtr xmpsidecar)
{
    // Having a sidecar is a special situation.
    // The sidecar data often "dominates", see in particular bug 309058 for important aspects:
    // If a field is removed from the sidecar, we must ignore (older) data for this field in the file.

    // First: Ignore file XMP, only use sidecar XMP

    xmpMetadata()     = xmpsidecar->xmpData();
    loadedFromSidecar = true;

    // EXIF
    // Four groups of properties are mapped between EXIF and XMP:
    // Date/Time, Description, Copyright, Creator
    // A few more tags are defined "writeback" tags in the XMP specification, the sidecar value therefore overrides the Exif value.
    // The rest is kept side-by-side.
    // (to understand, remember that the xmpsidecar's Exif data is actually XMP data mapped back to Exif)

    // Description, Copyright and Creator is dominated by the sidecar: Remove file Exif fields, if field not in XMP.

    ExifMetaEngineMergeHelper exifDominatedHelper;
    exifDominatedHelper << QLatin1String("Exif.Image.ImageDescription")
                        << QLatin1String("Exif.Photo.UserComment")
                        << QLatin1String("Exif.Image.Copyright")
                        << QLatin1String("Exif.Image.Artist");
    exifDominatedHelper.exclusiveMerge(xmpsidecar->exifData(), exifMetadata());

    // Date/Time and "the few more" from the XMP spec are handled as writeback
    // Note that Date/Time mapping is slightly contradictory in latest specs.

    ExifMetaEngineMergeHelper exifWritebackHelper;
    exifWritebackHelper << QLatin1String("Exif.Image.DateTime")
                        << QLatin1String("Exif.Photo.DateTimeOriginal")
                        << QLatin1String("Exif.Photo.DateTimeDigitized")
                        << QLatin1String("Exif.Image.Orientation")
                        << QLatin1String("Exif.Image.XResolution")
                        << QLatin1String("Exif.Image.YResolution")
                        << QLatin1String("Exif.Image.ResolutionUnit")
                        << QLatin1String("Exif.Image.Software")
                        << QLatin1String("Exif.Photo.RelatedSoundFile");
    exifWritebackHelper.mergeFields(xmpsidecar->exifData(), exifMetadata());

    // IPTC
    // These fields cover almost all relevant IPTC data and are defined in the XMP specification for reconciliation.

    IptcMetaEngineMergeHelper iptcDominatedHelper;
    iptcDominatedHelper << QLatin1String("Iptc.Application2.ObjectName")
                        << QLatin1String("Iptc.Application2.Urgency")
                        << QLatin1String("Iptc.Application2.Category")
                        << QLatin1String("Iptc.Application2.SuppCategory")
                        << QLatin1String("Iptc.Application2.Keywords")
                        << QLatin1String("Iptc.Application2.SubLocation")
                        << QLatin1String("Iptc.Application2.SpecialInstructions")
                        << QLatin1String("Iptc.Application2.Byline")
                        << QLatin1String("Iptc.Application2.BylineTitle")
                        << QLatin1String("Iptc.Application2.City")
                        << QLatin1String("Iptc.Application2.ProvinceState")
                        << QLatin1String("Iptc.Application2.CountryCode")
                        << QLatin1String("Iptc.Application2.CountryName")
                        << QLatin1String("Iptc.Application2.TransmissionReference")
                        << QLatin1String("Iptc.Application2.Headline")
                        << QLatin1String("Iptc.Application2.Credit")
                        << QLatin1String("Iptc.Application2.Source")
                        << QLatin1String("Iptc.Application2.Copyright")
                        << QLatin1String("Iptc.Application2.Caption")
                        << QLatin1String("Iptc.Application2.Writer");
    iptcDominatedHelper.exclusiveMerge(xmpsidecar->iptcData(), iptcMetadata());

    IptcMetaEngineMergeHelper iptcWritebackHelper;
    iptcWritebackHelper << QLatin1String("Iptc.Application2.DateCreated")
                        << QLatin1String("Iptc.Application2.TimeCreated")
                        << QLatin1String("Iptc.Application2.DigitizationDate")
                        << QLatin1String("Iptc.Application2.DigitizationTime");
    iptcWritebackHelper.mergeFields(xmpsidecar->iptcData(), iptcMetadata());

    /*
     * TODO: Exiv2 (referring to 0.23) does not correctly synchronize all times values as given below.
     * Time values and their synchronization:
     * Original Date/Time – Creation date of the intellectual content (e.g. the photograph),
     * rather than the creatio*n date of the content being shown
     *  Exif DateTimeOriginal (36867, 0x9003) and SubSecTimeOriginal (37521, 0x9291)
     *  IPTC DateCreated (IIM 2:55, 0x0237) and TimeCreated (IIM 2:60, 0x023C)
     *  XMP (photoshop:DateCreated)
     * Digitized Date/Time – Creation date of the digital representation
     *  Exif DateTimeDigitized (36868, 0x9004) and SubSecTimeDigitized (37522, 0x9292)
     *  IPTC DigitalCreationDate (IIM 2:62, 0x023E) and DigitalCreationTime (IIM 2:63, 0x023F)
     *  XMP (xmp:CreateDate)
     * Modification Date/Time – Modification date of the digital image file
     *  Exif DateTime (306, 0x132) and SubSecTime (37520, 0x9290)
     *  XMP (xmp:ModifyDate)
     */
}

#endif // _XMP_SUPPORT_

QString MetaEngine::Private::extractIptcTagString(const Exiv2::IptcData& iptcData, const Exiv2::Iptcdatum& iptcTag) const
{
    QString charSet = QLatin1String(iptcData.detectCharset());
    QString value;

    // NOTE: no need mutex lock here as this method is always called from a top level function by a parent mutex lock

    try
    {
        if ((iptcTag.typeId() == Exiv2::string) && !charSet.isNull())
        {
            // Perform Utf8 conversion from std::string
            // TODO: check if a parse of charset content can improve the string conversion if not Utf8 use.

            value = QString::fromStdString(iptcTag.toString());
        }
        else
        {
            // No characterset want mean ASCII-latin1
            // Decode the tag value with a user friendly output.

            std::ostringstream os;
            os << iptcTag;
            value = QString::fromStdString(os.str());
        }
    }
    catch (Exiv2::AnyError& e)
    {
        printExiv2ExceptionError(QLatin1String("Cannot decode Iptc tag string with right encoding using Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }

    return value;
}

} // namespace Digikam

// Restore warnings
#if defined(Q_CC_GNU) && !defined(Q_CC_CLANG)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif
