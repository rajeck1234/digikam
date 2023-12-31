/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-10-31
 * Description : QtAV: Multimedia framework based on Qt and FFmpeg
 *
 * SPDX-FileCopyrightText: 2012-2022 Wang Bin <wbsecg1 at gmail dot com>
 * SPDX-FileCopyrightText:      2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "AVOutput_p.h"

// Local includes

#include "Filter.h"
#include "FilterContext.h"
#include "FilterManager.h"
#include "OutputSet.h"
#include "digikam_debug.h"

namespace QtAV
{

AVOutputPrivate::~AVOutputPrivate()
{
    cond.wakeAll(); // WHY: failed to wake up
}

AVOutput::AVOutput()
{
}

AVOutput::AVOutput(AVOutputPrivate& d)
    : DPTR_INIT(&d)
{
}

AVOutput::~AVOutput()
{
    pause(false); // Does not work. cond may still waiting when destroyed
    detach();
    DPTR_D(AVOutput);

    if (d.filter_context)
    {
        delete d.filter_context;
        d.filter_context = nullptr;
    }

    Q_FOREACH (Filter* const f, d.pending_uninstall_filters)
    {
        d.filters.removeAll(f);
    }

    QList<Filter*>::iterator it = d.filters.begin();

    while (it != d.filters.end())
    {
        // if not uninstall here, if AVOutput is also an QObject (for example, widget based renderers)
        // then qobject children filters will be deleted when parent is destroying
        // and call FilterManager::uninstallFilter() and FilterManager::instance().unregisterFilter(filter, this)
        // too late that AVOutput is almost be destroyed

        uninstallFilter(*it);

        // 1 filter has 1 target. so if has output filter in manager, the output is this output

        /*FilterManager::instance().hasOutputFilter(*it) && */

        if ((*it)->isOwnedByTarget() && !(*it)->parent())
            delete *it;

        ++it;
    }

    d.filters.clear();
}

bool AVOutput::isAvailable() const
{
    return d_func().available;
}

void AVOutput::pause(bool p)
{
    DPTR_D(AVOutput);

    if (d.paused == p)
        return;

    d.paused = p;
}

bool AVOutput::isPaused() const
{
    return d_func().paused;
}

// TODO: how to call this automatically before write()?

bool AVOutput::tryPause()
{
    DPTR_D(AVOutput);

    if (!d.paused)
        return false;

    QMutexLocker lock(&d.mutex);

    Q_UNUSED(lock);

    d.cond.wait(&d.mutex);

    return true;
}

void AVOutput::addOutputSet(OutputSet* set)
{
    d_func().output_sets.append(set);
}

void AVOutput::removeOutputSet(OutputSet* set)
{
    d_func().output_sets.removeAll(set);
}

void AVOutput::attach(OutputSet* set)
{
    set->addOutput(this);
}

void AVOutput::detach(OutputSet* set)
{
    DPTR_D(AVOutput);  // cppcheck-suppress constVariable

    if (set)
    {
        set->removeOutput(this);

        return;
    }

    Q_FOREACH (OutputSet* const set2, d.output_sets)
    {
        set2->removeOutput(this);
    }
}

QList<Filter*>& AVOutput::filters()
{
    return d_func().filters;
}

void AVOutput::setStatistics(Statistics* statistics)
{
    DPTR_D(AVOutput);

    d.statistics = statistics;
}

bool AVOutput::installFilter(Filter* filter, int index)
{
    return onInstallFilter(filter, index);
}

bool AVOutput::onInstallFilter(Filter* filter, int index)
{
    if (!FilterManager::instance().registerFilter(filter, this, index))
    {
        return false;
    }

    DPTR_D(AVOutput);

    d.filters = FilterManager::instance().outputFilters(this);

    return true;
}

/*
 * FIXME: how to ensure thread safe using mutex etc? for a video filter, both are in main thread.
 * an audio filter on audio output may be in audio thread
 */

bool AVOutput::uninstallFilter(Filter* filter)
{
    return onUninstallFilter(filter);
}

bool AVOutput::onUninstallFilter(Filter* filter)
{
    DPTR_D(AVOutput);

    FilterManager::instance().unregisterFilter(filter, this);
    d.pending_uninstall_filters.push_back(filter);

    return true;
}

void AVOutput::hanlePendingTasks()
{
    onHanlePendingTasks();
}

bool AVOutput::onHanlePendingTasks()
{
    DPTR_D(AVOutput);

    if (d.pending_uninstall_filters.isEmpty())
        return false;

    Q_FOREACH (Filter* const filter, d.pending_uninstall_filters)
    {
        d.filters.removeAll(filter);
    }

    d.pending_uninstall_filters.clear();

    return true;
}

} // namespace QtAV
