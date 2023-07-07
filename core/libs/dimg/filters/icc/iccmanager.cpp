/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-12
 * Description : methods that implement color management tasks
 *
 * SPDX-FileCopyrightText: 2005-2006 by F.J. Cruz <fj dot cruz at supercable dot es>
 * SPDX-FileCopyrightText: 2005-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iccmanager.h"

// Qt includes

#include <QImage>

// Local includes

#include "digikam_debug.h"
#include "iccprofile.h"
#include "icctransform.h"

namespace Digikam
{

class Q_DECL_HIDDEN IccManager::Private
{
public:

    explicit Private()
        : profileMismatch(false),
          settings       (ICCSettingsContainer()),
          observer       (nullptr)
    {
    }

    DImg                 image;
    IccProfile           embeddedProfile;
    IccProfile           workspaceProfile;
    bool                 profileMismatch;
    ICCSettingsContainer settings;
    DImgLoaderObserver*  observer;
};

IccManager::IccManager(const DImg& image, const ICCSettingsContainer& settings)
    : d(new Private)
{
    d->image    = image;
    d->settings = settings;

    if (d->image.isNull())
    {
        return;
    }

    if (!d->settings.enableCM)
    {
        return;
    }

    d->embeddedProfile  = d->image.getIccProfile();
    d->workspaceProfile = IccProfile(d->settings.workspaceProfile);

    if (!d->workspaceProfile.open())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Cannot open workspace color profile"
                                  << d->workspaceProfile.filePath();
        return;
    }

    if (!d->embeddedProfile.isNull() && !d->embeddedProfile.open())
    {
        // Treat as missing profile

        d->embeddedProfile = IccProfile();
        qCWarning(DIGIKAM_DIMG_LOG) << "Encountered invalid embedded color profile in file"
                                    << d->image.attribute(QLatin1String("originalFilePath")).toString();
    }

    if (!d->embeddedProfile.isNull())
    {
        d->profileMismatch = !d->embeddedProfile.isSameProfileAs(d->workspaceProfile);
    }
}

IccManager::~IccManager()
{
    delete d;
}

DImg IccManager::image() const
{
    return d->image;
}

ICCSettingsContainer IccManager::settings() const
{
    return d->settings;
}

DImgLoaderObserver* IccManager::observer() const
{
    return d->observer;
}

void IccManager::setObserver(DImgLoaderObserver* const observer)
{
    d->observer = observer;
}

bool IccManager::hasValidWorkspace() const
{
    return d->workspaceProfile.isOpen();
}

void IccManager::transformDefault()
{
    if (d->image.isNull())
    {
        return;
    }

    if (!d->settings.enableCM)
    {
        return;
    }

    if      (isUncalibratedColor())
    {
        transform(d->settings.defaultUncalibratedBehavior);
    }
    else if (isMissingProfile())
    {
        transform(d->settings.defaultMissingProfileBehavior);
    }
    else if (isProfileMismatch())
    {
        transform(d->settings.defaultMismatchBehavior);
    }
}

bool IccManager::isUncalibratedColor() const
{
    return d->image.hasAttribute(QLatin1String("uncalibratedColor"));
}

bool IccManager::isMissingProfile() const
{
    return d->embeddedProfile.isNull();
}

bool IccManager::isProfileMismatch() const
{
    return d->profileMismatch;
}

ICCSettingsContainer::Behavior IccManager::safestBestBehavior() const
{
    if      (isUncalibratedColor())
    {
        return ICCSettingsContainer::InputToWorkspace;
    }
    else if (isMissingProfile())
    {
        // Assume sRGB (normal, untagged file) and stick with it

        return (ICCSettingsContainer::UseSRGB | ICCSettingsContainer::KeepProfile);
    }
    else if (isProfileMismatch())
    {
        return ICCSettingsContainer::EmbeddedToWorkspace;
    }
    else
    {
        return ICCSettingsContainer::PreserveEmbeddedProfile;
    }
}

