#!%BOURNE_SHELL%

# ignore some signals
trap : HUP INT QUIT TSTP

VLOCK_GRAB=%PREFIX%/sbin/vlock-grab
VLOCK_NEW=%PREFIX%/sbin/vlock-new
VLOCK_NOSYSRQ=%PREFIX%/sbin/vlock-nosysrq
VLOCK_LOCK=%PREFIX%/sbin/vlock-lock
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
  echo >&2 "       requires --all."
  echo >&2 "-s or --disable-sysrq: disable sysrq while consoles are locked to"
  echo >&2 "       prevent killing vlock with SAK, requires --all."
  echo >&2 "-v or --version: Print the version number of vlock and exit."
  echo >&2 "-h or --help: Print this help message and exit."
  exit $1
}


main() {
  local opts lock_all disable_sysrq programs switch_new

  opts=`getopt -o acnsvh --long current,all,new,disable-sysrq,version,help \
        -n vlock -- "$@"`

  if [ $? -ne 0 ] ; then
    print_help 1
  fi

  eval set -- "$opts"

  lock_all=0
  switch_new=0
  disable_sysrq=0

  while true ; do
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
        disable_sysrq=1
        shift
        ;;
      -n|--new)
        switch_new=1
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

  if [ $disable_sysrq -ne 0 ] && [ $lock_all -ne 0 ] ; then
    programs="$VLOCK_NOSYSRQ"
  fi

  if [ $switch_new -ne 0 ] && [ $lock_all -ne 0 ] ; then
    programs="$programs $VLOCK_NEW"
    # work around an annoying X11 bug
    sleep 1
  fi

  if [ $lock_all -ne 0 ] ; then
    programs="$programs $VLOCK_GRAB"
    VLOCK_MESSAGE="\
The entire console display is now completely locked.
You will not be able to switch to another virtual console.
"
  else
    VLOCK_MESSAGE="This TTY is now locked."
  fi

  export VLOCK_MESSAGE

  exec $programs $VLOCK_LOCK
}

main "$@"
