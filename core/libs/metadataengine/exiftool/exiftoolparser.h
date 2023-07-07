/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-11-28
 * Description : ExifTool process stream parser.
 *
 * SPDX-FileCopyrightText: 2020-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_EXIFTOOL_PARSER_H
#define DIGIKAM_EXIFTOOL_PARSER_H

// Qt Core

#include <QHash>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QProcess>
#include <QFileInfo>
#include <QByteArray>

// Local includes

#include "digikam_export.h"
#include "exiftoolprocess.h"
#include "metaengine.h"

namespace Digikam
{

class DIGIKAM_EXPORT ExifToolParser : public QObject
{
    Q_OBJECT

public:

    /**
     * A map used to store ExifTool data shared with ExifToolProcess class:
     *
     * With load() method, the container is used to get a map of
     * ExifTool tag name as key and tags properties as values:
     * key    = ExifTool Tag name            (QString - ExifTool Group 0.1.2.4.6)
     *                                       See -G Exiftool option (https://exiftool.org/exiftool_pod.html#Input-output-text-formatting).
     * values = ExifTool Tag value           (QString).
     *          ExifTool Tag type            (QString).
     *          ExifTool Tag description     (QString).
     *          ExifTool Tag numerical value (QString) - available if any .
     *
     * With loadChunk() method, the container is used to get
     * a EXV chunk as value:
     * key   = "EXV"                         (QString).
     * value = the Exiv2 metadata container  (QByteArray).
     *
     * With applyChanges() method, the container is used as argument to
     * store tupple of ExifTool tag name as key and tag value:
     * key   = ExifTool tag name             (QString).
     * value = ExifTool Tag value            (QString).
     *
     * With readableFormats() method, the container is used to get
     * a list of upper-case file format extensions supported by ExifTool for reading.
     * key   = "READ_FORMAT"                 (QString).
     * value = list of pairs (ext,desc)      (QStringList)
     *
     * With writableFormats() method, the container is used to get
     * a list of upper-case file format extensions supported by ExifTool for writing.
     * key   = "WRITE_FORMAT"                (QString).
     * value = list of pairs (ext,desc)      (QStringList).
     *
     * With translationsList() method, the container is used to get
     * a list of ExifTool languages available for translations.
     * key   = "TRANSLATIONS_LIST"           (QString).
     * value = list of languages as strings
     *         (aka fr, en, de, es, etc.)    (QStringList).
     *
     * With tagsDatabase() method, the container is used as argument to
     * store tupple of ExifTool tag name as key and tag description:
     * key    = ExifTool tag name            (QString).
     * values = ExifTool Tag description     (QString).
     *          ExifTool Tag type            (QString).
     *          ExifTool Tag writable        (QString).
     */
    typedef QHash<QString, QVariantList> ExifToolData;

public:

    //---------------------------------------------------------------------------------------------
    /// Constructor, Destructor, and Configuration Accessors. See exiftoolparser.cpp for details.
    //@{

    explicit ExifToolParser(QObject* const parent, bool async = false);
    ~ExifToolParser();

    void setExifToolProgram(const QString& path);

    QString      currentPath()        const;
    ExifToolData currentData()        const;
    QString      currentErrorString() const;

    /**
     * Check the ExifTool program availability.
     */
    bool exifToolAvailable()          const;

    //@}

public:

    //---------------------------------------------------------------------------------------------
    /// ExifTool Command Methods. See exiftoolparser_command.cpp for details.
    //@{

    /**
     * Load all metadata with ExifTool from a file.
     * Use currentData() to get the ExifTool map.
     */
    bool load(const QString& path);

    /**
     * Load Exif, Iptc, and Xmp chunk as Exiv2 EXV byte-array from a file.
     * Use currentData() to get the container.
     */
    bool loadChunk(const QString& path, bool copyToAll = false);

    /**
     * Apply tag changes to a target file using ExifTool with a list of tag properties.
     * Tags can already exists in target file or new ones can be created.
     * To remove a tag, pass an empty string as value.
     * @param path is the target files to change.
     * @param newTags is the list of tag properties.
     */
    bool applyChanges(const QString& path, const ExifToolData& newTags);

    /**
     * Apply tag changes to a target file using ExifTool with a EXV container.
     * Tags can already exists in target file or new ones can be created.
     * @param path is the target files to change.
     * @param exvTempFile is the list of changes embedded in EXV container.
     * @param hasExif if the EXV container has Exif metadata restore MarkerNotes.
     */
    bool applyChanges(const QString& path,
                      const QString& exvTempFile,
                      bool hasExif = true, bool hasIptcCSet = false);

    /**
     * Return a list of readable file format extensions.
     * Use currentData() to get the container as QStringList.
     */
    bool readableFormats();

    /**
     * Return a list of writable file format extensions.
     * Use currentData() to get the container as QStringList.
     */
    bool writableFormats();

    /**
     * Return a list of available translations.
     * Use currentData() to get the container as QStringList.
     */
    bool translationsList();

    /**
     * Return a list of all tags from ExifTool database.
     * Use currentData() to get the container.
     * Warning: This method get whole ExifTool database in XML format and take age.
     */
    bool tagsDatabase();

    /**
     * Return the current version of ExifTool.
     * Use currentData() to get the container as QString.
     */
    bool version();

    /**
     * Copy group of tags from one source file to a destination file, following copy operations defined by 'copyOps'.
     * @param copyOps is a OR combination of ExifToolProcess::CopyTagsSource values.
     * @param transOps is a OR combination of ExifToolProcess::TranslateTagsOps values.
     * @param writeModes is a OR combaniation of ExifToolProcess::WritingTagsMode values.
     * @param dst must be a writable file format supported by ExifTool.
     */
    bool copyTags(const QString& src,
                  const QString& dst,
                  unsigned char copyOps,
                  unsigned char writeModes = ExifToolProcess::ALL_MODES);

    /**
     * Translate group of tags in file.
     * @param transOps is a OR combination of ExifToolProcess::TranslateTagsOps values.
     */
    bool translateTags(const QString& path, unsigned char transOps);

    //@}

public:

    //---------------------------------------------------------------------------------------------
    /// ExifTool Output Management Methods. See exiftoolparser_output.cpp for details
    //@{

    void cmdCompleted(const ExifToolProcess::Result& result);

    void errorOccurred(const ExifToolProcess::Result& result,
                       QProcess::ProcessError error,
                       const QString& description);

    void finished();

public Q_SLOTS:

    void slotExifToolResult(int cmdId);

    //@}

Q_SIGNALS:

    void signalExifToolDataAvailable();
    void signalExifToolAsyncData(const ExifToolParser::ExifToolData& map);

public:

    /**
     * Unit-test method to check ExifTool stream parsing.
     */
    void setOutputStream(int cmdAction,
                         const QByteArray& cmdOutputChannel,
                         const QByteArray& cmdErrorChannel);

    /**
     * Helper conversion method to translate unordered tags database hash-table to ordered map.
     */
    static MetaEngine::TagsMap tagsDbToOrderedMap(const ExifToolData& tagsDb);

private:

    void printExifToolOutput(const QByteArray& stdOut);

    //@}

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_EXIFTOOL_PARSER_H
