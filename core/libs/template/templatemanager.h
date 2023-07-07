/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : metadata template manager.
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TEMPLATE_MANAGER_H
#define DIGIKAM_TEMPLATE_MANAGER_H

// Qt includes

#include <QList>
#include <QObject>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class Template;

class DIGIKAM_GUI_EXPORT TemplateManager : public QObject
{
    Q_OBJECT

public:

    bool load();
    bool save();
    void clear();

    void insert(const Template& t);
    void remove(const Template& t);

    Template fromIndex(int index)                 const;
    Template findByTitle(const QString& title)    const;
    Template findByContents(const Template& tref) const;
    QList<Template> templateList()                const;

    static TemplateManager* defaultManager();

Q_SIGNALS:

    void signalTemplateAdded(const Template&);
    void signalTemplateRemoved(const Template&);

private:

    void insertPrivate(const Template& t);
    void removePrivate(const Template& t);

private:

    // Disable
    TemplateManager();
    explicit TemplateManager(QObject*);
    ~TemplateManager() override;

private:

    friend class TemplateManagerCreator;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_TEMPLATE_MANAGER_H
