###########################################################################
##                                                                       ##
##                  Language Technologies Institute                      ##
##                     Carnegie Mellon University                        ##
##                         Copyright (c) 2004                            ##
##                        All Rights Reserved.                           ##
##                                                                       ##
##  Permission is hereby granted, free of charge, to use and distribute  ##
##  this software and its documentation without restriction, including   ##
##  without limitation the rights to use, copy, modify, merge, publish,  ##
##  distribute, sublicense, and/or sell copies of this work, and to      ##
##  permit persons to whom this work is furnished to do so, subject to   ##
##  the following conditions:                                            ##
##   1. The code must retain the above copyright notice, this list of    ##
##      conditions and the following disclaimer.                         ##
##   2. Any modifications must be clearly marked as such.                ##
##   3. Original authors' names are not deleted.                         ##
##   4. The authors' names are not used to endorse or promote products   ##
##      derived from this software without specific prior written        ##
##      permission.                                                      ##
##                                                                       ##
##  CARNEGIE MELLON UNIVERSITY AND THE CONTRIBUTORS TO THIS WORK         ##
##  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      ##
##  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   ##
##  SHALL CARNEGIE MELLON UNIVERSITY NOR THE CONTRIBUTORS BE LIABLE      ##
##  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    ##
##  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   ##
##  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          ##
##  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       ##
##  THIS SOFTWARE.                                                       ##
##                                                                       ##
###########################################################################
##  FDLIBM http://www.netlib.org/fdlibm/                                 ##
##  This has minimal changes to support compiling under Palm ARM         ##
##                                                                       ##
##  FDLIBM is copyright Sun Microsystems, Inc                            ##
##                                                                       ##
###########################################################################
TOP = ../..
DIRNAME = palm/fdlibm

H = 
SRCS = e_acos.c e_acosh.c e_asin.c e_atan2.c e_atanh.c e_cosh.c e_exp.c \
       e_fmod.c e_gamma.c e_gamma_r.c e_hypot.c e_j0.c e_j1.c e_jn.c \
       e_lgamma.c e_lgamma_r.c e_log.c e_log10.c e_pow.c e_rem_pio2.c \
       e_remainder.c e_scalb.c e_sinh.c e_sqrt.c k_cos.c k_rem_pio2.c \
       k_sin.c k_standard.c k_tan.c s_asinh.c s_atan.c s_cbrt.c s_ceil.c \
       s_copysign.c s_cos.c s_erf.c s_expm1.c s_fabs.c s_finite.c \
       s_floor.c s_frexp.c s_ilogb.c s_isnan.c s_ldexp.c s_lib_version.c \
       s_log1p.c s_logb.c s_matherr.c s_modf.c s_nextafter.c s_rint.c \
       s_scalbn.c s_signgam.c s_significand.c s_sin.c s_tan.c s_tanh.c \
       w_acos.c w_acosh.c w_asin.c w_atan2.c w_atanh.c w_cosh.c w_exp.c \
       w_fmod.c w_gamma.c w_gamma_r.c w_hypot.c w_j0.c w_j1.c w_jn.c \
       w_lgamma.c w_lgamma_r.c w_log.c w_log10.c w_pow.c w_remainder.c \
       w_scalb.c w_sinh.c w_sqrt.c
OBJS := $(SRCS:.c=.o)
FILES = Makefile $(H) $(SRCS) readme

LOCAL_INCLUDES = -D_IEEE_LIBM -DNO_ERRNO -DNO_FPUTS

LIBNAME = m

ALL = 

include $(TOP)/config/common_make_rules
