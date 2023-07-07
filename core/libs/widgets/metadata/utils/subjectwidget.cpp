/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-15
 * Description : IPTC subjects editor.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at googlemail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "subjectwidget.h"

// Qt includes

#include <QStandardPaths>
#include <QFile>
#include <QValidator>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QListWidget>
#include <QDomDocument>
#include <QDomElement>
#include <QApplication>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN SubjectWidget::Private
{
public:

    enum EditionMode
    {
        STANDARD = 0,
        CUSTOM
    };

public:

    explicit Private()
      : optionsBox      (nullptr),
        addSubjectButton(nullptr),
        delSubjectButton(nullptr),
        repSubjectButton(nullptr),
        iprLabel        (nullptr),
        refLabel        (nullptr),
        nameLabel       (nullptr),
        matterLabel     (nullptr),
        detailLabel     (nullptr),
        btnGroup        (nullptr),
        stdBtn          (nullptr),
        customBtn       (nullptr),
        refCB           (nullptr),
        subjectsBox     (nullptr)
    {
    }

    typedef QMap<QString, SubjectData> SubjectCodesMap;

    SubjectCodesMap                    subMap;

    QStringList                        subjectsList;

    QWidget*                           optionsBox;

    QPushButton*                       addSubjectButton;
    QPushButton*                       delSubjectButton;
    QPushButton*                       repSubjectButton;

    QLabel*                            iprLabel;
    QLabel*                            refLabel;
    QLabel*                            nameLabel;
    QLabel*                            matterLabel;
    QLabel*                            detailLabel;

    QButtonGroup*                      btnGroup;

    QRadioButton*                      stdBtn;
    QRadioButton*                      customBtn;

    QComboBox*                         refCB;

    QListWidget*                       subjectsBox;
};

// --------------------------------------------------------------------------------

