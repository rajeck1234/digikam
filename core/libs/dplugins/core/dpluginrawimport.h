/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-08
 * Description : RAW Import digiKam plugin definition.
 *
 * SPDX-FileCopyrightText: 2019-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DPLUGIN_RAW_IMPORT_H
#define DIGIKAM_DPLUGIN_RAW_IMPORT_H

// Local includes

#include "dplugin.h"
#include "dimg.h"
#include "loadingdescription.h"
#include "dpluginloader.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPluginRawImport : public DPlugin
{
    Q_OBJECT

public:

    /**
     * Constructor with optional parent object
     */
    explicit DPluginRawImport(QObject* const parent = nullptr);

    /**
     * Destructor
     */
    ~DPluginRawImport() override;

public:

    /**
     * This kind of plugin only provide one tool.
     */
    int count() const override { return 1; };

    /**
     * This kind of plugin do not use a category.
     */
    QStringList categories() const override { return QStringList(); };

    /**
     * This kind of plugin do not have GUI visibility attribute.
     */
    void setVisible(bool) override {};

    /**
     * Return the plugin interface identifier.
     */
    QString ifaceIid() const override { return QLatin1String(DIGIKAM_DPLUGIN_RAWIMPORT_IID); };

public:

    /**
     * Function to re-implement used to invoke Raw processor for a Raw file path
     * and a Default Raw decoding settings.
     */
    virtual bool run(const QString& path, const DRawDecoding& def) = 0;

Q_SIGNALS:

    /**
     * Signal emitted to notify host application to load Raw with these decoding settings.
     */
    void signalLoadRaw(const Digikam::LoadingDescription&);

    /**
     * Signal emitted to notify host application to load pre-decoded Raw preprocessed with these decoding settings.
     */
    void signalDecodedImage(const Digikam::LoadingDescription&, const Digikam::DImg&);
};

} // namespace Digikam

Q_DECLARE_INTERFACE(Digikam::DPluginRawImport, DIGIKAM_DPLUGIN_RAWIMPORT_IID)

#endif // DIGIKAM_DPLUGIN_RAW_IMPORT_H
