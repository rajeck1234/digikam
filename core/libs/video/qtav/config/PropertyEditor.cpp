/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-10-31
 * Description : QtAV: Multimedia framework based on Qt and FFmpeg
 *
 * SPDX-FileCopyrightText: 2012-2022 Wang Bin <wbsecg1 at gmail dot com>
 * SPDX-FileCopyrightText:      2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "PropertyEditor.h"

// Qt includes

#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QLayout>
#include <QScrollArea>
#include <QToolButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ClickableMenu.h"
#include "digikam_debug.h"

namespace QtAV
{

PropertyEditor::PropertyEditor(QObject* const parent)
    : QObject(parent)
{
}

void PropertyEditor::getProperties(QObject* const obj)
{
    mMetaProperties.clear();
    mProperties.clear();
    mPropertyDetails.clear();

    if (!obj)
        return;

    const QMetaObject* mo = obj->metaObject();

    for (int i = 0 ; i < mo->propertyCount() ; ++i)
    {
        QMetaProperty mp = mo->property(i);
        mMetaProperties.append(mp);
        QVariant v(mp.read(obj));

        if (mp.isEnumType())
        {
            mProperties.insert(QString::fromLatin1(mp.name()), v.toInt()); //mp.enumerator().valueToKey(v.toInt())); // always use string
        }
        else
        {
            mProperties.insert(QString::fromLatin1(mp.name()), v);
        }

        const QVariant detail = obj->property(QByteArray("detail_").append(mp.name()).constData());

        if (!detail.isNull())
            mPropertyDetails.insert(QString::fromLatin1(mp.name()), detail.toString());
    }

    mProperties.remove(QString::fromLatin1("objectName"));
}

void PropertyEditor::set(const QVariantHash& hash)
{
    QVariantHash::const_iterator it = mProperties.constBegin();

    for ( ; it != mProperties.constEnd() ; ++it)
    {
        QVariantHash::const_iterator ti = hash.find(it.key());

        if (ti == hash.constEnd())
            continue;

        mProperties[it.key()] = ti.value();
    }
}

void PropertyEditor::set(const QString&)
{
}

QString PropertyEditor::buildOptions()
{
    QString result;

    Q_FOREACH (const QMetaProperty& mp, mMetaProperties)
    {
        if (qstrcmp(mp.name(), "objectName") == 0)
            continue;

        result += QString::fromLatin1("  * %1: ").arg(QString::fromLatin1(mp.name()));

        if      (mp.isEnumType())
        {
            if (mp.isFlagType())
                result += QString::fromLatin1("flag ");
            else
                result += QString::fromLatin1("enum ");

            QMetaEnum me(mp.enumerator());

            for (int i = 0 ; i < me.keyCount() ; ++i)
            {
                result += QString::fromLatin1(me.key(i));
                result += QString::fromLatin1("=");
                result += QString::number(me.value(i));

                if (i < (me.keyCount() - 1))
                    result += QString::fromLatin1(",");
            }
        }
        else if (mp.type() == QVariant::Int)
        {
            result += QString::fromLatin1("int");
        }
        else if (mp.type() == QVariant::Double)
        {
            result += QString::fromLatin1("real");
        }
        else if (mp.type() == QVariant::String)
        {
            result += QString::fromLatin1("text");
        }
        else if (mp.type() == QVariant::Bool)
        {
            result += QString::fromLatin1("bool");
        }

        const QVariant detail = mPropertyDetails.value(QString::fromLatin1(mp.name()));

        if (!detail.isNull())
            result += QString::fromLatin1("\n    > property detail: %1").arg(detail.toString());

        result += QString::fromLatin1("\n");
    }

    return result;
}

QWidget* PropertyEditor::buildUi(QObject* const obj)
{
    if (mMetaProperties.isEmpty())
        return 0;

    QWidget* const w      = new QWidget();
    QGridLayout* const gl = new QGridLayout();

    w->setLayout(gl);
    int row               = 0;
    QVariant value;

    Q_FOREACH (const QMetaProperty& mp, mMetaProperties)
    {
        if (qstrcmp(mp.name(), "objectName") == 0)
            continue;

        value = mProperties[QString::fromLatin1(mp.name())];

        if      (mp.isEnumType())
        {
            if (mp.isFlagType())
            {
                gl->addWidget(createWidgetForFlags(QString::fromLatin1(mp.name()),
                              value, mp.enumerator(),
                              obj ? obj->property(QByteArray("detail_").append(mp.name()).constData()).toString() : QString()),
                              row, 0, Qt::AlignLeft | Qt::AlignVCenter);
            }
            else
            {
                gl->addWidget(new QLabel(i18n(mp.name())), row, 0, Qt::AlignRight | Qt::AlignVCenter);
                gl->addWidget(createWidgetForEnum(QString::fromLatin1(mp.name()),
                                                  value, mp.enumerator(),
                                                  obj ? obj->property(QByteArray("detail_").append(mp.name()).constData()).toString() : QString()),
                                                  row, 1, Qt::AlignLeft | Qt::AlignVCenter);
            }
        }
        else if ((mp.type() == QVariant::Int)      ||
                 (mp.type() == QVariant::UInt)     ||
                 (mp.type() == QVariant::LongLong) ||
                 (mp.type() == QVariant::ULongLong))
        {
            gl->addWidget(new QLabel(i18n(mp.name())), row, 0, Qt::AlignRight | Qt::AlignVCenter);
            gl->addWidget(createWidgetForInt(QString::fromLatin1(mp.name()), value.toInt(),
                                             obj ? obj->property(QByteArray("detail_").append(mp.name()).constData()).toString() : QString()),
                                             row, 1, Qt::AlignLeft | Qt::AlignVCenter);
        }
        else if (mp.type() == QVariant::Double)
        {
            gl->addWidget(new QLabel(i18n(mp.name())), row, 0, Qt::AlignRight | Qt::AlignVCenter);
            gl->addWidget(createWidgetForReal(QString::fromLatin1(mp.name()), value.toReal(),
                                              obj ? obj->property(QByteArray("detail_").append(mp.name()).constData()).toString() : QString()),
                                              row, 1, Qt::AlignLeft | Qt::AlignVCenter);
        }
        else if (mp.type() == QVariant::Bool)
        {
            gl->addWidget(createWidgetForBool(QString::fromLatin1(mp.name()), value.toBool(),
                                              obj ? obj->property(QByteArray("detail_").append(mp.name()).constData()).toString() : QString()),
                                              row, 0, 1, 2, Qt::AlignLeft);
        }
        else
        {
            gl->addWidget(new QLabel(i18n(mp.name())), row, 0, Qt::AlignRight | Qt::AlignVCenter);
            gl->addWidget(createWidgetForText(QString::fromLatin1(mp.name()), value.toString(), !mp.isWritable(),
                                              obj ? obj->property(QByteArray("detail_").append(mp.name()).constData()).toString() : QString()),
                                              row, 1, Qt::AlignLeft | Qt::AlignVCenter);
        }

        ++row;
    }

    return w;
}

QVariantHash PropertyEditor::exportAsHash()
{
    return mProperties;
}

QString PropertyEditor::exportAsConfig()
{
    return QString();
}

QWidget* PropertyEditor::createWidgetForFlags(const QString& name, const QVariant& value, QMetaEnum me, const QString& detail, QWidget* const parent)
{
    mProperties[name]      = value;
    QToolButton* const btn = new QToolButton(parent);

    if (!detail.isEmpty())
        btn->setToolTip(detail);

    btn->setObjectName(name);
    btn->setText(i18n(name.toUtf8().constData()));
    btn->setPopupMode(QToolButton::InstantPopup);
    ClickableMenu* const menu = new ClickableMenu(btn);
    menu->setObjectName(name);
    btn->setMenu(menu);

    for (int i = 0 ; i < me.keyCount() ; ++i)
    {
        QAction* const a = menu->addAction(QString::fromLatin1(me.key(i)));
        a->setCheckable(true);
        a->setData(me.value(i));
        a->setChecked(value.toInt() & me.value(i));
    }

    connect(menu, SIGNAL(triggered(QAction*)),
            this, SLOT(onFlagChange(QAction*)));

    return btn;
}

QWidget* PropertyEditor::createWidgetForEnum(const QString& name, const QVariant& value, QMetaEnum me, const QString& detail, QWidget* const parent)
{
    mProperties[name]    = value;
    QComboBox* const box = new QComboBox(parent);

    if (!detail.isEmpty())
        box->setToolTip(detail);

    box->setObjectName(name);
    box->setEditable(false);

    for (int i = 0 ; i < me.keyCount() ; ++i)
    {
        box->addItem(QString::fromLatin1(me.key(i)), me.value(i));
    }

    if (value.type() == QVariant::Int)
    {
        box->setCurrentIndex(box->findData(value));
    }
    else
    {
        box->setCurrentText(value.toString());
    }

    connect(box, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onEnumChange(int)));

