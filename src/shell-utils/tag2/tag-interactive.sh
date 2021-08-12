#!/bin/sh

set -e

buffer() {
    tmp="$(mktemp)"
    cat > "$tmp"
    cat "$tmp"
    rm "$tmp"
}

cmd_select() {
    list() {
	cmd_db <<EOF | buffer
SELECT DISTINCT type
FROM tag
EOF
    }

    name="$(list | rofi -dmenu -i -p "Select a db")"

    if test -z "$name"
    then
	exit 1
    else
	echo "$name" > db-name.txt
	set_type
    fi
}

set_type() {
    get_type() {
	( while ! test -f db-name.txt && ! test "$PWD" = "/"
	  do
	      cd ..
	  done

	  if test -f db-name.txt
	  then
	      cat db-name.txt
	  else
	      echo default
	  fi )
    }

    export TYPE="$(get_type)"
}

set_type

cmd_db() {
    tag db
}

tag_action() {
    tag_prompt() {	
	list() {
	    echo '+mode'
	    
	    cmd_db <<EOF
SELECT DISTINCT "+" || [type]
FROM tag;

SELECT DISTINCT name
FROM tag
WHERE type = "$TYPE"
EOF
	}
	
	mesg() {
	    cmd_db <<EOF
SELECT DISTINCT [type] || ": " || [name]
FROM tag
WHERE digest = "$digest"
EOF
	}

	list | rofi -dmenu -i -p "$1 [$TYPE]" -mesg "$(mesg)"
    }

    while true
    do
	digest="$(storefile digest "$1")"
        
	export TAG="$(tag_prompt "$action")"

	if test "$TAG" = '+mode'
	then
	    if test "$action" = "add files"
	    then
		cmd_remove "$@"
	    else
		cmd_add "$@"
	    fi

	    exit
	fi

	#if test "$TAG" = '+db'
	#then
	#    cmd_select
	#    continue
	#fi

	if echo "$TAG" | grep -q '^+'
	then
	    echo "$TAG" | cut -c2- > db-name.txt
	    export TYPE="$(cat db-name.txt)"
	    echo "Set type $TYPE"
	    continue
	fi
	
	if test -z "$TAG"
	then
	    exit
	else
	    tag $action "$@"
	fi
    done
}

cmd_add() {
    action="add files" tag_action "$@"
}

cmd_remove() {
    action="remove files" tag_action "$@"
}

cmd_view_list() {
    feh -f - \
	--keep-zoom-vp \
	--action1 ";"[tag]"$0 add %F" \
	--action2 ";"[search]"xterm -e view-image search %F" \
	--action3 ";"[spawn]"PARENT=%F tag-interactive viewimage &" \
	--action4 ";"[copy]"convert %F png:- | xclip -selection clipboard -t image/png" \
	--action5 [-asdf]"TAG=asdf-past TYPE=lewd tag add files %F; TAG=asdf TYPE=lewd tag remove files %F" \
	"$@"
}

cmd_link() {
    tag_prompt() {
	list() {
	    cmd_db <<EOF | buffer
SELECT DISTINCT [type] || " " || [name]
FROM tag
ORDER BY type
EOF
	}
	
	list | rofi -dmenu -i -p "link tags"
    }

    line="$(tag_prompt "$1")"
    if test -z "$line"
    then
	exit
    fi
    
    export TAG="$(echo "$line" | cut -d' ' -f2-)"
    export TYPE="$(echo "$line" | cut -d' ' -f1)"

    tag link
}

cmd_viewimage() {
    tag_prompt() {
	if ! test -z "$PARENT"
	then
	    digest="$(storefile digest "$PARENT")"
	else
	    digest=""
	fi
	
	mesg() {
	    cmd_db <<EOF
SELECT DISTINCT [type] || ": " || [name]
FROM tag
WHERE digest = "$digest"
EOF
	}
	
	list() {
	    cmd_db <<EOF | buffer
SELECT DISTINCT [type] || " " || [name]
FROM tag
WHERE digest = "$digest"
ORDER BY type;

SELECT DISTINCT [type] || " " || [name]
FROM tag
ORDER BY type
EOF
	}
	
	list | rofi -dmenu -i -p "view image [$(basename "$DB")]" -mesg "$(mesg)"
    }

    if test -f "$1"
    then
	printf '%s\n' "$@" | cmd_view_list
    else
	line="$(tag_prompt "$1")"
	if test -z "$line"
	then
	    exit
	fi
	
	export TAG="$(echo "$line" | cut -d' ' -f2-)"
	export TYPE="$(echo "$line" | cut -d' ' -f1)"

	list_digests() {
	    cmd_db <<EOF | buffer
SELECT digest
FROM tag
WHERE type = 'filename' AND digest IN (select digest from tag where name='asdf' and type='lewd') 
ORDER BY name;
EOF
	}
	tag list digests | buffer | storefile path | cmd_view_list "$@";
    fi
}

cmd_tagnew() {
    if test -z "$1"
    then
	cat <<EOF
usage:
	$0 [max tag count]
	$0 [min tag count] [max tag count]
EOF
	exit 1
    fi

    if (test -z "$2" && ! test "$1" -ge 0) && (! test -z "$2" && ! test "$1" -le "$2")
    then
	echo "Specified range is invalid"
	exit 1
    fi

    constraints() {
	test -z "$IGNORE_TAGS" || printf "AND name != '%s' " $IGNORE_TAGS
	test -z "$REQUIRE_TAGS" || printf "AND name = '%s' " $REQUIRE_TAGS
	test -z "$REQUIRE_TYPES" || printf "AND type = '%s' " $REQUIRE_TYPES
	test -z "$IGNORE_TYPES" || printf "AND type != '%s' " $IGNORE_TYPES
    }

    if test -z "$2"
    then
	min="-1"
	max="$1"
    else
	min="$1"
	max="$2"
    fi
    
    cmd_db <<EOF | buffer | storefile path | cmd_view_list
SELECT digest
FROM tag
WHERE digest IN ( SELECT digest
      	     	  FROM tag
		  WHERE $(constraints | cut -d' ' -f2-) )
GROUP BY digest
HAVING $min < COUNT(*) AND COUNT(*) <= $max
ORDER BY RANDOM()
EOF
}

cmd_"$@"
