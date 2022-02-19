#!/bin/bash

set -e

################################################################################
# Parameters
################################################################################
TEST_FILES=("alarm_tests"
            "cyclic_task_tests"
            "detection_tests"
            "kinect_frame_tests"
            "kinect_tests"
            "liveview_tests"
            "message_broker_tests"
            "state_persistence_tests")

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DEFAULT_BUILD_FOLDER="build"
ABS_BUILD_FOLDER="$SCRIPT_DIR/$DEFAULT_BUILD_FOLDER"
RUN_TESTS=false

################################################################################
# Option menu
################################################################################
usage() { echo "Usage: $0 [-b <build folder>] [-s <sdk folder>] [-r run tests] [-C clean]" 1>&2; exit 1; }

while getopts ":b:s:rC" o; do
    case "${o}" in
        b)
            FOLDER=${OPTARG}
            case $DIR in
            /*) ABS_BUILD_FOLDER="$FOLDER" ;;
            *) ABS_BUILD_FOLDER="$SCRIPT_DIR/$FOLDER" ;;
            esac
            ;;
        s)
            FOLDER=${OPTARG}
            source $FOLDER/environment-setup-*
            ;;
        r)
            RUN_TESTS=true
            ;;
        C)
            rm -rf $ABS_BUILD_FOLDER
            exit 0;
            ;;
        *)
            usage
            ;;
    esac
done
shift $((OPTIND-1))

echo $ABS_BUILD_FOLDER

################################################################################
# Create and enter the build folder
################################################################################
mkdir -p ${ABS_BUILD_FOLDER};cd ${ABS_BUILD_FOLDER};

################################################################################
# Cmake
################################################################################
cmake ${SCRIPT_DIR};

################################################################################
# Make
################################################################################
make -j$(nproc);

################################################################################
# Execute tests
################################################################################

if [ "$RUN_TESTS" == true ]; then
    for TEST in ${TEST_FILES[@]}; do
        $ABS_BUILD_FOLDER/$TEST --gtest_repeat=20 --gtest_shuffle --gtest_break_on_failure
    done
fi
