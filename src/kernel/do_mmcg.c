/***************************************************************************
 *   Copyright (C) 2005 by Marilisa Neri                                   *
 *   neri@sissa.it                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include "typedefs.h"
#include "network.h"
#include "vec.h"
#include "mtop_util.h"
// #include "types/enums.h"
#include "smalloc.h"
#include "gmx_ga2la.h"


int init_mmcg(	int nfile,		// number of files
		const t_filenm fnm[],	// file names
		t_inputrec* ir, 	// input record and box stuff
		gmx_mtop_t *top_global,	// global topology
		int *allcgnr, 		// charge groups number
		int *allcgid[],		// charge groups ids
		int *allsolnr, 		// solvent groups number
		int *allsolid[],	// solvent groups ids
		t_commrec *cr		// communicators
		)
{

      char *mmcgf, *topf, line[STRLEN+1], strtmp[STRLEN+1], *pos;
      FILE *mmcgfp, *topfp;
      int k0,k=0,i,resnr;
      int icg,cg0,cg1;
      int allatresnr, *allatresid;

      t_block all_cgs;
      all_cgs = gmx_mtop_global_cgs(top_global); // index of cgs for whole system

      mmcgf = ftp2fn(efGMX,nfile,fnm);

      if((mmcgfp = fopen(mmcgf,"r"))==NULL)
	{ // opening-of mmcg-data failed
        fprintf(stderr, "ERROR : Impossible to open the mmcg-data file\n");
	return 1;
	}
      while (!feof(mmcgfp))
	{ // counting lines in mmcg-data file
          if (fgets (line,STRLEN+1,mmcgfp)) allatresnr++;
        }

      // reading mmcg data
      snew (allatresid,allatresnr);
      rewind (mmcgfp);
      for (i=0,k=0; i<allatresnr;i++) {
         fgets(line,STRLEN+1,mmcgfp);
	 if(line[0] == '#') { 
             if(strstr(line,"nstwtlist")) { // number of step for list regeneration
		pos = strchr(line,'='); pos++; 
                ir->mmcg.nstwtlist = atoi(pos);
	     }
	     else if (strstr(line,"shell1wt")) { // inner limit of water droplet
		pos = strchr(line,'='); pos++; 
                ir->mmcg.shell1wt = atoi(pos);
	     }
	     else if (strstr(line,"shell2wt")) { // outer limit of water droplet
		pos = strchr(line,'='); pos++; 
                ir->mmcg.shell2wt = atoi(pos);
	     }
	     continue;
	}
        // reading all-atoms resids
        allatresid[k] = atoi(line);
        if(allatresid[k] != 0) k++;
      } // end for, mmcg file scanning 

      allatresnr = k;
      if (cr->nnodes!=1) gmx_barrier(cr);
      fclose(mmcgfp);

      // printing all-atoms resids in the log file
      fprintf(log,"MM/CG : gmx_resids: %d\n", allatresnr);
      for(i=0; i<allatresnr;i++)
	fprintf (log, "  %d\n", allatresid[i]);
      fprintf(log,"\n");

      // internal enumeration
      for (i=0; i<allatresnr;i++) (allatresid[i])--;

      // allocating charge groups and solvent
      snew(*allcgid, ncg_mtop(top_global));
      int maxsol;

      // It seems that GROMACS did not store the correct number of solvent
      // molecules in top_global... At this point the value of nmol is more than 10 million!
      // => srenew() at the end of the routine when allsolnr will be computed
      int moltype_id=0;
      while (strcmp(top_global->moltype->name[moltype_id],"SOL")) {
	moltype_id++;
      }
      maxsol = top_global->molblock[moltype_id].nmol;
      MPI_Bcast(&maxsol,1,MPI_INT,0,cr->mpi_comm_mysim);
      
      snew(*allsolid, maxsol);
      // charge groups to monitor, first and last indices
      cg0 = 0;
      cg1 = ncg_mtop(top_global); 
      char **atomname_=NULL,**resname_=NULL; // we need these to get the information of atoms
	  				     // cf. mtop_utils.h

      for(icg=cg0; icg<cg1; icg++) {
	// get residue number and name of the cg's first atom
	k0 = all_cgs.index[icg];
	gmx_mtop_atominfo_global(top_global,k0,&atomname_,&resnr,&resname_);

	if (!strcmp(resname_,"SOL")) { // Water
	  (*allsolid)[*allsolnr] = icg;
	  (*allsolnr)++; // counting waters
	}
	for (i=0; i<allatresnr; i++) {
	  if (resnr == allatresid[i]) { // touché !
	    (*allcgid)[*allcgnr] = icg;
	    (*allcgnr)++; // counting cg
	  }
        }
      }
      if(cr->nnodes!=1) gmx_barrier(cr);
      return 0;
}


 
void do_mmcg (int natoms,		// number of atoms in simulation
 	    t_inputrec   *inputrec, 	// input record and box stuff
	    t_mdatoms    *md,		// the atoms
	    t_state      *state,	// positions & velocities
	    gmx_mtop_t   *top,		// global topology
	    t_commrec    *cr,   	// communicators
	    rvec         *cg_cm,	// centre of mass of charge groups
	    int *allcgid, 		// charge groups ids
	    int allcgnr,		// charge groups number
	    int *allsolid,		// solvent groups ids
	    int allsolnr,		// solvent groups number
    	    FILE *log)			// logfile
{
 int i, j, p, q, qmin;
 real dvmod, shell2w2, dmin, d;
 rvec vecdist, dv, *v;
 shell2w2 = pow (inputrec->mmcg.shell2wt, 2.0);	// Threshold
 v = state->v;			               	// Velocities
 t_block cgs;					// charge groups
 cgs = gmx_mtop_global_cgs(top);
 rvec *all_cg_cm=NULL;
 snew(all_cg_cm,allcgnr);

 // for DD, we need to get cg_cm of all given charge groups (fr->cg_cm is local), 
 // the nearest one from a monitored water
 // may be more than one DD cell distant.
 if (DOMAINDECOMP(cr)) {
    if(cr->nnodes!=1) gmx_barrier(cr);
    for(i=0; i<allcgnr; i++) {
	int sender = 0,senderf,k;
	for(j=0; j<cr->dd->ncg_home; j++) { // is the cg a home cg ?
	   if(cr->dd->index_gl[j]==allcgid[i] ) {
		sender = cr->sim_nodeid;
		for(k=0;k<3;k++) all_cg_cm[i][k]=cg_cm[j][k];//FIXME improve ! compilation error when done with pointers
	   } 
	}
	MPI_Allreduce(&sender,&senderf,1,MPI_INT,MPI_SUM,cr->dd->mpi_comm_all);
	for(k=0;k<3;k++) MPI_Bcast(&all_cg_cm[i][k],sizeof(all_cg_cm[i][k]),MPI_BYTE,senderf,cr->dd->mpi_comm_all); //FIXME again !
    }
 }

 for(i=0; i<allsolnr; i++) {	// Loop over waters - START
    p = allsolid[i];
    if (PARTDECOMP(cr)) { // water i is in the node
        if((cgs.index[p]>=md->start) && (cgs.index[p]<(md->start+md->homenr))) {
      		for (j=0; j<allcgnr; j++) { // Looking for min dist (dmin)
		   q = allcgid[j];	    // and for the nearest charge group (qmin)
		   d = distance2(cg_cm[p],cg_cm[q]);
		   if(!j) 		{ dmin = d; qmin = q; } 
		   else if (d < dmin) 	{ dmin = d; qmin = q; }
        	}
	
      		if (dmin >= shell2w2) {	// Modifing velocity
		    rvec_sub(cg_cm[p], cg_cm[qmin], vecdist);
	 	    unitv (vecdist, vecdist);
		      for (j=cgs.index[p]; j<(3+cgs.index[p]); j++) {
	  		 dvmod = iprod (v[j], vecdist);
	  		 if (dvmod <= 0) continue;
	  		 svmul (2.0*dvmod, vecdist, dv);
	  		 rvec_dec (&v[j], dv); // Warning (=>?)				
		      }
      		}
	}
    }
    if (DOMAINDECOMP(cr)) { 
	int g_atnr; // global atom ID
	int l_atnr; // local atom ID in the DD cell
	int l_cgnr; // local charge group number

	for(g_atnr=cgs.index[p];g_atnr<=cgs.index[p]+2;g_atnr++) {
	    if(ga2la_get_home(cr->dd->ga2la,g_atnr,&l_atnr)) {// search in global to local lookup table 
							      // if the atom (not water) is in home atoms 
							      // and get local atom number
      		l_cgnr = cr->dd->la2lc[l_atnr]; // get local charge group number
	
		for (j=0; j<allcgnr; j++) { // Looking for min dist (dmin)
					    // and for the nearest charge group (qmin)
		   d = distance2(cg_cm[l_cgnr],all_cg_cm[j]);
		   if(!j) 		{ dmin = d; qmin = j; } 
		   else if (d < dmin) 	{ dmin = d; qmin = j; }
        	}

      		if (dmin >= shell2w2) {	// Modifing velocity
		    rvec_sub(cg_cm[l_cgnr], all_cg_cm[qmin], vecdist);
	 	    unitv (vecdist, vecdist);
	  	    dvmod = iprod (v[l_atnr], vecdist);
	  	    if (dvmod <= 0) continue;
	  	    svmul (2.0*dvmod, vecdist, dv);
	  	    rvec_dec (&v[l_atnr], dv);	// Warning (=>?)
      		}
	    }
	}
    }
  } // Loop over waters - END
  return;
}
