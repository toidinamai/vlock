#!/bin/sh

set -e

hooks() {
  while read hook_name ; do
    case "${hook_name}" in
      vlock_save)
        light_status=$(awk '/^status:/ {print $2}' /proc/acpi/ibm/light)

        if [ "${light_status}" = "on" ] ; then
          echo off | sudo tee /proc/acpi/ibm/light >/dev/null
        fi
      ;;
      vlock_save_abort)
        if [ "${light_status}" = "on" ] ; then
          echo on | sudo tee /proc/acpi/ibm/light >/dev/null
        fi
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
