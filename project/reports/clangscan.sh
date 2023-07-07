#!/bin/bash

# SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Run Clang static analyzer on whole digiKam source code.
# https://clang-analyzer.llvm.org/
# Dependencies : clang static analyzer version >= 3.9.0.
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
REPORT_DIR="${ORIG_WD}/report.scan"
WEBSITE_DIR="${ORIG_WD}/site"

# Get active git branches to create report description string
TITLE="digiKam-$(parseGitBranch)$(parseGitHash)"
echo "Clang Scan Static Analyzer task name: $TITLE"

# Clean up and prepare to scan.

rm -fr $REPORT_DIR
rm -fr $WEBSITE_DIR

cd ../..

rm -fr build.scan
mkdir -p build.scan
cd build.scan

scan-build cmake -G "Unix Makefiles" . \
      -DCMAKE_BUILD_TYPE=debug \
      -DBUILD_TESTING=ON \
      -DDIGIKAMSC_CHECKOUT_PO=OFF \
      -DDIGIKAMSC_CHECKOUT_DOC=OFF \
      -DDIGIKAMSC_COMPILE_PO=OFF \
      -DDIGIKAMSC_COMPILE_DOC=OFF \
      -DENABLE_KFILEMETADATASUPPORT=ON \
      -DENABLE_AKONADICONTACTSUPPORT=ON \
      -DENABLE_MYSQLSUPPORT=ON \
      -DENABLE_INTERNALMYSQL=ON \
      -DENABLE_MEDIAPLAYER=ON \
      -DENABLE_DBUS=ON \
      -DENABLE_APPSTYLES=ON \
      -DENABLE_QWEBENGINE=ON \
      -Wno-dev \
      ..

scan-build -o $REPORT_DIR \
           -no-failure-reports \
           --keep-empty \
           --html-title $TITLE \
           -v \
           -k \
           make -j$CPU_CORES

cd $ORIG_WD

SCANBUILD_DIR=$(find ${REPORT_DIR} -maxdepth 1 -not -empty -not -name `basename ${REPORT_DIR}`)
echo "Clang Report $TITLE to publish is located to $SCANBUILD_DIR"

# Remove unwanted lines in report accordingly with Krazy configuration.
# Note: Clang do not have an option to ignore directories to scan at compilation time.
# Se this entry in Clang bugzilla: https://bugs.llvm.org/show_bug.cgi?id=22594
krazySkipConfig

for DROP_ITEM in $KRAZY_FILTERS ; do
    echo -e "--- drop $DROP_ITEM from index.html with statistics adjustments"

    # List all report types including current pattern to drop.
    REPORT_ENTRIES=( $(grep $DROP_ITEM $SCANBUILD_DIR/index.html) ) || true

    # STAT_ENTRIES array contains the multi-entries list of statistic types to remove.
    STAT_ENTRIES=()   # clear array

    for RITEM in "${REPORT_ENTRIES[@]}" ; do
        if [[ $RITEM = *bt_* ]]; then
            STAT_ENTRIES[${#STAT_ENTRIES[*]}]=$(echo $RITEM | awk -F "\"" '{print $2}')
        fi
    done

    # to update total statistic with current pattern to drop
    TOTAL_COUNT=0

    # update report statistics values.
    for SITEM in "${STAT_ENTRIES[@]}" ; do
        ORG_STAT_LINE=$(grep "ToggleDisplay(this," $SCANBUILD_DIR/index.html | grep "${SITEM}")
        STAT_VAL=$(echo $ORG_STAT_LINE | grep -o -P '(?<=class=\"Q\">).*(?=<\/td><td>)')
        ORG_STR="class=\"Q\">$STAT_VAL<\/td><td>"
        STAT_VAL=$((STAT_VAL-1))

        if (( $STAT_VAL == 0 )); then
            # if counter is zero, the line can be removed from HTML report.
            sed -i '/$ORG_STAT_LINE/d' $SCANBUILD_DIR/index.html
        else
            NEW_STR="class=\"Q\">$STAT_VAL<\/td><td>"
            NEW_STAT_LINE=${ORG_STAT_LINE/$ORG_STR/$NEW_STR}
            sed -i "s|$ORG_STAT_LINE|$NEW_STAT_LINE|" $SCANBUILD_DIR/index.html
        fi

        TOTAL_COUNT=$((TOTAL_COUNT+1))
    done

    # decrease total statistics with current TOTAL_COUNT

    TOTAL_ORG_STAT_LINE=$(grep "CopyCheckedStateToCheckButtons(this)" $SCANBUILD_DIR/index.html | grep "AllBugsCheck")
    TOTAL_STAT_VAL=$(echo $TOTAL_ORG_STAT_LINE | grep -o -P '(?<=class=\"Q\">).*(?=<\/td><td>)')
    TOTAL_ORG_STR="class=\"Q\">$TOTAL_STAT_VAL<\/td><td>"
    TOTAL_STAT_VAL=$((TOTAL_STAT_VAL-TOTAL_COUNT))
    TOTAL_NEW_STR="class=\"Q\">$TOTAL_STAT_VAL<\/td><td>"
    TOTAL_NEW_STAT_LINE=${TOTAL_ORG_STAT_LINE/$TOTAL_ORG_STR/$TOTAL_NEW_STR}
    sed -i "s|$TOTAL_ORG_STAT_LINE|$TOTAL_NEW_STAT_LINE|" $SCANBUILD_DIR/index.html

    # Remove the report HTML files including current pattern to drop.

    # URL_ENTRIES array contains the external report html file name to remove.
    URL_ENTRIES=()    # clear array

    for RITEM in "${REPORT_ENTRIES[@]}" ; do
        if [[ $RITEM = *report-* ]]; then
            URL_ENTRIES[${#URL_ENTRIES[*]}]=$(echo $RITEM | awk -F "\"" '{print $2}' | awk -F "#" '{print $1}')
        fi
    done

    for UITEM in "${URL_ENTRIES[@]}" ; do
        rm -f $SCANBUILD_DIR/$UITEM

        # Remove the lines from index.html including current pattern to drop.
        grep -v "$UITEM" $SCANBUILD_DIR/index.html > $SCANBUILD_DIR/temp && mv $SCANBUILD_DIR/temp $SCANBUILD_DIR/index.html
    done

done

if [[ $1 != "--nowebupdate" ]] ; then

    # update www.digikam.org report section.
    updateReportToWebsite "clang" $SCANBUILD_DIR $TITLE $(parseGitBranch)

fi

cd $ORIG_DIR
