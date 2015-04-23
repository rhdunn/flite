#!/bin/sh

ln -sf CHANGELOG.md NEWS

ln -sf /usr/share/misc/config.guess .
ln -sf /usr/share/misc/config.sub .

autoconf
