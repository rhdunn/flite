/*************************************************************************/
/*                                                                       */
/*                  Language Technologies Institute                      */
/*                     Carnegie Mellon University                        */
/*                         Copyright (c) 2000                            */
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
/*             Authors:  Alan W Black (awb@cs.cmu.edu)                   */
/*    			 David Huggins-Daines (dhd@cepstral.com)	 */
/*               Date:  April 2001                                       */
/*************************************************************************/
/*                                                                       */
/*  clunits waveform synthesis                                           */
/*                                                                       */
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "cst_hrg.h"
#include "cst_utt_utils.h"
#include "cst_viterbi.h"
#include "cst_clunits.h"
#include "cst_units.h"
#include "cst_wave.h"
#include "cst_track.h"
#include "cst_sigpr.h"

#define CLUNITS_DEBUG 0

CST_VAL_REGISTER_TYPE_NODEL(clunit_db,cst_clunit_db)
CST_VAL_REGISTER_TYPE_NODEL(vit_cand,cst_vit_cand)

static cst_vit_cand *cl_cand(cst_item *i,
			     struct cst_viterbi_struct *vd);
static cst_vit_path *cl_path(cst_vit_path *p,
			     cst_vit_cand *c,
			     cst_viterbi *vd);
static const cst_cart *clunit_get_tree(cst_clunit_db *cludb, const char *name);
static int optimal_couple_frame(cst_clunit_db *cludb, int u0, int u1);
static int optimal_couple(cst_clunit_db *cludb,
			  int u0, int u1,
			  int *u0_move, int *u1_move);
static void clunit_set_unit_name(cst_item *s,cst_clunit_db *clunit_db);
static int unit_type_eq(cst_clunit_db *cludb, int ua, int ub);

static int frame_distance(const cst_clunit_db *cludb,
			  int a, int b,
			  const int *join_weights,
			  const int order);


cst_utterance *clunits_synth(cst_utterance *utt)
{
    /* Basically the same as the diphone code */
    clunits_select(utt);
    join_units(utt);

    return utt;
}

cst_utterance *clunits_select(cst_utterance *utt)
{
    cst_viterbi *vd;
    cst_relation *units,*segs;
    cst_item *s,*u;
    cst_clunit_db *clunit_db;
    int unit_entry;
    
    segs = utt_relation(utt,"Segment");
    vd = new_viterbi(cl_cand,cl_path);
    vd->num_states = -1;
    vd->big_is_good = FALSE;
    feat_set(vd->f,"clunit_db",feat_val(utt->features,"clunit_db"));
    clunit_db = val_clunit_db(feat_val(vd->f,"clunit_db"));
    utt_set_feat(utt,"sts_list",sts_list_val(clunit_db->sts));

    for (s=relation_head(segs); s; s=item_next(s))
	clunit_set_unit_name(s,clunit_db);

    viterbi_initialise(vd,segs);
    viterbi_decode(vd);
    if (!viterbi_result(vd,"selected_unit"))
    {
	cst_errmsg("clunits: can't find path\n");
	cst_error();
    }
    viterbi_copy_feature(vd, "unit_prev_move");
    viterbi_copy_feature(vd, "unit_this_move");
    delete_viterbi(vd);

    /* Construct unit stream with selected units */
    units = utt_relation_create(utt,"Unit");
    for (s=relation_head(segs); s; s=item_next(s))
    {
	u = relation_append(units,NULL);
	item_set_string(u,"name",item_name(s));

	unit_entry = item_feat_int(s,"selected_unit");
#if CLUNITS_DEBUG
	printf("selected %d %s\n", unit_entry,
	       clunit_db->units[unit_entry].name);
#endif

	/* Get stuff from unit_db */
	item_set(u,"unit_entry",item_feat(s,"selected_unit"));
	item_set(u,"clunit_name",item_feat(s,"clunit_name"));

	/* Use optimal join points if available */
	if (item_feat_present(s, "unit_this_move"))
	    item_set_int(u,"unit_start", item_feat_int(s, "unit_this_move"));
	else
	    item_set_int(u,"unit_start",clunit_db->units[unit_entry].start);

	if (item_next(s) && item_feat_present(item_next(s), "unit_prev_move"))
	    item_set_int(u,"unit_end", item_feat_int(item_next(s), "unit_prev_move"));
	else
	    item_set_int(u,"unit_end",clunit_db->units[unit_entry].end);

	item_set_int(u,"target_end",
		     (int)(item_feat_float(s,"end")*clunit_db->sts->sample_rate));
    }

    return utt;
}

