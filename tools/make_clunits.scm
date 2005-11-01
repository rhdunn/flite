;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;                                                                     ;;;
;;;                  Language Technologies Institute                    ;;;
;;;                     Carnegie Mellon University                      ;;;
;;;                         Copyright (c) 2000                          ;;;
;;;                        All Rights Reserved.                         ;;;
;;;                                                                     ;;;
;;; Permission is hereby granted, free of charge, to use and distribute ;;;
;;; this software and its documentation without restriction, including  ;;;
;;; without limitation the rights to use, copy, modify, merge, publish, ;;;
;;; distribute, sublicense, and/or sell copies of this work, and to     ;;;
;;; permit persons to whom this work is furnished to do so, subject to  ;;;
;;; the following conditions:                                           ;;;
;;;  1. The code must retain the above copyright notice, this list of   ;;;
;;;     conditions and the following disclaimer.                        ;;;
;;;  2. Any modifications must be clearly marked as such.               ;;;
;;;  3. Original authors' names are not deleted.                        ;;;
;;;  4. The authors' names are not used to endorse or promote products  ;;;
;;;     derived from this software without specific prior written       ;;;
;;;     permission.                                                     ;;;
;;;                                                                     ;;;
;;; CARNEGIE MELLON UNIVERSITY AND THE CONTRIBUTORS TO THIS WORK        ;;;
;;; DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING     ;;;
;;; ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT  ;;;
;;; SHALL CARNEGIE MELLON UNIVERSITY NOR THE CONTRIBUTORS BE LIABLE     ;;;
;;; FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   ;;;
;;; WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN  ;;;
;;; AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,         ;;;
;;; ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF      ;;;
;;; THIS SOFTWARE.                                                      ;;;
;;;                                                                     ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;             Author: Alan W Black (awb@cs.cmu.edu)                   ;;;
;;;               Date: April 2001                                      ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;                                                                     ;;;
;;; Convert festvox voice to flie                                       ;;;
;;;                                                                     ;;;
;;;   clunits: catalogue, carts and param                               ;;;
;;;                                                                     ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


(defvar lpc_min -2.709040)
(defvar lpc_max 2.328840)
(defvar mcep_min -5.404620)
(defvar mcep_max 4.540220)

(define (clunits_convert name clcatfnfileordered clcatfnunitordered 
			 cltreesfn festvoxdir odir)
  "(clunits_convert name clcatfn clcatfnordered cltreesfn festvoxdir odir)
Convert a festvox clunits (processed) voice into a C file."
   (clunitstoC clcatfnfileordered clcatfnunitordered name 
	       (path-append festvoxdir "sts")
	       (path-append festvoxdir "mcep")
	       odir)

   (set! ofd (fopen (path-append odir (string-append name "_clunits.c")) "a"))

   (clunits_make_carts cltreesfn clcatfnunitordered name odir ofd)

   (format ofd "\n\n")
   (format ofd "static int %s_join_weights[] = \n" name)
   (format ofd " { 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,\n")
   (format ofd "   32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768 };\n\n")

   (format ofd "extern const cst_cart * const %s_carts[];\n" name )
   (format ofd "extern cst_sts_list %s_sts, %s_mcep;\n\n" name name )

   (format ofd "cst_clunit_db %s_db = {\n" name)
   (format ofd "  \"%s\",\n\n" name)
   (format ofd "  %s_unit_types,\n" name)
   (format ofd "  %s_carts,\n" name)
   (format ofd "  %s_units,\n\n" name)

   (format ofd "  %s_num_unit_types,\n" name)
   (format ofd "  %s_num_units,\n\n" name)

   (format ofd "  &%s_sts,\n\n" name)

   (format ofd "  &%s_mcep,\n\n" name)

   (format ofd "  %s_join_weights,\n\n" name)
   (format ofd "  1, /* optimal coupling */\n")
   (format ofd "  5, /* extend selections */\n")
   (format ofd "  100, /* f0 weight */\n")
   (format ofd "  0  /* unit_name function */\n")
   
   (format ofd "};\n")

   (fclose ofd)
)

(define (unit_type u)
  (apply
   string-append
   (reverse
    (symbolexplode 
     (string-after 
      (apply
       string-append
       (reverse (symbolexplode u)))
      "_")))))

(define (unit_occur u)
  (apply
   string-append
   (reverse
    (symbolexplode 
     (string-before
      (apply
       string-append
       (reverse (symbolexplode u)))
      "_")))))

