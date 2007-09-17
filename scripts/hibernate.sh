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

if [ $# -ne 1 ] ; then
  echo >&2 "Usage: $0 <command>"
  exit 1
fi

case "$1" in
  hooks)
    hooks
  ;;
  requires)
    echo "all"
  ;;
  conflicts)
    echo "new"
  ;;
  preceeds)
    echo "all"
  ;;
  *)
    echo >&2 "$0: invalid command"
    exit 1
  ;;
esac
