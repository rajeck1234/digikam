/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "vidslideimagespage.h"

// Qt includes

#include <QIcon>
#include <QPixmap>
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "vidslidewizard.h"
#include "ditemslist.h"
#include "dlayoutbox.h"

namespace DigikamGenericVideoSlideShowPlugin
{

class Q_DECL_HIDDEN VidSlideImagesPage::Private
{
public:

    explicit Private(QWizard* const dialog)
      : imageList(nullptr),
        wizard(nullptr),
        iface(nullptr)
    {
        wizard = dynamic_cast<VidSlideWizard*>(dialog);

        if (wizard)
        {
            iface = wizard->iface();
        }
    }

    DItemsList*     imageList;
    VidSlideWizard*  wizard;
    DInfoInterface*  iface;
};

VidSlideImagesPage::VidSlideImagesPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    setObjectName(QLatin1String("ImagesSelectorPage"));

    DVBox* const vbox  = new DVBox(this);
    QLabel* const desc = new QLabel(vbox);
    desc->setText(i18n("<p>This view list all items to include on video stream.</p>"
                       "<p>The first one is on top of the list, the last one</p>"
                       "<p>on the bottom. You can adjust the order of each item.</p>"));

    d->imageList       = new DItemsList(vbox);
    d->imageList->setObjectName(QLatin1String("VideoSlideshow ImagesList"));
    d->imageList->setControlButtonsPlacement(DItemsList::ControlButtonsBelow);

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("image-stack")));

    connect(d->imageList, SIGNAL(signalImageListChanged()),
            this, SIGNAL(completeChanged()));
}

VidSlideImagesPage::~VidSlideImagesPage()
{
    delete d;
}

void VidSlideImagesPage::setItemsList(const QList<QUrl>& urls)
{
    d->imageList->slotAddImages(urls);
}

void VidSlideImagesPage::initializePage()
{
    d->imageList->setIface(d->iface);
    d->imageList->listView()->clear();

    if (d->wizard->settings()->selMode == VidSlideSettings::IMAGES)
    {
        d->imageList->loadImagesFromCurrentSelection();
    }
    else
    {
        setItemsList(d->wizard->settings()->inputImages);
    }
}

bool VidSlideImagesPage::validatePage()
{
    if (d->imageList->imageUrls().isEmpty())
        return false;

    d->wizard->settings()->inputImages = d->imageList->imageUrls();

    return true;
}

bool VidSlideImagesPage::isComplete() const
{
    return (!d->imageList->imageUrls().isEmpty());
}

} // namespace DigikamGenericVideoSlideShowPlugin
