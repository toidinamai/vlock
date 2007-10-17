#!/bin/sh
###########################################################################
# cpufreq_low.sh, version 0.1.1
# 2007-10-15
#
# This vlock plugin script tries to set the cpu frequency scaling to the
# lowest possible value. It requires the cpufreq utilities to be installed.
#
# Copyright (C) 2007 Rene Kuettner <rene@bitkanal.net>
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; version 2.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307  USA
#
###########################################################################

set -e

DEPENDS="all"

saved_frequencies=()
saved_governors=()
cpu_n=()

cpufreq_low_init() {
  #FIXME: some cpus may not be able to switch the frequency
  cpu_n=$(awk '/^processor/ {printf(" "$3)}' /proc/cpuinfo)
}

cpufreq_low_set() {
  for i in $cpu_n ; do
    lowest_frequency=$(cpufreq-info -c $i -l | awk '/[0-9]+/ {print $1}')

    saved_frequencies[$i]=$(cpufreq-info -c $i -f)
    saved_governors[$i]=$(cpufreq-info -c $i -p | awk '{print $3}')

    sudo cpufreq-set -c $i -g userspace
    sudo cpufreq-set -c $i -f $lowest_frequency
  done
}

cpufreq_low_restore() {
  for i in $cpu_n ; do
    sudo cpufreq-set -c $i -f ${saved_frequencies[$i]}
    sudo cpufreq-set -c $i -g ${saved_governors[$i]}
  done
}

hooks() {
  while read hook_name ; do
    case "${hook_name}" in
      vlock_start)
        cpufreq_low_init
        cpufreq_low_set
      ;;
      vlock_end)
        cpufreq_low_restore
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
  preceeds)
    echo "${PRECEEDS}"
  ;;
  succeeds)
    echo "${SUCCEEDS}"
  ;;
  requires)
    echo "${REQUIRES}"
  ;;
  needs)
    echo "${NEEDS}"
  ;;
  depends)
    echo "${DEPENDS}"
  ;;
  conflicts)
    echo "${CONFLICTS}"
  ;;
  *)
    echo >&2 "$0: unknown command '$1'"
    exit 1
  ;;
esac
