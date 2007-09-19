#!/bin/sh

set -e

hooks() {
  while read hook_name ; do
    case "${hook_name}" in
      vlock_start)
        amixer -q set Master mute
      ;;
      vlock_end)
        amixer -q set Master unmute
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
  depends)
    echo "all"
  ;;
esac
