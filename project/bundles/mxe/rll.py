#!/usr/bin/python

# Python script that takes an EXE file, automatically figures out all the DLL dependencies,
# and copies them next to the EXE.
#
# SPDX-FileCopyrightText:      2015 by Martin Preisler <martin at preisler dot me>
# SPDX-FileCopyrightText: 2016-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# Blog post         : https://martin.preisler.me/2015/03/mingw-bundledlls-automatically-bundle-dlls/
# Github repository : https://github.com/mpreisler/mingw-bundledlls
#
# SPDX-License-Identifier: BSD-3-Clause

import subprocess
import os.path
import argparse
import shutil
import string

# -----------------------------------------------

# Blacklist of native Windows dlls (may need extending)
# Note : Lowercase file names.
blacklist = [
    "advapi32.dll",
    "kernel32.dll",
    "msvcrt.dll",
    "ole32.dll",
    "user32.dll",
    "ws2_32.dll",
    "comdlg32.dll",
    "gdi32.dll",
    "imm32.dll",
    "oleaut32.dll",
    "shell32.dll",
    "winmm.dll",
    "winspool.drv",
    "wldap32.dll",
    "ntdll.dll",
    "d3d9.dll",
    "mpr.dll",
    "crypt32.dll",
    "dnsapi.dll",
    "shlwapi.dll",
    "version.dll",
    "iphlpapi.dll",
    "msimg32.dll",
    "netapi32.dll",
    "userenv.dll",
    "opengl32.dll",
    "secur32.dll",
    "psapi.dll",
    "wsock32.dll",
    "setupapi.dll",
    "avicap32.dll",
    "avifil32.dll",
    "comctl32.dll",
    "msvfw32.dll",
    "shfolder.dll",
    "odbc32.dll",
    "dxva2.dll",
    "evr.dll",
    "mf.dll",
    "mfplat.dll",
    "glu32.dll",
    "dwmapi.dll",
    "uxtheme.dll",
    "bcrypt.dll",
    "wtsapi32.dll",
    "d2d1.dll",
    "d3d11.dll",
    "dxgi.dll",
    "dwrite.dll",
    "ncrypt.dll",
    "dbghelp.dll",      # blacklisted dll from DrMinGW as it use MSVC dll to show debug dialog.
    "dbgcore.dll",      # blacklisted dll from DrMinGW.
    "gdiplus.dll",
]

# -----------------------------------------------

def find_full_path(filename, path_prefixes):

    path = None
    #print(filename)

    for path_prefix in path_prefixes:
        path_candidate1 = os.path.join(path_prefix, filename)
        #print(path_candidate1)

        if os.path.exists(path_candidate1):
            path = path_candidate1
            #print("Found")
            break

        path_candidate2 = os.path.join(path_prefix, filename.lower())
        #print(path_candidate2)

        if os.path.exists(path_candidate2):
            path = path_candidate2
            #print("Found")
            break

    if path is None:
        raise RuntimeError(
            "Can't find " + filename + ". If it is an inbuilt Windows DLL, "
            "please add it to the blacklist variable in this script.")

    return path

# -----------------------------------------------

def gather_deps(path, path_prefixes, seen):

    ret    = [path]
    output = subprocess.check_output(["objdump", "-p", path]).decode('utf-8').split("\n")

    for line in output:

        if not line.startswith("\tDLL Name: "):
            continue

        dep  = line.split("DLL Name: ")[1].strip()
        ldep = dep.lower()

        #print("Searching: " + ldep)

        if ldep in blacklist:
            continue

        if ldep in seen:
            continue

        dep_path = find_full_path(dep, path_prefixes)
        seen.extend([ldep])
        subdeps  = gather_deps(dep_path, path_prefixes, seen)

        ret.extend(subdeps)

    return ret

# -----------------------------------------------

def main():

    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--installprefix",
        type = str,
        action = "store",
        help = "Install prefix path in build directory."
    )

    parser.add_argument(
        "--efile",
        type = str,
        action = "store",
        help = "EXE or DLL file that you need to bundle dependencies for"
    )

    parser.add_argument(
        "--odir",
        type = str,
        action = "store",
        help = "Directory to store found dlls"
    )

    parser.add_argument(
        "--copy",
        action = "store_true",
        help   = "In addition to printing out the dependencies, also copy them next to the exe_file"
    )

    parser.add_argument(
        "--upx",
        action = "store_true",
        help   = "Only valid if --copy is provided. Run UPX on all the DLLs and EXE. See https://en.wikipedia.org/wiki/UPX for details"
    )

    args = parser.parse_args()

    if args.upx and not args.copy:
        raise RuntimeError("Can't run UPX if --copy hasn't been provided.")

    print("Scan dependencies for " + args.efile)

    # The mingw paths matches in MXE build directory
    default_path_prefixes = [
        args.installprefix + "/qt5/bin/",
        args.installprefix + "/bin/",
    ]

    all_deps = set(gather_deps(args.efile, default_path_prefixes, []))
    all_deps.remove(args.efile)

    #print("\n".join(all_deps))

    if args.copy:

        #print("Copying enabled, will now copy recursively all dependencies near to the exe file.\n")

        for dep in all_deps:
            target = os.path.join(args.odir, os.path.basename(dep))

            # Only copy target file to bundle only if it do not exists yet.
            if not os.path.exists(target):
                print("Copying '%s' to '%s'" % (dep, target))
                shutil.copy(dep, args.odir)

                if args.upx:
                    subprocess.call(["upx", target])

# -----------------------------------------------

if __name__ == "__main__":
    main()
