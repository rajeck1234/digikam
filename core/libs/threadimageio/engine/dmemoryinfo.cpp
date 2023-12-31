/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-10
 * Description : list of memory features from host computer
 *
 * SPDX-FileCopyrightText:      2010 Pino Toscano <pino at kde dot org>
 * SPDX-FileCopyrightText:      2011 Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2021-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmemoryinfo.h"

// Qt includes

#include <QSharedData>

// Local includes

#include "digikam_debug.h"

#if defined(Q_OS_WINDOWS)   // krazy:exclude=cpp
#   include <windows.h>     // windows.h must stay above pspapi.h
#   include <psapi.h>
#elif defined(Q_OS_LINUX) || defined(Q_OS_ANDROID)
#   include <QByteArray>
#   include <QFile>
#   if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#       include <QByteArrayView>
#   endif
#elif defined(Q_OS_MACOS)
#   include <mach/mach.h>
#   include <sys/sysctl.h>
#elif defined(Q_OS_FREEBSD)
#   include <fcntl.h>
#   include <kvm.h>
#   include <sys/sysctl.h>
#endif

namespace Digikam
{

class Q_DECL_HIDDEN DMemoryInfo::Private : public QSharedData
{
public:

    Private()
    {
    }

    quint64 m_totalPhysical     = 0;
    quint64 m_availablePhysical = 0;
    quint64 m_freePhysical      = 0;
    quint64 m_totalSwapFile     = 0;
    quint64 m_freeSwapFile      = 0;
    quint64 m_cached            = 0;
    quint64 m_buffers           = 0;
};

DMemoryInfo::DMemoryInfo()
    : d(new Private)
{
    update();
}

DMemoryInfo::~DMemoryInfo()
{
}

DMemoryInfo::DMemoryInfo(const DMemoryInfo& other)
    : d(other.d)
{
}

DMemoryInfo& DMemoryInfo::operator=(const DMemoryInfo& other)
{
    d = other.d;

    return *this;
}

bool DMemoryInfo::operator==(const DMemoryInfo& other) const
{
    if (this == &other)
    {
        return true;
    }

    return ((d->m_availablePhysical == other.d->m_availablePhysical) &&
            (d->m_freePhysical      == other.d->m_freePhysical)      &&
            (d->m_freeSwapFile      == other.d->m_freeSwapFile)      &&
            (d->m_cached            == other.d->m_cached)            &&
            (d->m_buffers           == other.d->m_buffers)           &&
            (d->m_totalSwapFile     == other.d->m_totalSwapFile)     &&
            (d->m_totalPhysical     == other.d->m_totalPhysical));
}

bool DMemoryInfo::operator!=(const DMemoryInfo& other) const
{
    return !operator==(other);
}

bool DMemoryInfo::isNull() const
{
    return (d->m_totalPhysical == 0);
}

quint64 DMemoryInfo::totalPhysical() const
{
    return d->m_totalPhysical;
}

quint64 DMemoryInfo::freePhysical() const
{
    return d->m_freePhysical;
}

quint64 DMemoryInfo::availablePhysical() const
{
    return d->m_availablePhysical;
}

quint64 DMemoryInfo::cached() const
{
    return d->m_cached;
}

quint64 DMemoryInfo::buffers() const
{
    return d->m_buffers;
}

quint64 DMemoryInfo::totalSwapFile() const
{
    return d->m_totalSwapFile;
}

quint64 DMemoryInfo::freeSwapFile() const
{
    return d->m_freeSwapFile;
}

#if defined(Q_OS_WINDOWS)   // krazy:exclude=cpp

struct Q_DECL_HIDDEN SwapInfo
{
    quint64 totalPageFilePages = 0;
    quint64 freePageFilePages  = 0;
};

BOOL __stdcall pageInfo(LPVOID pContext, PENUM_PAGE_FILE_INFORMATION pPageFileInfo, LPCWSTR lpFilename)
{
    Q_UNUSED(lpFilename)

    if (auto sw = static_cast<SwapInfo*>(pContext))
    {
        sw->totalPageFilePages += pPageFileInfo->TotalSize;
        sw->freePageFilePages  += (pPageFileInfo->TotalSize - pPageFileInfo->TotalInUse);

        return true;
    }

    return false;
}

bool DMemoryInfo::update()
{
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);

    if (!GlobalMemoryStatusEx(&statex))
    {
        return false;
    }

    PERFORMANCE_INFORMATION pi;
    DWORD pisz = sizeof(pi);

    if (!GetPerformanceInfo(&pi, pisz))
    {
        return false;
    }

    SwapInfo si;

    if (!EnumPageFiles(pageInfo, &si))
    {
        return false;
    }

