#/bin/sh
##################
# Build examples #
##################

set -xe 

CFLAGS="-o3 -Wall -Wextra"
LIBS=""

clang $CFLAGS -o example_json_one ./examples/example_json_one.c $LIBS
clang $CFLAGS -o example_json_omit_cols ./examples/example_json_omit_cols.c $LIBS
