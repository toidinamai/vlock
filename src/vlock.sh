#!%BOURNE_SHELL%

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

# read a user settings
if [ -r "$HOME/.vlockrc" ] ; then
  . "$HOME/.vlockrc"
fi

# ignore some signals
trap : HUP INT QUIT TSTP

VLOCK_ALL=%PREFIX%/sbin/vlock-all
VLOCK_NEW=%PREFIX%/sbin/vlock-new
VLOCK_NOSYSRQ=%PREFIX%/sbin/vlock-nosysrq
VLOCK_CURRENT=%PREFIX%/sbin/vlock-current
VLOCK_VERSION=%VLOCK_VERSION%

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
  echo >&2 "-s or --disable-sysrq: disable sysrq while consoles are locked to"
  echo >&2 "       prevent killing vlock with SAK, requires --all."
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
  local opts lock_all lock_new nosysrq

  # test for gnu getopt
  ( getopt -T >/dev/null )

  if [ $? -eq 4 ] ; then
    # gnu getopt
    opts=`getopt -o acnsvh --long current,all,new,disable-sysrq,version,help \
          -n vlock -- "$@"`
  else
    # other getopt, e.g. BSD
    opts=`getopt acnsvh "$@"`
  fi

  if [ $? -ne 0 ] ; then
    print_help 1
  fi

  eval set -- "$opts"

  lock_all=0
  lock_new=0
  nosysrq=0

  while : ; do
    case "$1" in
      -a|--all)
        lock_all=1
        shift
        ;;
      -c|--current)
        lock_all=0
        shift
        ;;
      -s|--disable-sysrq)
        nosysrq=1
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
      --) shift ; break ;;
      *) 
        echo "getopt error: $1" >&2
        exit 1
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

    if [ $nosysrq -ne 0 ] ; then
      if [ $lock_new -ne 0 ] ; then
        export VLOCK_NEW=1
      else
        unset VLOCK_NEW
      fi

      checked_exec "$VLOCK_NOSYSRQ"
    elif [ $lock_new -ne 0 ] ; then
      checked_exec "$VLOCK_NEW"
    else
      checked_exec "$VLOCK_ALL"
    fi
  else
    : ${VLOCK_MESSAGE:="$VLOCK_CURRENT_MESSAGE"}

    checked_exec "$VLOCK_CURRENT"
  fi
}

main "$@"