    d->m_totalPhysical     = statex.ullTotalPhys;
    d->m_availablePhysical = statex.ullAvailPhys;
    d->m_freePhysical      = statex.ullAvailPhys;
    d->m_totalSwapFile     = si.totalPageFilePages * pi.PageSize;
    d->m_freeSwapFile      = si.freePageFilePages  * pi.PageSize;
    d->m_cached            = pi.SystemCache        * pi.PageSize;
    d->m_buffers           = 0;

    return true;
}

#elif defined(Q_OS_LINUX) || defined(Q_OS_ANDROID)

#   if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
using ByteArrayView = QByteArray;
#   else
using ByteArrayView = QByteArrayView;
#   endif

bool extractBytes(quint64& value, const QByteArray& buffer, const ByteArrayView& beginPattern, qsizetype& from)
{
    ByteArrayView endPattern("kB");

    auto beginIdx = buffer.indexOf(beginPattern, from);

    if (beginIdx > -1)
    {
        auto start  = beginIdx + beginPattern.size();
        auto endIdx = buffer.indexOf(endPattern, start);

        if (endIdx > -1)
        {
            from    = endIdx + endPattern.size();
            auto ok = false;
            value   = buffer.mid(start, endIdx - start).toULongLong(&ok) * 1024;

            return ok;
        }
    }

    if (from)
    {
        // Wrong order? Restart from the beginning

        qCWarning(DIGIKAM_GENERAL_LOG)
            << "DMemoryInfo: extractBytes: wrong order when extracting"
            << beginPattern;

        from = 0;

        return extractBytes(value, buffer, beginPattern, from);
    }

    return false;
}

bool DMemoryInfo::update()
{
    QFile file(QStringLiteral("/proc/meminfo"));

    if (!file.open(QFile::ReadOnly))
    {
        return false;
    }

    auto meminfo           = file.readAll();
    file.close();

    qsizetype miFrom       = 0;
    quint64 totalPhys      = 0;

    if (!extractBytes(totalPhys, meminfo, "MemTotal:", miFrom))
    {
        return false;
    }

    quint64 freePhys       = 0;

    if (!extractBytes(freePhys, meminfo, "MemFree:", miFrom))
    {
        return false;
    }

    quint64 availPhys      = 0;

    if (!extractBytes(availPhys, meminfo, "MemAvailable:", miFrom))
    {
        return false;
    }

    quint64 buffers        = 0;

    if (!extractBytes(buffers, meminfo, "Buffers:", miFrom))
    {
        return false;
    }

    quint64 cached         = 0;

    if (!extractBytes(cached, meminfo, "Cached:", miFrom))
    {
        return false;
    }

    quint64 swapTotal      = 0;

    if (!extractBytes(swapTotal, meminfo, "SwapTotal:", miFrom))
    {
        return false;
    }

    quint64 swapFree       = 0;

    if (!extractBytes(swapFree, meminfo, "SwapFree:", miFrom))
    {
        return false;
    }

    quint64 sharedMem      = 0;

    if (!extractBytes(sharedMem, meminfo, "Shmem:", miFrom))
    {
        return false;
    }

    quint64 sReclaimable   = 0;

    if (!extractBytes(sReclaimable, meminfo, "SReclaimable:", miFrom))
    {
        return false;
    }

    // Source HTOP: https://github.com/htop-dev/htop/blob/main/linux/LinuxProcessList.c

    d->m_totalPhysical     = totalPhys;

    // NOTE: another viable solution: d->m_availablePhysical = std::min(availPhys, totalPhys - (committedAs - cached - (swapTotal - swapFree)))

    d->m_availablePhysical = availPhys ? std::min(availPhys, totalPhys) : freePhys;
    d->m_freePhysical      = freePhys;
    d->m_totalSwapFile     = swapTotal;
    d->m_freeSwapFile      = swapFree;
    d->m_cached            = cached + sReclaimable - sharedMem;
    d->m_buffers           = buffers;

    return true;
}

#elif defined(Q_OS_MACOS)

template<class T>
bool sysctlread(const char* name, T& var)
{
    auto sz = sizeof(var);

    return (sysctlbyname(name, &var, &sz, nullptr, 0) == 0);
}