void IccManager::transform(ICCSettingsContainer::Behavior behavior, const IccProfile& specifiedProfile)
{
    if (d->image.isNull())
    {
        return;
    }

    if (!d->settings.enableCM)
    {
        return;
    }

    if      (behavior == ICCSettingsContainer::AskUser)
    {
        if      (isUncalibratedColor())
        {
            d->image.setAttribute(QLatin1String("uncalibratedColorAskUser"), true);
        }
        else if (isMissingProfile())
        {
            d->image.setAttribute(QLatin1String("missingProfileAskUser"), true);
        }
        else if (isProfileMismatch())
        {
            d->image.setAttribute(QLatin1String("profileMismatchAskUser"), true);
        }

        return;
    }
    else if (behavior == ICCSettingsContainer::SafestBestAction)
    {
        behavior = safestBestBehavior();
    }

    IccTransform trans;
    getTransform(trans, behavior, specifiedProfile);

    if (trans.willHaveEffect())
    {
        trans.apply(d->image, d->observer);
        setIccProfile(trans.outputProfile());
    }
}

bool IccManager::needsPostLoadingManagement(const DImg& img)
{
    return (img.hasAttribute(QLatin1String("missingProfileAskUser"))  ||
            img.hasAttribute(QLatin1String("profileMismatchAskUser")) ||
            img.hasAttribute(QLatin1String("uncalibratedColorAskUser")));
}

/*
 * "postLoadingManagement" is implemented in the class IccPostLoadingManager
 */

// -- Behavior implementation ----------------------------------------------------------------------------------


IccProfile IccManager::imageProfile(ICCSettingsContainer::Behavior behavior, const IccProfile& specifiedProfile)
{
    if      (behavior & ICCSettingsContainer::UseEmbeddedProfile)
    {
        return d->embeddedProfile;
    }
    else if (behavior & ICCSettingsContainer::UseWorkspace)
    {
        return d->workspaceProfile;
    }
    else if (behavior & ICCSettingsContainer::UseSRGB)
    {
        return IccProfile::sRGB();
    }
    else if (behavior & ICCSettingsContainer::UseDefaultInputProfile)
    {
        return IccProfile(d->settings.defaultInputProfile);
    }
    else if (behavior & ICCSettingsContainer::UseSpecifiedProfile)
    {
        return specifiedProfile;
    }
    else if (behavior & ICCSettingsContainer::AutomaticColors)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Let the RAW loader do automatic color conversion";

        return IccProfile();
    }
    else if (behavior & ICCSettingsContainer::DoNotInterpret)
    {
        return IccProfile();
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "No input profile: invalid Behavior flags" << (int)behavior;

    return IccProfile();
}

void IccManager::getTransform(IccTransform& trans, ICCSettingsContainer::Behavior behavior, const IccProfile& specifiedProfile)
{
    IccProfile inputProfile = imageProfile(behavior, specifiedProfile);
    IccProfile outputProfile;

    trans.setIntent(d->settings.renderingIntent);
    trans.setUseBlackPointCompensation(d->settings.useBPC);

    // Output

    if (behavior & ICCSettingsContainer::ConvertToWorkspace)
    {
        outputProfile = d->workspaceProfile;
    }

    if (inputProfile.isNull())
    {
        return;
    }

    // Assigning the _input_ profile, if necessary. If output profile is not null, it needs to be assigned later.

    if ((inputProfile != d->embeddedProfile) && !(behavior & ICCSettingsContainer::LeaveFileUntagged))
    {
        setIccProfile(inputProfile);
    }

    if (!outputProfile.isNull())
    {
        trans.setInputProfile(inputProfile);
        trans.setOutputProfile(outputProfile);
    }
}

void IccManager::setIccProfile(const IccProfile& profile)
{
    d->image.setIccProfile(profile);
    d->embeddedProfile = profile;

    if (!d->embeddedProfile.isNull())
    {
        d->profileMismatch = !d->embeddedProfile.isSameProfileAs(d->workspaceProfile);
    }
}

// -- Display -------------------------------------------------------------------------------------------

