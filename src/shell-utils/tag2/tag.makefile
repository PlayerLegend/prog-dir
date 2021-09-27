tag2-utils: bin/tag
tag2-utils: bin/tag-interactive
tag2-utils: bin/image
tag2-utils: bin/image-randomize
shell-utils: tag2-utils

bin/tag: src/shell-utils/tag2/tag.sh
bin/tag-interactive: src/shell-utils/tag2/tag-interactive.sh
bin/image: src/shell-utils/tag2/image.sh
bin/image-randomize: src/shell-utils/tag2/image-randomize.sh

SH_PROGRAMS += bin/tag
SH_PROGRAMS += bin/tag-interactive
SH_PROGRAMS += bin/image
SH_PROGRAMS += bin/image-randomize

