/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-01-18
 * Description : triangle geometry calculation class.
 *
 * SPDX-FileCopyrightText: 2005-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_EDITOR_PERSPECTIVE_TRIANGLE_H
#define DIGIKAM_EDITOR_PERSPECTIVE_TRIANGLE_H

// Qt includes

#include <QPoint>

namespace DigikamEditorPerspectiveToolPlugin
{

class PerspectiveTriangle
{

public:

    PerspectiveTriangle(const QPoint& A, const QPoint& B, const QPoint& C);
    ~PerspectiveTriangle() = default;

    float angleABC() const;
    float angleACB() const;
    float angleBAC() const;

private:

    float distanceP2P(const QPoint& p1, const QPoint& p2) const;

private:

    float m_a;
    float m_b;
    float m_c;
};

} // namespace DigikamEditorPerspectiveToolPlugin

#endif // DIGIKAM_EDITOR_PERSPECTIVE_TRIANGLE_H