(define (sort_clentries entries clcatfnunitorder)
  (let ((neworder nil))
    (mapcar
     (lambda (unit)
       (set! neworder (cons (assoc_string (car unit) entries)
			    neworder)))
     (load clcatfnunitorder t))
    (reverse neworder)))

(define (clunitstoC clcatfnfileordered clcatfnunitordered 
		    name stsdir mcepdir odir)
  "(clunitstoC clcatfnfileordered clcatfnunitordered name mcepdir stsdir odir)
Convert a clunits catalogue and its sts representations into a
compilable single C file."
  (let 
    ((clindex (load clcatfnfileordered t))
     (lofdidx (fopen (path-append odir (string-append name "_lpc.c")) "w"))
     (lofdh (fopen (path-append odir (string-append name "_lpc.h")) "w"))
     (cofdidx (fopen (path-append odir (string-append name "_clunits.c")) "w"))
     (cofdh (fopen (path-append odir (string-append name "_clunits.h")) "w"))
     (mofdidx (fopen (path-append odir (string-append name "_mcep.c")) "w"))
     (mofdh (fopen (path-append odir (string-append name "_mcep.h")) "w")))

    (format lofdidx "/*****************************************************/\n")
    (format lofdidx "/**  Autogenerated Clunits index for %s    */\n" name)
    (format lofdidx "/*****************************************************/\n")
    (format lofdidx "\n")
    (format lofdidx "#include \"cst_clunits.h\"\n")
    (format lofdidx "#include \"%s_lpc.h\"\n" name)
    (format mofdidx "/*****************************************************/\n")
    (format mofdidx "/**  Autogenerated Clunits index for %s    */\n" name)
    (format mofdidx "/*****************************************************/\n")
    (format mofdidx "\n")
    (format mofdidx "#include \"%s_mcep.h\"\n" name)
    (format mofdidx "#include \"cst_clunits.h\"\n")

    (set! pm_pos 0)
    (set! sample_pos 0)
    (set! times nil)
    (set! clunits_entries nil)
    (set! done_files nil)
    (set! num_unit_entries (length clindex))
    (set! residual_sizes nil)

    (set! n 500)
    (set! f 0)
    (while clindex
     (if (equal? n 500)
	 (begin
	   (if (> f 0)
	       (begin (fclose lofdbit) (fclose mofdbit)))
	   (set! lofdbit 
		 (fopen (format nil "%s/%s_lpc_%03d.c" odir name f) "w"))
	   (set! mofdbit 
		 (fopen (format nil "%s/%s_mcep_%03d.c" odir name f) "w"))
	   (set! n 0)
	   (set! f (+ 1 f))))
     (set! n (+ 1 n))
     (set! pms (find_pm_pos 
		name
		(car clindex)
		stsdir
		mcepdir
		lofdbit
		lofdh
		mofdbit
		mofdh
		))

     ;; Output unit_entry for this unit
     (set! clunits_entries
	   (cons
	    (list 
	     (nth 0 (car clindex))
	     (nth 2 pms) ; start_pm
	     (nth 3 pms) ; phone_boundary_pm
	     (nth 4 pms) ; end_pm
	     (nth 5 (car clindex))
	     (nth 6 (car clindex))
	     )
	    clunits_entries))
     (set! clindex (cdr clindex)))

    (set! awb_clunits_entries clunits_entries)
    (set! clunits_entries (sort_clentries clunits_entries clcatfnunitordered))

    (format lofdidx "\n\n")
    (format mofdidx "\n\n")

    (format lofdidx "const cst_sts %s_sts_vals[] = { \n" name)
    (set! i 0)
    (mapcar
     (lambda (size)
       (format lofdidx "  { %s_frame_%d, %d, %s_res_%d }, \n"
	       name (car size) (cadr size) name (car size))
       (set! i (+ 1 i)))
     (reverse residual_sizes))
    (format lofdidx "   { 0, 0, 0 }};\n\n")
    (format lofdidx "const cst_sts_list %s_sts = {\n" name)
    (format lofdidx "  %s_sts_vals,\n" name)
    (format lofdidx "  NULL, NULL, NULL,\n")
    (format lofdidx "  %d,\n" i)
    (format lofdidx "  %d,\n" lpc_order)
    (format lofdidx "  %d,\n" sample_rate)
    (format lofdidx "  %f,\n" lpc_min)
    (format lofdidx "  %f,\n" lpc_range)
    (format lofdidx "  %f, \n" 0.0)
    (format lofdidx "  %d \n" 1)
    (format lofdidx "};\n\n")

    (format mofdidx "const cst_sts %s_mcep_vals[] = { \n" name)
    (set! i 0)
    (mapcar
     (lambda (time)
       (format mofdidx "  { %s_mcep_frame_%d, 0, 0 }, \n"
	       name i)
       (set! i (+ 1 i)))
     times)
    (format mofdidx "   { 0, 0, 0 }};\n\n")

    (format mofdidx "const cst_sts_list %s_mcep = {\n" name)
    (format mofdidx "  %s_mcep_vals,\n" name)
    (format mofdidx "  NULL, NULL, NULL,\n")
    (format mofdidx "  %d,\n" i)
    (format mofdidx "  %d,\n" mcep_order)
    (format mofdidx "  %d,\n" sample_rate)
    (format mofdidx "  %f,\n" mcep_min)
    (format mofdidx "  %f,\n" (- mcep_max mcep_min))
    (format mofdidx "  %f, \n" 0.0)
    (format mofdidx "  %d \n" 1)
    (format mofdidx "};\n\n")

    (format cofdidx "/*****************************************************/\n")
    (format cofdidx "/**  Autogenerated Clunits index for %s    */\n" name)
    (format cofdidx "/*****************************************************/\n")
    (format cofdidx "\n")
    (format cofdidx "#include \"cst_clunits.h\"\n")
    (format cofdidx "#include \"%s_clunits.h\"\n" name)
    (format cofdidx "#include \"%s_cltrees.h\"\n" name)

    (format cofdidx "\n\n")
    (set! unitbase_count 0)
    (set! unitbases nil)
    (mapcar
     (lambda (p)
       (if (and (not (string-matches (car p) ".*#.*"))
		(not (member_string (string-before (car p) "_") unitbases)))
	   (begin
	     (format cofdidx "#define unitbase_%s %d\n" 
		     (string-before (car p) "_") unitbase_count)
	     (set! unitbases (cons (string-before (car p) "_") unitbases))
	     (set! unitbase_count (+ 1 unitbase_count)))))
     clunits_entries)
    (format cofdidx "\n\n")

    (format cofdidx "const cst_clunit %s_units[] = { \n" name)
    (set! num_entries 0)
    (set! this_ut "")
    (set! this_ut_count 0)
    (mapcar
     (lambda (e)
       (if (not (string-equal this_ut (unit_type (nth 0 e))))
	   (begin
	     (if (> this_ut_count 0)
		 (format cofdh "#define unit_%s_num %d\n"
			 this_ut this_ut_count))
	     (format cofdh "#define unit_%s_start %d\n"
		     (unit_type (nth 0 e)) num_entries)
	     (set! this_ut (unit_type (nth 0 e)))
	     (set! this_ut_count 0)
	     ))
       (format cofdh "#define unit_%s %d\n" (nth 0 e) num_entries)
       (set! num_entries (+ 1 num_entries))
       (set! this_ut_count (+ 1 this_ut_count))
       (format cofdidx "   { /* %s */ unit_type_%s, unitbase_%s, %d,%d, %s, %s },\n"
               (nth 0 e)
               (unit_type (nth 0 e))
	       (string-before (nth 0 e) "_")
	       (nth 1 e) ; start_pm
	       (nth 3 e) ; end_pm
	       (nth 4 e) ; prev
	       (nth 5 e) ; next
	       ))
     clunits_entries)
    (format cofdidx "   { 0,0,0,0 } };\n\n")
    (format cofdidx "#define %s_num_units %d\n" name num_entries)
    (if (> this_ut_count 0)
	(format cofdh "#define unit_%s_num %d\n"
		this_ut this_ut_count))
    (fclose lofdh)
    (fclose lofdidx)
    (fclose mofdh)
    (fclose mofdidx)
    (fclose cofdh)
    (fclose cofdidx)
    ))