void IccManager::transformForDisplay()
{
    transformForDisplay(displayProfile());
}

void IccManager::transformForDisplay(QWidget* widget)
{
    transformForDisplay(displayProfile(widget));
}

void IccManager::transformForDisplay(const IccProfile& profile)
{
    if (d->image.isNull())
    {
        return;
    }

    if (!d->settings.enableCM)
    {
        return;
    }

    IccProfile outputProfile = profile;

    if (outputProfile.isNull())
    {
        outputProfile = displayProfile();
    }

    if (isUncalibratedColor())
    {
        // set appropriate outputColorSpace in RawLoadingSettings

        qCDebug(DIGIKAM_DIMG_LOG) << "Do not use transformForDisplay for uncalibrated data "
                                     "but let the RAW loader do the conversion to sRGB";
    }

    IccTransform trans = displayTransform(outputProfile);

    if (trans.willHaveEffect())
    {
        trans.apply(d->image, d->observer);
        setIccProfile(trans.outputProfile());
    }
}

IccProfile IccManager::displayProfile(QWidget* const displayingWidget)
{
    if (!IccSettings::instance()->isEnabled())
    {
        return IccProfile::sRGB();
    }

    IccProfile profile = IccSettings::instance()->monitorProfile(displayingWidget);

    if (profile.open())
    {
        return profile;
    }

    return IccProfile::sRGB();
}

IccTransform IccManager::displayTransform(QWidget* const displayingWidget)
{
    return displayTransform(displayProfile(displayingWidget));
}

IccTransform IccManager::displayTransform(const IccProfile& displayProfile)
{
    if (d->image.isNull())
    {
        return IccTransform();
    }

    if (!d->settings.enableCM)
    {
        return IccTransform();
    }

    IccTransform trans;
    trans.setIntent(d->settings.renderingIntent);
    trans.setUseBlackPointCompensation(d->settings.useBPC);

    if (isUncalibratedColor())
    {
        // set appropriate outputColorSpace in RawLoadingSettings

        qCDebug(DIGIKAM_DIMG_LOG) << "Do not use transformForDisplay for uncalibrated data "
                                     "but let the RAW loader do the conversion to sRGB";
    }
    else if (isMissingProfile())
    {
        ICCSettingsContainer::Behavior missingProfileBehavior = d->settings.defaultMissingProfileBehavior;

        if ((missingProfileBehavior == ICCSettingsContainer::AskUser) ||
            (missingProfileBehavior == ICCSettingsContainer::SafestBestAction))
        {
            missingProfileBehavior = safestBestBehavior();
        }

        IccProfile assumedImageProfile = imageProfile(missingProfileBehavior, IccProfile());
        IccProfile outputProfile(displayProfile);

        if (!assumedImageProfile.isSameProfileAs(outputProfile))
        {
            trans.setInputProfile(d->embeddedProfile);
            trans.setOutputProfile(outputProfile);
        }
    }
    else
    {
        IccProfile outputProfile(displayProfile);

        if (!d->embeddedProfile.isSameProfileAs(outputProfile))
        {
            trans.setInputProfile(d->embeddedProfile);
            trans.setOutputProfile(outputProfile);
        }
    }

    return trans;
}

IccTransform IccManager::displaySoftProofingTransform(const IccProfile& deviceProfile, QWidget* const displayingWidget)
{
    return displaySoftProofingTransform(deviceProfile, displayProfile(displayingWidget));
}

IccTransform IccManager::displaySoftProofingTransform(const IccProfile& deviceProfile, const IccProfile& displayProfile)
{
    IccTransform transform = displayTransform(displayProfile);
    transform.setProofProfile(deviceProfile);
    transform.setCheckGamut(d->settings.doGamutCheck);
    transform.setProofIntent(d->settings.proofingRenderingIntent);
    transform.setCheckGamutMaskColor(d->settings.gamutCheckMaskColor);

    return transform;
}

// -- sRGB and Output -------------------------------------------------------------------------------------------------

