#!/bin/sh

recent_file=/tmp/recent-workspaces.txt

touch "$recent_file"

cmd_list() {
    i3-msg -t get_workspaces | jq '.[].name' | sed -e 's/^"//g' -e 's/"$//g'
}

cmd_add_recent() {
    recent_line="$1"

    local tmp="$(mktemp)"

    grep -vFx "$recent_line" "$recent_file" > "$tmp"

    ( echo "$recent_line"
      cat "$tmp" ) > "$recent_file"

    rm "$tmp"
}

cmd_list_recent() {
    input="$(mktemp)"
    recent_sort="$(mktemp)"

    cmd_list | sort > "$input"
    sort "$recent_file" > "$recent_sort"
    
    cat "$recent_file" | while read name
    do
        grep -Fx "$name" "$input"
    done

    comm -23 "$input" "$recent_sort"

    rm "$input" "$recent_sort"
}

select_workspace() {
    cmd_list_recent | rofi -dmenu -i -p "$1"
}

cmd_run() {
    workspace="$(select_workspace "$@")"

    cmd_add_recent "$workspace"

    i3 "$@" "$workspace"
}

cmd_"$@"
