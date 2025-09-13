#!/bin/sh

[ ! -f AUTHORS   ] && touch AUTHORS
[ ! -f ChangeLog ] && touch ChangeLog
[ ! -f NEWS      ] && touch NEWS

autoreconf --install --force
