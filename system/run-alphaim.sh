#!/bin/sh

set -eu

alphaimd >/dev/null 2>&1
exec alphaim-ui
