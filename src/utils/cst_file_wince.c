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
/*             Author:  David Huggins-Daines <dhd@cepstral.com>          */
/*               Date:  August 2001                                      */
/*************************************************************************/
/*                                                                       */
/*  File I/O wrappers for defective platforms.                           */
/*                                                                       */
/*************************************************************************/

#include <winbase.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "cst_file.h"
#include "cst_alloc.h"

cst_file_t cst_fopen(const char *path, int mode)
{
	int count = strlen(path)+1;
	wchar_t *wpath = cst_alloc(wchar_t,count);
	cst_file_t fh;
	long access,creation;

	mbstowcs(wpath,path,count);
	wpath[count]=L'\0'; /* mbstowcs should null-terminate, but doesn't... */

	if (((mode & CST_OPEN_READ) && (mode & CST_OPEN_WRITE))
		|| ((mode & CST_OPEN_READ) && (mode & CST_OPEN_APPEND)))
	{
		access = GENERIC_READ|GENERIC_WRITE;
		creation = OPEN_ALWAYS;
	}
	else if (mode & CST_OPEN_READ)
	{
		access = GENERIC_READ;
		creation = OPEN_EXISTING;
	}
	else if (mode & CST_OPEN_WRITE)
	{
		access = GENERIC_WRITE;
		creation = CREATE_ALWAYS; /* FIXME: Does this truncate?  Argh! */
	}
	else if (mode & CST_OPEN_APPEND)
	{
		access = GENERIC_WRITE;
		creation = OPEN_ALWAYS;
	}

	/* Note, we are ignoring CST_FILE_BINARY entirely since there is no
	   CRLF translation done by these APIs (we hope).  This might cause
	   problems for other Windows programs that try to read our output. */

	fh = CreateFile(wpath,access,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
		creation,FILE_ATTRIBUTE_NORMAL,NULL);
	if (fh == INVALID_HANDLE_VALUE) {
		long foo = GetLastError();
		return NULL;
	}

	if (mode & CST_OPEN_APPEND)
		SetFilePointer(fh,0,NULL,FILE_END);

	return fh;
}

long cst_fwrite(cst_file_t fh, const void *buf, long size, long count)
{
	long rv;

	if (!WriteFile(fh,buf,size*count,&rv,NULL))
		return -1;

	return rv;
}

long cst_fread(cst_file_t fh, void *buf, long size, long count)
{
	long rv;

	if (!ReadFile(fh,buf,size*count,&rv,NULL))
		return -1;
	return rv;
}

int cst_fgetc(cst_file_t fh)
{
	char c;
	if (cst_fread(fh, &c, 1, 1) == 0)
		return EOF;
	return c;
}

long cst_ftell(cst_file_t fh)
{
	return SetFilePointer(fh,0,NULL,FILE_CURRENT);
}

long cst_fseek(cst_file_t fh, long pos, int whence)
{
	int w = 0;

	if (whence = CST_SEEK_ABSOLUTE)
		w = FILE_BEGIN;
	else if (whence == CST_SEEK_RELATIVE)
		w = FILE_CURRENT;
	else if (whence == CST_SEEK_ENDREL)
		w = FILE_END;

	return SetFilePointer(fh,pos,NULL,w);
}

int cst_fprintf(cst_file_t fh, char *fmt, ...)
{
	va_list args;
	char outbuf[512];
	int count;

	va_start(args,fmt);
	count = vsprintf(outbuf,fmt,args); /* You use WinCE, you lose. */
	va_end(args);
	return cst_fwrite(fh,outbuf,1,count);
}

int cst_fclose(cst_file_t fh)
{
	return CloseHandle(fh);
}
