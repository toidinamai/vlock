#!/bin/sh

set -e

if [ -n "$1" ] ; then
  exit
fi

light_status=$(awk '/^status:/ {print $2}' /proc/acpi/ibm/light)

if [ "${light_status}" != "off" ] ; then
  exit
fi

while read hook_name ; do
  case "${hook_name}" in
    vlock_save)
      echo off > /proc/acpi/ibm/light
    ;;
    vlock_save_abort)
      echo on > /proc/acpi/ibm/light
    ;;
  esac
done
