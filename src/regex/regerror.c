#include <stdio.h>
#include <stdlib.h>
#include "regexp.h"
#include "cst_error.h"

void
hs_regerror(const char *s)
{
#ifdef ERRAVAIL
	error("regexp: %s", s);
#else
	cst_errmsg("regexp(3): %s\n", s);
	exit(1);
	return;	  /* let std. egrep handle errors */
#endif
	/* NOTREACHED */
}
