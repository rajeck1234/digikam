/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : a widget to draw sketch.
 *
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_SKETCH_WIDGET_H
#define DIGIKAM_SKETCH_WIDGET_H

// Qt includes

#include <QWidget>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace Digikam
{

class SketchWidget : public QWidget
{
    Q_OBJECT

public:

    explicit SketchWidget(QWidget* const parent = nullptr);
    ~SketchWidget() override;

    QColor  penColor() const;
    int     penWidth() const;
    bool    isClear()  const;

    QImage  sketchImage() const;
    void    setSketchImage(const QImage& image);

    /**
     * This method return the drawing line history
     * as XML, to be stored in database as SAlbum data.
     */
    void sketchImageToXML(QXmlStreamWriter& writer);
    QString sketchImageToXML();

    /**
     * This method set sketch image using XML data based
     * on drawing line history.
     * Return true if data are imported successfully.
     */
    bool setSketchImageFromXML(QXmlStreamReader& reader);
    bool setSketchImageFromXML(const QString& xml);

Q_SIGNALS:

    void signalSketchChanged(const QImage&);
    void signalPenSizeChanged(int);
    void signalPenColorChanged(const QColor&);
    void signalUndoRedoStateChanged(bool hasUndo, bool hasRedo);

public Q_SLOTS:

    void setPenColor(const QColor& newColor);
    void setPenWidth(int newWidth);
    void slotClear();
    void slotUndo();
    void slotRedo();

protected:

    void mousePressEvent(QMouseEvent*)      override;
    void mouseMoveEvent(QMouseEvent*)       override;
    void wheelEvent(QWheelEvent*)           override;
    void mouseReleaseEvent(QMouseEvent*)    override;
    void keyPressEvent(QKeyEvent*)          override;
    void keyReleaseEvent(QKeyEvent*)        override;
    void paintEvent(QPaintEvent*)           override;

private:

    void updateDrawCursor();
    void replayEvents(int index);
    void drawLineTo(const QPoint& endPoint);
    void drawLineTo(int width, const QColor& color, const QPoint& start, const QPoint& end);
    void drawPath(int width, const QColor& color, const QPainterPath& path);
    void addPath(QXmlStreamReader& reader);
/*
    QDomElement addXmlTextElement(QDomDocument& document, QDomElement& target,
                                  const QString& tag, const QString& text);
*/
private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_SKETCH_WIDGET_H
