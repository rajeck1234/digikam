/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-28
 * Description : a tool to export image to a KIO accessible
 *               location
 *
 * SPDX-FileCopyrightText: 2006-2009 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_FT_EXPORT_WIDGET_H
#define DIGIKAM_FT_EXPORT_WIDGET_H

// Qt includes

#include <QWidget>
#include <QUrl>
#include <QList>

// Local includes

#include "dinfointerface.h"
#include "ditemslist.h"

using namespace Digikam;

namespace DigikamGenericFileTransferPlugin
{

class FTExportWidget: public QWidget
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent the parent widget
     */
    explicit FTExportWidget(DInfoInterface* const iface, QWidget* const parent);

    /**
     * Destructor.
     */
    ~FTExportWidget()               override;

    /**
     * Returns a pointer to the imagelist that is displayed.
     */
    DItemsList* imagesList() const;

    /**
     * Returns the currently selected target url. Maybe invalid.
     */
    QUrl targetUrl()         const;

    /**
     * Sets the target url this widget should point at.
     */
    void setTargetUrl(const QUrl& url);

    QList<QUrl> history()    const;
    void setHistory(const QList<QUrl>& urls);

private Q_SLOTS:

    void slotLabelUrlChanged();
    void slotShowTargetDialogClicked(bool checked);

Q_SIGNALS:

    void signalTargetUrlChanged(const QUrl& target);

private:

    void updateTargetLabel();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericFileTransferPlugin

#endif // DIGIKAM_FT_EXPORT_WIDGET_H
