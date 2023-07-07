/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : colors theme manager
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_THEME_MANAGER_H
#define DIGIKAM_THEME_MANAGER_H

// Qt includes

#include <QObject>
#include <QPixmap>
#include <QString>

// Local includes

#include "digikam_export.h"

class QMenu;

namespace Digikam
{

class DXmlGuiWindow;
class Theme;

class DIGIKAM_EXPORT ThemeManager : public QObject
{
    Q_OBJECT

public:

    ~ThemeManager()                   override;
    static ThemeManager* instance();

    QString currentThemeName() const;
    void    setCurrentTheme(const QString& name);

    QString defaultThemeName() const;

    void    setThemeMenuAction(QMenu* const action);
    void    registerThemeActions(DXmlGuiWindow* const win);

    void    updateThemeMenu();

Q_SIGNALS:

    void signalThemeChanged();

private Q_SLOTS:

    void slotChangePalette();

private:

    // Disable
    ThemeManager();
    explicit ThemeManager(QObject*);

    void    populateThemeMenu();
    QString currentDesktopdefaultTheme() const;
    void    updateCurrentDesktopDefaultThemePreview();

private:

    friend class ThemeManagerCreator;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_THEME_MANAGER_H
