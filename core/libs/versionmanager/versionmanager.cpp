/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-18
 * Description : class for determining new file name in terms of version management
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "versionmanager.h"

// Qt includes

#include <QDir>
#include <QUrl>

// KDE includes

#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "dimgfiltermanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN VersionNameCreator
{
public:

    VersionNameCreator(const VersionFileInfo& loadedFile,
                       const DImageHistory& m_resolvedInitialHistory, const DImageHistory& m_currentHistory,
                       VersionManager* const q);

    void checkNeedNewVersion();
    void fork();
    void setSaveDirectory();
    void setSaveFormat();
    void setSaveFileName();
    void setSaveDirectory(const QString& path);
    void setSaveFormat(const QString& format);
    void setSaveFileName(const QString& name);
    void initOperation();
    void checkIntermediates();

protected:

    VersionFileInfo nextIntermediate(const QString& format);
    void setFileSuffix(QString& fileName, const QString& format) const;
    void addFileSuffix(QString& baseName, const QString& format, const QString& originalName = QString()) const;

public:

    VersionManagerSettings m_settings;

    VersionFileInfo        m_result;
    VersionFileInfo        m_loadedFile;

    VersionFileOperation   m_operation;

    const DImageHistory    m_resolvedInitialHistory;
    const DImageHistory    m_currentHistory;

    bool                   m_fromRaw;
    bool                   m_newVersion;

    QVariant               m_version;
    QVariant               m_intermediateCounter;
    QString                m_baseName;
    QString                m_intermediatePath;

    VersionManager* const  q;
};

VersionNameCreator::VersionNameCreator(const VersionFileInfo& loadedFile,
                                       const DImageHistory& m_resolvedInitialHistory,
                                       const DImageHistory& m_currentHistory,
                                       VersionManager* const q)
    : m_settings(q->settings()),
      m_loadedFile(loadedFile),
      m_resolvedInitialHistory(m_resolvedInitialHistory),
      m_currentHistory(m_currentHistory),
      m_fromRaw(false),
      m_newVersion(false), q(q)
{
    m_loadedFile.format   = m_loadedFile.format.toUpper();
    m_fromRaw             = (m_loadedFile.format.startsWith(QLatin1String("RAW"))); // also accept RAW-... format
    m_version             = q->namingScheme()->initialCounter();
    m_intermediateCounter = q->namingScheme()->initialCounter();
}

void VersionNameCreator::checkNeedNewVersion()
{
    // First we check if we have any other files available.
    // The resolved initial history contains only referred files found in the collection
    // Note: The loaded file will have type Current

    qCDebug(DIGIKAM_GENERAL_LOG) << m_resolvedInitialHistory.hasReferredImageOfType(HistoryImageId::Original)
                                 << m_resolvedInitialHistory.hasReferredImageOfType(HistoryImageId::Intermediate)
                                 << m_fromRaw << q->workspaceFileFormats().contains(m_loadedFile.format);

    if       (!m_resolvedInitialHistory.hasReferredImageOfType(HistoryImageId::Original) &&
              !m_resolvedInitialHistory.hasReferredImageOfType(HistoryImageId::Intermediate))
    {
        m_newVersion = true;
    }
    else if (m_fromRaw || !q->workspaceFileFormats().contains(m_loadedFile.format))
    {
        // We check the loaded format: If it is not one of the workspace formats, or even raw, we need a new version
        m_newVersion = true;
    }
    else
    {
        m_newVersion = false;
    }
}

void VersionNameCreator::fork()
{
    m_newVersion = true;
}

void VersionNameCreator::setSaveDirectory()
{
    m_result.path      = q->namingScheme()->directory(m_loadedFile.path, m_loadedFile.fileName);
    m_intermediatePath = q->namingScheme()->directory(m_loadedFile.path, m_loadedFile.fileName);
}

void VersionNameCreator::setSaveDirectory(const QString& path)
{
    m_result.path      = path;
    m_intermediatePath = path;
}

