/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Embedded preview loading.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Local includes

#include "metaengine_previews.h"
#include "metaengine_p.h"
#include "digikam_debug.h"
#include "digikam_config.h"

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace Digikam
{

class Q_DECL_HIDDEN MetaEnginePreviews::Private
{
public:

    explicit Private()
        : manager(nullptr)
    {
    }

    ~Private()
    {
        delete manager;
    }

    void load(Exiv2::Image::AutoPtr image_)     // clazy:exclude=function-args-by-ref
    {
        QMutexLocker lock(&s_metaEngineMutex);

        try
        {

#if EXIV2_TEST_VERSION(0,27,99)

            image                              = std::move(image_);

#else

            image                              = image_;

#endif
            image->readMetadata();

            manager                            = new Exiv2::PreviewManager(*image);
            Exiv2::PreviewPropertiesList props = manager->getPreviewProperties();

            // reverse order of list, which is smallest-first

            Exiv2::PreviewPropertiesList::reverse_iterator it;

            for (it = props.rbegin() ; it != props.rend() ; ++it)
            {
                properties << *it;
            }
        }
        catch (Exiv2::AnyError& e)
        {
            MetaEngine::Private::printExiv2ExceptionError(QLatin1String("Cannot load preview data with Exiv2:"), e);
        }
        catch (...)
        {
            qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
        }
    }

public:

    Exiv2::Image::AutoPtr           image;
    Exiv2::PreviewManager*          manager;
    QList<Exiv2::PreviewProperties> properties;
};

MetaEnginePreviews::MetaEnginePreviews(const QString& filePath)
    : d(new Private)
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {

#if defined Q_OS_WIN && defined EXV_UNICODE_PATH

        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const wchar_t*)filePath.utf16());

#elif defined Q_OS_WIN

        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(QFile::encodeName(filePath).constData());

#else

        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filePath.toUtf8().constData());

#endif

#if EXIV2_TEST_VERSION(0,27,99)

        d->load(std::move(image));

#else

        d->load(image);

#endif

    }
    catch (Exiv2::AnyError& e)
    {
        MetaEngine::Private::printExiv2ExceptionError(QLatin1String("Cannot load metadata with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }
}

MetaEnginePreviews::MetaEnginePreviews(const QByteArray& imgData)
    : d(new Private)
{
    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((Exiv2::byte*)imgData.data(), imgData.size());

#if EXIV2_TEST_VERSION(0,27,99)

        d->load(std::move(image));

#else

        d->load(image);

#endif

    }
    catch (Exiv2::AnyError& e)
    {
        MetaEngine::Private::printExiv2ExceptionError(QLatin1String("Cannot load metadata with Exiv2:"), e);
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";
    }
}

MetaEnginePreviews::~MetaEnginePreviews()
{
    delete d;
}

bool MetaEnginePreviews::isEmpty()
{
    return d->properties.isEmpty();
}

QSize MetaEnginePreviews::originalSize() const
{
    if (d->image.get())
    {
        return QSize(d->image->pixelWidth(), d->image->pixelHeight());
    }

    return QSize();
}

QString MetaEnginePreviews::originalMimeType() const
{
    if (d->image.get())
    {
       return QString::fromStdString(d->image->mimeType());
    }

    return QString();
}

int MetaEnginePreviews::count() const
{
    return d->properties.size();
}

int MetaEnginePreviews::size() const
{
    return count();
}

int MetaEnginePreviews::dataSize(int index)
{
    if ((index < 0) || (index >= size()))
    {
        return 0;
    }

    return d->properties[index].size_;
}

int MetaEnginePreviews::width(int index)
{
    if ((index < 0) || (index >= size()))
    {
        return 0;
    }

    return d->properties[index].width_;
}

int MetaEnginePreviews::height(int index)
{
    if ((index < 0) || (index >= size()))
    {
        return 0;
    }

    return d->properties[index].height_;
}

QString MetaEnginePreviews::mimeType(int index)
{
    if ((index < 0) || (index >= size()))
    {
        return QString();
    }

    return QString::fromStdString(d->properties[index].mimeType_);
}

QString MetaEnginePreviews::fileExtension(int index)
{
    if ((index < 0) || (index >= size()))
    {
        return QString();
    }

    return QString::fromStdString(d->properties[index].extension_);
}

QByteArray MetaEnginePreviews::data(int index)
{
    if ((index < 0) || (index >= size()))
    {
        return QByteArray();
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << "index     : " << index;
    qCDebug(DIGIKAM_METAENGINE_LOG) << "properties: " << count();

    QMutexLocker lock(&s_metaEngineMutex);

    try
    {
        Exiv2::PreviewImage image = d->manager->getPreviewImage(d->properties[index]);

        return QByteArray((const char*)image.pData(), image.size());
    }
    catch (Exiv2::AnyError& e)
    {
        MetaEngine::Private::printExiv2ExceptionError(QLatin1String("Cannot load metadata with Exiv2:"), e);

        return QByteArray();
    }
    catch (...)
    {
        qCCritical(DIGIKAM_METAENGINE_LOG) << "Default exception from Exiv2";

        return QByteArray();
    }
}

QImage MetaEnginePreviews::image(int index)
{
    QByteArray previewData = data(index);
    QImage     image;

    if (previewData.isEmpty() || !image.loadFromData(previewData))
    {
        return QImage();
    }

    return image;
}

} // namespace Digikam

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif
