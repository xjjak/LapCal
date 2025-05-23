#!/usr/bin/env nix-shell
#! nix-shell shell.nix -i bash

racket "$(dirname $0)/src/main.rkt" "$@"
