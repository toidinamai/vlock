#!/bin/sh

set -e

hooks() {
  light_status=$(awk '/^status:/ {print $2}' /proc/acpi/ibm/light)

  if [ "${light_status}" != "on" ] ; then
    exit
  fi

  while read hook_name ; do
    case "${hook_name}" in
      vlock_save)
        echo off | sudo tee /proc/acpi/ibm/light >/dev/null
      ;;
      vlock_save_abort)
        echo on | sudo tee /proc/acpi/ibm/light >/dev/null
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
esac
