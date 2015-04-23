dnl########################################################################
dnl                                                                      ##
dnl                 Language Technologies Institute                      ##
dnl                    Carnegie Mellon University                        ##
dnl                     Copyright (c) 1999-2001                          ##
dnl                       All Rights Reserved.                           ##
dnl                                                                      ##
dnl Permission is hereby granted, free of charge, to use and distribute  ##
dnl this software and its documentation without restriction, including   ##
dnl without limitation the rights to use, copy, modify, merge, publish,  ##
dnl distribute, sublicense, and/or sell copies of this work, and to      ##
dnl permit persons to whom this work is furnished to do so, subject to   ##
dnl the following conditions:                                            ##
dnl  1. The code must retain the above copyright notice, this list of    ##
dnl     conditions and the following disclaimer.                         ##
dnl  2. Any modifications must be clearly marked as such.                ##
dnl  3. Original authors' names are not deleted.                         ##
dnl  4. The authors' names are not used to endorse or promote products   ##
dnl     derived from this software without specific prior written        ##
dnl     permission.                                                      ##
dnl                                                                      ##
dnl CARNEGIE MELLON UNIVERSITY AND THE CONTRIBUTORS TO THIS WORK         ##
dnl DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      ##
dnl ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   ##
dnl SHALL CARNEGIE MELLON UNIVERSITY NOR THE CONTRIBUTORS BE LIABLE      ##
dnl FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    ##
dnl WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   ##
dnl AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          ##
dnl ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       ##
dnl THIS SOFTWARE.                                                       ##
dnl                                                                      ##
dnl########################################################################
AC_INIT(include/flite.h)

AC_CANONICAL_SYSTEM
AC_PROG_CC
if test "x$GCC" = "xyes"; then
	CFLAGS="$CFLAGS -Wall"
fi
AC_PROG_RANLIB
AC_CHECK_TOOL(AR, ar)

AC_C_BIGENDIAN

AC_ARG_ENABLE( sockets, 
               [  --disable-sockets     without socket support],
	       [case "${enableval}" in
                 yes) sockets=true ;;
                 no) sockets=false ;;
                 *) AC_MSG_ERROR(bad value ${enableval} for sockets options) ;;
                 esac],
               [sockets=true])
if test "$sockets" = false; then
   AC_DEFINE(CST_NO_SOCKETS)
fi

dnl Does this compiler support initialization of unions
AC_TRY_COMPILE([#include <stdio.h>],
               [ struct a { union { float b; int c; } d; };
                 const struct a e = { .d={ .b=3.14 } }; ],
               [unioninit=yes],
               [unioninit=no])
if test "$unioninit" = no; then
   AC_DEFINE(NO_UNION_INITIALIZATION)
fi

dnl
dnl determine audio type or use none if none supported on this platform
dnl
AUDIOTYPE=-DCST_AUDIO_NONE
AC_CHECK_FILE(/usr/include/sys/soundcard.h,
              [AUDIOTYPE=-DCST_AUDIO_LINUX])
AC_CHECK_FILE(/usr/include/machine/soundcard.h,
              [AUDIOTYPE=-DCST_AUDIO_FREEBSD])
AC_CHECK_FILE(/usr/include/sys/audioio.h,
              [AUDIOTYPE=-DCST_AUDIO_SUNOS])
AC_ARG_WITH( audio, [  --with-audio          with specific audio support (LINUX, FREEBSD, NONE etc) ],
        AUDIOTYPE="-DCST_AUDIO_"$with_audio )
AC_SUBST(AUDIOTYPE)

dnl
dnl language/lexicon/voice parameters (we don't really support multiple
dnl ones yet, but its included as a start
dnl
AC_ARG_WITH( lang, [  --with-lang           with language ],
        FL_LANG=$with_lang )
if test "x$with_lang" = "x"; then
        FL_LANG="usenglish"
fi
AC_SUBST(FL_LANG)

AC_ARG_WITH( vox, [  --with-vox            with vox ],
        FL_VOX=$with_vox )
if test "x$with_vox" = "x"; then
        FL_VOX="cmu_us_kal"
fi
AC_SUBST(FL_VOX)

AC_ARG_WITH( lex, [  --with-lex            with lexicon ],
        FL_LEX=$with_lex )
if test "x$with_lex" = "x"; then
        FL_LEX="cmulex"
fi
AC_SUBST(FL_LEX)


AC_OUTPUT(config/config config/system.mak)