(define (find_pm_pos name entry stsdir mcepdir lofdsts  lofdh mofdsts mofdh)
  "(find_pm_pos entry lpddir)
Diphone dics give times in seconds here we want them as indexes.  This
function converts the lpc to ascii and finds the pitch marks that
go with this unit.  These are written to ofdsts with ulaw residual
as short term signal."
  (let ((sts_coeffs (load
		     (format nil "%s/%s.sts" stsdir (cadr entry))
		     t))
 	(mcep_coeffs (load_ascii_track
		      (format nil "%s/%s.mcep" mcepdir (cadr entry))
		      (nth 2 entry)))
	(start_time (nth 2 entry))
	(phoneboundary_time (nth 3 entry))
	(end_time (nth 4 entry))
	start_pm pb_pm end_pm
	(ltime 0))
    (format t "%l\n" entry)
;    (if (not (member_string (nth 1 entry) done_files))
;	(begin
;	  (format ofdsts "static const unsigned char res_%s[] = { \n"
;		  (nth 1 entry))
;	  (mapcar
;	   (lambda (d) (format ofdsts "%d, " d))
;	   (cadr sts_coeffs))
;	  (format ofdsts " 255 };\n")
;	  (set! done_files (cons (nth 1 entry) done_files))))
    (set! sts_info (car sts_coeffs))
    (set! sts_coeffs (cdr sts_coeffs))
;    (set! sts_coeffs (cddr sts_coeffs)) ; skip infor and residual
    (set! ltime 0)
    (set! size_to_now 0)
    (while (and sts_coeffs (cdr sts_coeffs)
	    (> (absdiff start_time (car (car sts_coeffs)))
	      (absdiff start_time (car (cadr sts_coeffs)))))
     (set! ltime (car (car sts_coeffs)))
     (set! mcep_coeffs (cdr mcep_coeffs))
     (set! sts_coeffs (cdr sts_coeffs)))
    (set! sample_rate (nth 2 sts_info))
    (set! lpc_order (nth 1 sts_info))
    (set! lpc_min (nth 3 sts_info))
    (set! lpc_range (nth 4 sts_info))
    (set! start_pm pm_pos)
    (while (and sts_coeffs (cdr sts_coeffs)
	    (> (absdiff phoneboundary_time (car (car sts_coeffs)))
	       (absdiff phoneboundary_time (car (cadr sts_coeffs)))))
     (output_mcep name (car mcep_coeffs) mofdsts mofdh)
     (output_sts name (car sts_coeffs) (nth 1 entry) lofdsts lofdh)
     (set! mcep_coeffs (cdr mcep_coeffs))
     (set! sts_coeffs (cdr sts_coeffs)))
    (set! pb_pm pm_pos)
    (while (and sts_coeffs (cdr sts_coeffs)
	    (> (absdiff end_time (car (car sts_coeffs)))
	       (absdiff end_time (car (cadr sts_coeffs)))))
     (output_mcep name (car mcep_coeffs) mofdsts mofdh)
     (output_sts name (car sts_coeffs) (nth 1 entry) lofdsts lofdh)
     (set! mcep_coeffs (cdr mcep_coeffs))
     (set! sts_coeffs (cdr sts_coeffs)))
    (set! end_pm pm_pos)

    (list 
     (car entry)
     (cadr entry)
     start_pm
     pb_pm
     end_pm)))

