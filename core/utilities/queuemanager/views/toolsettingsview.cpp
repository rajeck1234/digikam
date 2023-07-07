/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-27
 * Description : a view to show Batch Tool Settings.
 *
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "toolsettingsview.h"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QFont>
#include <QPushButton>
#include <QScrollArea>
#include <QString>
#include <QIcon>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "thememanager.h"
#include "batchtoolsfactory.h"
#include "dpluginbqm.h"
#include "dpluginaboutdlg.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class Q_DECL_HIDDEN ToolSettingsView::Private
{

public:

    enum ToolSettingsViewMode
    {
        MessageView = 0,
        SettingsView
    };

public:

    explicit Private()
      : messageView      (nullptr),
        settingsViewIcon (nullptr),
        settingsViewTitle(nullptr),
        settingsViewReset(nullptr),
        settingsViewHelp (nullptr),
        settingsViewAbout(nullptr),
        settingsView     (nullptr),
        tool             (nullptr)
    {
    }

    QLabel*      messageView;
    QLabel*      settingsViewIcon;
    QLabel*      settingsViewTitle;

    QPushButton* settingsViewReset;
    QPushButton* settingsViewHelp;
    QPushButton* settingsViewAbout;

    QScrollArea* settingsView;

    BatchToolSet set;
    BatchTool*   tool;
};

ToolSettingsView::ToolSettingsView(QWidget* const parent)
    : QStackedWidget(parent),
      d             (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    // --------------------------------------------------------------------------

    d->messageView = new QLabel(this);
    d->messageView->setAlignment(Qt::AlignCenter);

    insertWidget(Private::MessageView, d->messageView);

    // --------------------------------------------------------------------------

    DVBox* const vbox            = new DVBox(this);
    QFrame* const toolDescriptor = new QFrame(vbox);
    d->settingsViewIcon          = new QLabel();
    d->settingsViewTitle         = new QLabel();
    QFont font                   = d->settingsViewTitle->font();
    font.setBold(true);
    d->settingsViewTitle->setFont(font);

    d->settingsViewReset = new QPushButton();
    d->settingsViewReset->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->settingsViewReset->setToolTip(i18n("Reset current tool settings to default values."));

    d->settingsViewHelp = new QPushButton();
    d->settingsViewHelp->setIcon(QIcon::fromTheme(QLatin1String("globe")));
    d->settingsViewHelp->setToolTip(i18n("Tool online handbook..."));

    d->settingsViewAbout = new QPushButton();
    d->settingsViewAbout->setIcon(QIcon::fromTheme(QLatin1String("help-about")));
    d->settingsViewAbout->setToolTip(i18n("About this tool..."));

    QString frameStyle = QString::fromUtf8("QFrame {"
                                           "color: %1;"
                                           "border: 1px solid %2;"
                                           "border-radius: 5px;"
                                           "background-color: %3;"
                                           "}")
                         .arg(qApp->palette().color(QPalette::HighlightedText).name())
                         .arg(qApp->palette().color(QPalette::HighlightedText).name())
                         .arg(qApp->palette().color(QPalette::Highlight).name());

    QString noFrameStyle = QLatin1String("QFrame {"
                                         "border: none;"
                                         "}");

    d->settingsViewIcon->setStyleSheet(noFrameStyle);
    d->settingsViewTitle->setStyleSheet(noFrameStyle);
    d->settingsViewReset->setStyleSheet(noFrameStyle);
    d->settingsViewHelp->setStyleSheet(noFrameStyle);
    d->settingsViewAbout->setStyleSheet(noFrameStyle);
    toolDescriptor->setStyleSheet(frameStyle);

    d->settingsViewIcon->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    d->settingsViewTitle->setAlignment(Qt::AlignCenter);

    QGridLayout* const descrLayout = new QGridLayout();
    descrLayout->addWidget(d->settingsViewIcon,  0, 0, 1, 1);
    descrLayout->addWidget(d->settingsViewTitle, 0, 1, 1, 1);
    descrLayout->addWidget(d->settingsViewReset, 0, 2, 1, 1);
    descrLayout->addWidget(d->settingsViewHelp , 0, 3, 1, 1);
    descrLayout->addWidget(d->settingsViewAbout, 0, 4, 1, 1);
    descrLayout->setColumnStretch(1, 10);
    toolDescriptor->setLayout(descrLayout);

    // --------------------------------------------------------------------------

    d->settingsView = new QScrollArea(vbox);
    d->settingsView->setWidgetResizable(true);

    vbox->setSpacing(0);
    vbox->setContentsMargins(QMargins());
    vbox->setStretchFactor(d->settingsView, 10);

    insertWidget(Private::SettingsView, vbox);
    setToolSettingsWidget(new QWidget(this));
    setViewMode(Private::MessageView);

    // --------------------------------------------------------------------------

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(d->settingsViewAbout, SIGNAL(clicked()),
            this, SLOT(slotAboutPlugin()));

    connect(d->settingsViewHelp, SIGNAL(clicked()),
            this, SLOT(slotHelpPlugin()));
}