SubjectWidget::SubjectWidget(QWidget* const parent, bool sizeLimited)
    : QScrollArea(parent),
      d          (new Private)
{
    QWidget* const panel   = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    // Load subject codes provided by IPTC/NAA as xml file.
    // See iptc.cms.apa.at/std/topicset/topicset.iptc-subjectcode.xml for details.

    QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                          QLatin1String("digikam/metadata/topicset.iptc-subjectcode.xml"));

    // NOTE: use dynamic binding as this virtual method can be re-implemented in derived classes.

    if (!this->loadSubjectCodesFromXML(QUrl::fromLocalFile(path)))
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "Cannot load IPTC/NAA subject codes XML database";
    }

    // --------------------------------------------------------

    // Subject Reference Number only accept digit.

    QRegularExpression refDigitRx(QLatin1String("^[0-9]{8}$"));
    QValidator* const refValidator = new QRegularExpressionValidator(refDigitRx, this);

    // --------------------------------------------------------

    m_subjectsCheck        = new QCheckBox(i18nc("@title: subject widget", "Use structured definition of the subject matter:"), this);
    d->optionsBox          = new QWidget;
    d->btnGroup            = new QButtonGroup(this);
    d->stdBtn              = new QRadioButton;
    d->customBtn           = new QRadioButton;
    d->refCB               = new QComboBox;
    QLabel* const codeLink = new QLabel(QString::fromUtf8("%1 <b><a href='https://www.iptc.org/site/NewsCodes'>%2</a></b>")
                                        .arg(i18nc("@label: subject widget", "Use standard"))
                                        .arg(i18nc("@label: subject widget", "reference code")));
    codeLink->setOpenExternalLinks(true);
    codeLink->setWordWrap(false);

    // By default, check box is not visible.

    m_subjectsCheck->setVisible(false);
    m_subjectsCheck->setEnabled(false);

    QLabel* const customLabel = new QLabel(i18nc("@label: subject widget", "Use custom definition"));

    d->btnGroup->addButton(d->stdBtn,    Private::STANDARD);
    d->btnGroup->addButton(d->customBtn, Private::CUSTOM);
    d->btnGroup->setExclusive(true);
    d->stdBtn->setChecked(true);

    for (Private::SubjectCodesMap::Iterator it = d->subMap.begin() ; it != d->subMap.end() ; ++it)
    {
        d->refCB->addItem(it.key());
    }

    // --------------------------------------------------------

    m_iprEdit = new QLineEdit;
    m_iprEdit->setClearButtonEnabled(true);
    m_iprEdit->setPlaceholderText(i18nc("@info", "Set here the Informative Provider Reference"));

    if (sizeLimited)
    {
        m_iprEdit->setMaxLength(32);
    }

    // --------------------------------------------------------

    m_refEdit = new QLineEdit;
    m_refEdit->setClearButtonEnabled(true);
    m_refEdit->setValidator(refValidator);
    m_refEdit->setPlaceholderText(i18nc("@info", "Set here the Subject Reference Number"));

    if (sizeLimited)
    {
        m_refEdit->setMaxLength(8);
    }

    // --------------------------------------------------------

    m_nameEdit = new DTextEdit;
    m_nameEdit->setLinesVisible(1);
    m_nameEdit->setPlaceholderText(i18nc("@info", "Set here the Subject Name"));

    if (sizeLimited)
    {
        m_nameEdit->setMaxLength(64);
    }

    // --------------------------------------------------------

    m_matterEdit = new DTextEdit;
    m_matterEdit->setLinesVisible(1);
    m_matterEdit->setPlaceholderText(i18nc("@info", "Set here the Subject Matter Name"));

    if (sizeLimited)
    {
        m_matterEdit->setMaxLength(64);
    }

    // --------------------------------------------------------

    m_detailEdit = new DTextEdit;
    m_detailEdit->setLinesVisible(1);
    m_detailEdit->setPlaceholderText(i18nc("@info", "Set here the Subject Detail Name"));

    if (sizeLimited)
    {
        m_detailEdit->setMaxLength(64);
    }

    // --------------------------------------------------------

    d->iprLabel    = new QLabel(i18nc("@label: Information Provider Reference: "
                                      "A name, registered with the IPTC/NAA, "
                                      "identifying the provider that guarantees "
                                      "the uniqueness of the UNO", "I.P.R:"));
    d->refLabel    = new QLabel(i18nc("@label: subject properties", "Reference:"));
    d->nameLabel   = new QLabel(i18nc("@label: subject properties", "Name:"));
    d->matterLabel = new QLabel(i18nc("@label: subject properties", "Matter:"));
    d->detailLabel = new QLabel(i18nc("@label: subject properties", "Detail:"));

    // --------------------------------------------------------

    d->subjectsBox = new QListWidget;
    d->subjectsBox->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    d->addSubjectButton = new QPushButton(i18nc("@action: subject properties", "&Add"));
    d->delSubjectButton = new QPushButton(i18nc("@action: subject properties", "&Delete"));
    d->repSubjectButton = new QPushButton(i18nc("@action: subject properties", "&Replace"));
    d->addSubjectButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")).pixmap(16, 16));
    d->delSubjectButton->setIcon(QIcon::fromTheme(QLatin1String("edit-delete")).pixmap(16, 16));
    d->repSubjectButton->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")).pixmap(16, 16));
    d->delSubjectButton->setEnabled(false);
    d->repSubjectButton->setEnabled(false);

    // --------------------------------------------------------

    m_note = new QLabel;
    m_note->setMaximumWidth(150);
    m_note->setOpenExternalLinks(true);
    m_note->setWordWrap(true);
    m_note->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    // --------------------------------------------------------

    QGridLayout* const optionsBoxLayout = new QGridLayout;
    optionsBoxLayout->addWidget(d->stdBtn,      0, 0, 1, 1);
    optionsBoxLayout->addWidget(codeLink,       0, 1, 1, 2);
    optionsBoxLayout->addWidget(d->refCB,       0, 3, 1, 1);
    optionsBoxLayout->addWidget(d->customBtn,   1, 0, 1, 4);
    optionsBoxLayout->addWidget(customLabel,    1, 1, 1, 4);
    optionsBoxLayout->addWidget(d->iprLabel,    2, 0, 1, 1);
    optionsBoxLayout->addWidget(m_iprEdit,      2, 1, 1, 4);
    optionsBoxLayout->addWidget(d->refLabel,    3, 0, 1, 1);
    optionsBoxLayout->addWidget(m_refEdit,      3, 1, 1, 1);
    optionsBoxLayout->addWidget(d->nameLabel,   4, 0, 1, 1);
    optionsBoxLayout->addWidget(m_nameEdit,     4, 1, 1, 4);
    optionsBoxLayout->addWidget(d->matterLabel, 5, 0, 1, 1);
    optionsBoxLayout->addWidget(m_matterEdit,   5, 1, 1, 4);
    optionsBoxLayout->addWidget(d->detailLabel, 6, 0, 1, 1);
    optionsBoxLayout->addWidget(m_detailEdit,   6, 1, 1, 4);
    optionsBoxLayout->setColumnStretch(4, 10);
    optionsBoxLayout->setContentsMargins(QMargins());
    optionsBoxLayout->setSpacing(qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                                      QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing)));
    d->optionsBox->setLayout(optionsBoxLayout);

    // --------------------------------------------------------

    QGridLayout* const mainLayout = new QGridLayout(widget());
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->addWidget(m_subjectsCheck,     0, 0, 1, 4);
    mainLayout->addWidget(d->optionsBox,       1, 0, 1, 4);
    mainLayout->addWidget(d->subjectsBox,      2, 0, 5, 3);
    mainLayout->addWidget(d->addSubjectButton, 2, 3, 1, 1);
    mainLayout->addWidget(d->delSubjectButton, 3, 3, 1, 1);
    mainLayout->addWidget(d->repSubjectButton, 4, 3, 1, 1);
    mainLayout->addWidget(m_note,              5, 3, 1, 1);
    mainLayout->setRowStretch(6, 10);
    mainLayout->setColumnStretch(2, 1);

    int spacing = qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                       QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));

    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->subjectsBox, &QListWidget::itemSelectionChanged,
            this, &SubjectWidget::slotSubjectSelectionChanged);

    connect(d->addSubjectButton, &QPushButton::clicked,
            this, &SubjectWidget::slotAddSubject);

    connect(d->delSubjectButton, &QPushButton::clicked,
            this, &SubjectWidget::slotDelSubject);

    connect(d->repSubjectButton, &QPushButton::clicked,
            this, &SubjectWidget::slotRepSubject);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))

    connect(d->btnGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::idReleased),
            this, &SubjectWidget::slotEditOptionChanged);

