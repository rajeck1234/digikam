/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-04
 * Description : image editor canvas management class
 *
 * SPDX-FileCopyrightText: 2013-2014 by Yiou Wang <geow812 at gmail dot com>
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2004-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_IMAGE_EDITOR_CANVAS_H
#define DIGIKAM_IMAGE_EDITOR_CANVAS_H

// Qt includes

#include <QRect>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>

// Local includes

#include "digikam_export.h"
#include "dimg.h"
#include "graphicsdimgview.h"
#include "editorcore.h"

class QWheelEvent;
class QKeyEvent;

namespace Digikam
{

class EditorCore;
class ExposureSettingsContainer;
class ICCSettingsContainer;
class IccTransform;
class IOFileSettings;

class DIGIKAM_EXPORT Canvas : public GraphicsDImgView
{
    Q_OBJECT

public:

    explicit Canvas(QWidget* const parent = nullptr);
    ~Canvas() override;

    void load(const QString& filename, IOFileSettings* const IOFileSettings);
    void preload(const QString& filename);

    void resetImage();
    void abortSaving();
    void setModified();
    void makeDefaultEditingCanvas();

    QString     ensureHasCurrentUuid()   const;

    /**
     * Return the core interface instance of editor.
     */
    EditorCore* interface()              const;

    /**
     * Return a copy of current image loaded in editor.
     */
    DImg        currentImage()           const;

    /**
     * Return the type mime of current image loaded in editor.
     */
    QString     currentImageFileFormat() const;

    /**
     * Return the file path of current image loaded in editor.
     */
    QString     currentImageFilePath()   const;

    /**
     * Return the width of current image loaded in editor.
     */
    int         imageWidth()             const;

    /**
     * Return the height of current image loaded in editor.
     */
    int         imageHeight()            const;

    /**
     * Return the rectangle information of current canvas selection.
     */
    QRect       getSelectedArea()        const;

    /**
     * If current image file format is only available in read only,
     * typically all RAW image file formats.
     */
    bool        isReadOnly()             const;

    /**
     * Apply Color management settings (typically screen profile).
     */
    void setICCSettings(const ICCSettingsContainer& cmSettings);

    /**
     * Apply Color Management transformation to image (typically working color space).
     */
    void applyTransform(const IccTransform& transform);

    /**
     * Apply under.over exposure indicator settings.
     */
    void setExposureSettings(ExposureSettingsContainer* const expoSettings);

    /**
     * Turn on/off Color Management Soft proofing mode.
     */
    void setSoftProofingEnabled(bool enable);

    /**
     * Rotate image following Exif information.
     */
    void setExifOrient(bool exifOrient);

    /**
     * Return true if image have been rotated following Exif information.
     */
    bool exifRotated() const;

    /**
     * Change zoom level to fit current selection on canvas size.
     */
    void fitToSelect();

Q_SIGNALS:

    void signalChanged();
    void signalSelected(bool);
    void signalRightButtonClicked();
    void signalShowNextImage();
    void signalShowPrevImage();
    void signalPrepareToLoad();
    void signalLoadingStarted(const QString& filename);
    void signalLoadingFinished(const QString& filename, bool success);
    void signalLoadingProgress(const QString& filePath, float progress);
    void signalSavingStarted(const QString& filename);
    void signalSavingFinished(const QString& filename, bool success);
    void signalSavingProgress(const QString& filePath, float progress);
    void signalSelectionChanged(const QRect&);
    void signalSelectionSetText(const QRect&);
    void signalToggleOffFitToWindow();
    void signalUndoSteps(int);
    void signalRedoSteps(int);
    void signalZoomChanged(double);
    void signalAddedDropedItems(QDropEvent*);

public Q_SLOTS:

    /// image modifiers
    void slotRotate90();
    void slotRotate180();
    void slotRotate270();
    void slotFlipHoriz();
    void slotFlipVert();
    void slotCrop();

    void slotRestore();
    void slotUndo(int steps = 1);
    void slotRedo(int steps = 1);

    void slotCopy();

    void slotSelectAll();
    void slotSelectNone();
    void slotSelected();
    void slotSelectionMoved();

protected:

    void keyPressEvent(QKeyEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void addRubber();

    void dragMoveEvent(QDragMoveEvent*) override;
    void dragEnterEvent(QDragEnterEvent*) override;
    void dropEvent(QDropEvent*) override;

private:

    QRect calcSelectedArea() const;
    void  reset();

private Q_SLOTS:

    void slotModified();
    void slotImageLoaded(const QString& filePath, bool success);
    void slotImageSaved(const QString& filePath, bool success);
    void slotAddItemStarted(const QPointF& pos);
    void slotAddItemMoving(const QRectF& rect);
    void slotAddItemFinished(const QRectF& rect);
    void cancelAddItem();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_IMAGE_EDITOR_CANVAS_H
