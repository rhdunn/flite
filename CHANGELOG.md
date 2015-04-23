# Change Log

This file documents the changes made to the flite project in order to comply
with clause 2 of the project's [BSD license](COPYING).

## [1.4]

Upstream version 1.4:

  * Crude multi-voice selection support (may change).
  * 4 basic voices are included 3 clustergen (awb, rms and slt) plus
    the kal diphone database.
  * CMULEX now uses maximum onset for syllabification.
  * ALSA support.
  * Clustergen support (including mlpg with mixed excitation),
    but is still slow on limited processors.
  * Windows support with Visual Studio (specifically for the Olympus
    Spoken Dialog System).
  * WinCE support is redone with cegcc/mingw32ce with example
    example TTS app: Fluwm: Flite on Windows Mobile.
  * Speed-ups in feature interpretation limiting calls to alloc.
  * Speed-ups (and fixes) for converting clunits festvox voices.

## [1.3-1]

  * Restore ALSA support, preserving the backported ALSA fixes from [1.1-1](#1.1-1).

## [1.3]

Upstream version 1.3:

  * Fixes to lpc residual extraction to give better quality output
  * An updated lexicon (`festlex_CMU` from festival-2.0) and better
    compression its about 30% of the previous size, with about
    the same accuracy.
  * Fairly substantial code movements to better support PalmOS and
    multi-platform cross compilation builds.
  * A PalmOS 5.0 port with an small example talking app ("flop").
  * Runs under `ix86_64` linux.

## [1.2-1]

  * Backport the build fix for `tools/find_sts_main.c` from flite 1.3.

## [1.2]

Upstream version 1.2:

  * A build process for diphone and clunits/ldom voices.
  * FestVox voices can be converted (sometimes) automatically.
  * Various bug fixes.
  * Initial support for Mac OS X (not talking to audio device yet)
    but compiles and runs.
  * Text files can be synthesized to a single audio file.
  * Optional shared library support (Linux).

## [1.1-1]

  * Backport building `au_alsa.c` on modern systems from flite 1.4.
  * Build improvements from [1.0-1](#1.0-1).

## [1.1]

Upstream version 1.1.

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

Initial beta version developed by Kevin A. Lenzo and Alan W. Black.