#else

    connect(d->btnGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonReleased),
            this, &SubjectWidget::slotEditOptionChanged);

#endif

    connect(d->refCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this, &SubjectWidget::slotRefChanged);

    // --------------------------------------------------------

    connect(m_subjectsCheck, &QCheckBox::toggled,
            this, &SubjectWidget::slotSubjectsToggled);

    // --------------------------------------------------------

    connect(m_subjectsCheck, &QCheckBox::toggled,
            this, &SubjectWidget::signalModified);

    connect(d->addSubjectButton, &QPushButton::clicked,
            this, &SubjectWidget::signalModified);

    connect(d->delSubjectButton, &QPushButton::clicked,
            this, &SubjectWidget::signalModified);

    connect(d->repSubjectButton, &QPushButton::clicked,
            this, &SubjectWidget::signalModified);

    // --------------------------------------------------------

    // NOTE: use dynamic binding as this virtual method can be re-implemented in derived classes.

    this->slotEditOptionChanged(d->btnGroup->id(d->btnGroup->checkedButton()));
}

SubjectWidget::~SubjectWidget()
{
    delete d;
}

void SubjectWidget::slotSubjectsToggled(bool b)
{
    d->optionsBox->setEnabled(b);
    d->subjectsBox->setEnabled(b);
    d->addSubjectButton->setEnabled(b);
    d->delSubjectButton->setEnabled(b);
    d->repSubjectButton->setEnabled(b);
    slotEditOptionChanged(d->btnGroup->id(d->btnGroup->checkedButton()));
}

void SubjectWidget::slotEditOptionChanged(int b)
{
    if (b == Private::CUSTOM)
    {
        d->refCB->setEnabled(false);
        m_iprEdit->setEnabled(true);
        m_refEdit->setEnabled(true);
        m_nameEdit->setEnabled(true);
        m_matterEdit->setEnabled(true);
        m_detailEdit->setEnabled(true);
    }
    else
    {
        d->refCB->setEnabled(true);
        m_iprEdit->setEnabled(false);
        m_refEdit->setEnabled(false);
        m_nameEdit->setEnabled(false);
        m_matterEdit->setEnabled(false);
        m_detailEdit->setEnabled(false);
        slotRefChanged();
    }
}

