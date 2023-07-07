/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : a class that manages the files to be renamed
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "advancedrenamemanager.h"

// C++ includes

#include <algorithm>

// Qt includes

#include <QMap>
#include <QList>
#include <QUuid>
#include <QFileInfo>
#include <QStorageInfo>
#include <QRegularExpression>

// Local includes

#include "digikam_debug.h"
#include "advancedrenamewidget.h"
#include "parsesettings.h"
#include "parser.h"
#include "defaultrenameparser.h"
#include "importrenameparser.h"
#include "iteminfo.h"

namespace Digikam
{

struct SortByNameCaseInsensitive
{
    bool operator() (const QString& s1, const QString& s2) const
    {
        return (s1.toLower() < s2.toLower());
    }
};

struct SortByDate
{
    bool operator() (const QString& s1, const QString& s2) const
    {
        QFileInfo fi1(s1);
        QFileInfo fi2(s2);

        return (fi1.birthTime() < fi2.birthTime());
    }
};

struct SortBySize
{
    bool operator() (const QString& s1, const QString& s2) const
    {
        QFileInfo fi1(s1);
        QFileInfo fi2(s2);

        return (fi1.size() < fi2.size());
    }
};

class Q_DECL_HIDDEN AdvancedRenameManager::Private
{
public:

    explicit Private()
      : parser       (nullptr),
        widget       (nullptr),
        parserType   (AdvancedRenameManager::DefaultParser),
        sortAction   (AdvancedRenameManager::SortCustom),
        sortDirection(AdvancedRenameManager::SortAscending),
        startIndex   (1),
        cutFileName  (0)
    {
    }

    QStringList                          files;

    QList<QString>                       randomList;

    QMap<QString, int>                   fileIndexMap;
    QMap<QString, int>                   folderIndexMap;
    QMap<QString, int>                   fileGroupIndexMap;
    QMap<QString, int>                   fileCounterIndexMap;

    QMap<QString, QDateTime>             fileDatesMap;
    QMap<QString, QString>               renamedFiles;

    Parser*                              parser;
    AdvancedRenameWidget*                widget;
    AdvancedRenameManager::ParserType    parserType;
    AdvancedRenameManager::SortAction    sortAction;
    AdvancedRenameManager::SortDirection sortDirection;