ToolSettingsView::~ToolSettingsView()
{
    Q_FOREACH (BatchTool* const tool, BatchToolsFactory::instance()->toolsList())
    {
        tool->deleteSettingsWidget();
    }

    delete d;
}

void ToolSettingsView::slotAboutPlugin()
{
    if (d->tool)
    {
        if (d->tool->plugin())
        {
            QPointer<DPluginAboutDlg> dlg = new DPluginAboutDlg(dynamic_cast<DPlugin*>(d->tool->plugin()));
            dlg->exec();
            delete dlg;
        }
    }
}

void ToolSettingsView::slotHelpPlugin()
{
    if (d->tool)
    {
        if (d->tool->plugin())
        {
            openOnlineDocumentation(d->tool->plugin()->handbookSection(),
                                        d->tool->plugin()->handbookChapter(),
                                        d->tool->plugin()->handbookReference());
        }
    }
}

void ToolSettingsView::setBusy(bool b)
{
    d->settingsView->viewport()->setEnabled(!b);
    d->settingsViewReset->setEnabled(!b);
}

void ToolSettingsView::setToolSettingsWidget(QWidget* const w)
{
    QWidget* wdt = nullptr;

    if (!w)
    {
        wdt = new QWidget;
    }
    else
    {
        wdt = w;
    }

    d->settingsView->takeWidget();
    wdt->setParent(d->settingsView->viewport());
    d->settingsView->setWidget(wdt);
    setViewMode(Private::SettingsView);
}

void ToolSettingsView::slotThemeChanged()
{
    QPalette palette;
    palette.setColor(d->messageView->backgroundRole(), qApp->palette().color(QPalette::Base));
    d->messageView->setPalette(palette);

    QPalette palette2;
    palette2.setColor(d->settingsView->backgroundRole(), qApp->palette().color(QPalette::Base));
    d->settingsView->setPalette(palette2);
}

int ToolSettingsView::viewMode() const
{
    return indexOf(currentWidget());
}

void ToolSettingsView::setViewMode(int mode)
{
    if ((mode != Private::MessageView) && (mode != Private::SettingsView))
    {
        return;
    }

    if (mode == Private::MessageView)
    {
        d->settingsViewReset->setEnabled(false);
    }
    else
    {
        d->settingsViewReset->setEnabled(true);
    }

    setCurrentIndex(mode);
}

void ToolSettingsView::slotToolSelected(const BatchToolSet& set)
{
    d->set = set;

    if (d->tool)
    {
        disconnect(d->tool, SIGNAL(signalSettingsChanged(BatchToolSettings)),
                   this, SLOT(slotSettingsChanged(BatchToolSettings)));

        disconnect(d->settingsViewReset, SIGNAL(clicked()),
                   d->tool, SLOT(slotResetSettingsToDefault()));
    }

    d->tool = BatchToolsFactory::instance()->findTool(d->set.name, d->set.group);

    if (d->tool)
    {
        d->settingsViewIcon->setPixmap(d->tool->toolIcon().pixmap(22));
        d->settingsViewTitle->setText(d->tool->toolTitle());
        d->tool->setSettings(d->set.settings);

        // Only set on Reset button if Manager is not busy (settings widget is disabled in this case).

        d->settingsViewReset->setEnabled(d->settingsView->viewport()->isEnabled());

        d->settingsViewAbout->setEnabled(d->tool->plugin());

        setToolSettingsWidget(d->tool->settingsWidget());

        connect(d->tool, SIGNAL(signalSettingsChanged(BatchToolSettings)),
                this, SLOT(slotSettingsChanged(BatchToolSettings)));

        connect(d->settingsViewReset, SIGNAL(clicked()),
                d->tool, SLOT(slotResetSettingsToDefault()));
    }
    else
    {
        d->settingsViewIcon->clear();
        d->settingsViewTitle->clear();
        d->settingsViewReset->setEnabled(false);
        setToolSettingsWidget(nullptr);
    }
}

void ToolSettingsView::slotSettingsChanged(const BatchToolSettings& settings)
{
    d->set.settings = settings;

    Q_EMIT signalSettingsChanged(d->set);
}

} // namespace Digikam
