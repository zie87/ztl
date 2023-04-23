#!/usr/bin/env bash

set -e
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
ROOT_DIR="${SCRIPT_DIR}/../"

BUILD_BASE_DIR="build/gcc/"

function meson_build() {
    BUILD_DIR="${BUILD_BASE_DIR}/$1/$2" 
    BUILD_TYPE=$2

    if [ -d "${BUILD_DIR}" ]; then
        meson setup --reconfigure ${BUILD_DIR} --buildtype ${BUILD_TYPE} -Dcpp_std=$1
    else 
        meson setup ${BUILD_DIR} --buildtype ${BUILD_TYPE} -Dcpp_std=$1
    fi

    meson compile --clean -C ${BUILD_DIR}
    meson compile -v -C ${BUILD_DIR}
    meson test -v -C ${BUILD_DIR} --print-errorlogs
}

function meson_build_all() {
    BUILD_TYPE=$1

    meson_build c++98 ${BUILD_TYPE} 
    meson_build c++03 ${BUILD_TYPE} 
    meson_build c++11 ${BUILD_TYPE} 
    meson_build c++14 ${BUILD_TYPE} 
    meson_build c++17 ${BUILD_TYPE} 
    meson_build c++20 ${BUILD_TYPE} 
}

cd ${ROOT_DIR}

meson_build_all debug
meson_build_all release 