bool DMemoryInfo::update()
{
    quint64 memSize  = 0;
    quint64 pageSize = 0;
    xsw_usage swapUsage;

    int mib[2]       = { 0 };
    size_t sz        = 0;

    mib[0]           = CTL_HW;
    mib[1]           = HW_MEMSIZE;
    sz               = sizeof(memSize);

    if (sysctl(mib, 2, &memSize, &sz, nullptr, 0) != KERN_SUCCESS)
    {
        return false;
    }

    mib[0]           = CTL_HW;
    mib[1]           = HW_PAGESIZE;
    sz               = sizeof(pageSize);

    if (sysctl(mib, 2, &pageSize, &sz, nullptr, 0) != KERN_SUCCESS)
    {
        return false;
    }

    mib[0]           = CTL_VM;
    mib[1]           = VM_SWAPUSAGE;
    sz               = sizeof(swapUsage);

    if (sysctl(mib, 2, &swapUsage, &sz, nullptr, 0) != KERN_SUCCESS)
    {
        return false;
    }

    quint64 zfs_arcstats_size    = 0;

    if (!sysctlread("kstat.zfs.misc.arcstats.size", zfs_arcstats_size))
    {
        zfs_arcstats_size = 0; // no ZFS used
    }

    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vmstat;

    if (host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vmstat, &count) != KERN_SUCCESS)
    {
        return false;
    }

    d->m_totalPhysical     = memSize;
    d->m_availablePhysical = memSize -
                             (vmstat.internal_page_count + vmstat.compressor_page_count + vmstat.wire_count) *
                             pageSize;
    d->m_freePhysical      = vmstat.free_count * pageSize;
    d->m_totalSwapFile     = swapUsage.xsu_total;
    d->m_freeSwapFile      = swapUsage.xsu_avail;
    d->m_cached            = vmstat.external_page_count * pageSize + zfs_arcstats_size;
    d->m_buffers           = 0;

    return true;
}

#elif defined(Q_OS_FREEBSD)

template<class T>
bool sysctlread(const char* name, T& var)
{
    auto sz = sizeof(var);

    return (sysctlbyname(name, &var, &sz, nullptr, 0) == 0);
}

bool DMemoryInfo::update()
{
    quint64 memSize           = 0;
    quint64 pageSize          = 0;

    int mib[4]                = { 0 };
    size_t sz                 = 0;

    mib[0]                    = CTL_HW;
    mib[1]                    = HW_PHYSMEM;
    sz                        = sizeof(memSize);

    if (sysctl(mib, 2, &memSize, &sz, nullptr, 0) != 0)
    {
        return false;
    }

    mib[0]                    = CTL_HW;
    mib[1]                    = HW_PAGESIZE;
    sz                        = sizeof(pageSize);

    if (sysctl(mib, 2, &pageSize, &sz, nullptr, 0) != 0)
    {
        return false;
    }

    quint32 v_pageSize = 0;

    if (sysctlread("vm.stats.vm.v_page_size", v_pageSize))
    {
        pageSize = v_pageSize;
    }

    quint64 zfs_arcstats_size = 0;

    if (!sysctlread("kstat.zfs.misc.arcstats.size", zfs_arcstats_size))
    {
        zfs_arcstats_size = 0; // no ZFS used
    }

    quint32 v_cache_count     = 0;

    if (!sysctlread("vm.stats.vm.v_cache_count", v_cache_count))
    {
        return false;
    }

    quint32 v_inactive_count  = 0;

    if (!sysctlread("vm.stats.vm.v_inactive_count", v_inactive_count))
    {
        return false;
    }

    quint32 v_free_count      = 0;

    if (!sysctlread("vm.stats.vm.v_free_count", v_free_count))
    {
        return false;
    }

    quint64 vfs_bufspace      = 0;

    if (!sysctlread("vfs.bufspace", vfs_bufspace))
    {
        return false;
    }

    quint64 swap_tot          = 0;
    quint64 swap_free         = 0;

    if (auto kd = kvm_open("/dev/null", "/dev/null", "/dev/null", O_RDONLY, "kvm_open"))
    {
        struct kvm_swap swap;

        // if you specify a maxswap value of 1, the function will typically return the
        // value 0 and the single kvm_swap structure will be filled with the grand total over all swap devices.

        auto nswap = kvm_getswapinfo(kd, &swap, 1, 0);

        if (nswap == 0)
        {
            swap_tot  = swap.ksw_total;
            swap_free = swap.ksw_used;
        }

        swap_free  = (swap_tot - swap_free) * pageSize;
        swap_tot  *= pageSize;
    }

    // Source HTOP: https://github.com/htop-dev/htop/blob/main/freebsd/FreeBSDProcessList.c

    d->m_totalPhysical     = memSize;
    d->m_availablePhysical = pageSize *
                             (v_cache_count + v_free_count + v_inactive_count) +
                             vfs_bufspace + zfs_arcstats_size;
    d->m_freePhysical      = pageSize * v_free_count;
    d->m_totalSwapFile     = swap_tot;
    d->m_freeSwapFile      = swap_free;
    d->m_cached            = pageSize * v_cache_count + zfs_arcstats_size;
    d->m_buffers           = vfs_bufspace;

    return true;
}

#else

bool DMemoryInfo::update()
{
    qCWarning(DIGIKAM_GENERAL_LOG) << "DMemoryInfo: unsupported platform!";

    return false;
}

#endif

} // namespace Digikam
