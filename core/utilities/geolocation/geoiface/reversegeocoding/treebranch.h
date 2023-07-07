/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-12
 * Description : A model to hold information about image tags.
 *
 * SPDX-FileCopyrightText: 2010 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TREE_BRANCH_H
#define DIGIKAM_TREE_BRANCH_H

// Qt includes

#include <QPersistentModelIndex>
#include <QList>

// Local includes

#include "gpsitemcontainer.h"

namespace Digikam
{

class TreeBranch
{
public:

    explicit TreeBranch()
      : parent(nullptr),
        type  (TypeChild)
    {
    }

    ~TreeBranch()
    {
        qDeleteAll(oldChildren);
    }

public:

    QPersistentModelIndex sourceIndex;
    TreeBranch*           parent;
    QString               data;
    QString               help;
    Type                  type;
    QList<TreeBranch*>    oldChildren;
    QList<TreeBranch*>    spacerChildren;
    QList<TreeBranch*>    newChildren;
};

} // namespace Digikam

#endif // DIGIKAM_TREE_BRANCH_H
