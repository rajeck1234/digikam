/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : A widget stack to embed editor view.
 *
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_IMAGE_EDITOR_STACK_VIEW_H
#define DIGIKAM_IMAGE_EDITOR_STACK_VIEW_H

// Qt includes

#include <QStackedWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class Canvas;
class GraphicsDImgView;

class DIGIKAM_EXPORT EditorStackView : public QStackedWidget
{
    Q_OBJECT

public:

    enum StackViewMode
    {
        CanvasMode = 0,
        ToolViewMode
    };

public:

    explicit EditorStackView(QWidget* const parent = nullptr);
    ~EditorStackView()                                    override;

    void     setCanvas(Canvas* const canvas);
    Canvas*  canvas()                               const;

    void     setToolView(QWidget* const view);
    QWidget* toolView()                             const;

    void     setViewMode(int mode);
    int      viewMode()                             const;

    void     increaseZoom();
    void     decreaseZoom();
    void     toggleFitToWindow();
    void     fitToSelect();
    void     zoomTo100Percent();

    double   zoomMax()                              const;
    double   zoomMin()                              const;
    bool     isZoomablePreview()                    const;

Q_SIGNALS:

    void signalZoomChanged(bool isMax, bool isMin, double zoom);
    void signalToggleOffFitToWindow();

public Q_SLOTS:

    void setZoomFactor(double);
    void slotZoomSliderChanged(int);

private Q_SLOTS:

    void slotZoomChanged(double);
    void slotToggleOffFitToWindow(bool);

private:

    GraphicsDImgView* previewWidget()               const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_IMAGE_EDITOR_STACK_VIEW_H
