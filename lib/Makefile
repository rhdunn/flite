###########################################################################
##                                                                       ##
##                  Language Technologies Institute                      ##
##                     Carnegie Mellon University                        ##
##                        Copyright (c) 1999                             ##
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
##                                                                       ##
##    FLITE libraries                                                    ##
##                                                                       ##
###########################################################################
TOP=..
DIRNAME=lib
BUILD_DIRS = 
ALL_DIRS=
FILES = Makefile
LIBNAMES = flite flite_cmulex flite_usenglish \
           flite_cmu_us_kal flite_cmu_us_kal16 flite_cmu_time_awb
STATICLIBS= $(LIBNAMES:%=lib%.a)
SHAREDARLIBS= $(LIBNAMES:%=lib%.shared.a)
SHAREDLIBS = $(LIBNAMES:%=lib%.so)
VERSIONSHAREDLIBS = $(SHAREDLIBS:%=%.${PROJECT_VERSION}) \
                    $(SHAREDLIBS:%=%.${PROJECT_SHLIB_VERSION})
ALL_LIBS = $(STATICLIBS) $(SHAREDLIBS) $(VERSIONSHAREDLIBS)

ALL = shared_libs

LOCAL_CLEAN=*.a *.so *.so.${PROJECT_VERSION} *.so.${PROJECT_SHLIB_VERSION}

include $(TOP)/config/common_make_rules

ifdef SHFLAGS
shared_libs: $(SHAREDLIBS)
else
shared_libs: nothing
endif

install:
	@ tar cvf - $(ALL_LIBS) | ( cd $(INSTALLLIBDIR) && tar xf -)
