/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor tool to emboss
 *               an image.
 *
 * SPDX-FileCopyrightText: 2004-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_EDITOR_EMBOSS_TOOL_H
#define DIGIKAM_EDITOR_EMBOSS_TOOL_H

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEditorEmbossToolPlugin
{

class EmbossTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit EmbossTool(QObject* const parent);
    ~EmbossTool()               override;

private Q_SLOTS:

    void slotResetSettings()    override;

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

} // namespace DigikamEditorEmbossToolPlugin

#endif // DIGIKAM_EDITOR_EMBOSS_TOOL_H
