#!/bin/sh

cmd_tag() {
    xterm -e "DB_DIR=/pool1/tag/db STORE_DIR=/pool1/tag/store tag-interactive add \"$1\""
}

cmd_copy() {
    header="$(find -mindepth 2 -maxdepth 2 -name "$1" | rev | cut -d/ -f2- | rev)"
    while true
    do
	dir="$(find -type d -not -name moved | rofi -dmenu -i -mesg "$header")"
	if test -z "$dir"
	then
	    break
	fi
	
	mkdir -p "$dir"
	cp -lL "$1" "$dir"/ || cp -L "$1" "$dir"/
    done
}

cmd_move() {
    dir=moved/"$(date +%Y-%b-%d)"
    mkdir -p "$dir"
    mv "$1" "$dir"/
}

cmd_copy() {
    dir=copied/"$(date +%Y-%b-%d)"
    mkdir -p "$dir"
    cp -lL "$1" "$dir"/"$2" || cp -L "$1" "$dir"/"$2"
}

cmd_sort_old() {
    cmd="$1"

    if ! which "$cmd"
    then
	return 1
    fi

    shift

    find_existing() {
	escape_name() {
	    echo "$@" | sed 's/\[/\\[/g'
	}
	
	find -L * -type f -name "$(escape_name "$1")" | grep -v '\.zipped' | while read line
	do
	    dir="$(dirname "$line")"
	    printf '%s\t%s\n' \
		   "`ls "$dir" | wc -l`" \
		   "$dir"
	done
    }

    choose() {
	( while ! cd .sort_root/
	  do
	      if test / = "$(pwd -P)"
	      then
		  return 1
	      fi
	      
	      cd ..
	  done
	  
	  find -L * -type d | rofi -dmenu -i -p "$cmd" -mesg "$(find_existing "$@")" | tr '\n' '\0' | xargs -0 realpath )
    }

    while true
    do
	choice="$(choose "$@")"
	
	echo "Choice: $choice"
	
	if test -z "$choice"
	then
	    return 1
	else
	    mkdir -p "$choice"
    	    "$cmd" "$@" "$choice"/
	fi
    done
}

cmd_search() {
    tmp="$(mktemp -d)"
    
    file="$1"

    size="$(du "$file" | cut -f1)"

    if test "$size" -gt 5000
    then
	echo "Resizing"
	new_file=/tmp/"$(basename "$size")"
	convert -resize 25% "$file" "$new_file"
	file="$new_file"
    fi	

    echo '<base href="http://saucenao.com/"></base>' > "$tmp"/file.html

    echo "Searching"
    curl -s -L 'http://saucenao.com/search.php' \
	 -X POST \
	 -F 'file=@'"$file" \
	 -F 'press=submit' >> "$tmp"/file.html

    firefox "$tmp"/file.html

    sleep 3

    rm -r "$tmp"
}

cmd_directory() {
    list="$(mktemp)"

    find "$@" -type f | sort -V > "$list"
    
    feh -f "$list" \
	--keep-zoom-vp \
	--action1 [tag]"xterm -e $0 tag %F" \
	--action2 [search]"xterm -e $0 search %F" \
	--action3 [copy]"$0 copy %F" \
	--action4 [move]"$0 move %F"

    rm "$list"
}

cmd_image() {
        feh "$@" \
		--keep-zoom-vp \
		--action1 [tag]"xterm -e tag-interactive add %F" \
		--action2 [search]"xterm -e $0 search %F" \
		--action3 [copy]"$0 copy %F" \
		--action4 [move]"$0 move %F"

	#feh "$@" \
	    #--keep-zoom-vp \
	    #--action2 [tag]"$0 tag %F" \
	    #--action3 [search]"$0 search %F"

}

cmd_comic() {
    tmp="$(mktemp -d --suffix=-comic)"
    realpath "$@" | ( cd "$tmp" && tr '\n' '\0' | xargs -0 7z x )
    find "$tmp" -not -name '*.txt' | sort -V > "$tmp"/list.txt
    feh -f "$tmp"/list.txt \
	--keep-zoom-vp \
	--action1 [tag-comic]"xterm -e $0 tag \"$1\"" \
	--action2 [search]"xterm -e $0 search %F" \
	--action3 [copy]"xterm -e $0 copy '$@'" \
	--action4 [tag-page]"xterm -e $0 tag %F" \
	--action5 [copy-page]"xterm -e $0 copy '$@' '$@ '%F" \

    rm -r "$tmp"
}

cmd_"$@"
