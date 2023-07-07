/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-08-16
 * Description : Spell-check and localize Settings Container.
 *
 * SPDX-FileCopyrightText: 2021-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_LOCALIZE_CONTAINER_H
#define DIGIKAM_LOCALIZE_CONTAINER_H

// Qt includes

#include <QDebug>
#include <QStringList>

// Local includes

#include "digikam_export.h"
#include "donlinetranslator.h"

class KConfigGroup;

namespace Digikam
{

/**
 * The class LocalizeContainer encapsulates all spell-check and localize related settings.
 */
class DIGIKAM_EXPORT LocalizeContainer
{
public:

    explicit LocalizeContainer();
    ~LocalizeContainer();

public:

    void readFromConfig(KConfigGroup& group);
    void writeToConfig(KConfigGroup& group) const;

public:

    // SpellCheck settings
    bool                        enableSpellCheck;       ///< Enable spell-checking feature.
    QString                     defaultLanguage;        ///> Default language code to use with x-default (empty for auto-detection).
    QStringList                 ignoredWords;           ///< Words to ignore with spell-checking.

    // Localize settings
    DOnlineTranslator::Engine   translatorEngine;       ///< Online translator to use.
    QStringList                 translatorLang;         ///< List of langues to use with Online translator.
    QStringList                 alternativeLang;        ///< List of langges to use with Alternative Languages Text editor.
};

//! qDebug() stream operator. Writes property @a inf to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const LocalizeContainer& inf);

} // namespace Digikam

#endif // DIGIKAM_LOCALIZE_CONTAINER_H
