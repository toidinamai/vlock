#!/bin/sh

set -e

if [ "$#" -gt 2 ] ; then
  exit 1
elif [ "$#" -eq 1 ] ; then
  case "$1" in
    requires)
      echo "all"
    ;;
    conflicts)
      echo "new"
    ;;
    before)
      echo "all"
    ;;
  esac

  exit
fi

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
