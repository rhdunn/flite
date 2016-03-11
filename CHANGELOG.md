# Change Log

This file documents the changes made to the flite project in order to comply
with clause 2 of the project's [BSD license](COPYING).
 
## [2.0-3]

  * Build fixes for `make_cmulex` from [1.3-3](#1.3-3) and [1.4-3](#1.4-3).

## [2.0-2]

  * Fix a typo in README.md.
  * The location of the voices has changed, so use the new location in
    README.md.
  * Remove duplicated include files.
  * Build fixes to the `testsuite` from [1.9.0-2](#1.9.0-2), [1.3-2](#1.3-2),
    [1.1-2](#1.1-2) and [1.0-2](#1.0-2).

## [2.0-1]

  * Removed the android directory as the android files are no longer there.
  * Build improvements from [1.0-1](#1.0-1).

## [2.0]

Upstream version 2.0.0:

  * Indic language support (Hindi, Tamil and Telugu)
  * A set of 13 CG voices available externally to load.
  * Random forest (multimodel support) improves voice quality.
  * Thread safe (again) after initialization.
  * Improved Android support.
  * Companion multilingual ebook reader
    [Bard Storyteller](http://festvox.org/bard/).

## [1.9.0-3]

  * Build fixes for `make_cmulex` from [1.3-3](#1.3-3) and [1.4-3](#1.4-3).

## [1.9.0-2]

  * Fix compilation with gcc >= 5.0 and -o0.
  * Fix the calls to `lex_lookup` in the testsuite. Based on a patch by
    Åke Forslund <ake.forslund@gmail.com>.
  * Fix building `testsuite/multi_thread`. Based on a patch by Åke Forslund
    <ake.forslund@gmail.com>.
  * Build fixes to the `testsuite` from [1.3-2](#1.3-2), [1.1-2](#1.1-2) and
    [1.0-2](#1.0-2).

## [1.9.0-1]

  * Build improvements from [1.0-1](#1.0-1).

## [1.9.0]

Upstream version 1.9.0:

  * Indic language support.
  * Supports diffrent sample rates/mgc order to tune for speed.
  * Removed PalmOS support from distribution.

## [1.5.6-3]

  * Build fixes for `make_cmulex` from [1.3-3](#1.3-3) and [1.4-3](#1.4-3).

## [1.5.6-2]

  * Build fixes to the `testsuite` from [1.3-2](#1.3-2), [1.1-2](#1.1-2) and
    [1.0-2](#1.0-2).

## [1.5.6-1]

  * Build improvements from [1.0-1](#1.0-1).

## [1.5.6]

Upstream version 1.5.6:

  * SSML support.
  * Generalized tokenstreams (used in Bard Storyteller).
  * Simple-Pulseaudio support.

## [1.5.4-3]

  * Build fixes for `make_cmulex` from [1.3-3](#1.3-3) and [1.4-3](#1.4-3).

## [1.5.4-2]

  * Build fixes to the `testsuite` from [1.3-2](#1.3-2), [1.1-2](#1.1-2) and
    [1.0-2](#1.0-2).

## [1.5.4-1]

  * Build improvements from [1.0-1](#1.0-1).

## [1.5.4]

Upstream version 1.5.4:

  * CG voices as files accessilble by `file:///` and `http://`.
  * Kal diphone 500K smaller.
  * Fixed lots of API issues.

## [1.5.1-3]

  * Build fixes for `make_cmulex` from [1.3-3](#1.3-3) and [1.4-3](#1.4-3).

## [1.5.1-2]

  * Build fixes to the `testsuite` from [1.3-2](#1.3-2), [1.1-2](#1.1-2) and
    [1.0-2](#1.0-2).

## [1.5.1-1]

  * Build improvements from [1.0-1](#1.0-1).

## [1.5.1]

Upstream version 1.5.1.

## [1.4.2-3]

  * Build fixes for `make_cmulex` from [1.3-3](#1.3-3) and [1.4-3](#1.4-3).

## [1.4.2-2]

  * Build fixes to the `testsuite` from [1.3-2](#1.3-2), [1.1-2](#1.1-2) and
    [1.0-2](#1.0-2).

## [1.4.2-1]

  * Build improvements from [1.0-1](#1.0-1).

## [1.4.2]

Upstream version 1.4.2:

  * Better SSML support (actually does something).
  * Better clunit support (smaller).
  * Android support.

## [1.4-3]

  * Build fixes for `make_cmulex` from [1.3-3](#1.3-3).
  * Allow specifying a custom location for the `festival` application in `make_cmulex`.
  * Fix the flite directory location in `make_cmulex`.
  * Fix lexicon pruning.
  * Export `LANG`, `LC_ALL` and `FLITEDIR` in `make_cmulex`.

## [1.4-2]

  * Fix a typo in README.md.
  * Build fixes to the `testsuite` from [1.3-2](#1.3-2), [1.1-2](#1.1-2) and
    [1.0-2](#1.0-2).

## [1.4-1]

  * Build improvements from [1.0-1](#1.0-1).

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
    example TTS app: Flowm: Flite on Windows Mobile.
  * Speed-ups in feature interpretation limiting calls to alloc.
  * Speed-ups (and fixes) for converting clunits festvox voices.

## [1.3-3]

  * Backport the fix for checking if `FLITEDIR` is not defined in `make_cmulex`
    from flite 1.4.2.
  * Backport the fix for running `wfst_build` in `make_lts_wfst.scm` from
    flite 1.4.
  * Backport the fix for running `huff_table` from flite 1.4.
  * Allow specifying a custom location for the `festival` application in `make_cmulex`.
  * Work around setting the heap with festival/speech-tools 2.4 in `make_cmulex`.
  * Fix lexicon pruning.

## [1.3-2]

  * Fix the calls to `ts_open` in the testsuite. Based on a patch by
    Åke Forslund <ake.forslund@gmail.com>.
  * Build fixes to the `testsuite` from [1.1-2](#1.1-2) and [1.0-2](#1.0-2).

## [1.3-1]

  * Restore ALSA support, preserving the backported ALSA fixes from [1.1-1](#1.1-1).
  * Build improvements from [1.0-1](#1.0-1).

## [1.3]

Upstream version 1.3:

  * Fixes to lpc residual extraction to give better quality output
  * An updated lexicon (`festlex_CMU` from festival-2.0.95) and better
    compression its about 30% of the previous size, with about
    the same accuracy.
  * Fairly substantial code movements to better support PalmOS and
    multi-platform cross compilation builds.
  * A PalmOS 5.0 port with an small example talking app ("flop").
  * Runs under `ix86_64` linux.

## [1.2-2]

  * Build fixes to the `testsuite` from [1.1-2](#1.1-2) and [1.0-2](#1.0-2).

## [1.2-1]

  * Backport the build fix for `tools/find_sts_main.c` from flite 1.3.
  * Build improvements from [1.0-1](#1.0-1).

## [1.2]

Upstream version 1.2:

  * A build process for diphone and clunits/ldom voices.
  * FestVox voices can be converted (sometimes) automatically.
  * Various bug fixes.
  * Initial support for Mac OS X (not talking to audio device yet)
    but compiles and runs.
  * Text files can be synthesized to a single audio file.
  * Optional shared library support (Linux).

## [1.1-2]

  * Fix building `testsuite/record_wave_main.c` (backported from flite 1.2).
  * Add `play_wave_sync` from flite 1.3 to fix `testsuite/play_sync_main.c`.
  * Build fixes to the `testsuite` from [1.0-2](#1.0-2).

## [1.1-1]

  * Backport building `au_alsa.c` on modern systems from flite 1.4.
  * Build improvements from [1.0-1](#1.0-1).

## [1.1]

Upstream version 1.1.

## [1.0-2]

  * Fix building `testsuite/play_sync_main.c`.
  * Fix building `testsuite/find_sts_main.c` (backported from flite 1.3).
  * Build the `testsuite` programs as part of the main build.
  * Fix a typo in README.md.

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