void VersionNameCreator::setSaveFormat()
{
    m_result.format = m_settings.format;
/*
    if (m_fromRaw)
    {
        m_result.format = m_settings.formatForStoringRAW;
    }
    else
    {
        m_result.format = m_settings.formatForStoringSubversions;
    }
*/
}

void VersionNameCreator::setSaveFormat(const QString& override)
{
    m_result.format = override;
}

void VersionNameCreator::setSaveFileName()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "need new version" << m_newVersion;

    VersionNamingScheme* const scheme = q->namingScheme();

    // initialize m_baseName, m_version, and m_intermediateCounter for intermediates

    m_baseName                        = scheme->baseName(m_loadedFile.path,
                                                         m_loadedFile.fileName,
                                                         &m_version,
                                                         &m_intermediateCounter);

    qCDebug(DIGIKAM_GENERAL_LOG) << "analyzing file" << m_loadedFile.fileName << m_version << m_intermediateCounter;

    if (!m_newVersion)
    {
        m_result.fileName = m_loadedFile.fileName;

        if (m_loadedFile.format != m_result.format)
        {
            setFileSuffix(m_result.fileName, m_result.format);
        }
    }
    else
    {
        QDir dirInfo(m_result.path);

        // To find the right number for the new version, go through all the items in the given dir,
        // the version number won't be bigger than count()

        for (uint i = 0 ; i <= dirInfo.count() ; ++i)
        {
            QString suggestedName = scheme->versionFileName(m_result.path, m_baseName, m_version);

            // Note: Always give a hard guarantee that the file does not exist

            if (dirInfo.entryList(QStringList() << suggestedName + QLatin1String(".*"), QDir::Files).isEmpty())
            {
                m_result.fileName = suggestedName;
                addFileSuffix(m_result.fileName, m_result.format, m_loadedFile.fileName);
                break;
            }

            // increment for next attempt

            m_version = scheme->incrementedCounter(m_version);
        }
    }
}

void VersionNameCreator::setSaveFileName(const QString& fileName)
{
    m_result.fileName = fileName;
    m_baseName        = fileName.section(QLatin1Char('.'), 0, 0);

    // m_version remains unknown
}

void VersionNameCreator::initOperation()
{
    m_operation.loadedFile = m_loadedFile;
    m_operation.saveFile   = m_result;

    if (m_newVersion)
    {
        m_operation.tasks |= VersionFileOperation::NewFile;
    }
    else
    {
        if (m_result.fileName == m_loadedFile.fileName)
        {
            m_operation.tasks |= VersionFileOperation::Replace;
        }
        else
        {
            m_operation.tasks |= VersionFileOperation::SaveAndDelete;
        }
    }
}

