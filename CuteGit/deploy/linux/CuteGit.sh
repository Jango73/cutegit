#!/bin/bash
# Get the full name of this file
base_full="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/$(basename "${BASH_SOURCE[0]}")"
# Get the directory of this file
base=$(dirname "$0")
# Enter directory of this file
cd $base
# Make OS look for librairies in "./"
export LD_LIBRARY_PATH=.
#export QT_DEBUG_PLUGINS=1
# Launch CuteGit specifying this file as a stub
./CuteGit --stub "$base_full" $@
