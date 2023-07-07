/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-01
 * Description : Qt model view for Showfoto item - the delegate Private
 *
 * SPDX-FileCopyrightText: 2013      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef SHOW_FOTO_DELEGATE_P_H
#define SHOW_FOTO_DELEGATE_P_H

#include "showfotodelegate.h"

// Qt includes

#include <QPainter>
#include <QApplication>
#include <QRect>
#include <QCache>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "showfotoitemmodel.h"
#include "showfotofiltermodel.h"
#include "showfotothumbnailbar.h"
#include "showfotoiteminfo.h"
#include "showfotothumbnailmodel.h"
#include "showfotosettings.h"
#include "showfotoitemviewdelegate_p.h"

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoDelegate::ShowfotoDelegatePrivate : public ShowfotoItemViewDelegatePrivate
{
public:

    ShowfotoDelegatePrivate()
      : contentWidth       (0),
        drawImageFormat    (true),
        drawFocusFrame     (true),
        drawMouseOverFrame (true),
        currentView        (nullptr),
        currentModel       (nullptr)
    {
        actualPixmapRectCache.setMaxCost(250);
    }

    int                    contentWidth;

    QRect                  dateRect;
    QRect                  pixmapRect;
    QRect                  nameRect;
    QRect                  resolutionRect;
    QRect                  sizeRect;
    QRect                  imageInformationRect;
    QRect                  groupRect;
    QRect                  coordinatesRect;

    bool                   drawImageFormat;
    bool                   drawFocusFrame;
    bool                   drawMouseOverFrame;

    QCache<int, QRect>     actualPixmapRectCache;

    ShowfotoThumbnailBar*  currentView;
    QAbstractItemModel*    currentModel;

public:

    void clearRects() override;
};

// --- ShowfotoThumbnailDelegate ----------------------------------------------------

class Q_DECL_HIDDEN ShowfotoThumbnailDelegatePrivate : public ShowfotoDelegate::ShowfotoDelegatePrivate
{
public:

    ShowfotoThumbnailDelegatePrivate()
      : flow              (QListView::LeftToRight)
    {
        // switch off drawing of frames

        drawMouseOverFrame = false;
        drawFocusFrame     = false;
    }

    void init(ShowfotoThumbnailDelegate* const q);

public:

    QListView::Flow flow;
    QRect           viewSize;
};

// --- ShowfotoNormalDelegate ----------------------------------------------------

class Q_DECL_HIDDEN ShowfotoNormalDelegatePrivate : public ShowfotoDelegate::ShowfotoDelegatePrivate
{
public:

    ShowfotoNormalDelegatePrivate()
    {
    }

    void init(ShowfotoNormalDelegate* const q, ShowfotoThumbnailBar* const parent);
};

} // namespace Showfoto

#endif // SHOW_FOTO_DELEGATE_P_H
