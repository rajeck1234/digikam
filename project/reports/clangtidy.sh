#!/bin/bash

# SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Run Clang tidy lint static analyzer on whole digiKam source code.
#
# If '--nowebupdate' is passed as argument, static analyzer results are just created locally.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

. ./common.sh

checksCPUCores

ORIG_WD="`pwd`"
REPORT_DIR="${ORIG_WD}/report.tidy"
WEBSITE_DIR="${ORIG_WD}/site"

# Get active git branches to create report description string
TITLE="digiKam-$(parseGitBranch)$(parseGitHash)"
echo "Clang Tidy Static Analyzer task name: $TITLE"

# Clean up and prepare to scan.

rm -fr $REPORT_DIR
rm -fr $WEBSITE_DIR

mkdir -p $REPORT_DIR

/usr/share/clang/run-clang-tidy.py -quiet -j$CPU_CORES -p  ../../build/ | tee $REPORT_DIR/clang-tidy.log

python3 ./clangtidy_visualizer.py $REPORT_DIR/clang-tidy.log

#rm -f $REPORT_DIR/clang-tidy.log
mv tidy.html $REPORT_DIR/index.html

if [[ $1 != "--nowebupdate" ]] ; then

    updateReportToWebsite "tidy" $REPORT_DIR $TITLE $(parseGitBranch)

fi

cd $ORIG_DIR