void SubjectWidget::slotRefChanged()
{
    QString key = d->refCB->currentText();
    QString name, matter, detail;

    for (Private::SubjectCodesMap::Iterator it = d->subMap.begin() ;
         it != d->subMap.end() ; ++it)
    {
        if (key == it.key())
        {
            name   = it.value().name;
            matter = it.value().matter;
            detail = it.value().detail;
        }
    }

    m_refEdit->setText(key);
    m_nameEdit->setText(name);
    m_matterEdit->setText(matter);
    m_detailEdit->setText(detail);
}

QString SubjectWidget::buildSubject() const
{
    QString subject = m_iprEdit->text();

    subject.append(QLatin1Char(':'));
    subject.append(m_refEdit->text());
    subject.append(QLatin1Char(':'));
    subject.append(m_nameEdit->text());
    subject.append(QLatin1Char(':'));
    subject.append(m_matterEdit->text());
    subject.append(QLatin1Char(':'));
    subject.append(m_detailEdit->text());

    if (subject.right(4) == QLatin1String("::::"))
    {
        subject.clear();
    }

    return subject;
}

void SubjectWidget::slotDelSubject()
{
    QListWidgetItem* const item = d->subjectsBox->currentItem();

    if (!item)
    {
        return;
    }

    d->subjectsBox->takeItem(d->subjectsBox->row(item));

    delete item;
}

void SubjectWidget::slotRepSubject()
{
    QString newSubject = buildSubject();

    if (newSubject.isEmpty())
    {
        return;
    }

    if (!d->subjectsBox->selectedItems().isEmpty())
    {
        d->subjectsBox->selectedItems()[0]->setText(newSubject);
        m_iprEdit->setText(m_iprDefault);
        m_refEdit->clear();
        m_nameEdit->clear();
        m_matterEdit->clear();
        m_detailEdit->clear();
    }
}

void SubjectWidget::slotSubjectSelectionChanged()
{
    if (!d->subjectsBox->selectedItems().isEmpty())
    {
        QString subject = d->subjectsBox->selectedItems()[0]->text();
        m_iprEdit->setText(subject.section(QLatin1Char(':'), 0, 0));
        m_refEdit->setText(subject.section(QLatin1Char(':'), 1, 1));
        m_nameEdit->setText(subject.section(QLatin1Char(':'), 2, 2));
        m_matterEdit->setText(subject.section(QLatin1Char(':'), 3, 3));
        m_detailEdit->setText(subject.section(QLatin1Char(':'), 4, 4));
        d->delSubjectButton->setEnabled(true);
        d->repSubjectButton->setEnabled(true);
    }
    else
    {
        d->delSubjectButton->setEnabled(false);
        d->repSubjectButton->setEnabled(false);
    }
}

void SubjectWidget::slotAddSubject()
{
    QString newSubject = buildSubject();

    if (newSubject.isEmpty())
    {
        return;
    }

    bool found = false;

    for (int i = 0 ; i < d->subjectsBox->count() ; ++i)
    {
        QListWidgetItem* const item = d->subjectsBox->item(i);

        if (newSubject == item->text())
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        d->subjectsBox->insertItem(d->subjectsBox->count(), newSubject);
        m_iprEdit->setText(m_iprDefault);
        m_refEdit->clear();
        m_nameEdit->clear();
        m_matterEdit->clear();
        m_detailEdit->clear();
    }
}

