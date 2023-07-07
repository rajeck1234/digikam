/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-24
 * Description : images transition manager.
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TRANSITION_MNGR_H
#define DIGIKAM_TRANSITION_MNGR_H

// Qt includes

#include <QMap>
#include <QString>
#include <QImage>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT TransitionMngr
{
public:

    enum TransType
    {
        None = 0,
        ChessBoard,
        MeltDown,
        Sweep,
        Mosaic,
        Cubism,
        Growing,
        HorizontalLines,
        VerticalLines,
        CircleOut,
        MultiCircleOut,
        SpiralIn,
        Blobs,
        Fade,
        SlideL2R,
        SlideR2L,
        SlideT2B,
        SlideB2T,
        PushL2R,
        PushR2L,
        PushT2B,
        PushB2T,
        SwapL2R,
        SwapR2L,
        SwapT2B,
        SwapB2T,
        BlurIn,
        BlurOut,
        Random
    };

public:

    explicit TransitionMngr();
    ~TransitionMngr();

    void setOutputSize(const QSize& size);
    void setTransition(TransType type);
    void setInImage(const QImage& iimg);
    void setOutImage(const QImage& oimg);

    QImage currentFrame(int& tmout);

    static QMap<TransType, QString> transitionNames();

private:

    // Disable
    TransitionMngr(const TransitionMngr&)            = delete;
    TransitionMngr& operator=(const TransitionMngr&) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_TRANSITION_MNGR_H
