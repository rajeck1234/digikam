/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-20
 * Description : settings container for versioning
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_VERSION_MANAGER_SETTINGS_H
#define DIGIKAM_VERSION_MANAGER_SETTINGS_H

// Qt includes

#include <QFlags>
#include <QString>

// Local includes

#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT VersionManagerSettings
{
public:

    enum IntermediateSavepoint
    {
        NoIntermediates     = 0,
        AfterEachSession    = 1 << 0,
        AfterRawConversion  = 1 << 1,
        WhenNotReproducible = 1 << 2
    };
    Q_DECLARE_FLAGS(IntermediateBehavior, IntermediateSavepoint)

    enum ShowInViewFlag
    {
        OnlyShowCurrent     = 0,
        ShowOriginal        = 1 << 0,
        ShowIntermediates   = 1 << 1
    };
    Q_DECLARE_FLAGS(ShowInViewFlags, ShowInViewFlag)

    enum EditorClosingMode
    {
        AlwaysAsk,
        AutoSave
    };

public:

    VersionManagerSettings();

    void readFromConfig(KConfigGroup& group);
    void writeToConfig(KConfigGroup& group) const;

public:

    bool                 enabled;

    IntermediateBehavior saveIntermediateVersions;
    ShowInViewFlags      showInViewFlags;
    EditorClosingMode    editorClosingMode;

    /// Image format string as defined for database, in upper case
    QString              format;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::VersionManagerSettings::IntermediateBehavior)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::VersionManagerSettings::ShowInViewFlags)

#endif // DIGIKAM_VERSION_MANAGER_SETTINGS_H