bool SubjectWidget::loadSubjectCodesFromXML(const QUrl& url)
{
    QFile xmlfile(url.toLocalFile());

    if (!xmlfile.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QDomDocument xmlDoc(QLatin1String("NewsML"));

    if (!xmlDoc.setContent(&xmlfile))
    {
        xmlfile.close();
        return false;
    }

    QDomElement xmlDocElem = xmlDoc.documentElement();

    if (xmlDocElem.tagName() != QLatin1String("NewsML"))
    {
        xmlfile.close();
        return false;
    }

    for (QDomNode nbE1 = xmlDocElem.firstChild() ;
         !nbE1.isNull() ; nbE1 = nbE1.nextSibling())
    {
        QDomElement newsItemElement = nbE1.toElement();

        if (newsItemElement.isNull())
        {
            continue;
        }

        if (newsItemElement.tagName() != QLatin1String("NewsItem"))
        {
            continue;
        }

        for (QDomNode nbE2 = newsItemElement.firstChild() ;
            !nbE2.isNull() ; nbE2 = nbE2.nextSibling())
        {
            QDomElement topicSetElement = nbE2.toElement();

            if (topicSetElement.isNull())
            {
                continue;
            }

            if (topicSetElement.tagName() != QLatin1String("TopicSet"))
            {
                continue;
            }

            for (QDomNode nbE3 = topicSetElement.firstChild();
                !nbE3.isNull(); nbE3 = nbE3.nextSibling())
            {
                QDomElement topicElement = nbE3.toElement();

                if (topicElement.isNull())
                {
                    continue;
                }

                if (topicElement.tagName() != QLatin1String("Topic"))
                {
                    continue;
                }

                QString type, name, matter, detail, ref;

                for (QDomNode nbE4 = topicElement.firstChild();
                    !nbE4.isNull(); nbE4 = nbE4.nextSibling())
                {
                    QDomElement topicSubElement = nbE4.toElement();

                    if (topicSubElement.isNull())
                    {
                        continue;
                    }

                    if (topicSubElement.tagName() == QLatin1String("TopicType"))
                    {
                        type = topicSubElement.attribute(QLatin1String("FormalName"));
                    }

                    if (topicSubElement.tagName() == QLatin1String("FormalName"))
                    {
                        ref = topicSubElement.text();
                    }

                    if (topicSubElement.tagName() == QLatin1String("Description") &&
                        topicSubElement.attribute(QLatin1String("Variant")) == QLatin1String("Name"))
                    {
                        if      (type == QLatin1String("Subject"))
                        {
                            name = topicSubElement.text();
                        }
                        else if (type == QLatin1String("SubjectMatter"))
                        {
                            matter = topicSubElement.text();
                        }
                        else if (type == QLatin1String("SubjectDetail"))
                        {
                            detail = topicSubElement.text();
                        }
                    }
                }

                d->subMap.insert(ref, SubjectData(name, matter, detail));
            }
        }
    }

    xmlfile.close();

    // Set the Subject Name everywhere on the map.

    for (Private::SubjectCodesMap::Iterator it = d->subMap.begin() ;
         it != d->subMap.end() ; ++it)
    {
        QString name, keyPrefix;

        if (it.key().endsWith(QLatin1String("00000")))
        {
            keyPrefix = it.key().left(3);
            name      = it.value().name;

            for (Private::SubjectCodesMap::Iterator it2 = d->subMap.begin() ;
                 it2 != d->subMap.end() ; ++it2)
            {
                if (it2.key().startsWith(keyPrefix) &&
                    !it2.key().endsWith(QLatin1String("00000")))
                {
                    it2.value().name = name;
                }
            }
        }
    }

    // Set the Subject Matter Name everywhere on the map.

    for (Private::SubjectCodesMap::Iterator it = d->subMap.begin() ;
         it != d->subMap.end() ; ++it)
    {
        QString matter, keyPrefix;

        if (it.key().endsWith(QLatin1String("000")))
        {
            keyPrefix = it.key().left(5);
            matter    = it.value().matter;

            for (Private::SubjectCodesMap::Iterator it2 = d->subMap.begin() ;
                it2 != d->subMap.end() ; ++it2)
            {
                if (it2.key().startsWith(keyPrefix) &&
                    !it2.key().endsWith(QLatin1String("000")))
                {
                    it2.value().matter = matter;
                }
            }
        }
    }

    return true;
}

void SubjectWidget::setSubjectsList(const QStringList& list)
{
    d->subjectsList = list;

    blockSignals(true);
    d->subjectsBox->clear();

    if (m_subjectsCheck->isEnabled())
    {
        m_subjectsCheck->setChecked(false);
    }

    if (!d->subjectsList.isEmpty())
    {
        d->subjectsBox->insertItems(0, d->subjectsList);

        if (m_subjectsCheck->isEnabled())
        {
            m_subjectsCheck->setChecked(true);
        }
    }

    blockSignals(false);

    if (m_subjectsCheck->isEnabled())
    {
        slotSubjectsToggled(m_subjectsCheck->isChecked());
    }
}

QStringList SubjectWidget::subjectsList() const
{
    QStringList newSubjects;

    for (int i = 0 ; i < d->subjectsBox->count() ; ++i)
    {
        QListWidgetItem* item = d->subjectsBox->item(i);
        newSubjects.append(item->text());
    }

    return newSubjects;
}

} // namespace Digikam
