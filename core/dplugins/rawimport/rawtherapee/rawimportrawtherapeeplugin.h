/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-18
 * Description : RawTherapee raw import plugin.
 *
 * SPDX-FileCopyrightText: 2019-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_RAW_IMPORT_RAWTHERAPEE_PLUGIN_H
#define DIGIKAM_RAW_IMPORT_RAWTHERAPEE_PLUGIN_H

// Qt includes

#include <QProcess>

// Local includes

#include "dpluginrawimport.h"

#define DPLUGIN_IID "org.kde.digikam.plugin.rawimport.RawTherapee"

using namespace Digikam;

namespace DigikamRawImportRawTherapeePlugin
{

// cppcheck-suppress noConstructor
class RawTherapeeRawImportPlugin : public DPluginRawImport
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DPLUGIN_IID)
    Q_INTERFACES(Digikam::DPluginRawImport)

public:

    explicit RawTherapeeRawImportPlugin(QObject* const parent = nullptr);
    ~RawTherapeeRawImportPlugin()                              override;

    QString name()                                       const override;
    QString iid()                                        const override;
    QIcon   icon()                                       const override;
    QString details()                                    const override;
    QString description()                                const override;
    QList<DPluginAuthor> authors()                       const override;
    QString handbookSection()                            const override;
    QString handbookChapter()                            const override;
    QString handbookReference()                          const override;

    void setup(QObject* const)                                 override;

    bool run(const QString& filePath, const DRawDecoding& def) override;

private Q_SLOTS:

    void slotErrorOccurred(QProcess::ProcessError);
    void slotProcessFinished(int, QProcess::ExitStatus);
    void slotProcessReadyRead();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamRawImportRawTherapeePlugin

#endif // DIGIKAM_RAW_IMPORT_RAWTHERAPEE_PLUGIN_H
