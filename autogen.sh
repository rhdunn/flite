#!/bin/sh

ln -sf /usr/share/misc/config.guess .
ln -sf /usr/share/misc/config.sub .

autoconf
