# Change Log

This file documents the changes made to the flite project in order to comply
with clause 2 of the project's [BSD license](COPYING).

## [1.1-1]

  * Backport building `au_alsa.c` on modern systems from flite 1.4.
  * Build improvements from [1.0-1](#1.0-1).

## [1.1]

  * Upstream version 1.1.

## [1.0-1]

Add and modernize the standard project files:

  * Added a CHANGELOG.md file to track the project changes.
  * Convert the README file to markdown and clean it up for typos and
    readability.
  * Added a .gitignore file to ignore the build output.
  * Renamed configure.in to configure.ac.
  * Support building HTML versions of the markdown files with `kramdown`.

Support the GNU standard project layout:

  * Added the COPYING file from the 1.1 upstream release.
  * Add an AUTHORS file.
  * Link NEWS and README to other files in the project.

Generate the autotools files instead of using old versions:

  * Added an autogen.sh script to setup the configure script.
  * Link to the automake files: config.{guess,sub}, install-sh, missing,
    mkinstalldirs and INSTALL.
  * Use `autoconf` to generate the configure script.

## [1.0]

  * Initial beta version developed by Kevin A. Lenzo and Alan W. Black.
