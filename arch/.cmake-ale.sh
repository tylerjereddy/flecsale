#!/bin/bash
#~----------------------------------------------------------------------------~#
# placeholder
#~----------------------------------------------------------------------------~#

#------------------------------------------------------------------------------#
# Get the path to the project from which this script was called
#------------------------------------------------------------------------------#

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

SRC_DIR=${SCRIPT_DIR}/../..

#------------------------------------------------------------------------------#
# load environment
#------------------------------------------------------------------------------#

source ${SCRIPT_DIR}/env.sh

#------------------------------------------------------------------------------#
# Check required environment variables
#------------------------------------------------------------------------------#

if [ -z "${TPL_INSTALL_PREFIX}" ] ; then
  TPL_INSTALL_PREFIX=${TPL_DEFAULT_INSTALL_PREFIX}
  echo "TPL_INSTALL_PREFIX not set, using ${TPL_INSTALL_PREFIX}"
  echo "To change, set TPL_INSTALL_PREFIX in your environment"
fi

#------------------------------------------------------------------------------#
# Call CMake command
#------------------------------------------------------------------------------#

cmake \
  -DENABLE_UNIT_TESTS=ON \
  -DENABLE_JENKINS_OUTPUT=ON \
  -DENABLE_IO=ON \
  -DENABLE_VORO=ON \
  -DCMAKE_PREFIX_PATH=${TPL_INSTALL_PREFIX} \
  -DBUILD_SHARED_LIBS=OFF
  $SRC_DIR

#------------------------------------------------------------------------------#
# vim: syntax=sh
#------------------------------------------------------------------------------#

#~---------------------------------------------------------------------------~-#
# placeholder
#~---------------------------------------------------------------------------~-#