static cst_vit_cand *cl_cand(cst_item *i,cst_viterbi *vd)
{
    const char *unit_type;
    int nu;
    int e;
    const cst_val *clist,*c;
    cst_vit_cand *p,*all,*gt,*lc;
    cst_clunit_db *clunit_db;

    clunit_db = val_clunit_db(feat_val(vd->f,"clunit_db"));
    unit_type = item_feat_string(i,"clunit_name");

    /* get tree */
    clist = cart_interpret(i,clunit_get_tree(clunit_db,unit_type));

    all = 0;
    for (c=clist; c; c=val_cdr(c))
    {
	p = new_vit_cand();
	p->next = all;
	p->item = i;
	p->score = 0;
	vit_cand_set_int(p, clunit_get_unit_index(clunit_db,
						  unit_type,
						  val_int(val_car(c))));
	all = p;
    }

    if ((clunit_db->extend_selections > 0) && (item_prev(i)))
    {
	lc = val_vit_cand(item_feat(item_prev(i),"clunit_cands"));
	for (e=0; lc && (e < clunit_db->extend_selections); lc=lc->next)
	{
	    nu = clunit_db->units[lc->ival].next;
	    if (nu == CLUNIT_NONE)
		continue;
	    for (gt=all; gt; gt=gt->next)
		if (nu == gt->ival)
		    break;  /* we've got this one already */
	    if ((gt == 0) && unit_type_eq(clunit_db, nu, all->ival))
	    {
		p = new_vit_cand();
		p->next = all;
		p->item = i;
		p->score = 0;
		vit_cand_set_int(p, nu);
		all = p;
		e++;
	    }
	}
    }
    item_set(i,"clunit_cands",vit_cand_val(all));

#if CLUNITS_DEBUG
    printf("search candidates for %s:\n", unit_type);
    for (p = all; p; p = p->next) {
	printf("%d(%s) ", p->ival, clunit_db->units[p->ival].name);
    }
    printf("\n");
#endif

    return all;
}

static cst_vit_path *cl_path(cst_vit_path *p,
			     cst_vit_cand *c,
			     cst_viterbi *vd)
{
    int cost;
    cst_vit_path *np;
    cst_clunit_db *cludb;
    int u0,u1;
    int u0_move, u1_move;

    np = new_vit_path();

    cludb = val_clunit_db(feat_val(vd->f,"clunit_db"));

    np->cand = c;
    np->from = p;
    
    if ((p==0) || (p->cand == 0))
	cost = 0;
    else
    {
	u0 = p->cand->ival;
	u1 = c->ival;
	if (cludb->optimal_coupling == 1) {
	    if (np->f == NULL)
		np->f = new_features();
	    cost = optimal_couple(cludb, u0, u1, &u0_move, &u1_move);
	    feat_set(np->f, "unit_prev_move", int_val(u0_move));
	    feat_set(np->f, "unit_this_move", int_val(u1_move));
	} else if (cludb->optimal_coupling == 2)
	    cost = optimal_couple_frame(cludb,u0,u1);
	else
	    cost = 0;
    }

    cost *= cludb->continuity_weight;
    np->state = c->pos;
    if (p==0)
	np->score = cost + c->score;
    else
	np->score = cost + c->score + p->score;

#if CLUNITS_DEBUG
    printf("joined %s %s score %d cscore %d\n",
	   (p ? cludb->units[p->cand->ival].name : "none"),
	   cludb->units[c->ival].name,
	   cost,np->score);
#endif
	   
    return np;
}

