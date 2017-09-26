/* -*- mode: c; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; c-file-style: "stroustrup"; -*-
 *
 * 
 *                This source code is part of
 * 
 *                 G   R   O   M   A   C   S
 * 
 *          GROningen MAchine for Chemical Simulations
 * 
 *                        VERSION 4.6.0
 * Written by Christoph Junghans, Brad Lambeth, and possibly others.
 * Copyright (c) 2009 Christoph Junghans, Brad Lambeth.
 * All rights reserved.

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * If you want to redistribute modifications, please consider that
 * scientific software is very special. Version control is crucial -
 * bugs must be traceable. We will be happy to consider code for
 * inclusion in the official distribution, but derived work must not
 * be called official GROMACS. Details are found in the README & COPYING
 * files - if they are missing, get the official version at www.gromacs.org.
 * 
 * To help us fund GROMACS development, we humbly ask that you cite
 * the papers on the package - you can find them in the top README file.
 * 
 * For more info, check our website at http://www.gromacs.org
 * 
 * And Hey:
 * GROningen Mixture of Alchemy and Childrens' Stories
 */

#include "readir.h"
#include "names.h"
#include "smalloc.h"
#include "gmx_fatal.h"

#define MAXPTR 254

static char adress_refs[STRLEN], adress_refs_2[STRLEN], adress_tf_grp_names[STRLEN], adress_cg_grp_names[STRLEN];

void read_adressparams(int *ninp_p,t_inpfile **inp_p,t_adress *adress, warninp_t wi)
{
    int     nadress_refs, nadress_refs_2, i;
    const char *tmp;
    char    *ptr1[MAXPTR];
    char    *ptr1_2[MAXPTR];
    
    
    int    ninp;
    t_inpfile *inp;
    
    ninp   = *ninp_p;
    inp    = *inp_p;
    
    EETYPE("adress_type",                adress->type,         eAdresstype_names);
    RTYPE ("adress_const_wf",            adress->const_wf,     1);
    RTYPE ("adress_ex_width",            adress->ex_width,     0);
    RTYPE ("adress_hy_width",            adress->hy_width,     0);
    RTYPE ("adress_ex_forcecap",         adress->ex_forcecap,     0);
    EETYPE("adress_interface_correction",adress->icor,         eAdressICtype_names);
    EETYPE("adress_site",                adress->site,         eAdressSITEtype_names);
    STYPE ("adress_reference_coords",    adress_refs,             NULL);
    STYPE ("adress_reference_coords_lower_h",    adress_refs_2,             NULL);
    STYPE ("adress_tf_grp_names",        adress_tf_grp_names,     NULL);
    STYPE ("adress_cg_grp_names",        adress_cg_grp_names,     NULL);
    EETYPE("adress_do_hybridpairs",      adress->do_hybridpairs, yesno_names);
    EETYPE("adress_do_drift",      adress->do_drift, yesno_names);
    RTYPE("adress_deltaU", adress->deltaU, 0);
    EETYPE("adress_onthefly_TI",      adress->onthefly_TI, yesno_names);
    
    nadress_refs = str_nelem(adress_refs,MAXPTR,ptr1);
    
    for(i=0; (i<nadress_refs); i++) /*read vector components*/
        adress->refs[i]=strtod(ptr1[i],NULL);
    for( ;(i<DIM); i++) /*remaining undefined components of the vector set to zero*/
        adress->refs[i]=0;
    
    nadress_refs_2 = str_nelem(adress_refs_2,MAXPTR,ptr1_2);
    
    for(i=0; (i<nadress_refs_2); i++) /*read vector components*/
        adress->refs_2[i]=strtod(ptr1_2[i],NULL);
    for( ;(i<DIM); i++) /*remaining undefined components of the vector set to zero*/
        adress->refs_2[i]=0;

    
    
    
    
}



void do_adress_index(t_adress *adress, gmx_groups_t *groups,char **gnames,t_grpopts *opts,warninp_t wi){
  int nr,i,j,k;
  char    *ptr1[MAXPTR];
  int     nadress_cg_grp_names, nadress_tf_grp_names;
    
  /* AdResS coarse grained groups input */
  
  nr = groups->grps[egcENER].nr;
  snew(adress->group_explicit, nr);
  for (i=0; i <nr; i++){
      adress->group_explicit[i] = TRUE;
  }
  adress->n_energy_grps = nr;

  nadress_cg_grp_names = str_nelem(adress_cg_grp_names,MAXPTR,ptr1);

  if (nadress_cg_grp_names > 0){
        for (i=0; i <nadress_cg_grp_names; i++){
            /* search for the group name mathching the tf group name */
            k = 0;
            while ((k < nr) &&
                 gmx_strcasecmp(ptr1[i],(char*)(gnames[groups->grps[egcENER].nm_ind[k]])))
              k++;
            if (k==nr) gmx_fatal(FARGS,"Adress cg energy group %s not found\n",ptr1[i]);
            adress->group_explicit[k] = FALSE;
            printf ("AdResS: Energy group %s is treated as coarse-grained \n",
              (char*)(gnames[groups->grps[egcENER].nm_ind[k]]));
    }
        /* set energy group exclusions between all coarse-grained and explicit groups */
      for (j = 0; j < nr; j++) {
            for (k = 0; k < nr; k++) {
                if (!(adress->group_explicit[k] == adress->group_explicit[j])){
                    opts->egp_flags[nr * j + k] |= EGP_EXCL;
                    if (debug) fprintf(debug,"AdResS excl %s %s \n",
                        (char*)(gnames[groups->grps[egcENER].nm_ind[j]]),
                        (char*)(gnames[groups->grps[egcENER].nm_ind[k]]));
                }
            }
      }
  }else{
      warning(wi,"For an AdResS simulation at least one coarse-grained energy group has to be specified in adress_cg_grp_names");
  }


  /* AdResS multiple tf tables input */
  nadress_tf_grp_names = str_nelem(adress_tf_grp_names,MAXPTR,ptr1);
  adress->n_tf_grps = nadress_tf_grp_names;
  snew(adress->tf_table_index, nadress_tf_grp_names);

  nr = groups->grps[egcENER].nr;

  if (nadress_tf_grp_names > 0){
        for (i=0; i <nadress_tf_grp_names; i++){
            /* search for the group name mathching the tf group name */
            k = 0;
            while ((k < nr) &&
                 gmx_strcasecmp(ptr1[i],(char*)(gnames[groups->grps[egcENER].nm_ind[k]])))
              k++;
            if (k==nr) gmx_fatal(FARGS,"Adress tf energy group %s not found\n",ptr1[i]);
            
            adress->tf_table_index[i] = k;
            if (debug) fprintf(debug,"found tf group %s id %d \n",ptr1[i], k);
            if (adress->group_explicit[k]){
                gmx_fatal(FARGS,"Thermodynamic force group %s is not a coarse-grained group in adress_cg_grp_names. The thermodynamic force has to act on the coarse-grained vsite of a molecule.\n",ptr1[i]);
            }

    }
  }
  /* end AdResS multiple tf tables input */
   
  
}
