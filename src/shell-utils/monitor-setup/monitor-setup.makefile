SH_PROGRAMS += bin/monitor-setup

shell-utils: bin/monitor-setup

bin/monitor-setup: src/shell-utils/monitor-setup/monitor-setup.sh

utils: bin/monitor-setup
