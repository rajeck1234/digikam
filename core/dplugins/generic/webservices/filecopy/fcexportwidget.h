/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-03-27
 * Description : a tool to export items to a local storage
 *
 * SPDX-FileCopyrightText: 2006-2009 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_FC_EXPORT_WIDGET_H
#define DIGIKAM_FC_EXPORT_WIDGET_H

// Qt includes

#include <QWidget>
#include <QUrl>

// Local includes

#include "dinfointerface.h"
#include "fccontainer.h"
#include "ditemslist.h"

using namespace Digikam;

namespace DigikamGenericFileCopyPlugin
{

class FCExportWidget: public QWidget
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent the parent widget
     */
    explicit FCExportWidget(DInfoInterface* const iface, QWidget* const parent);

    /**
     * Destructor.
     */
    ~FCExportWidget() override;

    /**
     * Returns a pointer to the imagelist that is displayed.
     */
    DItemsList* imagesList() const;

    /**
     * Returns the currently selected target url. Maybe invalid.
     */
    QUrl targetUrl() const;

    /**
     * Returns settings container.
     */
    FCContainer getSettings() const;

    /**
     * Set widget from settings container.
     */
    void setSettings(const FCContainer& settings);

private Q_SLOTS:

    void slotLabelUrlChanged();

    void slotFileCopyButtonChanged(bool enabled);

Q_SIGNALS:

    void signalTargetUrlChanged(const QUrl& target);

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericFileCopyPlugin

#endif // DIGIKAM_FC_EXPORT_WIDGET_H
