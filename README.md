# Flite: Festival Lite

[Flite](http://cmuflite.org) is a small fast run-time speech synthesis
engine. It is the latest addition to the suite of free software synthesis
tools including University of Edinburgh's Festival Speech Synthesis
System and Carnegie Mellon University's FestVox project, tools, scripts
and documentation for building synthetic voices.  However, flite itself
does not require either of these systems to compile and run.

This is the first beta release of the code, although the system is
basically functional it is not complete.  An example voice is
included but deserves much more work.

The core Flite library was developed by [Alan W. Black](mailto:awb@cs.cmu.edu)
(mostly in his so-called spare time) while employed in the Language
Technologies Institute at Carnegie Mellon University.  The name
`flite`, originally chosen to mean `festival-lite` is perhaps doubly
appropriate as a substantial part of design and coding was done over
30,000ft while awb was travelling.

The voices, lexicon and language components of flite, both their
compression techniques and their actual contents were developed by
[Kevin A. Lenzo](mailto:lenzo@cs.cmu.edu) and
[Alan W Black](mailto:awb@cs.cmu.edu).

Flite is the answer to the complaint that Festival is too big, too slow,
and not portable enough.

* Flite is designed for very small devices, such as PDAs, and also
  for large server machine with lots of ports.
* Flite is not a replacement for Festival but an alternative run time
  engine for voices developed in the FestVox framework where size and
  speed is crucial.
* Flite is all in ANSI C, it contains no C++ or Scheme, thus requires
  more care in programming, and is harder to customize at run time.
* It is thread safe.
* Voices, lexicons and language descriptions can be compiled
  (mostly automatically) into C representations from their FestVox formats.
* All voices, lexicons and language model data are const and in the
  text segment (i.e. they may be put in ROM).  As they are linked in
  at compile time, there is virtually no startup delay.
* Although the synthesized output is not exactly the same as the same
  voice in Festival they are effectively equivalent.  That is flite
  doesn't sound better or worse than the equivalent voice in festival,
  just faster, smaller and scalable.
* For standard diphone voices, maximum run time memory
  requirements are approximately less than twice the memory requirement
  for the waveform generated.  For 32bit archtectures
  this effectively means under 1M. (Later versions will include a
  streaming option which will reduce this to less than one quarter).
* The flite program supports, synthesis of individual strings or files
  (utterance by utterance) to direct audio devices or to waveform files.
* The flite library offers simple functions suitable for use in specific
  applications.

Flite is distributed with a single 8K diphone voice (derived from the
`cmu_us_kal` voice), an pruned lexicon (derived from
cmulex) and a set of models for US English.  Here are comparisons
with Festival using basically the same 8KHz diphone voice

|           | Flite | Festival |
|-----------|-------|----------|
| core code | 50K   | 2.6M     |
| USEnglish | 35K   | ??       |
| lexicon   | 1.6M  | 5M       |
| diphone   | 2.1M  | 2.1M     |
| runtime   | <1M   | 16-20M   |

On a 500Mhz PIII, a timing test of the first two chapters of
[Alice in Wonderland](doc/alice) was done.  This produces about
1300 seconds of speech.  With flite it takes 19.128 seconds (about
70.6 times faster than real time) with Festival it takes 97 seconds
(13.4 times faster than real time).  On the ipaq (with the 16KHz diphones)
flite synthesizes 9.79 time faster than real time.

Requirements:

* A good C compiler, some of these files are quite large and some
  C compilers might choke on these, gcc is fine.  Sun CC 3.01 has been
  tested too.  We have not yet tested the compiled under Visual C
* GNU Make
* An audio device isn't required as flite can write its output to
  a waveform file.

Supported platforms:

* Various Intel Linux systems (and iPaq Linux)
* FreeBSD 3.x and 4.x
* Solaris 5.7
* OSF1 V4.0 (gives an unimportant warning about sizes when compiled `cst_val.c`)
* Windows (current only crossed compiled from Linux, and may need a little
  work to get running again)
* WinCE (2.11 and 3.0) (probably)

Other similar platforms should just work, we have also cross compiled
on a Linux machine for StrongARM.  However note that new byte order
architectures may not work directly as there is some careful
byte order constraints in some structures.  These are portable but may
require reordering of some fields, contact us if you are moving to
a new archiecture.

## Compilation

In general

    tar zxvf flite-1.0-beta.tar.gz
    cd flite
    make

Where tar is gnu tar (gtar), and make is gnu make (gmake).

Configuration should be automatic, but mayeb doesn't work in all cases
especially if you have some new compiler.  You can explicitly set to
compiler in config/config and add any options you see fit.   Configure
tries to guess these but it might be able for cross compilation cases
Interesting options there are:

  * `-DWORDS_BIGENDIAN=1` for bigendian machines (e.g. Sparc, M68x);
  * `-DNO_UNION_INITIALIZATION=1` for compilers without C 99 union inintialization;
  * `-DCST_AUDIO_NONE` if you don't need/want audio support.

To compile for the ipaq Linux distribution (we've done this on Familiar
and Intimate), no automatic cross compilation configuration is
set up yet.  Thus:

  *  run `configure` on a Linux machine;
  *  edit `config/config`;
  *  change `gcc`, `ar` and `ranlib` to their arm-linux equivalents;
  *  change `FL_VOX` to `cmu_us_kal16`;
  *  run:

         make clean
         make
         arm-linux-strip bin/flite

  *  copy `bin/flite` to the ipaq.  This binary is also available from
     [flite-1.0_bin_arm-linux.tar.gz](http://cmuflite.org/packed/flite-1.0/flite-1.0_bin_arm-linux.tar.gz).
     Because the Linux ipaq audio driver only supports 16KHz (and more)
     we include this larger voice.  This voice also used fixed point
     rather floating point as the StrongARM doesn't have floating
     point instructions.  We are working on a much smaller voice for
     the ipaq hopefully small enough to fit in the flash.

# Usage

If it compiles properly a binary will be put in `bin/`. By
default the `-g` option is enabled, so it will be bigger that is actually required.

  *  `./bin/flite "Flite is a small fast run-time synthesis engine" flite.wav`

     Will produce an 8KHz riff headered waveform file (riff is Microsoft's
     wave format often called .WAV).

  *  `./bin/flite doc/alice`

     Will play the text file [doc/alice](doc/alice).  If the first argument
     contains a space it is treated as text otherwise it is treated as a
     filename. If a second argument is given a waveform file is written to it,
     if no argument is given or `play` is given it will attempt to
     write directly to the audio device (if supported).  if `none`
     is given the audio is simply thrown away (used for benchmarking).
     Explicit options are also available.

  *  `./bin/flite -v doc/alice none`

     Will synthesize the file without playig the audio and give a summary
     of the speed.

An additional set of feature setting options are available, these are
*debug* options, Voices are represented as sets of feature values (see
`lang/cmu_us_kal/cmu_us_kal.c`) and you can override values on the
command line.  This can stop flite from working if malicious values
are set and therefore this facility is not intended to be made
available for standard users.  But these are useful for
debugging.

Some typical examples are:

  *  `./bin/flite --sets join_type=simple_join doc/intro`

     Use simple concatenation of diphones without prosodic modification.

  *  `./bin/flite --seti verbosity=1 doc/alice`

     Print sentences as they are said.

  *  `./bin/flite --setf Duration_Stretch=1.5 doc/alice`

     Make it speak slower.

  *  `./bin/flite --setf int_f0_target_mean=145 doc/alice`

     Make it speak higher.

The talking clock is an example talking clock as discussed on
[ldom](http://festvox.org/ldom). It requires a single argument in the
format `HH:MM`. Under Unix you can call it as:

    ./bin/flite_time `date +%H:%M`

# Voice Quality

So you've eagerly downloaded flite, compiled it and run it, now you
are disappointed that is doesn't sound wonderful, sure its fast and
small but what you really hoped for was the dulcit tones of a deep
baritone voice that would make you desperately hang on every phrase it
sang.  But instead you get an 8Khz diphone voice that sounds like it
came from the last millenium.

Well, first, you are right, it is an 8KHz diphone voice from the last
millenium, and that was actually deliberate.  As we developed flite we
wanted a voice that was stable and that we could directly compare with
that very same voice in Festival.  Flite is an *engine*.  We want to
be able take voices built with the FestVox process and compile them
for flite, the result should be exactly the same quality (though of
course trading the size for quality in flite is also an option).  The
included voice is just an sample voice that was used in the testing
process.  We have better voices in Festival and are working on the
coversion process to make it both more automatic and more robust and
tunable, but we haven't done that yet, so in this first beta release.
This old poor sounding voice is all we have, sorry, we'll provide you
with free, high-quality, scalable, configurable, natural sounding
voices for flite, in all languages and dialects, with the tools to
built new voices efficiently and robustly as soon as we can.  Though
in the mean time, a few higher quality voices will be released with
the next version.

# Todo

This release is just the beginning, there is much to do and this can
be a lot faster and smaller.  We have already seriously considered some
of the following but they didn't make this release.  In near
future versions we will add

* A (default) compilation method from a FestVox built voice.
* Streaming synthesis so no large buffers of waveforms need be held
  (we've got an initial pass at that in Cepstral but not read for this
  release).
* Better compression of the lexicon, and unit databases.
* Better quality speech (we have better diphone databases which
  haven't been released yet but do give better quality synthesis).
* Documentation to allow people to more easily integrate flite
  into applications.
* Some reasonable voices based on our newer voice building work.

# License

The flite project is released under a [4-clause BSD license](COPYING) with
the following copyright:

    Copyright Carnegie Mellon University 1999-2001
    All rights reserved

The changes to the project are described in the [CHANGELOG.md](CHANGELOG.md)
file in order to comply with clause 2 of the BSD license.