    int                                  startIndex;
    int                                  cutFileName;
};

AdvancedRenameManager::AdvancedRenameManager()
    : d(new Private)
{
    setParserType(DefaultParser);
}

AdvancedRenameManager::AdvancedRenameManager(const QList<ParseSettings>& files)
    : d(new Private)
{
    setParserType(DefaultParser);
    addFiles(files);
}

AdvancedRenameManager::~AdvancedRenameManager()
{
    clearAll();

    delete d->parser;
    delete d;
}

void AdvancedRenameManager::setSortAction(SortAction action)
{
    d->sortAction = action;
    initialize();

    QList<QUrl> list;

    Q_FOREACH (const QString& file, d->files)
    {
        list << QUrl::fromLocalFile(file);
    }

    Q_EMIT signalSortingChanged(list);
}

AdvancedRenameManager::SortAction AdvancedRenameManager::sortAction() const
{
    return d->sortAction;
}

void AdvancedRenameManager::setSortDirection(SortDirection direction)
{
    d->sortDirection = direction;
    initialize();

    QList<QUrl> list;

    Q_FOREACH (const QString& file, d->files)
    {
        list << QUrl::fromLocalFile(file);
    }

    Q_EMIT signalSortingChanged(list);
}

AdvancedRenameManager::SortDirection AdvancedRenameManager::sortDirection() const
{
    return d->sortDirection;
}

void AdvancedRenameManager::setStartIndex(int index)
{
    d->startIndex = (index < 1) ? 1 : index;
    initialize();
}

void AdvancedRenameManager::setCutFileName(int index)
{
    d->cutFileName = index;
}

void AdvancedRenameManager::setWidget(AdvancedRenameWidget* widget)
{
    if (!widget)
    {
        d->widget = nullptr;
        return;
    }

    d->widget = widget;
    setParserType(d->parserType);
}

void AdvancedRenameManager::setParserType(ParserType type)
{
    delete d->parser;

    if (type == ImportParser)
    {
        d->parser = new ImportRenameParser();
    }
    else
    {
        d->parser = new DefaultRenameParser();
    }

    d->parserType = type;

    if (d->widget)
    {
        d->widget->setParser(d->parser);

        if (type == ImportParser)
        {
            d->widget->setLayoutStyle(AdvancedRenameWidget::LayoutCompact);
        }
        else
        {
            d->widget->setLayoutStyle(AdvancedRenameWidget::LayoutNormal);
        }
    }
}

Parser* AdvancedRenameManager::getParser() const
{
    return d->parser;
}

void AdvancedRenameManager::parseFiles()
{
    if (!d->widget)
    {
        return;
    }

    parseFiles(d->widget->parseString());
}

void AdvancedRenameManager::parseFiles(const ParseSettings& _settings)
{
    if (!d->widget)
    {
        if (!(_settings.parseString.isEmpty()))
        {
            parseFiles(_settings.parseString, _settings);
        }
    }
    else
    {
        parseFiles(d->widget->parseString(), _settings);
    }
}

void AdvancedRenameManager::parseFiles(const QString& parseString)
{
    if (!d->parser)
    {
        return;
    }

    d->parser->reset();

    Q_FOREACH (const QString& file, d->files)
    {
        QUrl url              = QUrl::fromLocalFile(file);
        ParseSettings settings;
        settings.fileUrl      = url;
        settings.parseString  = parseString;
        settings.startIndex   = d->startIndex;
        settings.cutFileName  = d->cutFileName;
        settings.creationTime = d->fileDatesMap[file];
        settings.manager      = this;
        d->renamedFiles[file] = d->parser->parse(settings);
    }
}

void AdvancedRenameManager::parseFiles(const QString& parseString, const ParseSettings& _settings)
{
    if (!d->parser)
    {
        return;
    }

    d->parser->reset();

    Q_FOREACH (const QString& file, d->files)
    {
        QUrl url               = QUrl::fromLocalFile(file);
        ParseSettings settings = _settings;
        settings.fileUrl       = url;
        settings.parseString   = parseString;
        settings.startIndex    = d->startIndex;
        settings.manager       = this;

        d->renamedFiles[file]  = d->parser->parse(settings);
    }
}

void AdvancedRenameManager::addFiles(const QList<ParseSettings>& files)
{
    Q_FOREACH (const ParseSettings& ps, files)
    {
        addFile(ps.fileUrl.toLocalFile(), ps.creationTime);
    }

    initialize();
}

void AdvancedRenameManager::clearMappings()
{
    d->fileIndexMap.clear();
    d->folderIndexMap.clear();
    d->fileGroupIndexMap.clear();
    d->fileCounterIndexMap.clear();
    d->renamedFiles.clear();
}

void AdvancedRenameManager::clearAll()
{
    d->files.clear();
    clearMappings();
}

void AdvancedRenameManager::reset()
{
    clearAll();
    resetState();
}

void AdvancedRenameManager::resetState()
{
    d->parser->reset();
    d->startIndex = 1;
}

void AdvancedRenameManager::initializeFileList()
{
    QStringList tmpFiles = d->files;

    switch (d->sortAction)
    {
        case SortName:
        {
            std::sort(tmpFiles.begin(), tmpFiles.end(), SortByNameCaseInsensitive());
            break;
        }

        case SortDate:
        {
            std::sort(tmpFiles.begin(), tmpFiles.end(), SortByDate());
            break;
        }

        case SortSize:
        {
            std::sort(tmpFiles.begin(), tmpFiles.end(), SortBySize());
            break;
        }

        case SortCustom:
        default:
            break;
    }

    if ((d->sortAction != SortCustom) && (d->sortDirection == SortDescending))
    {
        std::reverse(tmpFiles.begin(), tmpFiles.end());
    }

    d->files = tmpFiles;
}

QStringList AdvancedRenameManager::fileList() const
{
    return d->files;
}

QMap<QString, QString> AdvancedRenameManager::newFileList(bool checkFileSystem) const
{
    if (checkFileSystem)
    {
        QMap<QString, QString> renamedFiles;

        Q_FOREACH (const QString& fileName, d->renamedFiles.keys())
        {
            renamedFiles[fileName] = newName(fileName);
        }

        return renamedFiles;
    }

    return d->renamedFiles;
}

bool AdvancedRenameManager::initialize()
{
    if (d->files.isEmpty())
    {
        return false;
    }

    // clear mappings

    clearMappings();

    // initialize the file list according to the sort action and direction

    initializeFileList();

    // fill normal index map and file random string map

    {
        int counter = 1;

        Q_FOREACH (const QString& file, d->files)
        {
            QString random = QUuid::createUuid().toString();
            random.remove(QLatin1Char('-'));

            d->fileIndexMap[file] = counter++;
            d->randomList        << random.mid(1, 32);
        }
    }

    // fill file group index map

    {
        int counter = 1;

        Q_FOREACH (const QString& file, d->files)
        {
            if (!d->fileGroupIndexMap.contains(fileGroupKey(file)))
            {
                d->fileGroupIndexMap[fileGroupKey(file)] = counter++;
            }
        }
    }

    // fill folder group index map

    QMap<QString, QList<QString> > dirMap;

    {
        Q_FOREACH (const QString& file, d->files)
        {
            QFileInfo fi(file);
            QString path = fi.absolutePath();

            if (!path.isEmpty())
            {
                if (!dirMap.contains(path))
                {
                    dirMap[path] = QList<QString>();
                }

                dirMap[path].append(file);
            }
        }

        Q_FOREACH (const QString& dir, dirMap.keys())
        {
            int index = 0;

            Q_FOREACH (const QString& f, dirMap[dir])
            {
                if (!d->folderIndexMap.contains(f))
                {
                    d->folderIndexMap[f] = ++index;
                }
            }
        }
    }

    // fill file counter index map

    {
        QString regExp(QLatin1String("(.*?)(\\d+)((_v\\d+[.])|[.])(.*)?"));
        QRegularExpression counterRegExp(QRegularExpression::anchoredPattern(regExp));

        Q_FOREACH (const QString& path, dirMap.keys())
        {
            QDir dir(path);
            d->fileCounterIndexMap[path] = 0;

            Q_FOREACH (const QString& file, dir.entryList(QDir::Files))
            {
                if (d->files.contains(dir.filePath(file)) ||
                    file.endsWith(QLatin1String(".xmp")))
                {
                    continue;
                }

                QRegularExpressionMatch match = counterRegExp.match(file);

                if (match.hasMatch())
                {
                    int count = match.captured(2).toInt();

                    if (d->fileCounterIndexMap[path] < count)
                    {
                        d->fileCounterIndexMap[path] = count;
                    }
                }
            }
        }
    }

    return true;
}

QString AdvancedRenameManager::fileGroupKey(const QString& filename) const
{
    QFileInfo fi(filename);
    QString tmp = fi.absolutePath() + QLatin1Char('/');
    tmp        += fi.completeBaseName().mid(d->cutFileName) + QLatin1Char('.');

    return tmp;
}

int AdvancedRenameManager::indexOfFile(const QString& filename)
{
    return d->fileIndexMap.value(filename, -1);
}

int AdvancedRenameManager::indexOfFolder(const QString& filename)
{
    return d->folderIndexMap.value(filename, -1);
}

int AdvancedRenameManager::indexOfFileGroup(const QString& filename)
{
    return d->fileGroupIndexMap.value(fileGroupKey(filename), -1);
}

int AdvancedRenameManager::indexOfFileCounter(const QString& filename)
{
    QFileInfo fi(filename);

    return d->fileCounterIndexMap.value(fi.absolutePath(), -1);
}

QString AdvancedRenameManager::randomStringOfIndex(int index)
{
    if ((index < 0) || (index >= d->randomList.size()))
    {
        return QString();
    }

    return d->randomList.at(index);
}

QString AdvancedRenameManager::newName(const QString& filename) const
{
    // For the Linux or Windows file system,
    // we need to replace unsupported characters.

    QStorageInfo info(QFileInfo(filename).path());

    QString regExpStr = QLatin1String("[?*");
    QString newName   = d->renamedFiles.value(filename, filename);
    QString sysType   = QString::fromLatin1(info.fileSystemType()).toUpper();

    if (sysType.contains(QLatin1String("FAT"))  ||
        sysType.contains(QLatin1String("NTFS")) ||
        sysType.contains(QLatin1String("FUSEBLK")))
    {
        regExpStr.append(QLatin1String("<>:=\"|\\\\/"));

        if (sysType.contains(QLatin1String("FAT")))
        {
            regExpStr.append(QLatin1Char('+'));
        }
    }

    QRegularExpression regExp(regExpStr + QLatin1Char(']'));
    newName.replace(regExp, QLatin1String("_"));

    return newName;
}

void AdvancedRenameManager::addFile(const QString& filename) const
{
    d->files << filename;
}

void AdvancedRenameManager::addFile(const QString& filename, const QDateTime& datetime) const
{
    d->files << filename;
    d->fileDatesMap[filename] = datetime;
}

} // namespace Digikam
