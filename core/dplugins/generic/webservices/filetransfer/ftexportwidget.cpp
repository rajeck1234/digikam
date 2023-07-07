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

#include "ftexportwidget.h"

// Qt includes

#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPointer>
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>
#include <kcombobox.h>
#include <kurlrequester.h>

// Local includes

#include "digikam_debug.h"
#include "dfiledialog.h"
#include "ditemslist.h"
#include "wstoolutils.h"
#include "dlayoutbox.h"

namespace DigikamGenericFileTransferPlugin
{

class Q_DECL_HIDDEN FTExportWidget::Private
{
public:

    explicit Private()
      : targetLabel        (nullptr),
        targetSearchButton (nullptr),
        imageList          (nullptr)
    {
    }

    KUrlComboRequester* targetLabel;
    QPushButton*        targetSearchButton;
    QUrl                targetUrl;
    DItemsList*         imageList;
};

FTExportWidget::FTExportWidget(DInfoInterface* const iface, QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    // setup remote target selection

    DHBox* const hbox   = new DHBox(this);
    QLabel* const label = new QLabel(hbox);
    d->targetLabel      = new KUrlComboRequester(hbox);

    if (d->targetLabel->button())
    {
        d->targetLabel->button()->hide();
    }

    d->targetLabel->comboBox()->setEditable(true);

    label->setText(i18n("Target location: "));
    d->targetLabel->setWhatsThis(i18n("Sets the target address to upload the images to. "
                                      "This can be any address as used in Dolphin or Konqueror, "
                                      "e.g. ftp://my.server.org/sub/folder."));

    d->targetSearchButton = new QPushButton(i18n("Select export location..."), this);
    d->targetSearchButton->setIcon(QIcon::fromTheme(QLatin1String("folder-remote")));

    // setup image list

    d->imageList = new DItemsList(this);
    d->imageList->setObjectName(QLatin1String("FTExport ImagesList"));
    d->imageList->setIface(iface);
    d->imageList->loadImagesFromCurrentSelection();
    d->imageList->setAllowRAW(true);
    d->imageList->listView()->setWhatsThis(i18n("This is the list of images to upload "
                                                "to the specified target."));

    // layout dialog

    QVBoxLayout* const layout = new QVBoxLayout(this);

    layout->addWidget(hbox);
    layout->addWidget(d->targetSearchButton);
    layout->addWidget(d->imageList);
    layout->setSpacing(qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                             QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing)));
    layout->setContentsMargins(QMargins());

    // ------------------------------------------------------------------------

    connect(d->targetSearchButton, SIGNAL(clicked(bool)),
            this, SLOT(slotShowTargetDialogClicked(bool)));

    connect(d->targetLabel, SIGNAL(textChanged(QString)),
            this, SLOT(slotLabelUrlChanged()));

    // ------------------------------------------------------------------------

    updateTargetLabel();
}

FTExportWidget::~FTExportWidget()
{
    delete d;
}

QUrl FTExportWidget::targetUrl() const
{
    return d->targetUrl;
}

QList<QUrl> FTExportWidget::history() const
{
    QList<QUrl> urls;

    for (int i = 0 ; i <= d->targetLabel->comboBox()->count() ; ++i)
    {
        urls << QUrl(d->targetLabel->comboBox()->itemText(i));
    }

    return urls;
}

void FTExportWidget::setHistory(const QList<QUrl>& urls)
{
    d->targetLabel->comboBox()->clear();

    Q_FOREACH (const QUrl& url, urls)
    {
        d->targetLabel->comboBox()->addUrl(url);
    }
}

void FTExportWidget::setTargetUrl(const QUrl& url)
{
    d->targetUrl = url;
    updateTargetLabel();
}

void FTExportWidget::slotShowTargetDialogClicked(bool checked)
{
    Q_UNUSED(checked);

    QPointer<DFileDialog> targetDialog = new DFileDialog(this, i18nc("@title:window", "Select Target..."),
                                                         d->targetUrl.toString(), i18n("All Files (*)"));
    targetDialog->setAcceptMode(QFileDialog::AcceptSave);
    targetDialog->setFileMode(QFileDialog::Directory);
    targetDialog->setOptions(QFileDialog::ShowDirsOnly);
    targetDialog->exec();

    if (targetDialog->hasAcceptedUrls())
    {
        d->targetUrl = targetDialog->selectedUrls().first();
        updateTargetLabel();

        Q_EMIT signalTargetUrlChanged(d->targetUrl);
    }

    delete targetDialog;
}

void FTExportWidget::updateTargetLabel()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Call for url "
                                     << d->targetUrl.toDisplayString()
                                     << ", valid = "
                                     << d->targetUrl.isValid();

    QString urlString = i18n("<i>not selected</i>");

    if (d->targetUrl.isValid())
    {
        urlString = d->targetUrl.toDisplayString();
        d->targetLabel->setUrl(QUrl(urlString));
    }
}

DItemsList* FTExportWidget::imagesList() const
{
    return d->imageList;
}

void FTExportWidget::slotLabelUrlChanged()
{
    d->targetUrl = d->targetLabel->url();

    Q_EMIT signalTargetUrlChanged(d->targetUrl);
}

} // namespace DigikamGenericFileTransferPlugin
