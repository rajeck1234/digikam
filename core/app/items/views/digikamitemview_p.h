/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-10-03
 * Description : Private Qt model-view for items
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Johannes Wienke <languitar at semipol do de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DIGIKAMITEM_VIEW_P_H
#define DIGIKAM_DIGIKAMITEM_VIEW_P_H

// Qt includes

#include <QObject>

// Local includes

#include "digikam_config.h"
#include "digikamitemview.h"
#include "digikamitemdelegate.h"
#include "itemrotationoverlay.h"
#include "itemfullscreenoverlay.h"
#include "applicationsettings.h"
#include "facepipeline.h"

namespace Digikam
{

class DigikamItemDelegate;
class ItemFaceDelegate;

class Q_DECL_HIDDEN DigikamItemView::Private : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DigikamItemView)

public:

    explicit Private(DigikamItemView* const qq);
    ~Private() override;

    void updateOverlays();
    void triggerRotateAction(const char* actionName);

public:

    ItemViewUtilities*       utilities;

    FacePipeline             editPipeline;

    DigikamItemDelegate*     normalDelegate;
    ItemFaceDelegate*        faceDelegate;

    bool                     overlaysActive;
    bool                     fullscreenActive;

    ItemRotateOverlay*       rotateLeftOverlay;
    ItemRotateOverlay*       rotateRightOverlay;
    ItemFullScreenOverlay*   fullscreenOverlay;

    bool                     faceMode;

private:

    DigikamItemView*         q_ptr;

private:

    // Disable
    Private() = delete;
};

} // namespace Digikam

#endif // DIGIKAM_DIGIKAMITEM_VIEW_P_H
