/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-26
 * Description : a digiKam image editor tool to restore
 *               a photograph
 *
 * SPDX-FileCopyrightText: 2005-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_EDITOR_RESTORATION_TOOL_H
#define DIGIKAM_EDITOR_RESTORATION_TOOL_H

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEditorRestorationToolPlugin
{

class RestorationTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit RestorationTool(QObject* const parent);
    ~RestorationTool()          override;

private Q_SLOTS:

    void slotSaveAsSettings()   override;
    void slotLoadSettings()     override;
    void slotResetSettings()    override;
    void slotResetValues(int);

private:

    void readSettings()         override;
    void writeSettings()        override;
    void preparePreview()       override;
    void prepareFinal()         override;
    void setPreviewImage()      override;
    void setFinalImage()        override;

private:

    class Private;
    Private* const d;
};

} // namespace DigikamEditorRestorationToolPlugin

#endif // DIGIKAM_EDITOR_RESTORATION_TOOL_H
