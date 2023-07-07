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

#ifndef DIGIKAM_SUBJECT_WIDGET_H
#define DIGIKAM_SUBJECT_WIDGET_H

// Qt includes

#include <QByteArray>
#include <QMap>
#include <QStringList>
#include <QUrl>
#include <QScrollArea>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>

// Local includes

#include "digikam_export.h"
#include "dtextedit.h"

namespace Digikam
{

class DIGIKAM_EXPORT SubjectData
{
public:

    SubjectData(const QString& n, const QString& m, const QString& d)
      : name  (n),
        matter(m),
        detail(d)
    {
    }

    QString name;         ///< English and Name of subject.
    QString matter;       ///< English and Matter Name of subject.
    QString detail;       ///< English and Detail Name of subject.
};

// --------------------------------------------------------------------------------

class DIGIKAM_EXPORT SubjectWidget : public QScrollArea
{
    Q_OBJECT

public:

    explicit SubjectWidget(QWidget* const parent, bool sizesLimited = false);
    ~SubjectWidget() override;

    void setSubjectsList(const QStringList& list);
    QStringList subjectsList() const;

Q_SIGNALS:

    void signalModified();

protected Q_SLOTS:

    virtual void slotSubjectsToggled(bool);
    virtual void slotRefChanged();
    virtual void slotEditOptionChanged(int);
    virtual void slotSubjectSelectionChanged();
    virtual void slotAddSubject();
    virtual void slotDelSubject();
    virtual void slotRepSubject();

protected:

    virtual bool loadSubjectCodesFromXML(const QUrl& url);
    virtual QString buildSubject() const;

protected:

    QLabel*    m_note;

    QCheckBox* m_subjectsCheck;

    QLineEdit* m_iprEdit;
    QLineEdit* m_refEdit;
    DTextEdit* m_nameEdit;
    DTextEdit* m_matterEdit;
    DTextEdit* m_detailEdit;

    QString    m_iprDefault;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_SUBJECT_WIDGET_H
