#!/bin/sh

# Copyright (C) 2015 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>

#
# This script is called from debian/rules and copies the correct variant of
# libunity-scopeharness1.symbols for either vivid (gcc 4.9) or wily (gcc5),
# to allow for a single source tree and dual landings.
#

set -e  # Fail if any command fails.

[ $# -ne 0 ] && {
    echo "usage: $(basename $0)" >&2
    exit 1
}
dir=./debian

# Set soversions depending on whether we are running on vivid or wily and later.

distro=$(lsb_release -c -s)
echo "gen-debian-files: detected distribution: $distro"

harness_symbols_file="libscope-harness1.symbols"
in_harness_symbols_file="${harness_symbols_file}-wily"
if [ -f "${dir}/${harness_symbols_file}-${distro}" ]
then
    in_harness_symbols_file="${harness_symbols_file}-${distro}"
fi

cp "${dir}/${in_harness_symbols_file}" "${dir}/${harness_symbols_file}"
