/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-22
 * Description : TIFF DImg plugin.
 *
 * SPDX-FileCopyrightText: 2020-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DIMG_TIFF_PLUGIN_H
#define DIGIKAM_DIMG_TIFF_PLUGIN_H

// Qt includes

#include <QByteArray>
#include <QWidget>

// Local includes

#include "dplugindimg.h"
#include "dimg.h"
#include "dimgloader.h"

#define DPLUGIN_IID "org.kde.digikam.plugin.dimg.TIFF"

using namespace Digikam;

namespace DigikamTIFFDImgPlugin
{

class DImgTIFFPlugin : public DPluginDImg
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DPLUGIN_IID)
    Q_INTERFACES(Digikam::DPluginDImg)

public:

    explicit DImgTIFFPlugin(QObject* const parent = nullptr);
    ~DImgTIFFPlugin()                                                                             override;

    QString name()                                                                          const override;
    QString iid()                                                                           const override;
    QIcon   icon()                                                                          const override;
    QString details()                                                                       const override;
    QString description()                                                                   const override;
    QList<DPluginAuthor> authors()                                                          const override;
    QString handbookSection()                                                               const override;
    QString handbookChapter()                                                               const override;
    QString handbookReference()                                                             const override;

    void setup(QObject* const)                                                                    override;
    QMap<QString, QStringList> extraAboutData()                                             const override;

    QString     loaderName()                                                                const override;
    QString     typeMimes()                                                                 const override;
    int         canRead(const QFileInfo& fileInfo, bool magic)                              const override;
    int         canWrite(const QString& format)                                             const override;
    DImgLoader* loader(DImg* const image, const DRawDecoding& rawSettings = DRawDecoding()) const override;

    DImgLoaderSettings* exportWidget(const QString& format)                                 const override;
};

} // namespace DigikamTIFFDImgPlugin

#endif // DIGIKAM_DIMG_TIFF_PLUGIN_H