void VersionNameCreator::checkIntermediates()
{
    // call when task has been determined

    qCDebug(DIGIKAM_GENERAL_LOG) << "Will replace" << bool(m_operation.tasks & VersionFileOperation::Replace)
                                 << "save after each session" << bool(m_settings.saveIntermediateVersions & VersionManagerSettings::AfterEachSession)
                                 << "save after raw" << bool(m_settings.saveIntermediateVersions & VersionManagerSettings::AfterRawConversion)
                                 << "save when not repro" << bool(m_settings.saveIntermediateVersions & VersionManagerSettings::WhenNotReproducible);

    if ((m_settings.saveIntermediateVersions & VersionManagerSettings::AfterEachSession) &&
        (m_operation.tasks & VersionFileOperation::Replace))
    {
        // We want a snapshot after each session. The main file will be overwritten by the current state.
        // So we consider the current file as snapshot of the last session and move
        // it to an intermediate before it is overwritten

        m_operation.tasks |= VersionFileOperation::MoveToIntermediate;
        m_operation.intermediateForLoadedFile = nextIntermediate(m_loadedFile.format);

        // this amounts to storing firstStep - 1
    }

    // These are, inclusively, the first and last step the state after which we may have to store.
    // m_resolvedInitialHistory.size() - 1 is the loaded file
    // m_currentHistory.size() - 1 is the current version

    int firstStep = m_resolvedInitialHistory.size();
    int lastStep  = m_currentHistory.size() - 2; // index of last but one entry

    qCDebug(DIGIKAM_GENERAL_LOG) << "initial history" << m_resolvedInitialHistory.size()
                                 << "current history" << m_currentHistory.size()
                                 << "first step" << firstStep << "last step" << lastStep;

    if (lastStep < firstStep)
    {
        // only a single editing step, or even "backwards" in history (possible with redo)
        return;
    }

    if (!firstStep)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Invalid history: resolved initial history has no entries";
        firstStep = 1;
    }

    if (m_settings.saveIntermediateVersions & VersionManagerSettings::AfterRawConversion)
    {
        int rawConversionStep = -1;

        for (int i = firstStep ; i <= lastStep ; ++i)
        {
            if (DImgFilterManager::instance()->isRawConversion(m_currentHistory.action(i).identifier()))
            {
                rawConversionStep = i;
                // break? multiple raw conversion steps??
            }
        }

        if (rawConversionStep != -1)
        {
            m_operation.intermediates.insert(rawConversionStep, VersionFileInfo());
        }
    }

    if (m_settings.saveIntermediateVersions & VersionManagerSettings::WhenNotReproducible)
    {
        for (int i = firstStep ; i <= lastStep ; ++i)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "step" << i
                                         << "is reproducible"
                                         << (m_currentHistory.action(i).category() == FilterAction::ReproducibleFilter);

            switch (m_currentHistory.action(i).category())
            {
                case FilterAction::ReproducibleFilter:
                    break;

                case FilterAction::ComplexFilter:
                case FilterAction::DocumentedHistory:
                    m_operation.intermediates.insert(i, VersionFileInfo());
                    break;
            }
        }
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Save intermediates after steps" << m_operation.intermediates.keys();

    if (!m_operation.intermediates.isEmpty())
    {
        m_operation.tasks |= VersionFileOperation::StoreIntermediates;

        // now all steps are available, already ordered thanks to QMap. Just fill in the empty VersionFileInfos.

        QMap<int,VersionFileInfo>::iterator it;

        for (it = m_operation.intermediates.begin() ; it != m_operation.intermediates.end() ; ++it)
        {
            it.value() = nextIntermediate(m_result.format);
        }
    }
}

VersionFileInfo VersionNameCreator::nextIntermediate(const QString& format)
{
    VersionNamingScheme* const scheme = q->namingScheme();

    VersionFileInfo intermediate;
    intermediate.path                 = m_intermediatePath;
    intermediate.format               = format;

    QDir dirInfo(m_intermediatePath);

    for (uint i = 0 ; i <= dirInfo.count() ; ++i)
    {
        QString suggestedName = scheme->intermediateFileName(m_intermediatePath, m_baseName,
                                                             m_version, m_intermediateCounter);

        // it is important to increment before returning - we may have to produce a number of files

        m_intermediateCounter = scheme->incrementedCounter(m_intermediateCounter);

        // Note: Always give a hard guarantee that the file does not exist

        if (dirInfo.entryList(QStringList() << suggestedName + QLatin1String(".*"), QDir::Files).isEmpty())
        {
            intermediate.fileName = suggestedName;
            setFileSuffix(intermediate.fileName, format);
            break;
        }
    }

    return intermediate;
}

void VersionNameCreator::setFileSuffix(QString& fileName, const QString& format) const
{
    if (fileName.isEmpty())
    {
        return;
    }

    // QFileInfo uses the same string manipulation

    int lastDot  = fileName.lastIndexOf(QLatin1Char('.'));
    bool isLower = true;

    if (lastDot == -1)
    {
        fileName += QLatin1Char('.');
        lastDot   = fileName.size() - 1;
    }
    else
    {
        isLower = fileName.at(fileName.size() - 1).isLower();
    }

    int suffixBegin = lastDot + 1;

    if (fileName.mid(suffixBegin).compare(format, Qt::CaseInsensitive))
    {
        fileName.replace(suffixBegin, fileName.size() - suffixBegin, isLower ? format.toLower() : format);
    }
}

