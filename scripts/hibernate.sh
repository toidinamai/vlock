#!/bin/sh

set -e

hooks() {
  oldvt=$(fgconsole)

  while read hook_name ; do
    case "${hook_name}" in
      vlock_start)
        chvt 63
      ;;
      vlock_end)
        chvt "${oldvt}"
      ;;
      vlock_save)
        hibernate
      ;;
    esac
  done
}

if [ $# -eq 1 ] || [ "$2" = "hooks" ] ; then
  hooks
else
  echo "requires=all"
  echo "conflicts=new"
  echo "before=all"
fi