bool IccManager::isSRGB(const DImg& image)
{
    if (image.isNull() || !IccSettings::instance()->isEnabled())
    {
        return true;                // then, everything is sRGB
    }

    IccProfile embeddedProfile = image.getIccProfile();
    IccProfile outputProfile   = IccProfile::sRGB();

    if (embeddedProfile.isNull())   // assume sRGB
    {
        return true;
    }
    else
    {
        return embeddedProfile.isSameProfileAs(outputProfile);
    }
}

void IccManager::transformToSRGB()
{
    if (d->image.isNull())
    {
        return;
    }

    if (!d->settings.enableCM)
    {
        return;
    }

    if      (isUncalibratedColor())
    {
        // set appropriate outputColorSpace in RawLoadingSettings

        qCDebug(DIGIKAM_DIMG_LOG) << "Do not use transformForDisplay for uncalibrated data "
                                     "but let the RAW loader do the conversion to sRGB";
    }
    else if (isMissingProfile())
    {
        // all is fine, assume sRGB
        //TODO: honour options?
    }
    else
    {
        IccProfile outputProfile = IccProfile::sRGB();

        if (!d->embeddedProfile.isSameProfileAs(outputProfile))
        {
            IccTransform trans;
            trans.setInputProfile(d->embeddedProfile);
            trans.setOutputProfile(outputProfile);
            trans.setIntent(d->settings.renderingIntent);
            trans.setUseBlackPointCompensation(d->settings.useBPC);
            trans.apply(d->image, d->observer);
            setIccProfile(trans.outputProfile());
        }
    }
}

void IccManager::transformToSRGB(QImage& qimage, const IccProfile& input)
{
    if (qimage.isNull())
    {
        return;
    }

    if (input.isNull())
    {
        return;
    }

    IccProfile inputProfile(input);
    IccProfile outputProfile = IccProfile::sRGB();

    if (!inputProfile.isSameProfileAs(outputProfile))
    {
        IccTransform trans;
        trans.setInputProfile(inputProfile);
        trans.setOutputProfile(outputProfile);
        trans.setIntent(IccTransform::Perceptual);
        trans.apply(qimage);
    }
}

void IccManager::transformForDisplay(QImage& qimage, const IccProfile& displayProfile1)
{
    if (qimage.isNull())
    {
        return;
    }

    if (displayProfile1.isNull())
    {
        return;
    }

    IccProfile inputProfile = IccProfile::sRGB();
    IccProfile outputProfile(displayProfile1);

    if (!inputProfile.isSameProfileAs(outputProfile))
    {
        IccTransform trans;
        trans.setInputProfile(inputProfile);
        trans.setOutputProfile(outputProfile);
        trans.setIntent(IccTransform::Perceptual);
        trans.apply(qimage);
    }
}

void IccManager::transformForOutput(const IccProfile& prof)
{
    if (d->image.isNull())
    {
        return;
    }

    if (!d->settings.enableCM)
    {
        return;
    }

    IccProfile inputProfile;
    IccProfile outputProfile = prof;

    if      (isUncalibratedColor())
    {
        // set appropriate outputColorSpace in RawLoadingSettings

        qCDebug(DIGIKAM_DIMG_LOG) << "Do not use transformForOutput for uncalibrated data "
                                     "but let the RAW loader do the conversion to sRGB";
    }
    else if (isMissingProfile())
    {
        inputProfile = IccProfile::sRGB();

        //TODO: honour options?
    }
    else
    {
        if (!d->embeddedProfile.isSameProfileAs(outputProfile))
        {
            inputProfile = d->embeddedProfile;
        }
    }

    if (!inputProfile.isNull())
    {
        IccTransform trans;
        trans.setInputProfile(inputProfile);
        trans.setOutputProfile(outputProfile);
        trans.setIntent(d->settings.renderingIntent);
        trans.setUseBlackPointCompensation(d->settings.useBPC);
        trans.apply(d->image, d->observer);
        setIccProfile(trans.outputProfile());
    }
}

} // namespace Digikam
