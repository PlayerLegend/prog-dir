
fix_mouse() {
    while read mouse
    do
	xinput --set-prop "$mouse" 'libinput Accel Profile Enabled' 0, 1
    done <<EOF
Microsoft Microsoft Wheel Mouse Optical®
Microsoft Microsoft 3-Button Mouse with IntelliEye(TM)
SINOWEALTH Wired Gaming Mouse
TPPS/2 IBM TrackPoint
EOF
}

monitor() {
    setmode() {
	monitor="$1"
	x="$2"
	y="$3"
	r="$4"
	setmode_sub() {
	    echo "$@"
	    shift
	    ( set -x
	      xrandr --newmode "$@"
	      xrandr --delmode "$monitor" "$1"
	      xrandr --addmode "$monitor" "$1"
	      xrandr --output "$monitor" --mode "$1" --primary )
	}

	setmode_sub `cvt -r "$x" "$y" "$r" | tail -1 | sed 's/"//g'`
    }
    
    case `hostname` in
	marisa )
	    primary=HDMI-A-0
	    secondary=DVI-D-0

	    #setmode $primary 1920 1080 120
	    xrandr --output $primary --mode 1920x1080 -r 144 --primary
	    xrandr --output $secondary --mode 1920x1080 --right-of $primary
	    ;;
	
	x200 )
	    primary=DP2
	    secondary=LVDS1
	    xrandr --output $primary --left-of $secondary --mode 1920x1080 -r 120 --primary
	    ;;

	x220 )
	    primary=DP-2
	    secondary=LVDS-1
	    xrandr --output $primary --left-of $secondary --mode 1920x1080 -r 144 --primary
    esac

    xinput map-to-output '1060PRO Pen Pen (0)' $primary
    xinput map-to-output 'Wacom ISDv4 E6 Pen stylus' $secondary
    sh ~/.fehbg
}

monitor
fix_mouse
