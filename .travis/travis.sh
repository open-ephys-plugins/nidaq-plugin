#!/bin/bash
# Abort on Error
set -e

export PING_SLEEP=30s
export WORKDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export BUILD_OUTPUT=$WORKDIR/build.out

export PATH=$MSBUILD_PATH:$PATH

touch $BUILD_OUTPUT

dump_output_error() {
    tail -50 $BUILD_OUTPUT  
}

dump_output_success() {
	tail -5 $BUILD_OUTPUT  
}

error_handler() {
    echo ERROR: An error was encountered with the build.
    dump_output_error
    kill $PING_LOOP_PID
    exit 1
}
trap 'error_handler' ERR

bash -c "while true; do echo \$(date) - building ...; sleep $PING_SLEEP; done" &
PING_LOOP_PID=$!

.travis/travis_build.sh >> $BUILD_OUTPUT 2>&1

dump_output_success

kill $PING_LOOP_PID 