static int optimal_couple_frame(cst_clunit_db *cludb, int u0, int u1)
{
    int a,b;
    int u1_p;

    u1_p = cludb->units[u1].prev;

    if (u1_p == u0)
	return 0; /* Consecutive units win - FATALITY */

    if (u1_p == CLUNIT_NONE) /* No previous, so we can't score overlapping frames */
	a = cludb->units[u1].start;
    else 
	a = cludb->units[u1_p].end-1;

    b = cludb->units[u0].end-1;
 
    return 50000 + frame_distance(cludb, a, b,
				  cludb->join_weights,
				  cludb->mcep->num_channels);
}

static int optimal_couple(cst_clunit_db *cludb,
			  int u0, int u1,
			  int *u0_move, int *u1_move)
{
    int a,b;
    int u1_p;
    int i, fcount;
    int u0_st, u1_p_st, u0_end, u1_p_end;
    int u0_size, u1_p_size;
    int best_u0, best_u1_p;
    int dist, best_val, different_prev_factor;

    u1_p = cludb->units[u1].prev;

#if CLUNITS_DEBUG
    printf("optimal_coupling %s (%d,%d) %s (%d,%d)\n",
	   cludb->units[u0].name, cludb->units[u0].start, cludb->units[u0].end,
	   cludb->units[u1].name, cludb->units[u1].start, cludb->units[u1].end);
#endif
    /* u0_move is the new end for u0
       u1_move is the new start for u1

       This works based on the assumption that the STS frames for
       consecutive units in the recordings will also be consecutive.
       The voice compiler MUST preserve this assumption! */
    *u0_move = cludb->units[u0].end;
    *u1_move = cludb->units[u1].start; /* i.e. u1_p.end + 1 */

    if (u1_p == u0)
	return 0.0;
    if (u1_p == CLUNIT_NONE)
	return optimal_couple_frame(cludb, u0, u1); /* laziness */

    u0_size = cludb->units[u0].end - cludb->units[u0].start;
    u1_p_size = cludb->units[u1_p].end - cludb->units[u1_p].start;

    u0_end = u0_size;
    u1_p_end = u1_p_size;

    if (unit_type_eq(cludb, u0, u1_p)) {
	u0_st = u0_size / 3;
	u1_p_st = u1_p_size / 3;
	different_prev_factor = 1;
#if CLUNITS_DEBUG
	printf("%s == %s, sliding u0:(%d,%d) u1:(%d,%d)\n",
	       cludb->units[u0].name,
	       cludb->units[u1_p].name,
	       u0_st, u0_end, u1_p_st, u1_p_end);
#endif
    } else {
	/* Different phone, don't slide and just look at the last frame */
	u0_st = u0_end - 1;
	u1_p_st = u1_p_end - 1;
	different_prev_factor = 1000; /* FIXME: is this really needed? */
    }

    best_u0 = u0_end;
    best_u1_p = u1_p_end;
    best_val = INT_MAX;

    fcount = ((u0_end - u0_st) < (u1_p_end - u1_p_st)
	      ? (u0_end - u0_st)  : (u1_p_end - u1_p_st));
    for (i = 0; i < fcount; ++i) {
	a = cludb->units[u1_p].start + u1_p_st + i;
	b = cludb->units[u0].start + u0_st + i;
	dist = frame_distance(cludb, a, b,
			      cludb->join_weights,
			      cludb->mcep->num_channels);
	if (dist < best_val) {
	    best_val = dist;
	    best_u0 = u0_st + i;
	    best_u1_p = u1_p_st + i;
	}
    }

    *u0_move = cludb->units[u0].start + best_u0 + 1;
    *u1_move = cludb->units[u1_p].start + best_u1_p + 1;
#if CLUNITS_DEBUG
    printf("best_u0 %d = %d best_u1 %d = %d\n",
	   best_u0, *u0_move, best_u1_p, *u1_move);
#endif

    return 50000 + best_val * different_prev_factor;
}

