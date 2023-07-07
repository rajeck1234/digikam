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

#ifndef DIGIKAM_TRANSITION_MNGR_PRIVATE_H
#define DIGIKAM_TRANSITION_MNGR_PRIVATE_H

// C++ includes

#include <cmath>

// Qt includes

#include <QPainter>
#include <QPainterPath>
#include <QPolygon>
#include <QRandomGenerator>

// Local includes

#include "transitionmngr.h"
#include "digikam_config.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN TransitionMngr::Private
{
public:

    typedef int (TransitionMngr::Private::*TransMethod)(bool);

public:

    explicit Private()
      : eff_isRunning    (false),
        eff_curTransition(TransitionMngr::None),
        eff_x            (0),
        eff_y            (0),
        eff_w            (0),
        eff_h            (0),
        eff_dx           (0),
        eff_dy           (0),
        eff_ix           (0),
        eff_iy           (0),
        eff_i            (0),
        eff_j            (0),
        eff_subType      (0),
        eff_x0           (0),
        eff_y0           (0),
        eff_x1           (0),
        eff_y1           (0),
        eff_wait         (0),
        eff_fx           (0),
        eff_fy           (0),
        eff_alpha        (0),
        eff_fd           (0),
        eff_intArray     (nullptr),
        eff_pdone        (0),
        eff_pixelMatrix  (nullptr),
        eff_pa           (QPolygon(4)),
        eff_px           (0),
        eff_py           (0),
        eff_psx          (0),
        eff_psy          (0)
    {
        registerTransitions();
        randomGenerator = QRandomGenerator::global();
    }

    ~Private()
    {
        if (eff_intArray)
        {
            delete [] eff_intArray;
        }
    }

    QMap<TransitionMngr::TransType, TransMethod>  eff_transList;

    QImage                                        eff_inImage;
    QImage                                        eff_outImage;
    QImage                                        eff_curFrame;
    QSize                                         eff_outSize;

    bool                                          eff_isRunning;
    TransitionMngr::TransType                     eff_curTransition;

    // values for state of various transitions:
    int                                           eff_x;
    int                                           eff_y;
    int                                           eff_w;
    int                                           eff_h;
    int                                           eff_dx;
    int                                           eff_dy;
    int                                           eff_ix;
    int                                           eff_iy;
    int                                           eff_i;
    int                                           eff_j;
    int                                           eff_subType;
    int                                           eff_x0;
    int                                           eff_y0;
    int                                           eff_x1;
    int                                           eff_y1;
    int                                           eff_wait;
    double                                        eff_fx;
    double                                        eff_fy;
    double                                        eff_alpha;
    double                                        eff_fd;
    int*                                          eff_intArray;
    bool                                          eff_pdone;
    bool**                                        eff_pixelMatrix;

    //static
    QPolygon                                      eff_pa;

    int                                           eff_px;
    int                                           eff_py;
    int                                           eff_psx;
    int                                           eff_psy;

public:

    void registerTransitions();

    TransitionMngr::TransType getRandomTransition() const;

private:

    // Internal functions to render an effect frame.
    // aInit is to true when effect is initialized (first call).
    // The integer value is a tempo in ms to wait between frames,
    // or -1 if the effect is completed.

    int transitionNone(bool aInit);
    int transitionChessboard(bool aInit);
    int transitionMeltdown(bool aInit);
    int transitionSweep(bool aInit);
    int transitionMosaic(bool aInit);
    int transitionCubism(bool aInit);
    int transitionRandom(bool aInit);
    int transitionGrowing(bool aInit);
    int transitionHorizLines(bool aInit);
    int transitionVertLines(bool aInit);
    int transitionMultiCircleOut(bool aInit);
    int transitionSpiralIn(bool aInit);
    int transitionCircleOut(bool aInit);
    int transitionBlobs(bool aInit);
    int transitionFade(bool aInit);
    int transitionSlideL2R(bool aInit);
    int transitionSlideR2L(bool aInit);
    int transitionSlideT2B(bool aInit);
    int transitionSlideB2T(bool aInit);
    int transitionPushL2R(bool aInit);
    int transitionPushR2L(bool aInit);
    int transitionPushT2B(bool aInit);
    int transitionPushB2T(bool aInit);
    int transitionSwapL2R(bool aInit);
    int transitionSwapR2L(bool aInit);
    int transitionSwapT2B(bool aInit);
    int transitionSwapB2T(bool aInit);
    int transitionBlurIn(bool aInit);
    int transitionBlurOut(bool aInit);

private:

    QRgb   convertFromPremult(const QRgb& p)       const;
    QImage fastBlur(const QImage& img, int radius) const;
    QRandomGenerator *randomGenerator;
};

} // namespace Digikam

#endif // DIGIKAM_TRANSITION_MNGR_PRIVATE_H