(define (output_sts name frame fname ofd ofdh)
  "(output_sts frame residual ofd)
Ouput this LPC frame."
  (let ((time (nth 0 frame))
	(coeffs (nth 1 frame))
	(size (nth 2 frame))
	(r (nth 3 frame)))
    (set! times (cons time times))

    (format ofd "const unsigned short %s_frame_%d[] = { \n" name pm_pos)
    (while (cdr coeffs)
     (format ofd " %d," (car coeffs))
     (set! coeffs (cdr coeffs))
     (if (not (cdr coeffs)) (format ofd " %d" (car coeffs))))
    (format ofd " };\n")

    (format ofd "const unsigned char %s_res_%d[] = { \n" name pm_pos)
    (set! s size)
    (while (cdr r)
     (format ofd " %d," (car r))
     (set! r (cdr r))
     (if (not (cdr r)) (format ofd " %d" (car r))))
    (format ofd " };\n")
;    (format ofd "#define res_%d &res_%s[%d]\n" pm_pos fname r)

    (set! residual_sizes (cons (list pm_pos size) residual_sizes))
    (format ofdh "extern const unsigned short %s_frame_%d[];\n" name pm_pos)
    (format ofdh "extern const unsigned char %s_res_%d[];\n" name pm_pos)
    (format ofdh "\n")

    (set! pm_pos (+ 1 pm_pos))
))

(define (lpccoeff_norm c)
  (* (/ (- c lpc_min) (- lpc_max lpc_min))
     65535))

(define (mcepcoeff_norm c)
  (* (/ (- c mcep_min) (- mcep_max mcep_min))
     65535))

