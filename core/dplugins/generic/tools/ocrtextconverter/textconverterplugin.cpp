/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-26
 * Description : a plugin to convert documented images to text
 *
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2022      by Quoc Hung Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "textconverterplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "textconverterdialog.h"

namespace DigikamGenericTextConverterPlugin
{

TextConverterPlugin::TextConverterPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

TextConverterPlugin::~TextConverterPlugin()
{
}

QString TextConverterPlugin::name() const
{
    return i18nc("@title", "OCR Text Converter");
}

QString TextConverterPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon TextConverterPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("text-x-generic"));
}

QString TextConverterPlugin::description() const
{
    return i18nc("@info", "A tool to batch convert documented images to text using OCR");
}

QString TextConverterPlugin::details() const
{
    return i18nc("@info", "This tool can convert documented images data to Text format "
                 "by using Tesseract, an open-source Optical Characters Recognition engine, originally developed at HP labs and now sponsored by Google.\n\n"
                 "It supports multiple languages and scripts (including right-to-left text) and page layout analysis.\n\n"
                 "See details on %1 for more information on Tesseract",
                 QLatin1String("<a href='https://github.com/tesseract-ocr/tesseract'>https://github.com/tesseract-ocr/tesseract</a>"));
}

QString TextConverterPlugin::handbookSection() const
{
    return QLatin1String("post_processing");
}

QString TextConverterPlugin::handbookChapter() const
{
    return QLatin1String("ocrtext_converter");
}

QList<DPluginAuthor> TextConverterPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("TRAN Quoc Hung"),
                      QString::fromUtf8("quochungtran at gmail dot com"),
                      QString::fromUtf8("(C) 2022"))
            ;
}

void TextConverterPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "OCR Text Converter..."));
    ac->setObjectName(QLatin1String("textconverter"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac,   SIGNAL(triggered(bool)),
            this, SLOT(slotTextConverter()));

    addAction(ac);
}

void TextConverterPlugin::slotTextConverter()
{
    QPointer<TextConverterDialog> dialog = new TextConverterDialog(nullptr, infoIface(sender()));
    dialog->setPlugin(this);
    dialog->exec();
    delete dialog;
}

} // namespace DigikamGenericTextConverterPlugin
