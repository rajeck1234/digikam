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

#include "PacketBuffer.h"

// Qt includes

#include <QDateTime>

// Local includes

#include "digikam_debug.h"

namespace QtAV
{

static const int kAvgSize = 16;

PacketBuffer::PacketBuffer()
    : m_mode     (BufferTime),
      m_buffering(true),            // in buffering state at the beginning
      m_max      (1.5),
      m_buffer   (0),
      m_value0   (0),
      m_value1   (0),
      m_history  (kAvgSize)
{
}

PacketBuffer::~PacketBuffer()
{
}

void PacketBuffer::setBufferMode(BufferMode mode)
{
    m_mode = mode;

    if (queue.isEmpty())
    {
        m_value0 = m_value1 = 0;

        return;
    }

    if (m_mode == BufferTime)
    {
        m_value0 = qint64(queue[0].pts * 1000.0);
    }
    else
    {
        m_value0 = 0;
    }
}

BufferMode PacketBuffer::bufferMode() const
{
    return m_mode;
}

void PacketBuffer::setBufferValue(qint64 value)
{
    m_buffer = value;
}

qint64 PacketBuffer::bufferValue() const
{
    return m_buffer;
}

void PacketBuffer::setBufferMax(qreal max)
{
    if (max < 1.0)
    {
        qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
            << QString::asprintf("max (%f) must >= 1.0", max);

        return;
    }

    m_max = max;
}

qreal PacketBuffer::bufferMax() const
{
    return m_max;
}

qint64 PacketBuffer::buffered() const
{
    Q_ASSERT(m_value1 >= m_value0);

    return (m_value1 - m_value0);
}

bool PacketBuffer::isBuffering() const
{
    return m_buffering;
}

qreal PacketBuffer::bufferProgress() const
{
    const qreal p = qreal(buffered()) / qreal(bufferValue());

    return qMax<qreal>(qMin<qreal>(p, 1.0), 0.0);
}

qreal PacketBuffer::bufferSpeed() const
{
    return calc_speed(false);
}

qreal PacketBuffer::bufferSpeedInBytes() const
{
    return calc_speed(true);
}

bool PacketBuffer::checkEnough() const
{
    return (buffered() >= bufferValue());
}

bool PacketBuffer::checkFull() const
{
    return (buffered() >= qint64(qreal(bufferValue()) * bufferMax()));
}

void PacketBuffer::onPut(const Packet& p)
{
    if      (m_mode == BufferTime)
    {
        m_value1 = qint64(p.pts        * 1000.0); // FIXME: what if no pts
        m_value0 = qint64(queue[0].pts * 1000.0); // must compute here because it is reset to 0 if take from empty
/*
        if (isBuffering())
        {
            qCDebug(DIGIKAM_QTAV_LOG).noquote()
                << QString::asprintf("+buffering progress: %.1f%%=%.1f/%.1f~%.1fs %d-%d",
                    bufferProgress()*100.0, (qreal)buffered()/1000.0, (qreal)bufferValue()/1000.0, qreal(bufferValue())*bufferMax()/1000.0, m_value1, m_value0);
        }
*/
    }
    else if (m_mode == BufferBytes)
    {
        m_value1 += p.data.size();
    }
    else
    {
        m_value1++;
    }

    if (!m_buffering)
        return;

    if (checkEnough())
    {
        m_buffering = false;
    }

    if (!m_buffering)
    {
        // buffering => buffered

        m_history = ring<BufferInfo>(kAvgSize);

        return;
    }

    BufferInfo bi;
    bi.bytes = p.data.size();

    if (!m_history.empty())
        bi.bytes += m_history.back().bytes;

    bi.v     = m_value1;
    bi.t     = QDateTime::currentMSecsSinceEpoch();
    m_history.push_back(bi);
}

void PacketBuffer::onTake(const Packet& p)
{
    if (checkEmpty())
    {
        m_buffering = true;
    }

    if (queue.isEmpty())
    {
        m_value0 = 0;
        m_value1 = 0;

        return;
    }

    if      (m_mode == BufferTime)
    {
        m_value0 = qint64(queue[0].pts * 1000.0);
/*
        if (isBuffering())
        {
            qCDebug(DIGIKAM_QTAV_LOG).noquote()
                << QString::asprintf("-buffering progress: %.1f=%.1f/%.1fs",
                    bufferProgress(), (qreal)buffered()/1000.0, (qreal)bufferValue()/1000.0);
        }
*/
    }
    else if (m_mode == BufferBytes)
    {
        m_value1 -= p.data.size();
        m_value1  = qMax<qint64>(0LL, m_value1);
    }
    else
    {
        m_value1--;
    }
}

qreal PacketBuffer::calc_speed(bool use_bytes) const
{
    if (m_history.empty())
        return 0;

    const qreal dt = (double)QDateTime::currentMSecsSinceEpoch() / 1000.0 - m_history.front().t / 1000.0;

    // dt should be always > 0 because history stores absolute time

    if (qFuzzyIsNull(dt))
        return 0;

    const qint64 delta = use_bytes ? m_history.back().bytes - m_history.front().bytes
                                   : m_history.back().v     - m_history.front().v;

    if (delta < 0)
    {
        qCWarning(DIGIKAM_QTAV_LOG_WARN) << "PacketBuffer internal error. delta (bytes"
                                         << use_bytes << "):" << delta;

        return 0;
    }

    return ((qreal)delta / dt);
}

} // namespace QtAV
