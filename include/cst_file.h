/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                        Copyright (c) 1999                             */
/*                        All Rights Reserved.                           */
/*                                                                       */
/*  Permission is hereby granted, free of charge, to use and distribute  */
/*  this software and its documentation without restriction, including   */
/*  without limitation the rights to use, copy, modify, merge, publish,  */
/*  distribute, sublicense, and/or sell copies of this work, and to      */
/*  permit persons to whom this work is furnished to do so, subject to   */
/*  the following conditions:                                            */
/*   1. The code must retain the above copyright notice, this list of    */
/*      conditions and the following disclaimer.                         */
/*   2. Any modifications must be clearly marked as such.                */
/*   3. Original authors' names are not deleted.                         */
/*   4. The authors' names are not used to endorse or promote products   */
/*      derived from this software without specific prior written        */
/*      permission.                                                      */
/*                                                                       */
/*  CARNEGIE MELLON UNIVERSITY AND THE CONTRIBUTORS TO THIS WORK         */
/*  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      */
/*  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   */
/*  SHALL CARNEGIE MELLON UNIVERSITY NOR THE CONTRIBUTORS BE LIABLE      */
/*  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    */
/*  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   */
/*  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          */
/*  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       */
/*  THIS SOFTWARE.                                                       */
/*                                                                       */
/*************************************************************************/
/*             Author:  Alan W Black (awb@cs.cmu.edu)                    */
/*               Date:  August 2000                                      */
/*************************************************************************/
/*                                                                       */
/*  Some File stuff                                                      */
/*                                                                       */
/*************************************************************************/
#ifndef _CST_FILE_H__
#define _CST_FILE_H__

#define CST_WRONG_FORMAT -2
#define CST_ERROR_FORMAT -1
#define CST_OK_FORMAT     0

/* WinCE lacks a complete implementation of stdio */
#ifdef UNDER_CE
#include <winbase.h>
typedef HANDLE cst_file_t;
#else
typedef FILE * cst_file_t;
#endif

#define CST_OPEN_WRITE (1<<0)
#define CST_OPEN_READ (1<<1)
#define CST_OPEN_APPEND (1<<2)
#define CST_OPEN_BINARY (1<<3)

#define CST_SEEK_ABSOLUTE 0
#define CST_SEEK_RELATIVE 1
#define CST_SEEK_ENDREL 2

cst_file_t cst_fopen(const char *path, int mode);
long cst_fwrite(cst_file_t fh, const void *buf, long size, long count);
long cst_fread(cst_file_t fh, void *buf, long size, long count);
int cst_fprintf(cst_file_t fh, char *fmt, ...);
int cst_fclose(cst_file_t fh);
int cst_fgetc(cst_file_t fh);

/* These aren't LFS-compliant.  I don't think we'll need >2G files. */
long cst_ftell(cst_file_t fh);
long cst_fseek(cst_file_t fh, long pos, int whence);

#endif
