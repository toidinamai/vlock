#!%BOURNE_SHELL%

# ignore some signals
trap : HUP INT QUIT TSTP

# exit on error
set -e

# magic characters to clear the terminal
CLEAR_SCREEN="`echo -e '\033[H\033[J'`"

# message that is displayed when console switching is disabled
VLOCK_ALL_MESSAGE="${CLEAR_SCREEN}\
The entire console display is now completely locked.
You will not be able to switch to another virtual console.

Please press [ENTER] to unlock."

# message that is displayed when only the current terminal is locked
VLOCK_CURRENT_MESSAGE="${CLEAR_SCREEN}\
This TTY is now locked.

Please press [ENTER] to unlock."

# read user settings
if [ -r "$HOME/.vlockrc" ] ; then
  . "$HOME/.vlockrc"
fi

VLOCK_ALL="%PREFIX%/sbin/vlock-all"
VLOCK_NEW="%PREFIX%/sbin/vlock-new"
VLOCK_MAIN="%PREFIX%/sbin/vlock-main"
VLOCK_PLUGIN_DIR="%PREFIX%/lib/vlock/modules"
VLOCK_VERSION="%VLOCK_VERSION%"

# global arrays for plugin settings
declare -a plugin_name plugin_short_option plugin_long_option plugin_help

load_plugins() {
  local plugin_file long_option short_option help i=0

  for plugin_file in "$VLOCK_PLUGIN_DIR"/*.sh ; do
    # clear the variables that should be set by the plugin file
    unset short_option long_option help

    # load the plugin file
    . "$plugin_file"

    # remember the plugin in the global arrays
    # strip the VLOCK_PLUGIN_DIR start and the ".sh" end
    plugin_name[$i]="${plugin_file:${#VLOCK_PLUGIN_DIR}+1:${#plugin_file}-${#VLOCK_PLUGIN_DIR}-4}"
    plugin_short_option[$i]="$short_option"
    plugin_long_option[$i]="$long_option"
    plugin_help[$i]="$help"

    : $((i++))
  done
}

print_plugin_help() {
  local help

  for help in "${plugin_help[@]}" ; do
    if [ -n "$help" ] ; then
      echo >&2 "$help"
    fi
  done
}

print_help() {
  echo >&2 "vlock: locks virtual consoles, saving your current session."
  echo >&2 "Usage: vlock [options]"
  echo >&2 "       Where [options] are any of:"
  echo >&2 "-c or --current: lock only this virtual console, allowing user to"
  echo >&2 "       switch to other virtual consoles."
  echo >&2 "-a or --all: lock all virtual consoles by preventing other users"
  echo >&2 "       from switching virtual consoles."
  echo >&2 "-n or --new: allocate a new virtual console before locking,"
  echo >&2 "       implies --all."
  echo >&2 "-t <seconds> or --timeout <seconds>: run screen locking plugins"
  echo >&2 "       after the given amount of time."
  print_plugin_help
  echo >&2 "-v or --version: Print the version number of vlock and exit."
  echo >&2 "-h or --help: Print this help message and exit."
  exit $1
}

checked_exec() {
  if [ -f "$1" ] && [ ! -x "$1" ] ; then
    echo >&2 "vlock: cannot execute \`$1': Permission denied"
    echo >&2 "Please check the documentation for more information."
    exit 1
  else
    exec "$@"
  fi
}

main() {
  local options long_options short_options
  local opt
  local lock_all=0 lock_new=0

  short_options="acnt:vh"
  long_options="current,all,new,timeout:,version,help"

  load_plugins

  for opt in "${plugin_short_option[@]}" ; do
    if [ -n "$opt" ] ; then
      short_options="$short_options$opt"
    fi
  done

  for opt in "${plugin_long_option[@]}" ; do
    if [ -n "$opt" ] ; then
      long_options="$long_options,$opt"
    fi
  done

  # test for gnu getopt
  ( getopt -T >/dev/null )

  if [ $? -eq 4 ] ; then
    # gnu getopt
    options=`getopt -o "$short_options" --long "$long_options" -n vlock -- "$@"`
  else
    # other getopt, e.g. BSD
    options=`getopt "$short_options" "$@"`
  fi

  if [ $? -ne 0 ] ; then
    print_help 1
  fi

  eval set -- "$options"

  while : ; do
    case "$1" in
      --)
        # option list end
        shift
        break
        ;;
      -a|--all)
        lock_all=1
        shift
        ;;
      -c|--current)
        lock_all=0
        shift
        ;;
      -t|--timeout)
        shift
        VLOCK_TIMEOUT="$1"
        export VLOCK_TIMEOUT
        shift
        ;;
      -n|--new)
        lock_new=1
        lock_all=1
        shift
        ;;
      -h|--help)
       print_help 0
       ;;
      -v|--version)
        echo "vlock version $VLOCK_VERSION" >&2
        exit
        ;;
      *) 
        local plugin="" i=0

        for opt in "${plugin_short_option[@]}" ; do
          if [ -n "$opt" ] && [ "$1" = "-$opt" ] ; then
            plugin="${plugin_name[$i]}"
            break
          fi
          : $((i++))
        done

        if [ -z "$plugin" ] ; then
          i=0

          for opt in "${plugin_long_option[@]}" ; do
            if [ -n "$opt" ] && [ "$1" = "--$opt" ] ; then
              plugin="${plugin_name[$i]}"
              long_options="$long_options,$opt"
            fi
          done
        fi

        if [ -n "$plugin" ] ; then
          plugins[${#plugins[@]}]="$plugin"
          shift
        else
          echo "getopt error: $1" >&2
          exit 1
        fi
        ;;
    esac
  done

  if [ $lock_new -ne 0 ] && [ -n "$DISPLAY" ] ; then
    # work around an annoying X11 bug
    sleep 1
  fi

  # export variables for vlock-current
  export VLOCK_MESSAGE VLOCK_PROMPT_TIMEOUT

  if [ $lock_all -ne 0 ] ; then
    : ${VLOCK_MESSAGE:="$VLOCK_ALL_MESSAGE"}

    if [ $lock_new -ne 0 ] ; then
      checked_exec "$VLOCK_NEW" "${plugins[$@]}" "$@"
    else
      checked_exec "$VLOCK_ALL" "${plugins[$@]}" "$@"
    fi
  else
    : ${VLOCK_MESSAGE:="$VLOCK_CURRENT_MESSAGE"}

    checked_exec "$VLOCK_MAIN" "${plugins[$@]}" "$@"
  fi
}

main "$@"