(define (output_mcep name frame ofd ofdh)
  "(output_mcep frame duration residual ofd)
Ouput this MCEP frame."
  (let ()
    (set! mcep_order (- (length frame) 3))

    (format ofd "const unsigned short %s_mcep_frame_%d[] = { \n" name pm_pos)
    (set! frame (cddr frame)) ;; skip the "1"
    (set! frame (cdr frame)) ;; skip the energy
    (while (cdr frame)
     (format ofd " %d," (mcepcoeff_norm (car frame)))
     (set! frame (cdr frame))
     (if (not (cdr frame)) (format ofd " %d" (mcepcoeff_norm (car frame)))))
    (format ofd " };\n")

    (format ofdh "extern const unsigned short %s_mcep_frame_%d[];\n" 
	    name pm_pos)
))

(define (load_ascii_track trackfilename starttime)
   "(load_ascii_track trackfilename)
Coverts trackfilename to simple ascii representation."
   (let ((tmpfile (make_tmp_filename))
	 (nicestarttime (if (> starttime 0.100)
			    (- starttime 0.100)
			    starttime))
	 b)
     (system (format nil "$ESTDIR/bin/ch_track -otype est -start %f %s | 
                        awk '{if ($1 == \"EST_Header_End\")
                                 header=1;
                              else if (header == 1)
                                 printf(\"( %%s )\\n\",$0)}'>%s" 
		     nicestarttime trackfilename tmpfile))
     (set! b (load tmpfile t))
     (delete-file tmpfile)
     b))


(define (absdiff a b)
  (let ((d (- a b )))
    (if (< d 0)
	(* -1 d)
	d)))

(define (carttoC_extract_answer_list ofdh tree)
  "(carttoC_extract_answer_list tree)
Get list of answers from leaf node."
;  (carttoC_val_table ofdh 
;		     (car (last (car tree)))
;		     'none)
;  (format t "%l\n" (car tree))
  (cellstovals "cl" (mapcar car (caar tree)) ofdh)
  (format nil "cl_%04d" cells_count))

(define (sort_cltrees trees clcatfn)
  (let ((neworder nil))
    (mapcar
     (lambda (unit)
       (if (not (assoc_string (unit_type (car unit)) neworder))
	   (set! neworder (cons (assoc_string (unit_type (car unit)) trees)
				neworder))))
     (load clcatfn t))
    (reverse neworder)))

(define (clunits_make_carts cartfn clcatfn name odir cofd)
 "(define clunits_make_carts cartfn name)
Output clunit selection carts into odir/name_carts.c"
 (let (ofd ofdh)
 ;; Set up to dump full list of things at leafs
 (set! carttoC_extract_answer carttoC_extract_answer_list)
 (load cartfn)

 (set! ofd (fopen (format nil "%s/%s_cltrees.c" odir name) "w"))
 (set! ofdh (fopen (format nil "%s/%s_cltrees.h" odir name) "w"))
 (format ofd "\n")
 (format ofd "#include \"cst_string.h\"\n")
 (format ofd "#include \"cst_cart.h\"\n")
 (format ofd "#include \"cst_regex.h\"\n")
 (format ofd "#include \"%s_cltrees.h\"\n" name)

 (set! val_table nil)

 (set! clunits_selection_trees (sort_cltrees clunits_selection_trees clcatfn))

 (mapcar
  (lambda (cart)
    (set! current_node -1)
    (set! feat_nums nil)
    (do_carttoC ofd ofdh 
		(format nil "%s_%s" name (car cart))
		(cadr cart)))
  clunits_selection_trees)
 
 (format ofd "\n\n")
 (format ofd "const cst_cart * const %s_carts[] = {\n" name)
 (mapcar
  (lambda (cart)
    (format ofd " &%s_%s_cart,\n" name (car cart))
    )
  clunits_selection_trees)
 (format ofd " 0 };\n")

 (format cofd "\n\n")
 (format cofd "#define %s_num_unit_types %d\n" 
	 name (length clunits_selection_trees))

 (format cofd "\n\n")
 (format cofd "const cst_clunit_type %s_unit_types[] = {\n" name)
 (set! n 0)
 (mapcar
  (lambda (cart)
    (format ofdh "#define unit_type_%s %d\n" 
	    (car cart) n)
    (format cofd "  { \"%s\", unit_%s_start, unit_%s_num},\n" 
	    (car cart) (car cart) (car cart))
    (set! n (+ 1 n))
    )
  clunits_selection_trees)
 (format cofd "  { NULL, CLUNIT_NONE, CLUNIT_NONE } };\n")

 (fclose ofd)
 (fclose ofdh)

 )
)

(provide 'make_clunits)

