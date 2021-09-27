upward-make-utils: bin/upmake
shell-utils: upward-make-utils

bin/upmake: src/shell-utils/upward-make/upward-make.sh

SH_PROGRAMS += bin/upmake

