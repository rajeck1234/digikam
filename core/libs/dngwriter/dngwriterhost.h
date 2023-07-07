/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-25
 * Description : a tool to convert RAW file to DNG
 *
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DNG_WRITER_HOST_H
#define DIGIKAM_DNG_WRITER_HOST_H

// Local includes

#include "dngwriter_p.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DNGWriterHost : public dng_host
{

public:

    explicit DNGWriterHost(DNGWriter::Private* const priv, dng_memory_allocator* const allocator=nullptr);
    ~DNGWriterHost();

private:

    void SniffForAbort();

private:

    DNGWriter::Private* const m_priv;
};

} // namespace Digikam

#endif // DIGIKAM_DNG_WRITER_HOST_H
