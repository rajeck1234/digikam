/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-12-09
 * Description : image selection widget used by ratio crop tool.
 *
 * SPDX-FileCopyrightText: 2007      by Jaromir Malenko <malenko at email.cz>
 * SPDX-FileCopyrightText: 2008      by Roberto Castagnola <roberto dot castagnola at gmail dot com>
 * SPDX-FileCopyrightText: 2004-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_EDITOR_RATIO_CROP_WIDGET_H
#define DIGIKAM_EDITOR_RATIO_CROP_WIDGET_H

// Qt includes

#include <QWidget>
#include <QRect>
#include <QColor>

// Local includes

#include "imageiface.h"

using namespace Digikam;

namespace DigikamEditorRatioCropToolPlugin
{

class RatioCropWidget : public QWidget
{
    Q_OBJECT

public:

    enum RatioAspect               ///< Constrained Aspect Ratio list. See RatioCropWidget::setSelectionAspectRatioType() method for crop-value implementations.
    {
        RATIOCUSTOM = 0,           ///< Custom aspect ratio.
        RATIO01X01,                ///< 1:1
        RATIO02x01,                ///< 2:1
        RATIO02x03,                ///< 2:3
        RATIO03X01,                ///< 3:1
        RATIO03X04,                ///< 3:4
        RATIO04X01,                ///< 4:1
        RATIO04X05,                ///< 4:5
        RATIO05x07,                ///< 5:7
        RATIO07x10,                ///< 7:10
        RATIO08x05,                ///< 8:5
        RATIO16x09,                ///< 19:9
        RATIODINA0,                ///< DIN A
        RATIOGOLDEN,               ///< Golden ratio : 1:1.618
        RATIOCURRENT,              ///< Current loaded image aspect ratio
        RATIONONE                  ///< No aspect ratio.
    };

    enum Orient
    {
        Landscape = 0,
        Portrait
    };

    enum CenterType
    {
        CenterWidth = 0,           ///< Center selection to the center of image width.
        CenterHeight,              ///< Center selection to the center of image height.
        CenterImage                ///< Center selection to the center of image.
    };

    // Proportion : Golden Ratio and Rule of Thirds. More information at this url:
    // photoinf.com/General/Robert_Berdan/Composition_and_the_Elements_of_Visual_Design.htm

    enum GuideLineType
    {
        RulesOfThirds = 0,         ///< Line guides position to 1/3 width and height.
        DiagonalMethod,            ///< Diagonal Method to improve composition.
        HarmoniousTriangles,       ///< Harmonious Triangle to improve composition.
        GoldenMean,                ///< Guides tools using Phi ratio (1.618).
        GuideNone                  ///< No guide line.
    };

public:

    RatioCropWidget(int width, int height, QWidget* const parent = nullptr);
    RatioCropWidget(int width, int height, bool initDrawing, QWidget* const parent = nullptr);
    ~RatioCropWidget() override;

    void  setBackgroundColor(const QColor& bg);
    void  setCenterSelection(int centerType=CenterImage);
    void  setSelectionX(int x);
    void  setSelectionY(int y);
    void  setSelectionWidth(int w);
    void  setSelectionHeight(int h);
    void  setSelectionOrientation(int orient);
    void  setIsDrawingSelection(bool draw);
    void  setPreciseCrop(bool precise);
    void  setAutoOrientation(bool orientation);
    void  setSelectionAspectRatioType(int aspectRatioType);
    void  setSelectionAspectRatioValue(int widthRatioValue, int heightRatioValue);
    void  setGoldenGuideTypes(bool drawGoldenSection,  bool drawGoldenSpiralSection,
                              bool drawGoldenSpiral,   bool drawGoldenTriangle,
                              bool flipHorGoldenGuide, bool flipVerGoldenGuide);

    int   getOriginalImageWidth()   const;
    int   getOriginalImageHeight()  const;
    QRect getRegionSelection()      const;

    int   getMinWidthRange()        const;
    int   getMinHeightRange()       const;
    int   getMaxWidthRange()        const;
    int   getMaxHeightRange()       const;
    int   getWidthStep()            const;
    int   getHeightStep()           const;

    bool  preciseCropAvailable()    const;

    void  resetSelection();
    void  maxAspectSelection();

    ImageIface* imageIface()        const;

public Q_SLOTS:

    void slotGuideLines(int guideLinesType);
    void slotChangeGuideColor(const QColor& color);
    void slotChangeGuideSize(int size);

Q_SIGNALS:

    void signalSelectionMoved(const QRect& rect);
    void signalSelectionChanged(const QRect& rect);
    void signalSelectionOrientationChanged(int newOrientation);

protected:

    void paintEvent(QPaintEvent*)           override;
    void mousePressEvent(QMouseEvent*)      override;
    void mouseReleaseEvent(QMouseEvent*)    override;
    void mouseMoveEvent(QMouseEvent*)       override;
    void resizeEvent(QResizeEvent*)         override;

private:

    // Recalculate the target selection position and emit 'signalSelectionMoved'.
    void   regionSelectionMoved();
    void   regionSelectionChanged();

    QPoint convertPoint(const QPoint& pm, bool localToReal=true) const;
    QPoint convertPoint(int x, int y, bool localToReal=true)     const;
    QPoint opposite()                                            const;

    void   normalizeRegion();
    void   reverseRatioValues();
    void   applyAspectRatio(bool useHeight, bool repaintWidget=true);
    void   updatePixmap();
    void   placeSelection(const QPoint& pm, bool symmetric, const QPoint& center);
    void   setCursorResizing();

    float  distance(const QPoint& a, const QPoint& b) const;
    int    computePreciseSize(int size, int step)     const;

    // drawing functions for the various guide types
    void   drawRulesOfThirds(QPainter& p, const int& xThird, const int& yThird);
    void   drawDiagonalMethod(QPainter& p, const int& w, const int& h);
    void   drawHarmoniousTriangles(QPainter& p, const int& dst);
    void   drawGoldenMean(QPainter& p, const QRect& R1, const QRect& R2,
                          const QRect& R3, const QRect& R4, const QRect& R5,
                          const QRect& R6, const QRect& R7);

    void setup(int width, int height,
               int widthRatioValue = 1, int heightRatioValue = 1,
               int aspectRatio = RATIO01X01, int orient = Landscape,
               int guideLinesType = GuideNone);

private:

    class Private;
    Private* const d;
};

} // namespace DigikamEditorRatioCropToolPlugin

#endif // DIGIKAM_EDITOR_RATIO_CROP_WIDGET_H