    return box;
}

QWidget* PropertyEditor::createWidgetForInt(const QString& name, int value, const QString& detail, QWidget* const parent)
{
    mProperties[name]   = value;
    QSpinBox* const box = new QSpinBox(parent);

    if (!detail.isEmpty())
        box->setToolTip(detail);

    box->setObjectName(name);
    box->setValue(value);

    connect(box, SIGNAL(valueChanged(int)),
            this, SLOT(onIntChange(int)));

    return box;
}

QWidget* PropertyEditor::createWidgetForReal(const QString& name, qreal value, const QString& detail, QWidget* const parent)
{
    mProperties[name]         = value;
    QDoubleSpinBox* const box = new QDoubleSpinBox(parent);

    if (!detail.isEmpty())
        box->setToolTip(detail);

    box->setObjectName(name);
    box->setValue(value);

    connect(box, SIGNAL(valueChanged(double)),
            this, SLOT(onRealChange(qreal)));

    return box;
}

QWidget* PropertyEditor::createWidgetForText(const QString& name, const QString& value, bool readOnly, const QString& detail, QWidget* const parent)
{
    mProperties[name] = value;
    QWidget* w        = nullptr;

    if (readOnly)
    {
        QLabel* const label = new QLabel(parent);
        w                   = label;
        label->setText(value);
    }
    else
    {
        QLineEdit* const box = new QLineEdit(parent);
        w                    = box;
        box->setText(value);

        connect(box, SIGNAL(textChanged(QString)),
                this, SLOT(onTextChange(QString)));
    }

    if (!detail.isEmpty())
        w->setToolTip(detail);

    w->setObjectName(name);

    return w;
}

