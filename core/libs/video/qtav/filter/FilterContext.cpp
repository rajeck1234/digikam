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

#include "FilterContext.h"

// Qt includes

#include <QFontMetrics>
#include <QImage>
#include <QPainter>
#include <QTextDocument>

// Local includes

#include "VideoFrame.h"
#include "digikam_debug.h"

namespace QtAV
{

VideoFilterContext* VideoFilterContext::create(Type t)
{
    VideoFilterContext* ctx = nullptr;

    switch (t)
    {
        case QtPainter:
        {
            ctx = new QPainterFilterContext();
            break;
        }

#if QTAV_HAVE(X11)

        case X11:
        {
            ctx = new X11FilterContext();
            break;
        }

#endif

        default:
        {
            break;
        }
    }

    return ctx;
}

void VideoFilterContext::initializeOnFrame(VideoFrame* frame)
{
    Q_UNUSED(frame);
}

VideoFilterContext::VideoFilterContext()
    : painter         (nullptr),
      opacity         (1),
      paint_device    (nullptr),
      video_width     (0),
      video_height    (0),
      own_painter     (false),
      own_paint_device(false)
{
    font.setBold(true);
    font.setPixelSize(26);
    pen.setColor(Qt::white);
    rect = QRect(32, 32, 0, 0); // TODO: why painting will above the visible area if the draw at (0, 0)? ascent
}

VideoFilterContext::~VideoFilterContext()
{
    if (painter)
    {
        // painter is shared, so may be end() multiple times.

        // TODO: use shared ptr
/*
        if (painter->isActive())
            painter->end();
*/
        if (own_painter)
        {
            qCDebug(DIGIKAM_QTAV_LOG).noquote()
                << QString::asprintf("VideoFilterContext %p delete painter %p",
                    this, painter);

            delete painter;
            painter = nullptr;
        }
    }

    if (paint_device)
    {
        qCDebug(DIGIKAM_QTAV_LOG).noquote()
            << QString::asprintf("VideoFilterContext %p delete paint device in %p",
                this, paint_device);

        if (own_paint_device)
            delete paint_device; // delete recursively for widget

        paint_device = nullptr;
    }
}

void VideoFilterContext::drawImage(const QPointF& pos, const QImage& image,
                                   const QRectF& source, Qt::ImageConversionFlags flags)
{
    Q_UNUSED(pos);
    Q_UNUSED(image);
    Q_UNUSED(source);
    Q_UNUSED(flags);
}

void VideoFilterContext::drawImage(const QRectF& target, const QImage& image,
                                   const QRectF &source, Qt::ImageConversionFlags flags)
{
    Q_UNUSED(target);
    Q_UNUSED(image);
    Q_UNUSED(source);
    Q_UNUSED(flags);
}

void VideoFilterContext::drawPlainText(const QPointF& pos, const QString& text)
{
    Q_UNUSED(pos);
    Q_UNUSED(text);
}

void VideoFilterContext::drawPlainText(const QRectF& rect, int flags, const QString& text)
{
    Q_UNUSED(rect);
    Q_UNUSED(flags);
    Q_UNUSED(text);
}

void VideoFilterContext::drawRichText(const QRectF& rect, const QString& text, bool wordWrap)
{
    Q_UNUSED(rect);
    Q_UNUSED(text);
    Q_UNUSED(wordWrap);
}

void VideoFilterContext::shareFrom(VideoFilterContext* vctx)
{
    if (!vctx)
    {
        qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
            << QString::asprintf("shared filter context is null!");

        return;
    }

    painter          = vctx->painter;
    paint_device     = vctx->paint_device;
    own_painter      = false;
    own_paint_device = false;
    video_width      = vctx->video_width;
    video_height     = vctx->video_height;
}

QPainterFilterContext::QPainterFilterContext()
    : VideoFilterContext(),
      doc               (nullptr),
      cvt               (nullptr)
{
}

QPainterFilterContext::~QPainterFilterContext()
{
    if (doc)
    {
        delete doc;
        doc = nullptr;
    }

    if (cvt)
    {
        delete cvt;
        cvt = nullptr;
    }
}

// TODO: use drawPixmap?

void QPainterFilterContext::drawImage(const QPointF& pos, const QImage& image,
                                      const QRectF& source, Qt::ImageConversionFlags flags)
{
    if (!prepare())
        return;

    if (source.isNull())
        painter->drawImage(pos, image, QRectF(0, 0, image.width(), image.height()), flags);
    else
        painter->drawImage(pos, image, source, flags);

    painter->restore();
}

void QPainterFilterContext::drawImage(const QRectF& target, const QImage& image,
                                      const QRectF& source, Qt::ImageConversionFlags flags)
{
    if (!prepare())
        return;

    if (source.isNull())
        painter->drawImage(target, image, QRectF(0, 0, image.width(), image.height()), flags);
    else
        painter->drawImage(target, image, source, flags);

    painter->restore();
}

void QPainterFilterContext::drawPlainText(const QPointF& pos, const QString& text)
{
    if (!prepare())
        return;

    QFontMetrics fm(font);
    painter->drawText(pos + QPoint(0, fm.ascent()), text);
    painter->restore();
}

void QPainterFilterContext::drawPlainText(const QRectF& rect, int flags, const QString& text)
{
    if (!prepare())
        return;

    if (rect.isNull())
        painter->drawText(rect.topLeft(), text);
    else
        painter->drawText(rect, flags, text);

    painter->restore();
}

void QPainterFilterContext::drawRichText(const QRectF& rect, const QString& text, bool wordWrap)
{
    Q_UNUSED(rect);
    Q_UNUSED(text);

    if (!prepare())
        return;

    if (!doc)
        doc = new QTextDocument();

    doc->setHtml(text);
    painter->translate(rect.topLeft()); // drawContent() can not set target rect, so move here

    if (wordWrap)
        doc->setTextWidth(rect.width());

    doc->drawContents(painter);
    painter->restore();
}

bool QPainterFilterContext::isReady() const
{
    return (!!painter && painter->isActive());
}

bool QPainterFilterContext::prepare()
{
    if (!isReady())
        return false;

    painter->save();                // is it necessary?
    painter->setBrush(brush);
    painter->setPen(pen);
    painter->setFont(font);
    painter->setOpacity(opacity);

    if (!clip_path.isEmpty())
    {
        painter->setClipPath(clip_path);
    }

    // transform at last! because clip_path is relative to paint device coord

    painter->setTransform(transform);

    return true;
}

void QPainterFilterContext::initializeOnFrame(VideoFrame* vframe)
{
    if (!vframe)
    {
        if (!painter)
        {
            painter = new QPainter(); // warning: more than 1 painter on 1 device
        }

        if (!paint_device)
        {
            paint_device = painter->device();
        }

        if (!paint_device && !painter->isActive())
        {
            qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
                << QString::asprintf("No paint device and painter is not active. No painting!");

            return;
        }

        if (!painter->isActive())
            painter->begin(paint_device);

        return;
    }

    VideoFormat format = vframe->format();

    if (!format.isValid())
    {
        qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
            << QString::asprintf("Not a valid format");

        return;
    }

    if (format.imageFormat() == QImage::Format_Invalid)
    {
        format.setPixelFormat(VideoFormat::Format_RGB32);

        if (!cvt)
        {
            cvt = new VideoFrameConverter();
        }

        *vframe = cvt->convert(*vframe, format);
    }

    if (paint_device)
    {
        if (painter && painter->isActive())
        {
            painter->end(); // destroy a paint device that is being painted is not allowed!
        }

        delete paint_device;
        paint_device = nullptr;
    }

    Q_ASSERT((video_width > 0) && (video_height > 0));

    // direct draw on frame data, so use VideoFrame::constBits()

    paint_device = new QImage((uchar*)vframe->constBits(0),
                              video_width, video_height,
                              vframe->bytesPerLine(0),
                              format.imageFormat());

    if (!painter)
        painter = new QPainter();

    own_painter      = true;
    own_paint_device = true; // TODO: what about renderer if not a widget?
    painter->begin((QImage*)paint_device);
}

} // namespace QtAV
