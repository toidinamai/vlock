#!%BOURNE_SHELL%

trap : HUP INT QUIT TSTP

VLOCK_GRAB=%PREFIX%/sbin/vlock-grab
VLOCK_NOSYSRQ=%PREFIX%/sbin/vlock-nosysrq
VLOCK_AUTH=%PREFIX%/sbin/vlock-auth
VLOCK_VERSION=%VLOCK_VERSION%

print_help() {
  cat >&2 <<HELP
vlock: locks virtual consoles, saving your current session.
Usage: vlock [options]
       Where [options] are any of:
-c or --current: lock only this virtual console, allowing user to
       switch to other virtual consoles.
-a or --all: lock all virtual consoles by preventing other users
       from switching virtual consoles.
-s or --disable-sysrq: disable sysrq while consoles are locked to
       prevent killing vlock with SAK, requires --all.
-v or --version: Print the version number of vlock and exit.
-h or --help: Print this help message and exit.
HELP
  exit $1
}


main() {
  local opts lock_all disable_sysrq programs

  opts=`getopt -o acsvh --long current,all,disable-sysrq,version,help \
        -n vlock -- "$@"`

  if [ $? -ne 0 ] ; then
    print_help 1
  fi

  eval set -- "$opts"

  lock_all=0
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

  exec $programs $VLOCK_AUTH
}

main "$@"
