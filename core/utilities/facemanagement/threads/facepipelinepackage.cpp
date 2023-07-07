/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-03
 * Description : Integrated, multithread face detection / recognition
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "facepipeline_p.h"

// Local includes

#include "digikam_debug.h"
#include "databasewriter.h"
#include "detectionbenchmarker.h"
#include "detectionworker.h"
#include "recognitionbenchmarker.h"
#include "recognitionworker.h"
#include "trainerworker.h"
#include "facepreviewloader.h"
#include "faceitemretriever.h"
#include "parallelpipes.h"
#include "scanstatefilter.h"

namespace Digikam
{

FacePipelineFaceTagsIface::FacePipelineFaceTagsIface()
    : roles         (NoRole),
      assignedTagId (0)
{
}

FacePipelineFaceTagsIface::FacePipelineFaceTagsIface(const FaceTagsIface& face)
    : FaceTagsIface (face),
      roles         (NoRole),
      assignedTagId (0)
{
}

FacePipelineFaceTagsIface::~FacePipelineFaceTagsIface()
{
}

// ----------------------------------------------------------------------------------------

FacePipelineFaceTagsIfaceList::FacePipelineFaceTagsIfaceList()
{
}

FacePipelineFaceTagsIfaceList::~FacePipelineFaceTagsIfaceList()
{
}

FacePipelineFaceTagsIfaceList::FacePipelineFaceTagsIfaceList(const QList<FaceTagsIface>& faces)
{
    operator=(faces);
}

FacePipelineFaceTagsIfaceList& FacePipelineFaceTagsIfaceList::operator=(const QList<FaceTagsIface>& faces)
{
    Q_FOREACH (const FaceTagsIface& face, faces)
    {
        operator<<(FacePipelineFaceTagsIface(face));
    }

    return *this;
}

void FacePipelineFaceTagsIfaceList::setRole(FacePipelineFaceTagsIface::Roles role)
{
    for (iterator it = begin() ; it != end() ; ++it)
    {
        it->roles |= role;
    }
}

void FacePipelineFaceTagsIfaceList::clearRole(FacePipelineFaceTagsIface::Roles role)
{
    for (iterator it = begin() ; it != end() ; ++it)
    {
        it->roles &= ~role;
    }
}

void FacePipelineFaceTagsIfaceList::replaceRole(FacePipelineFaceTagsIface::Roles remove,
                                                FacePipelineFaceTagsIface::Roles add)
{
    for (iterator it = begin() ; it != end() ; ++it)
    {
        if (it->roles & remove)
        {
            it->roles &= ~remove;
            it->roles |= add;
        }
    }
}

QList<FaceTagsIface> FacePipelineFaceTagsIfaceList::toFaceTagsIfaceList() const
{
    QList<FaceTagsIface> faces;

    for (const_iterator it = constBegin() ; it != constEnd() ; ++it)
    {
        faces << *it;
    }

    return faces;
}

FacePipelineFaceTagsIfaceList FacePipelineFaceTagsIfaceList::facesForRole(FacePipelineFaceTagsIface::Roles role) const
{
    FacePipelineFaceTagsIfaceList faces;

    for (const_iterator it = constBegin() ; it != constEnd() ; ++it)
    {
        if (it->roles & role)
        {
            faces << *it;
        }
    }

    return faces;
}

// -----------------------------------------------------------------------------------------

FacePipelinePackage::FacePipelinePackage()
    : processFlags(NotProcessed)
{
}

FacePipelinePackage::~FacePipelinePackage()
{
}

// ----------------------------------------------------------------------------------------

FacePipelineExtendedPackage::FacePipelineExtendedPackage()
{
}

FacePipelineExtendedPackage::~FacePipelineExtendedPackage()
{
}

// ----------------------------------------------------------------------------------------

FacePipelineExtendedPackage::Ptr PackageLoadingDescriptionList::take(const LoadingDescription& description)
{
    FacePipelineExtendedPackage::Ptr                  package;
    QList<FacePipelineExtendedPackage::Ptr>::iterator it;

    for (it = begin() ; it != end() ; ++it)
    {
        if (*(*it) == description)
        {
            package = *it;
            erase(it);
            break;
        }
    }

    return package;
}

bool FacePipelineExtendedPackage::operator==(const LoadingDescription& description) const
{
    return (filePath == description.filePath);
}

// ----------------------------------------------------------------------------------------

PackageLoadingDescriptionList::PackageLoadingDescriptionList()
{
}

PackageLoadingDescriptionList::~PackageLoadingDescriptionList()
{
}

} // namespace Digikam