void VersionNameCreator::addFileSuffix(QString& fileName, const QString& format, const QString& originalName) const
{
    if (fileName.isEmpty())
    {
        return;
    }

    bool isLower = true;

    if (!originalName.isEmpty())
    {
        // if original name had upper case suffix, continue using upper case

        isLower = originalName.at(originalName.size() - 1).isLower();
    }

    if (!fileName.endsWith(QLatin1Char('.')))
    {
        fileName += QLatin1Char('.');
    }

    fileName += (isLower ? format.toLower() : format);
}

// ------------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN VersionManager::VersionManagerPriv
{
public:

    VersionManagerPriv()
    {
        scheme = nullptr;
    }

    VersionManagerSettings     settings;
    VersionNamingScheme*       scheme;

    DefaultVersionNamingScheme defaultScheme;
};

// ------------------------------------------------------------------------------------------------------

VersionManager::VersionManager()
    : d(new VersionManagerPriv)
{
}

VersionManager::~VersionManager()
{
    delete d->scheme;
    delete d;
}

void VersionManager::setSettings(const VersionManagerSettings& settings)
{
    d->settings = settings;
}

VersionManagerSettings VersionManager::settings() const
{
    return d->settings;
}

void VersionManager::setNamingScheme(VersionNamingScheme* scheme)
{
    d->scheme = scheme;
}

VersionNamingScheme* VersionManager::namingScheme() const
{
    if (d->scheme)
    {
        return d->scheme;
    }
    else
    {
        return &d->defaultScheme;
    }
}

VersionFileOperation VersionManager::operation(FileNameType request, const VersionFileInfo& loadedFile,
                                               const DImageHistory& initialResolvedHistory,
                                               const DImageHistory& currentHistory)
{
    VersionNameCreator name(loadedFile, initialResolvedHistory, currentHistory, this);

    if (request == CurrentVersionName)
    {
        name.checkNeedNewVersion();
    }
    else if (request == NewVersionName)
    {
        name.fork();
    }

    name.setSaveDirectory();
    name.setSaveFormat();
    name.setSaveFileName();
    name.initOperation();
    name.checkIntermediates();

    return name.m_operation;
}

VersionFileOperation VersionManager::operationNewVersionInFormat(const VersionFileInfo& loadedFile,
                                                                 const QString& format,
                                                                 const DImageHistory& initialResolvedHistory,
                                                                 const DImageHistory& currentHistory)
{
    VersionNameCreator name(loadedFile, initialResolvedHistory, currentHistory, this);

    name.fork();
    name.setSaveDirectory();
    name.setSaveFormat(format);
    name.setSaveFileName();
    name.initOperation();
    name.checkIntermediates();

    return name.m_operation;
}

VersionFileOperation VersionManager::operationNewVersionAs(const VersionFileInfo& loadedFile,
                                                           const VersionFileInfo& saveLocation,
                                                           const DImageHistory& initialResolvedHistory,
                                                           const DImageHistory& currentHistory)
{
    VersionNameCreator name(loadedFile, initialResolvedHistory, currentHistory, this);

    name.fork();
    name.setSaveDirectory(saveLocation.path);
    name.setSaveFormat(saveLocation.format);
    name.setSaveFileName(saveLocation.fileName);
    name.initOperation();
    name.checkIntermediates();

    return name.m_operation;
}

QString VersionManager::toplevelDirectory(const QString& path)
{
    Q_UNUSED(path);

    return QLatin1String("/");
}

QStringList VersionManager::workspaceFileFormats() const
{
    QStringList formats;
    formats << QLatin1String("JPG") << QLatin1String("PNG") << QLatin1String("TIFF") << QLatin1String("PGF") << QLatin1String("JP2");
    QString f = d->settings.format.toUpper();

    if (!formats.contains(f))
    {
        formats << f;
    }
/*
    f = d->settings.formatForStoringSubversions.toUpper();

    if (!formats.contains(f))
    {
        formats << f;
    }
*/
    return formats;
}

} // namespace Digikam
