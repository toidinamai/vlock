#!%BOURNE_SHELL%

# ignore some signals
trap : HUP INT QUIT TSTP

# exit on error
set -e

# magic characters to clear the terminal
CLEAR_SCREEN="`echo -e '\033[H\033[J'`"

VLOCK_ENTER_PROMPT="Please press [ENTER] to unlock."

# message that is displayed when console switching is disabled
VLOCK_ALL_MESSAGE="${CLEAR_SCREEN}\
The entire console display is now completely locked.
You will not be able to switch to another virtual console.

${VLOCK_ENTER_PROMPT}"

# message that is displayed when only the current terminal is locked
VLOCK_CURRENT_MESSAGE="${CLEAR_SCREEN}\
This TTY is now locked.

${VLOCK_ENTER_PROMPT}"

# read user settings
if [ -r "${HOME}/.vlockrc" ] ; then
  . "${HOME}/.vlockrc"
fi

VLOCK_MAIN="%PREFIX%/sbin/vlock-main"
VLOCK_PLUGIN_DIR="%PREFIX%/lib/vlock/modules"
VLOCK_VERSION="%VLOCK_VERSION%"

# global arrays for plugin settings
declare -a plugin_name plugin_short_option plugin_long_option plugin_help

load_plugins() {
  local plugin_file long_option short_option help i=0

  for plugin_file in "${VLOCK_PLUGIN_DIR}"/*.sh ; do
    # clear the variables that should be set by the plugin file
    unset short_option long_option help

    # load the plugin file
    . "${plugin_file}"

    # remember the plugin in the global arrays
    # strip the VLOCK_PLUGIN_DIR start and the ".sh" end
    plugin_name[$i]="${plugin_file:${#VLOCK_PLUGIN_DIR}+1:${#plugin_file}-${#VLOCK_PLUGIN_DIR}-4}"
    plugin_short_option[$i]="${short_option}"
    plugin_long_option[$i]="${long_option}"
    plugin_help[$i]="${help}"

    : $((i++))
  done
}

print_plugin_help() {
  local help

  for help in "${plugin_help[@]}" ; do
    if [ -n "${help}" ] ; then
      echo >&2 "${help}"
    fi
  done
}

print_help() {
  echo >&2 "vlock: locks virtual consoles, saving your current session."
  echo >&2 "Usage: vlock [options]"
  echo >&2 "       Where [options] are any of:"
  echo >&2 "-c or --current: lock only this virtual console, allowing user to"
  echo >&2 "       switch to other virtual consoles."
  print_plugin_help
  echo >&2 "-t <seconds> or --timeout <seconds>: run screen locking plugins"
  echo >&2 "       after the given amount of time."
  echo >&2 "-v or --version: Print the version number of vlock and exit."
  echo >&2 "-h or --help: Print this help message and exit."
}

main() {
  local options long_options short_options
  local opt
  declare -a plugins

  short_options="ct:vh"
  long_options="current,timeout:,version,help"

  load_plugins

  for opt in "${plugin_short_option[@]}" ; do
    if [ -n "${opt}" ] ; then
      short_options="${short_options}${opt}"
    fi
  done

  for opt in "${plugin_long_option[@]}" ; do
    if [ -n "${opt}" ] ; then
      long_options="${long_options},${opt}"
    fi
  done

  # test for gnu getopt
  ( getopt -T >/dev/null )

  if [ $? -eq 4 ] ; then
    # gnu getopt
    options=`getopt -o "${short_options}" --long "${long_options}" -n vlock -- "$@"` || getopt_error=1
  else
    # other getopt, e.g. BSD
    options=`getopt "${short_options}" "$@"` || getopt_error=1
  fi

  if [ -n "${getopt_error}" ] ; then
    print_help
    exit 1
  fi

  eval set -- "${options}"

  while : ; do
    case "$1" in
      --)
        # option list end
        shift
        break
        ;;
      -c|--current)
        unset plugins
        shift
        ;;
      -t|--timeout)
        shift
        VLOCK_TIMEOUT="$1"
        shift
        ;;
      -h|--help)
       print_help
       exit
       ;;
      -v|--version)
        echo "vlock version ${VLOCK_VERSION}" >&2
        exit
        ;;
      *) 
        local plugin="" i=0

        # find the plugin this option belongs to

        # try short options first
        for opt in "${plugin_short_option[@]}" ; do
          if [ -n "${opt}" ] && [ "$1" = "-$opt" ] ; then
            plugin="${plugin_name[$i]}"
            break
          fi
          : $((i++))
        done

        if [ -z "${plugin}" ] ; then
          i=0

          # now try long options
          for opt in "${plugin_long_option[@]}" ; do
            if [ -n "${opt}" ] && [ "$1" = "--${opt}" ] ; then
              plugin="${plugin_name[$i]}"
              break
            fi
          : $((i++))
          done
        fi

        if [ -n "${plugin}" ] ; then
          plugins[${#plugins[@]}]="${plugin}"
          shift
        else
          echo "getopt error: unknown option '$1'" >&2
          exit 1
        fi
        ;;
    esac
  done

  # export variables for vlock-main
  export VLOCK_TIMEOUT VLOCK_PROMPT_TIMEOUT
  export VLOCK_MESSAGE VLOCK_ALL_MESSAGE VLOCK_CURRENT_MESSAGE

  exec "${VLOCK_MAIN}" "${plugins[@]}" "$@"
}

main "$@"
