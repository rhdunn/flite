# Change Log

This file documents the changes made to the flite project in order to comply
with clause 2 of the project's [BSD license](COPYING).

## [Latest]

Add and modernize the standard project files:

  * Added a CHANGELOG.md file to track the project changes.
  * Added the COPYING file from the 1.1 upstream release.
  * Support the standard GNU project files: AUTHORS, NEWS.
  * Added a .gitignore file to ignore the build output.
  * Renamed configure.in to configure.ac.

Generate the autotools files instead of using old versions:

  * Added an autogen.sh script to setup the configure script.
  * Link to the automake files: config.{guess,sub}, install-sh, missing,
    mkinstalldirs.
  * Use `autoconf` to generate the configure script.

## [1.0]

  * Initial beta version developed by Kevin A. Lenzo and Alan W. Black.
