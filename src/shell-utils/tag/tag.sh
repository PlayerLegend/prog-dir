#!/bin/sh

#set -x

set -e

switch_mode_prompt="+mode"
switch_table_prompt="+table"

program="$0"

db=/pool1/art/sort.db

find_table() {
    if ! test -f ~/.tag-table
    then
	echo 'base' > ~/.tag-table
    fi

    cat ~/.tag-table
}

set_table() {
    table="$1"
    mfs_root="/tag/$table"

    if test -z "$table"
    then
	echo "Couldn't find a table"
	exit 1
    fi

}

set_table "$(find_table)"

runsql() {
    sqlite3 "$db"
}

init_db() {
    runsql <<EOF
create table if not exists $table ( digest TEXT NOT NULL, name TEXT NOT NULL, tag TEXT NOT NULL, unique(name,tag) );
EOF
}

set_digest_name() {
    if echo "$name" | grep -q "$digest"
    then
	digest_name="$name"
    else
	digest_name="$digest $name"
    fi
}

set_digest() {
    digest="$(ipfs add --dereference-args --pin=false -Q "$file")"
}

is_tagged() {
    ( sql_select() {
	  runsql <<EOF
select * from $table where digest="$digest" and name="$name" and tag="$tag";
EOF
      }
      
      test "$(sql_select | wc -l)" -gt 0 )
}

import_digest() {
    ( mfs_path="$mfs_root"/"$tag"/"$name"
      existing_digest="$(ipfs files stat "$mfs_path" 2>/dev/null | head -1)"
      if test -z "$existing_digest"
      then
	  ipfs files mkdir -p "$mfs_root"/"$tag"
	  ipfs files cp /ipfs/"$digest" "$mfs_root"/"$tag"/"$name"
      elif ! test "$digest" = "$existing_digest"
      then
	  echo "Filename '$name' already present in tag, but with a different hash"
      fi )
}

cmd_add_tag_files() {
    ( tag="$1"
      shift
      for file in "$@"
      do
	  name="$(basename "$file")"
	  echo "Adding '$name' to '$tag'"
	  set_digest
	  if runsql <<EOF
insert into $table values("$digest", "$name", "$tag")
EOF
	  then
	      import_digest
	  fi
      done )
}

cmd_list_digest_tags() {
    ( digest="$1"
      runsql <<EOF
select tag from $table where digest='$digest';
EOF
    )
}

cmd_list_file_tags() {
    ( file="$1"
      set_digest
      cmd_list_digest_tags "$digest" )
}

cmd_list_tag_files() {
    ( tag="$1"
      runsql <<EOF
.separator ' '
select digest, name from $table where tag='$tag'
EOF
    )
}

cmd_get_tag_files() {
    ( tag="$1"
      digest="$(ipfs files stat "$mfs_root" | head -1)"
      if test -z "$digest"
      then
	  exit 1
      fi
      #ipfs get "$digest"/"$tag"
      list_files() {
	  runsql <<EOF
select [digest] || '
-o
' || [name] from $table where tag='$tag'
EOF
      }
      ( mkdir -p "$tag" && cd "$tag" && list_files | tr '\n' '\0' | xargs -0 -n 3 -P 10 ipfs get )
    )
}

cmd_get_tag_files_interactive() {
    choice="$(cmd_list_tags | rofi -dmenu -i -p "get tag files")"
    if ! test -z "$choice"
    then
	cmd_get_tag_files "$choice"
    fi
}

cmd_list_tags() {
    runsql <<EOF | sort -u
select tag from $table
EOF
}

cmd_import() {
    import() {
	cmd_list_tags | while read tag
	do
	    cmd_list_tag_files "$tag" | while read line
	    do
		digest="$(echo "$line" | cut -d' ' -f1)"
		name="$(echo "$line" | cut -d' ' -f2-)"
		
		echo "[$tag] [$digest] $name"
		import_digest
	    done
	done
    }

    for table in `echo .tables | runsql`
    do
	( set_table "$table"
	  import )
    done
}

choose() {
    if test -z "$message"
    then
	rofi -dmenu -i -p "$prompt"
    else
	rofi -dmenu -i -p "$prompt" -mesg "$message"
    fi
}

cmd_add_file_tags_interactive() {
    ( while true
      do
	  prompt="add tags [$table]"
	  message="$(cmd_list_file_tags "$1")"
	  choice="$( (echo "$switch_mode_prompt"; echo "$switch_table_prompt"; cmd_list_tags) | choose)"
	  if test -z "$choice"
	  then
	      break
	  elif test "$choice" = "$switch_mode_prompt"
	  then
	      "$program" remove_file_tags_interactive "$@"
	      break
	  elif test "$choice" = "$switch_table_prompt"
	  then
	      cmd_choose_table
	  else
	      cmd_add_tag_files "$choice" "$@"
	  fi
      done )
}

apply_remove() {
    ( mfs_path="$mfs_root"/"$tag"/"$name"
      existing_digest="$(ipfs files stat "$mfs_path" 2>/dev/null | head -1)"

      if ! test -z "$digest" && test "$existing_digest" = "$digest"
      then
	  ipfs files rm "$mfs_path"
	  echo "Removed mfs $mfs_path"
      fi

      runsql <<EOF
delete from $table where digest="$digest" and name="$name" and tag="$tag";
EOF
    )
}

cmd_remove_tag_files() {
    ( tag="$1"
      shift
      for file in "$@"
      do
	  name="$(basename "$file")"
	  echo "Removing '$name' from '$tag' [$table]"
	  set_digest
	  apply_remove
      done )    
}

cmd_remove_file_tags_interactive() {
    ( while true
      do
	  prompt="remove tags [$table]"
	  message=""
	  choice="$( (echo "$switch_mode_prompt"; cmd_list_file_tags "$1") | choose)"
	  if test -z "$choice"
	  then
	      break
	  elif test "$choice" = "$switch_mode_prompt"
	  then
	      "$program" add_file_tags_interactive "$@"
	      break
	  else
	      cmd_remove_tag_files "$choice" "$@"
	  fi
      done )
}

cmd_remove_tag_files_interactive() {
    ( tag="$1"
      while true
      do
	  prompt="remove files from '$tag'"
	  message=""
	  choice="$(cmd_list_tag_files "$tag" | choose)"
	  if test -z "$choice"
	  then
	      break
	  else
	      digest="$(echo "$choice" | cut -d' ' -f1)"
	      name="$(echo "$choice" | cut -d' ' -f2-)"
	      apply_remove
	  fi
      done )
}

cmd_choose_table() {
    list_tables() {
	runsql <<EOF
.tables
EOF
    }

    table="$(printf '%s\n' `list_tables` | rofi -dmenu -i)"

    if test -z "$table"
    then
	table="$(cat ~/.tag-table)"
    else
	echo "$table" > ~/.tag-table
    fi
}

cmd_open() {
    runsql
}

cmd_init() {
    for arg in "$@"
    do
	set_table "$arg"
	init_db
    done
    
    echo "Initialized $db"
}

init_db
cmd_"$@"
