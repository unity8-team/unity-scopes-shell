#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "ERROR: Usage $0 SCOPE_ID"
  exit 1
fi

DIR="$(dirname "$0")"
APP_ID="example" qmlscene "$DIR/Settings.qml" -- "$1"