static int frame_distance(const cst_clunit_db *cludb,
			  int a, int b,
			  const int *join_weights,
			  const int order)
{
    const unsigned short *av, *bv;
    int r,diff;
    int i;

    bv = get_sts_frame(cludb->mcep, b);
    av = get_sts_frame(cludb->mcep, a);

    /* This is the sum of the deltas in each dimension, which is of
       course not actually the distance, but works well enough for our
       purposes. */
    for (r = 0, i = 0; i < order; i++)
    {
	diff = av[i]-bv[i];
	r += abs(diff) * join_weights[i] / 65536;
    }

    return r;
}

static const cst_cart *clunit_get_tree(cst_clunit_db *cludb, const char *name)
{
    int i;

    /* Binary search is not worth it, because cludb->num_types is
       always pretty small. */
    for (i = 0; i < cludb->num_types; ++i)
	if (cst_streq(cludb->types[i], name))
	    break;

    if (i == cludb->num_types)
    {
	cst_errmsg("clunits: can't find tree for %s\n",name);
	i = 0; /* "graceful" failure */
    }
    return cludb->trees[i];
}

static int unit_type_eq(cst_clunit_db *cludb, int ua, int ub)
{
	char const *na, *nb;
	char const *ca, *cb;

	na = cludb->units[ua].name;
	nb = cludb->units[ub].name;

	/* Find the last underscore */
	ca = cst_strrchr(na, '_');
	cb = cst_strrchr(nb, '_');

	if (ca - na != cb - nb)
	    return 0;

	/* Compare before it */
	return !strncmp(na, nb, ca - na);
}

static void clunit_set_unit_name(cst_item *s,cst_clunit_db *clunit_db)
{
    if (clunit_db->unit_name_func)
    {
	char *cname;
	cname = (clunit_db->unit_name_func)(s);
	item_set_string(s,"clunit_name",cname);
	cst_free(cname);
    }
    else
    {
	/* is just the name by default */
	item_set(s,"clunit_name",item_feat(s,"name"));
    }

}

char *clunits_ldom_phone_word(cst_item *s)
{
    const char *name;
    const char *pname;
    const char *wname;
    char *clname;
    char *dname, *p, *q;

    name = item_name(s);
    if (cst_streq(name,"pau"))  /* thats US English specific */
    {
	pname = ffeature_string(s,"p.name");
	clname = cst_alloc(char, 5+strlen(pname));
	sprintf(clname,"pau_%s",pname);
    }
    else
    {
	/* remove single quotes from name */
	wname = ffeature_string(s,"R:SylStructure.parent.parent.name");
	dname = cst_downcase(wname);
	for (q=p=dname; *p != '\0'; p++)
	    if (*p != '\'') *p = *q++;
	*q = '\0';
	clname = cst_alloc(char, strlen(dname)+2+strlen(dname));
	sprintf(clname,"%s_%s",name,dname);
	cst_free(dname);
    }
    return clname;
}

int clunit_get_unit_index(cst_clunit_db *clunit_db, 
			  const char *unit_type,
			  int instance)
{
    char *unit_name;
    int r;

    unit_name = cst_alloc(char,strlen(unit_type)+21);
    
    sprintf(unit_name,"%s_%d",unit_type,instance);

    r = clunit_get_unit_index_name(clunit_db,unit_name);

    cst_free(unit_name);

    return r;
}

int clunit_get_unit_index_name(cst_clunit_db *clunit_db,
			       const char *name)
{
    int start,end,mid,c;

    start = 0;
    end = clunit_db->num_units;

    while (start < end) {
	    mid = (start+end)/2;
	    c = strcmp(clunit_db->units[mid].name,name);

	    if (c == 0)
		    return mid;
	    else if (c > 0)
		    end = mid;
	    else
		    start = mid + 1;
    }

    cst_errmsg("clunits: unit \"%s\" not found\n",name);
    return 0;
}

