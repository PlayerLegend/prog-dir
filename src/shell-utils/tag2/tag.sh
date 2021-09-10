#!/bin/busybox sh

set -e

if test -z "$DB"
then
    >&2 echo "Environment variable DB is not defined, point it to a database location"
    exit 1
fi

cmd_db() {
    ( flock -x 200; sqlite3 "$DB" ) 200>"$DB".lock
}

cmd_db <<EOF
CREATE TABLE IF NOT EXISTS tag
( digest TEXT NOT NULL,
  type TEXT NOT NULL,
  name TEXT NOT NULL,
  UNIQUE(digest,type,name) );
EOF

tag_action() {
    if test -z "$TAG" || test -z "$TYPE"
    then
	>&2 echo "Environment variable TAG or TYPE is not defined"
	exit 1
    fi
    
    for file in "$@"
    do
	digest="$(storefile add "$file")"
	filename="$(basename "$file")"

	if test -z "$digest" || test -z "$filename"
	then
	    exit 1
	fi

	$action
    done
}

cmd_add() {

    tag_action_add() {
	cmd_db <<EOF
INSERT OR IGNORE INTO tag (digest,type,name) 
VALUES ("$digest","$TYPE","$TAG");

INSERT OR IGNORE INTO tag (digest,type,name) 
VALUES ("$digest","filename","$filename");
EOF
    }
    
    cmd_add_files() {
	action=tag_action_add tag_action "$@"
    }
    
    cmd_add_"$@"
}

cmd_remove() {
    tag_action_remove() {
	cmd_db <<EOF
DELETE FROM tag
WHERE digest='$digest' AND type="$TYPE" AND name="$TAG"
EOF
    }
    
    cmd_remove_files() {
	action=tag_action_remove tag_action "$@"
    }

    cmd_remove_"$@"
}

cmd_list() {
    cmd_list_digests() {
	if test -z "$TAG"
	then
	    >&2 echo "Environment variable TAG is not defined"
	    exit 1
	fi

	cmd_db <<EOF
SELECT DISTINCT digest
FROM tag
WHERE name = "$TAG" AND type = "$TYPE"
EOF
    }

    cmd_list_tags() {
	digest="$(storefile add "$1")"
	
	cmd_db <<EOF
SELECT name
FROM tag
WHERE digest = "$digest" AND type = "$TYPE"
EOF
    }

    cmd_list_names() {
	db_name() {
	    cmd_db <<EOF
SELECT name
FROM tag
WHERE digest = "$digest" AND type = "filename"
LIMIT 1
EOF
	}
	
	cmd_list_digests | while read digest
	do
	    echo "$digest"
	    db_name="$(db_name)"
	    if test -z "$db_name"
	    then
		echo "$digest"
	    else
		echo "$db_name"
	    fi
	done
    }

    cmd_list_"$@"
}

cmd_link() {
    if test -z "$TAG"
    then
	>&2 echo "Environment variable TAG is not defined"
	exit 1
    fi
    
    if test -z "$STORE_DIR"
    then
	>&2 echo "Environment variable STORE_DIR is not defined"
	exit 1
    fi

    cmd_list names | tr '\n' '\0' | ( mkdir -p "$TAG" && cd "$TAG" && xargs -0 -n 2 storefile symlink )
}

cmd_"$@"
