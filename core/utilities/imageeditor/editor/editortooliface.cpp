/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : Image editor interface used by editor tools.
 *
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "editortooliface.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "sidebar.h"
#include "canvas.h"
#include "statusprogressbar.h"
#include "editortool.h"
#include "editortoolsettings.h"
#include "editorstackview.h"
#include "editorwindow.h"
#include "imageguidewidget.h"
#include "imageregionwidget.h"
#include "previewlayout.h"
#include "dcategorizedview.h"

namespace Digikam
{

class Q_DECL_HIDDEN EditorToolIface::Private
{

public:

    explicit Private()
      : toolsIconView(nullptr),
        lastActiveTab(nullptr),
        tool         (nullptr),
        editor       (nullptr),
        splitterSize (0)
    {
    }

    DCategorizedView* toolsIconView;
    QWidget*          lastActiveTab;
    EditorTool*       tool;
    EditorWindow*     editor;

    int               splitterSize;
};

EditorToolIface* EditorToolIface::m_iface = nullptr;

EditorToolIface* EditorToolIface::editorToolIface()
{
    return m_iface;
}

EditorToolIface::EditorToolIface(EditorWindow* const editor)
    : QObject(),
      d      (new Private)
{
    d->editor = editor;
    m_iface   = this;
}

EditorToolIface::~EditorToolIface()
{
    delete d->tool;
    delete d;

    if (m_iface == this)
    {
        m_iface = nullptr;
    }
}

void EditorToolIface::setToolsIconView(DCategorizedView* const view)
{
    d->toolsIconView = view;
    d->editor->rightSideBar()->appendTab(d->toolsIconView,
                                         QIcon::fromTheme(QLatin1String("document-edit")),
                                         i18n("Tools"));
}

EditorTool* EditorToolIface::currentTool() const
{
    return d->tool;
}

void EditorToolIface::loadTool(EditorTool* const tool)
{
    if (d->tool)
    {
        unLoadTool();
    }

    d->tool          = tool;
    d->lastActiveTab = d->editor->rightSideBar()->getActiveTab();
    d->splitterSize  = d->editor->sidebarSplitter()->size(d->editor->rightSideBar());

    d->editor->editorStackView()->setToolView(d->tool->toolView());
    d->editor->editorStackView()->setViewMode(EditorStackView::ToolViewMode);

    d->editor->rightSideBar()->deleteTab(d->toolsIconView);
    d->editor->rightSideBar()->appendTab(d->tool->toolSettings(), d->tool->toolIcon(), d->tool->toolName());
    d->editor->rightSideBar()->setActiveTab(d->tool->toolSettings());

    int w = qMax(d->splitterSize, d->tool->toolSettings()->minimumSizeHint().width());
    d->editor->sidebarSplitter()->setSize(d->editor->rightSideBar(), w);

    d->editor->toggleActions(false);
    d->editor->toggleToolActions(d->tool);

    // If editor tool has zoomable preview, switch on zoom actions.

    d->editor->toggleZoomActions(d->editor->editorStackView()->isZoomablePreview());

    ImageGuideWidget* const view = dynamic_cast<ImageGuideWidget*>(d->tool->toolView());

    if (view)
    {
        connect(d->editor, SIGNAL(signalPreviewModeChanged(int)),
                view, SLOT(slotPreviewModeChanged(int)));

        view->slotPreviewModeChanged(d->editor->previewMode());
    }

    // To set zoomable preview zoom level and position accordingly with main canvas.

    ImageRegionWidget* const view2 = dynamic_cast<ImageRegionWidget*>(d->tool->toolView());

    if (view2)
    {
        connect(d->editor, SIGNAL(signalPreviewModeChanged(int)),
                view2, SLOT(slotPreviewModeChanged(int)));

        connect(d->editor->editorStackView(), SIGNAL(signalZoomChanged(bool,bool,double)),
                view2, SLOT(slotOriginalImageRegionChangedDelayed()));

        if (d->editor->editorStackView()->canvas()->layout()->isFitToWindow())
        {
            view2->fitToWindow();
        }
        else
        {
            view2->layout()->setZoomFactor(d->editor->editorStackView()->canvas()->layout()->zoomFactor());
            QPoint tl = d->editor->editorStackView()->canvas()->visibleArea().topLeft();
            view2->setContentsPos(tl.x(), tl.y());
        }

        view2->slotPreviewModeChanged(d->editor->previewMode());
    }

    themeChanged();
    updateExposureSettings();
    updateICCSettings();
    setToolInfoMessage(QString());

    connect(d->editor, SIGNAL(signalPreviewModeChanged(int)),
            d->tool, SLOT(slotPreviewModeChanged()));

    connect(d->tool, SIGNAL(okClicked()),
            this, SLOT(slotToolApplied()));

    d->tool->init();
}

void EditorToolIface::unLoadTool()
{
    if (!d->tool)
    {
        return;
    }

    // To restore  zoom level and position accordingly with zoomable preview.

    ImageRegionWidget* const view2 = dynamic_cast<ImageRegionWidget*>(d->tool->toolView());

    if (view2)
    {
        if (view2->layout()->isFitToWindow())
        {
            d->editor->editorStackView()->canvas()->fitToWindow();
        }
        else
        {
            d->editor->editorStackView()->canvas()->layout()->setZoomFactor(view2->layout()->zoomFactor());
            QPoint tl = view2->visibleArea().topLeft();
            d->editor->editorStackView()->canvas()->setContentsPos(tl.x(), tl.y());
        }
    }

    d->editor->editorStackView()->setViewMode(EditorStackView::CanvasMode);
    d->editor->editorStackView()->setToolView(nullptr);
    d->editor->rightSideBar()->deleteTab(d->tool->toolSettings());
    d->editor->rightSideBar()->appendTab(d->toolsIconView,
                                         QIcon::fromTheme(QLatin1String("document-edit")),
                                         i18n("Tools"));

    d->editor->rightSideBar()->setActiveTab(d->lastActiveTab);

    if (d->splitterSize > 0)
    {
        d->editor->sidebarSplitter()->setSize(d->editor->rightSideBar(),
                                              d->splitterSize);
    }
    else
    {
        d->editor->rightSideBar()->shrink();
    }

    d->editor->toggleActions(true);
    d->editor->toggleToolActions();
    d->editor->slotUpdateItemInfo();
    d->editor->setPreviewModeMask(PreviewToolBar::NoPreviewMode);

    delete d->tool;
    d->tool = nullptr;

    // Reset info label in status bar with canvas selection info.

    d->editor->slotSelected(!d->editor->m_canvas->getSelectedArea().isNull());
    d->editor->editorStackView()->canvas()->layout()->updateZoomAndSize();
}

void EditorToolIface::setToolInfoMessage(const QString& txt)
{
    d->editor->setToolInfoMessage(txt);
}

void EditorToolIface::setToolStartProgress(const QString& toolName)
{
    d->editor->setToolStartProgress(toolName);
    d->editor->toggleZoomActions(!d->editor->editorStackView()->isZoomablePreview());
}

void EditorToolIface::setToolProgress(int progress)
{
    d->editor->setToolProgress(progress);
}

void EditorToolIface::setToolStopProgress()
{
    d->editor->setToolStopProgress();
    d->editor->toggleZoomActions(d->editor->editorStackView()->isZoomablePreview());
}

void EditorToolIface::slotToolAborted()
{
    EditorToolThreaded* const tool = dynamic_cast<EditorToolThreaded*>(d->tool);

    if (tool)
    {
        tool->slotAbort();
    }
}

void EditorToolIface::slotCloseTool()
{
    EditorTool* const tool = dynamic_cast<EditorTool*>(d->tool);

    if (tool)
    {
        tool->slotCloseTool();
    }
}

void EditorToolIface::slotApplyTool()
{
    EditorTool* const tool = dynamic_cast<EditorTool*>(d->tool);

    if (tool)
    {
        tool->slotApplyTool();
    }
}

void EditorToolIface::setupICC()
{
    d->editor->slotSetupICC();
}

void EditorToolIface::themeChanged()
{
    EditorTool* const tool = dynamic_cast<EditorTool*>(d->tool);

    if (tool)
    {
        tool->setBackgroundColor(d->editor->m_bgColor);
    }
}

void EditorToolIface::updateICCSettings()
{
    EditorTool* const tool = dynamic_cast<EditorTool*>(d->tool);

    if (tool)
    {
        tool->ICCSettingsChanged();
    }
}

void EditorToolIface::updateExposureSettings()
{
    ExposureSettingsContainer* const expoSettings = d->editor->exposureSettings();
    d->editor->editorStackView()->canvas()->setExposureSettings(expoSettings);
    EditorTool* const tool                        = dynamic_cast<EditorTool*>(d->tool);

    if (tool)
    {
        tool->exposureSettingsChanged();
    }
}

void EditorToolIface::setPreviewModeMask(int mask)
{
    d->editor->setPreviewModeMask(mask);
}

void EditorToolIface::slotToolApplied()
{
    Q_EMIT d->editor->signalToolApplied();
}

} // namespace Digikam
