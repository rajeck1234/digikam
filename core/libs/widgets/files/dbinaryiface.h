/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-23
 * Description : Autodetect binary program and version
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DBINARY_IFACE_H
#define DIGIKAM_DBINARY_IFACE_H

// Qt includes

#include <QString>
#include <QStringList>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSet>
#include <QGridLayout>
#include <QUrl>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DBinaryIface : public QObject
{
    Q_OBJECT

public:

    DBinaryIface(const QString& binaryName,
                 const QString& projectName,
                 const QString& url,
                 const QString& pluginName,
                 const QStringList& args = QStringList(),
                 const QString& desc = QString()
                );
    DBinaryIface(const QString& binaryName,
                 const QString& minimalVersion,
                 const QString& header,
                 const int headerLine,
                 const QString& projectName,
                 const QString& url,
                 const QString& pluginName,
                 const QStringList& args = QStringList(),
                 const QString& desc = QString()
                );
    ~DBinaryIface() override;

public:

    bool                isFound()                   const { return m_isFound;                       }
    const QString&      version()                   const;
    bool                versionIsRight()            const;
    bool                versionIsRight(const float) const;
    inline bool         isValid()                   const { return (m_isFound && versionIsRight()); }
    inline bool         developmentVersion()        const { return m_developmentVersion;            }
    const QString&      description()               const { return m_description;                   }

    virtual void        setup(const QString& prev = QString());
    virtual bool        checkDir()                        { return checkDirForPath(m_pathDir);      }
    virtual bool        checkDirForPath(const QString& path);
    virtual bool        recheckDirectories();

    virtual QString     path(const QString& dir)    const;
    virtual QString     path()                      const { return path(m_pathDir);                 }
    virtual QString     directory()                 const { return m_pathDir;                       }
    virtual QString     baseName()                  const { return m_binaryBaseName;                }
    virtual QString     minimalVersion()            const { return m_minimalVersion;                }


    virtual QUrl        url()                       const { return m_url;                           }
    virtual QString     projectName()               const { return m_projectName;                   }

    static QString      goodBaseName(const QString& b);

public Q_SLOTS:

    virtual void        slotNavigateAndCheck();
    virtual void        slotAddPossibleSearchDirectory(const QString& dir);
    virtual void        slotAddSearchDirectory(const QString& dir);

Q_SIGNALS:

    void                signalSearchDirectoryAdded(const QString& dir);
    void                signalBinaryValid();

protected:

    QString             findHeader(const QStringList& output, const QString& header) const;
    virtual bool        parseHeader(const QString& output);
    void                setVersion(QString& version);

    virtual QString     readConfig();
    virtual void        writeConfig();

protected:

    const bool          m_checkVersion;
    const QString       m_headerStarts;
    const int           m_headerLine;
    const QString       m_minimalVersion;
    const QString       m_configGroup;
    const QString       m_binaryBaseName;
    const QStringList   m_binaryArguments;
    const QString       m_projectName;
    const QUrl          m_url;

    bool                m_isFound;
    bool                m_developmentVersion;

    QString             m_version;
    QString             m_pathDir;
    QString             m_description;

    QFrame*             m_pathWidget;
    QLabel*             m_binaryLabel;
    QLabel*             m_versionLabel;
    QPushButton*        m_pathButton;
    QLabel*             m_downloadButton;
    QLineEdit*          m_lineEdit;
    QLabel*             m_statusIcon;

    QSet<QString>       m_searchPaths;

private:

    // Disabled
    explicit DBinaryIface(QObject*);
};

} // namespace Digikam

#endif // DIGIKAM_DBINARY_IFACE_H
