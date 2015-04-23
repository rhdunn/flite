/*************************************************************************/
/*                                                                       */
/*                           Cepstral, LLC                               */
/*                        Copyright (c) 2001                             */
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
/*  CEPSTRAL, LLC AND THE CONTRIBUTORS TO THIS WORK DISCLAIM ALL         */
/*  WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED       */
/*  WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL         */
/*  CEPSTRAL, LLC NOR THE CONTRIBUTORS BE LIABLE FOR ANY SPECIAL,        */
/*  INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER          */
/*  RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION    */
/*  OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR  */
/*  IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.          */
/*                                                                       */
/*************************************************************************/
/*             Author:  David Huggins-Daines (dhd@cepstral.com)          */
/*               Date:  October 2001                                     */
/*************************************************************************/
/*                                                                       */
/*  regexcomp_main.c: pre-compile a cst_regex                            */
/*                                                                       */
/*************************************************************************/

#include "cst_error.h"
#include "cst_regex.h"

void
dump_cst_regex(cst_regex *crx, char *name)
{
	int i;

	printf("static const unsigned char %s_rxprog[] = {\n\t", name);
	for (i = 0; i < crx->regsize-1; ++i) {
		printf("%u, ", crx->program[i] & 0xff);
		if (i % 10 == 0)
			printf("\n\t");
	}
	printf("%u\n};\n", crx->program[i] & 0xff);
	printf("static const cst_regex %s_rx = {\n\t", name);
	printf("%u,\n\t", crx->regstart & 0xff);
	printf("%u,\n\t", crx->reganch & 0xff);
	if (crx->regmust == NULL)
		printf("NULL,\n\t");
	else
		printf("%s_rxprog + %d,\n\t", name, crx->regmust - crx->program);
	printf("%d,\n\t", crx->regmlen);
	printf("%d,\n\t", crx->regsize);
	printf("(char *) %s_rxprog\n};\n\n", name);
}

int
main(int argc, char *argv[])
{
	cst_regex *crx;

	if (argc != 3) {
		cst_errmsg("Usage: %s name regex\n", argv[0]);
		cst_error();
	}

	crx = new_cst_regex(argv[2]);
	dump_cst_regex(crx,argv[1]);
	delete_cst_regex(crx);
	return 0;
}