QWidget* PropertyEditor::createWidgetForBool(const QString& name, bool value, const QString& detail, QWidget* const parent)
{
    mProperties[name]    = value;
    QCheckBox* const box = new QCheckBox(i18n(name.toUtf8().constData()), parent);

    if (!detail.isEmpty())
        box->setToolTip(detail);

    box->setObjectName(name);
    box->setChecked(value);

    connect(box, SIGNAL(clicked(bool)),
            this, SLOT(onBoolChange(bool)));

    return box;
}

void PropertyEditor::updatePropertyValue(const QString& name, const QVariant& value)
{
    if (name.isEmpty())
        return;

    if (!mProperties.contains(name))
        return;

    qCDebug(DIGIKAM_AVPLAYER_LOG) << name << " >>> " << value;

    mProperties[name] = value;
}

void PropertyEditor::onFlagChange(QAction* action)
{
    int value = mProperties[sender()->objectName()].toInt();
    int flag  = action->data().toInt();

    if (action->isChecked())
    {
        value |= flag;
    }
    else
    {
        value &= ~flag;
    }

    updatePropertyValue(sender()->objectName(), value);
}

void PropertyEditor::onEnumChange(int value)
{
    QComboBox* const box = qobject_cast<QComboBox*>(sender());
    updatePropertyValue(sender()->objectName(), box->itemData(value));
}

void PropertyEditor::onIntChange(int value)
{
    updatePropertyValue(sender()->objectName(), value);
}

void PropertyEditor::onRealChange(qreal value)
{
    updatePropertyValue(sender()->objectName(), value);
}

void PropertyEditor::onTextChange(const QString& value)
{
    updatePropertyValue(sender()->objectName(), value);
}

void PropertyEditor::onBoolChange(bool value)
{
    updatePropertyValue(sender()->objectName(), value);
}

} // namespace QtAV
