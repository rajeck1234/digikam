/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-22
 * Description : a parser for the AdvancedRename utility used for importing images,
 *               excluding the database options
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_IMPORT_RENAME_PARSER_H
#define DIGIKAM_IMPORT_RENAME_PARSER_H

// Local includes

#include "parser.h"

namespace Digikam
{

class ImportRenameParser : public Parser
{

public:

    explicit ImportRenameParser();

private:

    // Disable
    ImportRenameParser(const ImportRenameParser&)            = delete;
    ImportRenameParser& operator=(const ImportRenameParser&) = delete;
};

} // namespace Digikam

#endif // DIGIKAM_IMPORT_RENAME_PARSER_H
