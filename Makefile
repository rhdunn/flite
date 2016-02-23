###########################################################################
##                                                                       ##
##                  Language Technologies Institute                      ##
##                     Carnegie Mellon University                        ##
##                      Copyright (c) 1999-2001                          ##
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
##    Fast efficient small run-time speech synthesis system              ##
##                                                                       ##
##       Authors:  Alan W Black (awb@cs.cmu.edu)                         ##
##                 Kevin A. Lenzo (lenzo@cs.cmu.edu)                     ##
##          Date:  August 2001                                           ##
##       Version:  1.0-beta                                              ## 
##                                                                       ## 
###########################################################################
TOP=.
DIRNAME=.
BUILD_DIRS = include src doc lang main testsuite
ALL_DIRS=config lib tools $(BUILD_DIRS)
CONFIG=configure configure.in config.sub config.guess \
       missing install-sh mkinstalldirs
FILES = Makefile README ACKNOWLEDGEMENTS $(CONFIG)

ALL = $(BUILD_DIRS)

config_dummy := $(shell test -f config/config || ( echo '*** '; echo '*** Making default config file ***'; echo '*** '; ./configure; )  >&2)

include $(TOP)/config/common_make_rules

config/config: config/config.in config.status
	./config.status

configure: configure.in
	autoconf

distclean: clean
	@ echo Removing configure-generated files ...
	@ $(RM) config.cache config.log config.status \
		config/config config/system.mak

backup: time-stamp
	@ $(RM) -f $(TOP)/FileList
	@ $(MAKE) file-list
	@ echo .time-stamp >>FileList
	@ sed 's/^\.\///' <FileList | sed 's/^/flite\//' >.file-list-all
	@ (cd ..; tar zcvf flite/$(PROJECT_PREFIX)-$(PROJECT_VERSION)-$(PROJECT_STATE).tar.gz `cat flite/.file-list-all`)
	@ $(RM) -f $(TOP)/.file-list-all
	@ ls -l $(PROJECT_PREFIX)-$(PROJECT_VERSION)-$(PROJECT_STATE).tar.gz

tags:
	@ $(RM) -f $(TOP)/FileList
	@ $(MAKE) file-list
	etags `cat FileList | grep "\.[ch]$$"`

time-stamp :
	@ echo $(PROJECT_NAME) >.time-stamp
	@ echo $(PROJECT_PREFIX) >>.time-stamp
	@ echo $(PROJECT_VERSION) >>.time-stamp
	@ echo $(PROJECT_DATE) >>.time-stamp
	@ echo $(PROJECT_STATE) >>.time-stamp
	@ echo $(LOGNAME) >>.time-stamp
	@ hostname >>.time-stamp
	@ date >>.time-stamp

test:
	@ $(MAKE) --no-print-directory -C testsuite test

%.html: %.md _layouts/webpage.html
	kramdown --template _layouts/webpage.html $< > $@

docs: CHANGELOG.html README.html
