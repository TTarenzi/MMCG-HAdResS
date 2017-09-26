#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
// #include "types/enums.h"
#include "typedefs.h"
#include "network.h"
#include "vec.h"
#include "mtop_util.h"

#include "smalloc.h"
#include "gmx_ga2la.h"

// WALL POTENTIAL STUFF -----------------------------------------------------------------------------------------------

/*

New structures (defined in include/types/inputrec.h): 

  t_user_potential : 

     Contains parameters and data relative to the added walls.

     wpot_gs (substructure since used inside t_user_potential) :

         Contains information about grid searches that save time instead of
         looping through all the surface points or the surface atoms
         (GS atoms still to be fixed)     // EMY_CHECK: WAS THIS FIXED?

     t_user_force_vdw (substructure since used inside t_user_potential) :
     
         Info about vdw-like force to reduce computation time (shift function coefficients)

*/  


// I/O ________________________________________________________________________________________________________________
// ____________________________________________________________________________________________________________________

void get_real_from_file(FILE* file, real *r, char* str) {

  char chaine[STRLEN+1], *pos;
  rewind(file);
  while (!feof(file)) {
    fgets(chaine,STRLEN,file);
    if (chaine[0] != '#' && strstr(chaine,str)) {
      pos = strchr(chaine,'='); pos++;
      while (*pos == ' ') pos++;
      *r = atof(pos);
    }
  }  
}


void get_int_from_file(FILE* file, int *r, char* str) {

  char chaine[STRLEN+1], *pos;
  rewind(file);
  while (!feof(file)) {
    fgets(chaine,STRLEN,file);
    if (chaine[0] != '#' && strstr(chaine,str)) {
      pos = strchr(chaine,'='); pos++;
      while (*pos == ' ') pos++;
      *r = atoi(pos);
    }
  }  
}


void get_char_from_file(FILE* file, char *r, char* str) {

  char chaine[STRLEN+1], *pos;
  rewind(file);
  while (!feof(file)) {
    fgets(chaine,STRLEN,file);
    if (chaine[0] != '#' && strstr(chaine,str)) {
      pos = strchr(chaine,'='); pos++;
      while (*pos == ' ') pos++;
      *r = pos[0];
    }
  }  
}


void get_string_from_file(FILE* file, char* r, char* str) {

  char chaine[STRLEN+1], *pos;
  rewind(file);
  while (!feof(file)) {
    fgets(chaine,STRLEN,file);
    if (chaine[0] != '#' && strstr(chaine,str)) {
      pos = strchr(chaine,'=');pos++;
      while (*pos == ' ') pos++;
      int pos2=1;
      while (pos[pos2] != '\n') pos2++;
      pos[pos2] = '\0';
      strcpy(r,pos);
    }
  }  
}


void write_gro_points(FILE* f, int n, rvec *x, rvec shift) {

  int i;
  fprintf(f,"Some title\n%5d\n",n);
  for (i=0;i<n;i++) {
    fprintf(f,"%5dWPT     CA%5d %7.3lf %7.3lf %7.3lf\n",i,i,x[i][0]+shift[0],x[i][1]+shift[1],x[i][2]+shift[2]);
  }
  fprintf(f,"   100.0    100.0   100.0\n");
}


void write_gro_points_detailed(FILE* f, int n, rvec *x, gmx_bool *prop1, rvec shift, char *title) {

  int i;
  fprintf(f,"%s\n%5d\n",title,n);
  for (i=0;i<n;i++) {
    if (!prop1[i])
      fprintf(f,"%5dWPT     CA%5d %7.3lf %7.3lf %7.3lf\n",i,i,x[i][0]+shift[0],x[i][1]+shift[1],x[i][2]+shift[2]);
    else
      fprintf(f,"%5dWPT     N %5d %7.3lf %7.3lf %7.3lf\n",i,i,x[i][0]+shift[0],x[i][1]+shift[1],x[i][2]+shift[2]);
  }
  fprintf(f,"   100.0    100.0   100.0\n");
}


void write_points_vtk(FILE* f, int n, rvec *x, rvec shift) {
  // files readable with paraview, visIt, etc.
  int i;
  fprintf(f,"# vtk DataFile Version 3.0\nforce\nASCII\nDATASET UNSTRUCTURED_GRID\n");
  fprintf(f,"POINTS %d float\n",n);
  for(i=0;i<n;i++) {
    fprintf(f,"%lf %lf %lf\n",x[i][0]+shift[0],x[i][1]+shift[1],x[i][2]+shift[2]);	
  }
  fprintf(f,"\nPOINT_DATA %d\n",n);
  fprintf(f,"SCALARS dist float\nLOOKUP_TABLE default\n");
  for (i=0;i<n;i++) {
    fprintf(f,"1.\n");
  }
}


void write_field_vtk(FILE* f, int n, rvec *x, rvec* ff, rvec *shift) {

  int i;
  fprintf(f,"# vtk DataFile Version 3.0\nforce\nASCII\nDATASET UNSTRUCTURED_GRID\n");
  fprintf(f,"POINTS %d float\n",n);
  for(i=0;i<n;i++) {
    fprintf(f,"%lf %lf %lf\n",x[i][0]+(*shift)[0],x[i][1]+(*shift)[1],x[i][2]+(*shift)[2]);	
  }
  fprintf(f,"\nPOINT_DATA %d\n",n);
  fprintf(f,"\nVECTORS vect float\n");
  for(i=0;i<n;i++) {  
    fprintf(f,"%lf %lf %lf\n",ff[i][0],ff[i][1],ff[i][2]);
  }
}


void write_scalar_vtk(FILE* f, int n, rvec *x, real* s, rvec *shift) {

  int i;
  fprintf(f,"# vtk DataFile Version 3.0\nforce\nASCII\nDATASET UNSTRUCTURED_GRID\n");
  fprintf(f,"POINTS %d float\n",n);
  for(i=0;i<n;i++) {
    fprintf(f,"%lf %lf %lf\n",x[i][0]+(*shift)[0],x[i][1]+(*shift)[1],x[i][2]+(*shift)[2]);	
  }
  fprintf(f,"\nPOINT_DATA %d\n",n);
  fprintf(f,"SCALARS scal float\nLOOKUP_TABLE default\n");
  for(i=0;i<n;i++) {  
    fprintf(f,"%lf\n",s[i]);
  }
}


void write_field_and_scalar_vtk(FILE* f, int n, rvec *x, real* s, rvec* ff, rvec *shift) {

  int i;
  fprintf(f,"# vtk DataFile Version 3.0\nforce\nASCII\nDATASET UNSTRUCTURED_GRID\n");
  fprintf(f,"POINTS %d float\n",n);
  for(i=0;i<n;i++) {
    fprintf(f,"%lf %lf %lf\n",x[i][0]+(*shift)[0],x[i][1]+(*shift)[1],x[i][2]+(*shift)[2]);	
  }
  fprintf(f,"\nPOINT_DATA %d\n",n);
  fprintf(f,"SCALARS scal float\nLOOKUP_TABLE default\n");
  for(i=0;i<n;i++) {  
    fprintf(f,"%lf\n",s[i]);
  }
  fprintf(f,"\nVECTORS vect float\n");
  for(i=0;i<n;i++) {  
    fprintf(f,"%lf %lf %lf\n",ff[i][0],ff[i][1],ff[i][2]);
  }
}


void write_gnu_files( t_user_potential *pot) {
  // Raw data files (binary); first line contains the center of the simulation box 
  // at the beginning of the first simulation, thus to make consecutive MMCG/Wpot 
  // simulations without changing the position of the surface and the membrane limits
  int i;

  FILE* gnuout=NULL;
  gnuout = fopen("surf_gnu.dat","w");
  fprintf(gnuout,"%lf %lf %lf\n",pot->center[pot->dim0],pot->center[pot->dim1],pot->center[pot->dim2]);
  for(i=0;i<pot->np_Surf;i++)
    fprintf(gnuout,"%lf %lf %lf %d\n",pot->Surf[i][pot->dim0],pot->Surf[i][pot->dim1],pot->Surf[i][pot->dim2],pot->sgs->prop1[i]);
  fclose(gnuout);
  gnuout = fopen("surf_atoms_gnu.dat","w");
  fprintf(gnuout,"%lf %lf %lf\n",pot->center[pot->dim0],pot->center[pot->dim1],pot->center[pot->dim2]);
  for(i=0;i<pot->Surf_natoms;i++)
    fprintf(gnuout,"%lf %lf %lf %d\n",pot->Surf_atoms[i][pot->dim0],pot->Surf_atoms[i][pot->dim1],pot->Surf_atoms[i][pot->dim2],pot->sags->prop1[i]);
  fclose(gnuout);

}


// FORCES _____________________________________________________________________________________________________________
// ____________________________________________________________________________________________________________________

  // EMY_CHANGE: Added the function for the computation of vdw-like forces due to the hemisphere, which is in principle different from the ones due to the coating wall
real user_force_vdw_hemisphere (t_user_potential *pot, real r) {

  // user_force_vdw_hemisphere provides (-dV/dr)
  // Adapted Lennard Jones potential
  // Power 2n and n instead of 12 and 6 (see init to change power, it can then be parametrized)
  // Shift function is applied to the potential between min_hs (minimum of the potential due to the definition of sigma_hs) and pot->vdw->rmax_hs
  // At pot->vdw->rmax_hs the potential goes to zero. pot->vdw->rmax_hs < pot->rcut so that the force does not become zero abruptly at pot->rcut when the force routine is not called anymore.                    
  // It is a third order interpolation polynom whose coefficients are stored during init_Wpotential

  // user_force_vdw_hemisphere always returns the force acting on the atom due to the walls

  if (r<=0) return 0.;

  // Too close, limit the force (linear approximation of the potential at short distance, so that the force is constant and continuous in r=sigma_hs) 
  if (r < pot->vdw->sigma_hs) {
    if (pot->bWpot_ener)
      pot->Wpotener += pot->c1*pot->vdw->VDW_reduc_hs*(pot->vdw->sigma_hs-r);
    return pot->c1*pot->vdw->VDW_reduc_hs ;
  }
  
  // Too far
  if (r > pot->vdw->rmax_hs) return 0.;

  // Actual force
  if (r < pot->vdw->min_hs) {
    // EMY_CHANGE: The separation of n=1 case allows one to get better performance
    if (pot->vdw->n_hs == 1) {
      if (pot->bWpot_ener)
	pot->Wpotener += pot->vdw->VDW_reduc_hs*4.*pot->vdw->epsilon_hs*(pot->vdw->sigma_hs*pot->vdw->sigma_hs/(r*r)-pot->vdw->sigma_hs/r);
      return pot->vdw->VDW_reduc_hs*4./r*pot->vdw->epsilon_hs*(2*pot->vdw->sigma_hs*pot->vdw->sigma_hs/(r*r)-pot->vdw->sigma_hs/r);
    }
    else {
      if (pot->bWpot_ener)
	pot->Wpotener += pot->vdw->VDW_reduc_hs*4.*pot->vdw->epsilon_hs*(pow(pot->vdw->sigma_hs/r,2.*pot->vdw->n_hs)-pow(pot->vdw->sigma_hs/r,pot->vdw->n_hs));
      return pot->vdw->VDW_reduc_hs*4.*pot->vdw->n_hs/r*pot->vdw->epsilon_hs*(2*pow(pot->vdw->sigma_hs/r,2.*pot->vdw->n_hs)-pow(pot->vdw->sigma_hs/r,pot->vdw->n_hs));
    }
  }

  // Shift: after the minimum distance the function goes more rapidly than standard LJ function to zero at rmax_hs
  // EMY_CHANGE: The separation of n=1 case allows one to get better performance
  if (pot->vdw->n_hs == 1) {
    if (pot->bWpot_ener)
      pot->Wpotener += pot->vdw->VDW_reduc_hs*4.*pot->vdw->epsilon_hs*(pot->vdw->sigma_hs*pot->vdw->sigma_hs/(r*r)-pot->vdw->sigma_hs/r) + (pot->vdw->D_hs+r*(pot->vdw->C_hs+r*(pot->vdw->B_hs+r*pot->vdw->A_hs)))/(pot->vdw->pm_hs*pot->vdw->pm_hs*pot->vdw->pm_hs);
    return pot->vdw->VDW_reduc_hs*4./r*pot->vdw->epsilon_hs*(2*pot->vdw->sigma_hs*pot->vdw->sigma_hs/(r*r)-pot->vdw->sigma_hs/r) - (pot->vdw->C_hs+r*(2.*pot->vdw->B_hs+3.*pot->vdw->A_hs*r))/(pot->vdw->pm_hs*pot->vdw->pm_hs*pot->vdw->pm_hs);
  }
  else {
    if (pot->bWpot_ener)
      pot->Wpotener += pot->vdw->VDW_reduc_hs*4.*pot->vdw->epsilon_hs*(pow(pot->vdw->sigma_hs/r,2.*pot->vdw->n_hs)-pow(pot->vdw->sigma_hs/r,pot->vdw->n_hs)) + (pot->vdw->D_hs+r*(pot->vdw->C_hs+r*(pot->vdw->B_hs+r*pot->vdw->A_hs)))/(pot->vdw->pm_hs*pot->vdw->pm_hs*pot->vdw->pm_hs);
    return pot->vdw->VDW_reduc_hs*4.*pot->vdw->n_hs/r*pot->vdw->epsilon_hs*(2*pow(pot->vdw->sigma_hs/r,2.*pot->vdw->n_hs)-pow(pot->vdw->sigma_hs/r,pot->vdw->n_hs)) - (pot->vdw->C_hs+r*(2.*pot->vdw->B_hs+3.*pot->vdw->A_hs*r))/(pot->vdw->pm_hs*pot->vdw->pm_hs*pot->vdw->pm_hs);
  }
}

real user_force_vdw (t_user_potential *pot, real r) {

  // user_force_vdw provides (-dV/dr)
  // Adapted Lennard Jones potential
  // Power 2n and n instead of 12 and 6 (see init to change power, it can then be parametrized)
  // Shift function is applied to the potential between r_protein (minimum of the potential due to the definition of sigma) and pot->vdw->rmax.
  // At pot->vdw->rmax the potential goes to zero. pot->vdw->rmax < pot->rcut so that the force does not become zero abruptly at pot->rcut when the force routine is not called anymore.  
  // It is a third order interpolation polynom whose coefficients are stored during init_Wpotential

  // user_force_vdw always returns the force acting on the atom due to the walls

  if (r<=0) return 0.;

  // Too close, limit the force (linear approximation of the potential at short distance, so that the force is constant and continuous in r=sigma) 
  if (r < pot->vdw->sigma) {
    if (pot->bWpot_ener)
      pot->Wpotener += pot->c1*pot->vdw->VDW_reduc*(pot->vdw->sigma-r);
    return pot->c1*pot->vdw->VDW_reduc ;
  }
  
  // Too far
  if (r > pot->vdw->rmax) return 0.;

  // Actual force
  if (r < pot->r_protein) {
    // EMY_CHANGE: The separation of n=1 case allows one to get better performance
    if (pot->vdw->n == 1) {
      if (pot->bWpot_ener)
	pot->Wpotener += pot->vdw->VDW_reduc*4.*pot->vdw->epsilon*(pot->vdw->sigma*pot->vdw->sigma/(r*r)-pot->vdw->sigma/r);
      return pot->vdw->VDW_reduc*4./r*pot->vdw->epsilon*(2*pot->vdw->sigma*pot->vdw->sigma/(r*r)-pot->vdw->sigma/r);
    }
    else {
      if (pot->bWpot_ener)
	pot->Wpotener += pot->vdw->VDW_reduc*4.*pot->vdw->epsilon*(pow(pot->vdw->sigma/r,2.*pot->vdw->n)-pow(pot->vdw->sigma/r,pot->vdw->n));
      return pot->vdw->VDW_reduc*4.*pot->vdw->n/r*pot->vdw->epsilon*(2*pow(pot->vdw->sigma/r,2.*pot->vdw->n)-pow(pot->vdw->sigma/r,pot->vdw->n));
    }
  }

  // Shift: after the minimum distance the function goes more rapidly than standard LJ function to zero at rmax_hs
  // EMY_CHANGE: The separation of n=1 case allows one to get better performance
  if (pot->vdw->n == 1) {
    if (pot->bWpot_ener)
      pot->Wpotener += pot->vdw->VDW_reduc*4.*pot->vdw->epsilon*(pot->vdw->sigma*pot->vdw->sigma/(r*r)-pot->vdw->sigma/r) + (pot->vdw->D+r*(pot->vdw->C+r*(pot->vdw->B+r*pot->vdw->A)))/(pot->vdw->pm*pot->vdw->pm*pot->vdw->pm);
    return pot->vdw->VDW_reduc*4./r*pot->vdw->epsilon*(2*pot->vdw->sigma*pot->vdw->sigma/(r*r)-pot->vdw->sigma/r) - (pot->vdw->C+r*(2.*pot->vdw->B+3.*pot->vdw->A*r))/(pot->vdw->pm*pot->vdw->pm*pot->vdw->pm);
  }
  else {
    if (pot->bWpot_ener)
      pot->Wpotener += pot->vdw->VDW_reduc*4.*pot->vdw->epsilon*(pow(pot->vdw->sigma/r,2.*pot->vdw->n)-pow(pot->vdw->sigma/r,pot->vdw->n)) + (pot->vdw->D+r*(pot->vdw->C+r*(pot->vdw->B+r*pot->vdw->A)))/(pot->vdw->pm*pot->vdw->pm*pot->vdw->pm);
    return pot->vdw->VDW_reduc*4.*pot->vdw->n/r*pot->vdw->epsilon*(2*pow(pot->vdw->sigma/r,2.*pot->vdw->n)-pow(pot->vdw->sigma/r,pot->vdw->n)) - (pot->vdw->C+r*(2.*pot->vdw->B+3.*pot->vdw->A*r))/(pot->vdw->pm*pot->vdw->pm*pot->vdw->pm);
  }
}

real user_force (t_user_potential * pot, real r) {

// user_force provides (-dV/dr)
// 1/r potential switched by a cos^2 function and corresponding force 
// Potential discontinuos in 0.01 
// Force continuous in 0.01 and almost constant between 0 and 0.01 (No divergence for r->0)
// For r=rcut force = 0 (due to the cosine function dependent on rcut). Then, it oscillates around 0 but this is not relevant because the force will act only between 0 and rcut (see compute_single_force).

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

  if (r<=0) return 0.;
  if (r < 0.01) { 
    real ordo = 0.01*cos(0.01/pot->rcut*M_PI/2.);
    if (pot->bWpot_ener)
     //  pot->Wpotener += pot->c1*(ordo-r)*pow(cos(r/pot->rcut*M_PI/2.),2);
      pot->Wpotener += pot->c1*(ordo-r)*cos(r/pot->rcut*M_PI/2.)*cos(r/pot->rcut*M_PI/2.);
    // EMY_CHANGE: Simplified
    //    return pot->c1*(pow(cos(r/pot->rcut*M_PI/2.),2)+2.*(ordo-r)*sin(r/pot->rcut*M_PI/2.)*M_PI/(2.*pot->rcut)*cos(r/pot->rcut*M_PI/2.));
    // EMY_CHANGE: More efficient
    // EMY_CHECK: Since force almost constant to c1 between 0 and 0.01, can we replace the formula with the constant value?
    return pot->c1*(cos(r/pot->rcut*M_PI/2.)*cos(r/pot->rcut*M_PI/2.)+(ordo-r)*sin(r/pot->rcut*M_PI/2.)*M_PI/pot->rcut*cos(r/pot->rcut*M_PI/2.));
  }

  if(pot->bWpot_ener)    
  //  pot->Wpotener += pot->c1*1.e-4/r*pow(cos(r*M_PI/(2.*pot->rcut)),2);
    pot->Wpotener += pot->c1*1.e-4/r*cos(r*M_PI/(2.*pot->rcut))*cos(r*M_PI/(2.*pot->rcut));
  // EMY_CHANGE: Simplified
  //  return pot->c1*1.e-4/r*(pow(cos(r*M_PI/(2.*pot->rcut)),2)/r + 2.*M_PI/(2.*pot->rcut)*sin(r*M_PI/(2.*pot->rcut))*cos(r/pot->rcut*M_PI/2.));
  // EMY_CHANGE: More efficient
  return pot->c1*1.e-4/r*(cos(r*M_PI/(2.*pot->rcut))*cos(r*M_PI/(2.*pot->rcut))/r + M_PI/pot->rcut*sin(r*M_PI/(2.*pot->rcut))*cos(r/pot->rcut*M_PI/2.));
}

// ___________________________________________________________________________________________________________________


// Grid search routine
void find_gs_cell (rvec *x, t_user_potential *pot, wpot_gs *gs,
                   int *xb, int *xe, int *yb, int *ye, int *zb, int *ze
                  ){

  int zlayer = (int)floor(((*x)[pot->dim2]-gs->z_min)/gs->dz);
  *zb = (int)max(min(zlayer-1,gs->nz),0);
  *ze = (int)min(max(0,zlayer+2),gs->nz);

  int ylayer = (int)floor(((*x)[pot->dim1]-gs->y_min)/gs->dy);
  *yb = (int)max(min(ylayer-1,gs->ny),0);
  *ye = (int)min(max(ylayer+2,0),gs->ny);

  int xlayer = (int)floor(((*x)[pot->dim0]-gs->x_min)/gs->dx);
  *xb = (int)max(min(xlayer-1,gs->nx),0);
  *xe = (int)min(max(xlayer+2,0),gs->nx);

}

// LEVEL-SET FUNCTION _________________________________________________________________________________________________
// ____________________________________________________________________________________________________________________

real phi (t_user_potential *pot, rvec *x) {

  // If only r_protein is defined:
  // phi = smoothen*r_protein - min_i || x - c_i ||
  // where c_i is the position of the surface atom i

  // If more radii (r_j) are defined (e.g. r_protein and r_panchor for anchor residues):
  // phi = max_j { smoothen*r_j - min_i_j || x - c_i_j || }
  // where c_i_j is the position of the of the surface atom i of type j

  // Since it loops through all (or some of) the monitored atoms
  // we want to call this function as less as possible
  real distmin = 1337.e42;
  real distmin_anchor = 1337.e42;

  // EMY_CHECK: Is this block ever used? The part of the code that set pot->bAtoms_GS=1 is commented.
  // If the grid search is selected
  if (pot->bAtoms_GS) {

    int xbegin,xend,ybegin,yend,zbegin,zend;

    find_gs_cell(x,pot,pot->sags,&xbegin,&xend,&ybegin,&yend,&zbegin,&zend);

    int i,ii,iii,iiii;
    for(i=zbegin;i<zend;i++) {
      for(ii=ybegin;ii<yend;ii++) {
        for(iii=xbegin;iii<xend;iii++) {
          int xxbegin = pot->sags->xi[i][ii][iii];
          int xxend   = pot->sags->xi[i][ii][iii+1];
          if (xxbegin == pot->Surf_natoms) xxbegin--;
          if (xxend == pot->Surf_natoms) xxend--;
	  for (iiii=xxbegin;iiii<xxend;iiii++) {
            real dist = sqrt(distance2(*x,pot->Surf_atoms[iiii]));
            if (!(pot->sags->prop1[iiii]))
              {if(distmin>dist) distmin = dist;}
            else
              {if(distmin_anchor>dist) distmin_anchor = dist;}
  	  }
    }}}
  }

  else { // all-atoms
    int i;
    for(i=0;i<pot->Surf_natoms;i++) {
      real dist = sqrt(distance2(*x,pot->Surf_atoms[i]));
      if (!(pot->sags->prop1[i]))
        {if(distmin>dist) distmin = dist;}
      else
        {if(distmin_anchor>dist) distmin_anchor = dist;}
    }
  }

    return max(pot->smoothen*pot->r_protein - distmin,pot->smoothen*pot->r_panchor - distmin_anchor);
}

// inner surface computation ________________________________________________

void add_virtual_atoms (t_user_potential *pot) {

  // With no or too low smoothing coefficient, some surface points may appear 
  // inside the cavity, we want to remove them to prevent potential wells
  // from happening.


  // attempt : in each layer, insert atoms between those which are too far
  // one from each other, staying near the center of the layer though
  // not "disturbing" the original outer surface 

  // ordering in z
  /*rvec *temp_x_atom;
  snew(temp_x_atom,1);
  int *layers, nlayers=(int)floor(1.2*(pot->sags->z_max-pot->sags->z_min)/pot->r_protein);
  snew(layers,nlayers+1);
  int i,j,k,l;
  int layer=0;

  for(i=0;i<pot->Surf_natoms;i++) {
    real zmin = pot->Surf_atoms[i][pot->dim2];
    int indmin=i;
    for(j=i+1;j<pot->Surf_natoms;j++) {
      if (zmin > pot->Surf_atoms[j][pot->dim2]) {
        zmin = pot->Surf_atoms[j][pot->dim2];
        indmin = j;
      }
    }
    copy_rvec(pot->Surf_atoms[i],*temp_x_atom);
    copy_rvec(pot->Surf_atoms[indmin],pot->Surf_atoms[i]);
    copy_rvec(*temp_x_atom,pot->Surf_atoms[indmin]);
    gmx_bool temp = pot->sags->prop1[indmin];
    pot->sags->prop1[indmin] = pot->sags->prop1[i];
    pot->sags->prop1[i] = temp;

    if ((zmin-pot->sags->z_min) >= layer*pot->r_protein) {
      int zlayermax = min((int)floor((zmin-pot->sags->z_min)/pot->r_protein),nlayers);
      int zzzz;
      for(zzzz=layer+1;zzzz<=zlayermax;zzzz++)
        layers[zzzz] = i;
      layer=zlayermax;
    }
  }
  
  int zzz; 
  for(zzz=layer+1;zzz<=nlayers;zzz++)
    layers[zzz] = pot->Surf_natoms; */  
  
  // computing layers centers

  /*int added=0,maxperlayer=20;
  srenew(pot->Surf_atoms,pot->Surf_natoms+2*maxperlayer*nlayers);
*/
  /*

  ...

  */


 // attempt : do not reduce r_p for inner atoms of anchored residues

  real dimmax = 0.25*min(pot->sags->y_max-pot->sags->y_min,pot->sags->x_max-pot->sags->x_min);
  int i;
  for(i=0;i<pot->Surf_natoms;i++) {
    if (pot->sags->prop1[i]) {
      real radius = sqrt(pow(pot->Surf_atoms[i][pot->dim0],2)+pow(pot->Surf_atoms[i][pot->dim1],2));
      if (radius<=dimmax) pot->sags->prop1[i]=FALSE;
    }
  }

  //sfree(layers);
  //sfree(temp_x_atom);

}

void build_index ( t_user_potential *pot,
  		   wpot_gs *gs,
                   int nd, rvec *d // data to index
                 ) {

  // For either atoms or surface points (EMY: However the part for atoms has to be fixed and has been commented)
  // This routine:
  //     - sorts the elements (points/atoms) according to their coordinates in z-direction.
  //     - divides the z-domain into layers whose thickness (dz) is defined before calling the function.
  //     - stores in zi[j] the index of the first element in the sorted list (starting counting from 0) belonging to layer j (starting counting from 0) in z-direction 
  //       (e.g. zi[2]=5 means that the sixth (5+1) element in the sorted list is the first element in the third (2+1) z-layer
  //     - repeats the same procedure for the elements in each z-layer considering the y-direction
  //       (e.g. yi[2][3]=7 means that among the elements in the third (2+1) z-layer, the eighth (7+1) one (sorted according to their coordinates in y-direction) is the first element in the fourth (3+1) y-layer)
  //     - repeats the same procedure for the elements in each y-sublayer of each z-layer considering the x-direction
  //       (e.g. xi[2][3][5]=2)

  int i,j,k,l;

  // EMY_CHANGE: The max function is useless here since (int)floor((gs->z_max - gs->z_min)/gs->dz) >= 0
  // gs->nz = max((int)floor((gs->z_max - gs->z_min)/gs->dz)+1,1);
  // gs->ny = max((int)floor((gs->y_max - gs->y_min)/gs->dy)+1,1);
  // gs->nx = max((int)floor((gs->x_max - gs->x_min)/gs->dx)+1,1);
  gs->nz = (int)floor((gs->z_max - gs->z_min)/gs->dz)+1;
  gs->ny = (int)floor((gs->y_max - gs->y_min)/gs->dy)+1;
  gs->nx = (int)floor((gs->x_max - gs->x_min)/gs->dx)+1;

  rvec temp_x_atom;
  snew(gs->zi,gs->nz+1);  
  // Sorting in z direction
  int zlayer=0; 
  for(i=0;i<nd;i++) {
    // Get the minimum for the i-th position on the d[] array
    real zmin = d[i][pot->dim2];
    int indmin=i;
    for(j=i+1;j<nd;j++) {
      if (zmin > d[j][pot->dim2]) {
        zmin = d[j][pot->dim2];
        indmin = j;
      }
    } 
    // Swap i with indmin
    gmx_bool temp = gs->prop1[indmin];
    gs->prop1[indmin] = gs->prop1[i];
    gs->prop1[i] = temp;
    copy_rvec(d[i],temp_x_atom);
    copy_rvec(d[indmin],d[i]);
    copy_rvec(temp_x_atom,d[indmin]);

    if ((zmin-gs->z_min) >= zlayer*gs->dz) {
      // EMY_CHECK: Is the min function useful here? nz always >= floor(zmin-gs->z_min)/gs->dz) + 1 by construction...
      int zlayermax = min((int)floor((zmin-gs->z_min)/gs->dz),gs->nz);
      int zzzz;
      for(zzzz=zlayer+1;zzzz<=zlayermax;zzzz++)
        gs->zi[zzzz] = i;
      zlayer=zlayermax;
    }
  }
  // EMY: gs->zi[nz] = number of elements
  int zzz;
  for(zzz=zlayer+1;zzz<=gs->nz;zzz++)
    gs->zi[zzz] = nd;   
    
  // y direction
  snew(gs->yi,gs->nz);
  for(l=0;l<gs->nz;l++) { // loop over zlayers
    // Sorting in y direction (one array for each zlayer)
    int ylayer=0;
    snew(gs->yi[l],gs->ny+1);
    gs->yi[l][0] = gs->zi[l];
    if (gs->zi[l]!=gs->zi[l+1]) {
      for(i=gs->zi[l];i<gs->zi[l+1];i++) {
        real ymin = d[i][pot->dim1];
        int indmin=i;
        for(j=i+1;j<gs->zi[l+1];j++) {
          if (ymin > d[j][pot->dim1]) {
            ymin = d[j][pot->dim1];
            indmin = j;
          }
        }
        gmx_bool temp = gs->prop1[indmin];
        gs->prop1[indmin] = gs->prop1[i];
        gs->prop1[i] = temp;
        copy_rvec(d[i],temp_x_atom);
        copy_rvec(d[indmin],d[i]);
        copy_rvec(temp_x_atom,d[indmin]);

        if ((ymin-gs->y_min) >= ylayer*gs->dy) {
          int ylayermax = min((int)floor((ymin-gs->y_min)/gs->dy),gs->ny);
          int yyyy;
          for(yyyy=ylayer+1;yyyy<=ylayermax;yyyy++)
            gs->yi[l][yyyy] = i;
          ylayer = ylayermax;
        }
      }
    }
    int yyy;
    for(yyy=ylayer+1;yyy<=gs->ny;yyy++)
    gs->yi[l][yyy] = gs->zi[l+1];   
  }

  // x dir
  snew(gs->xi,gs->nz);
  for(l=0;l<gs->nz;l++) { // loop over z layers
    snew(gs->xi[l],gs->ny);
    for(k=0;k<gs->ny;k++) { // loop over y[i] layers
      int xlayer=0; 
      snew(gs->xi[l][k],gs->nx+1);    
      gs->xi[l][k][0] = gs->yi[l][k];
      if (gs->yi[l][k]!=gs->yi[l][k+1]) {
        for(i=gs->yi[l][k];i<gs->yi[l][k+1];i++) {
          real xmin = d[i][pot->dim0];
          int indmin=i;
          for(j=i+1;j<gs->yi[l][k+1];j++) {
            if (xmin > d[j][pot->dim0]) {
              xmin = d[j][pot->dim0];
              indmin = j;
            }
          }
          gmx_bool temp = gs->prop1[indmin];
          gs->prop1[indmin] = gs->prop1[i];
          gs->prop1[i] = temp;
          copy_rvec(d[i],temp_x_atom);
          copy_rvec(d[indmin],d[i]);
          copy_rvec(temp_x_atom,d[indmin]);

          if ((xmin-gs->x_min) >= xlayer*gs->dx) {
            int xlayermax = min((int)floor((xmin-gs->x_min)/gs->dx),gs->nx);
            int xxxx;
            for(xxxx=xlayer+1;xxxx<=xlayermax;xxxx++)
              gs->xi[l][k][xxxx] = i;
            xlayer = xlayermax;
          }
        }
      }
      int xxx;
      for(xxx=xlayer+1;xxx<=gs->nx;xxx++)
        gs->xi[l][k][xxx] = gs->yi[l][k+1];
    }
  }

  /*
  // PRINTING INDEX for debugging
 
  fprintf(stderr,"\nZ index ");
  for(i=0;i<=gs->nz;i++) {
    fprintf(stderr," %d",gs->zi[i]);
  }
  for(i=0;i<gs->nz;i++) {
    fprintf(stderr,"\nY[%d]",i);
    for(j=0;j<=gs->ny;j++) {
      fprintf(stderr," %d",gs->yi[i][j]);
  }}
  for(i=0;i<gs->nz;i++) {
    for(j=0;j<gs->ny;j++) {
      fprintf(stderr,"\nx[%d][%d]",i,j);
      for(k=0;k<=gs->nx;k++) {
        fprintf(stderr," %d",gs->xi[i][j][k]);
  }}}
  fprintf(stderr,"\n");
  */

}

// ______________________________________________________________________________________________

void compute_surface_protein ( t_user_potential *pot,
			       gmx_mtop_t *top,
			       t_state *state,
			       t_commrec *cr // for debugging purpose
			       ) {
  
  // Computes the inner surface following the protein profile at a distance
  //  r_protein       from its C-alpha transmembrane atoms and 
  //  r_panchor       from its aromatic anchoring residues
  
  // step 1 : select the atoms to use for building the surface
  // step 2 : build surface
  
  // if surface file already exists from previous simulations, read it in step 1 instead of recalculating it
  
  
  // 1. SELECTING ATOMS
  int i,j,k,l;
  
  FILE* gnuin = fopen("surf_atoms_gnu.dat","r");
  
  if (!gnuin) { // Find the relevant atoms
    
    pot->Surf_natoms = 0; 
    pot->nindex = 0;
    pot->nindex2 = 0;
    int iii = 0;
    
    // min and max search to setup atoms grid search
    pot->sags->x_min = 1337.e42, pot->sags->x_max = -1337.e42;
    pot->sags->y_min = 1337.e42, pot->sags->y_max = -1337.e42;
    pot->sags->z_min = 1337.e42, pot->sags->z_max = -1337.e42;
    
    snew(pot->Surf_atoms,state->natoms);
    snew(pot->index,state->natoms);
    snew(pot->index2,state->natoms);
    snew(pot->sags->prop1,state->natoms);
    for(j=0;j<top->nmolblock;j++) { //enum all molecule types
      for(k=0;k<top->molblock[j].nmol;k++) { //enum all instances of each molecule type
	for(l=0;l<top->moltype[top->molblock[j].type].atoms.nr;l++) { //enum atoms
	  char** atomname=top->moltype[top->molblock[j].type].atoms.atomname[l];
	  int resind = top->moltype[top->molblock[j].type].atoms.atom[l].resind;
	  char** resname=top->moltype[top->molblock[j].type].atoms.resinfo[resind].name;
	  
	  // Is the atom in an anchor residue?
	  gmx_bool bAnchor = FALSE;
	  for(i=0;i<pot->anchor_nr;i++) if (!strcmp(pot->anchor[i],*resname)) bAnchor = TRUE;
	  
	  // Is the atom in the transmembrane region?
	  // The transmembrane region is the membrane region + r_protein in both directions
	  if( state->x[iii][pot->dim2] > pot->z_memb_b - pot->r_protein // - 0.0*(pot->z_memb_t-pot->z_memb_b)
	      && state->x[iii][pot->dim2] < pot->z_memb_t + pot->r_protein // + 0.0*(pot->z_memb_t-pot->z_memb_b) 
	      ) {
	    if(!strcmp(*atomname,"CA") || bAnchor ) {
	      copy_rvec(state->x[iii],pot->Surf_atoms[pot->Surf_natoms]);
	      // EMY: Surf_atoms contains coordinates with respect the geometric center
	      rvec_dec(pot->Surf_atoms[pot->Surf_natoms],pot->center);
	      if (pot->Surf_atoms[pot->Surf_natoms][pot->dim0]<pot->sags->x_min) pot->sags->x_min = pot->Surf_atoms[pot->Surf_natoms][pot->dim0];
	      if (pot->Surf_atoms[pot->Surf_natoms][pot->dim1]<pot->sags->y_min) pot->sags->y_min = pot->Surf_atoms[pot->Surf_natoms][pot->dim1];
	      if (pot->Surf_atoms[pot->Surf_natoms][pot->dim2]<pot->sags->z_min) pot->sags->z_min = pot->Surf_atoms[pot->Surf_natoms][pot->dim2];
	      if (pot->Surf_atoms[pot->Surf_natoms][pot->dim0]>pot->sags->x_max) pot->sags->x_max = pot->Surf_atoms[pot->Surf_natoms][pot->dim0];
	      if (pot->Surf_atoms[pot->Surf_natoms][pot->dim1]>pot->sags->y_max) pot->sags->y_max = pot->Surf_atoms[pot->Surf_natoms][pot->dim1];
	      if (pot->Surf_atoms[pot->Surf_natoms][pot->dim2]>pot->sags->z_max) pot->sags->z_max = pot->Surf_atoms[pot->Surf_natoms][pot->dim2];
	      // Differentiate anchors and regular CAs
	      pot->sags->prop1[pot->Surf_natoms] = FALSE;
	      if(strcmp(*atomname,"CA")) {
		pot->sags->prop1[pot->Surf_natoms] = TRUE;
	      }
	      pot->Surf_natoms++;
	    }	
	  }
	  // EMY_TO_CHECK: Why these two next checks are not done inside the transmembrane region check above?
	  // Store the atoms on which to apply the force
	  if(!strcmp(*atomname,"CA") || bAnchor || !strcmp(*resname,"SOL") || !strcmp(*resname,"ACQ")) {
	    pot->index[pot->nindex] = iii;
	    pot->nindex++;
	  }
	  // Store the atoms from which to compute the surface
	  if(!strcmp(*atomname,"CA") || bAnchor ) {
	    pot->index2[pot->nindex2] = iii;
	    pot->nindex2++;
	  }
	  iii++;
	}}}
    
    // Resize the arrays in order to remove all the not-used items  
    srenew(pot->index,pot->nindex);
    srenew(pot->index2,pot->nindex2);
    srenew(pot->Surf_atoms,pot->Surf_natoms);
  }
  else { // Read gnuplot file
    fprintf(stderr,"Reading surf_atoms_gnu.dat");
    char* line; snew(line,STRLEN);
    while(!feof(gnuin)) { // Counting lines
      if (fgets(line,STRLEN,gnuin)) pot->Surf_natoms++;    
    }
    pot->Surf_natoms--; // Exclude from the counting the line containing the coordinates of the geometric center
    rewind(gnuin); sfree(line);
    snew(pot->Surf_atoms,pot->Surf_natoms);
    snew(pot->index,state->natoms);
    snew(pot->index2,state->natoms);
    snew(pot->sags->prop1,pot->Surf_natoms);
    fscanf(gnuin,"%lf %lf %lf\n",&(pot->center[pot->dim0]),&(pot->center[pot->dim1]),&(pot->center[pot->dim2]));
    for (i=0;i<pot->Surf_natoms;i++) {
      int smurf;
      fscanf(gnuin,"%lf",&(pot->Surf_atoms[i][pot->dim0]));
      fscanf(gnuin,"%lf",&(pot->Surf_atoms[i][pot->dim1]));
      fscanf(gnuin,"%lf",&(pot->Surf_atoms[i][pot->dim2]));
      fscanf(gnuin,"%d",&smurf); 
      pot->sags->prop1[i]= (gmx_bool)(smurf==1);
    }
    int iii=0;
    
    pot->sags->x_min = 1337.e42, pot->sags->x_max = -1337.e42;
    pot->sags->y_min = 1337.e42, pot->sags->y_max = -1337.e42;
    pot->sags->z_min = 1337.e42, pot->sags->z_max = -1337.e42;
    
    // EMY: No transmembrane region check here because only the atoms in the transmembrane region has been stored in the surf_atoms_gnu.dat file
    for(j=0;j<pot->Surf_natoms;j++) {
      if (pot->Surf_atoms[j][pot->dim0] < pot->sags->x_min) pot->sags->x_min = pot->Surf_atoms[j][pot->dim0];
      if (pot->Surf_atoms[j][pot->dim1] < pot->sags->y_min) pot->sags->y_min = pot->Surf_atoms[j][pot->dim1];
      if (pot->Surf_atoms[j][pot->dim2] < pot->sags->z_min) pot->sags->z_min = pot->Surf_atoms[j][pot->dim2];
      if (pot->Surf_atoms[j][pot->dim0] > pot->sags->x_max) pot->sags->x_max = pot->Surf_atoms[j][pot->dim0];
      if (pot->Surf_atoms[j][pot->dim1] > pot->sags->y_max) pot->sags->y_max = pot->Surf_atoms[j][pot->dim1];
      if (pot->Surf_atoms[j][pot->dim2] > pot->sags->z_max) pot->sags->z_max = pot->Surf_atoms[j][pot->dim2];
    }
    
    for(j=0;j<top->nmolblock;j++) {//enum all molecule types
      for(k=0;k<top->molblock[j].nmol;k++) { //enum all instances of each molecule type
	for(l=0;l<top->moltype[ top->molblock[j].type].atoms.nr;l++) { //enum atoms
	  char** atomname = top->moltype[top->molblock[j].type].atoms.atomname[l];
	  int resind = top->moltype[top->molblock[j].type].atoms.atom[l].resind;
	  char** resname = top->moltype[top->molblock[j].type].atoms.resinfo[resind].name;
	  
	  // EMY_CHANGE: Introduced the same logic for the indices as the logic used for the case without the surf_atoms_gnu.dat file 
	  // Is the atom in an anchor residue?
	  gmx_bool bAnchor = FALSE;
	  for(i=0;i<pot->anchor_nr;i++) if (!strcmp(pot->anchor[i],*resname)) bAnchor = TRUE;
	  
	  if(!strcmp(*atomname,"CA") || bAnchor || !strcmp(*resname,"SOL") || !strcmp(*resname,"ACQ")) {
	    pot->index[pot->nindex] = iii;
	    pot->nindex++;
	  }
	  if(!strcmp(*atomname,"CA") || bAnchor ) {
	    pot->index2[pot->nindex2] = iii;
	    pot->nindex2++;
	  }
	  iii++;
	}}}
    fprintf(stderr," done\n");
    srenew(pot->index,pot->nindex);
    srenew(pot->index2,pot->nindex2);
    
  }
  
  //if (pot->Surf_natoms > 40) pot->bAtoms_GS = TRUE; FIXME Atoms grid search
  // grid search index for Surf_atoms
  //fprintf(stderr,"Adding some virtual atoms\n");
  //add_virtual_atoms(pot);
  
  /*if (pot->bAtoms_GS) {
    fprintf(stderr,"Preparing atoms grid search index\n");
    pot->sags->dz = max(pot->smoothen,3.)*pot->r_protein; 
    pot->sags->dy = max(pot->smoothen,3.)*pot->r_protein; 
    pot->sags->dx = max(pot->smoothen,3.)*pot->r_protein; 
    
    build_index(pot,pot->sags,pot->Surf_natoms,pot->Surf_atoms);
    }*/
  
  FILE *grofile = fopen("surf_atoms.gro","w");
  write_gro_points_detailed(grofile,pot->Surf_natoms,pot->Surf_atoms,pot->sags->prop1,pot->center,"ATOMS INVOLVED IN THE EMBEDDING SURFACE");
  fclose(grofile);
  
  
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
  
  // 2. BUILD SURFACE

  /*
    Build spheres of points around the selected atoms
    Compute level-set function for each point to eliminate the ones inside
    If they are located in the transmembrane region + some extra distance (to keep the protein in its cavity), then the points are selected
  */
  
  char filnm[] = "surf_gnu.dat";
  FILE* gnuin2 = NULL;
  gnuin2 = fopen(filnm,"r");
  // EMY: extradist is ~ pot->r_protein if smoothen=3
  real extradist = min(fabs(pot->sags->z_max+pot->center[pot->dim2]+0.33*pot->r_protein*pot->smoothen-pot->z_memb_t),
                       fabs(pot->sags->z_min+pot->center[pot->dim2]-0.33*pot->r_protein*pot->smoothen+pot->z_memb_b));

  // Create the file surf_gnu.dat

  if (!gnuin2) {
    
    fprintf(stderr,"Monitoring %d atoms...\n",pot->Surf_natoms);
    fprintf(stderr,"0        50       100\n");    
    fprintf(stderr,"|----|----|----|----|\n");
    
    // Distribute almost evenly spread points on the spheres around each selected atom
    // Use the Saff and Kuijlaars method (http://web.archive.org/web/20120421191837/http://www.cgafaq.info/wiki/Evenly_distributed_points_on_sphere#Spirals) to do that:
    // if z is the normal to the membrane, spiral in the xy plane + translation in the z direction
    // The number of rounds is such that the points distribution resembles an equilateral triangles pattern
    int npoints = max(200,floor(pow(3*pot->smoothen*pot->r_protein,2)*150));      // EMY_CHECK: What is the rationale for this?
    int rounds = (int)floor(sqrt(npoints*M_PI*sqrt(3.)/16.))+1;
    pot->np_Surf = 0;
    rvec *surf_temp;
    gmx_bool *selected_points, *tprop;
    snew(surf_temp,npoints*pot->Surf_natoms);
    snew(tprop,npoints*pot->Surf_natoms);
    snew(selected_points,npoints*pot->Surf_natoms);
    // EMY_CHECK: This is not the Saff and Kuijlaars method!
    // EMY_CHECK: Make it more computational efficient: dtheta=rounds*2.*M_PI/(real)npoints;
    real dtheta = rounds*2.*4.*atan(1.)/(real)npoints;
    gmx_bool bLoad = FALSE;
    rvec xt;
    for (i=0;i<pot->Surf_natoms;i++) { // Loop over the selected atoms

      // display with the stars the progresses
      if (!(((int)floor(i/(real)pot->Surf_natoms*100)+1)%5==0)) bLoad = TRUE;  // EMY_CHECK: What is used for?
      else if (bLoad) {
        fprintf(stderr,"*");
        bLoad = FALSE;
      }

      for(j=0;j<npoints;j++) {
        selected_points[i*npoints+j] = FALSE;
	// EMY: Start building points in a sphere of radius 1 centered on the origin
	// EMY: The z-coordinate of xt goes from 1 (j=0) to -1+2/npoints (j=npoints-1)
	// EMY_CHECK: It would be more uniform to start from 1-1/npoints at j=0 so that for j=npoints-1 z-coordinate of xt is -1+1/npoints
        xt[pot->dim2] = 1-2*j/(real)npoints;
	// EMY: |xt| = 1
        real rrr = sqrt(1.-xt[pot->dim2]*xt[pot->dim2]);
	// EMY: The polar angle of xt goes from 0 (j=0) to 2\pi - 2\pi/npoints (j=npoints-1)
        xt[pot->dim0] = rrr*cos(j*dtheta);
        xt[pot->dim1] = rrr*sin(j*dtheta);
        if (!(pot->sags->prop1[i])) {// If atom is CA then radius of the sphere = r_protein
          svmul(pot->r_protein,xt,xt);
          tprop[i*npoints+j] = FALSE;
        } else {                    // If atom belongs to an anchor residue then radius of the sphere = r_panchor
          svmul(pot->r_panchor,xt,xt);
          tprop[i*npoints+j] = TRUE;
        }
        svmul(pot->smoothen,xt,xt); // Smoothening strategy
	// EMY: Move the points around the real selected atom
        rvec_inc(xt,pot->Surf_atoms[i]);
        copy_rvec(xt,surf_temp[i*npoints+j]);
        // Finding the points that will form the surface
        if (fabs(phi(pot,&(surf_temp[i*npoints+j]))) <= 0.000001
	    && xt[pot->dim2]<pot->z_memb_t-pot->center[pot->dim2]+extradist
	    && xt[pot->dim2]>pot->z_memb_b-pot->center[pot->dim2]-extradist
  	 ) {
  	  pot->np_Surf++; selected_points[i*npoints+j] = TRUE;
        }
      }
    }
    fprintf(stderr,"\n");
    snew(pot->Surf,pot->np_Surf);
    snew(pot->sgs->prop1,pot->np_Surf);
    if (pot->np_Surf==0) {
      fprintf(stderr,"No surface points detected. Are the membrane coordinates correct?\n");
    }
    k=0;
    for (i=0;i<pot->Surf_natoms;i++) {
      for (j=0;j<npoints;j++) {
	if(selected_points[i*npoints+j]) {
	  copy_rvec(surf_temp[i*npoints+j],pot->Surf[k]);
	  pot->sgs->prop1[k] = tprop[i*npoints+j]; 
	  k++;
	}
      }}
    sfree(surf_temp);
    sfree(selected_points);
    sfree(tprop);
  } // End of the procedure to find the points forming the embedding surface

// Read the file surf_gnu.dat
  
  else {
    fprintf(stderr,"Reading surf_gnu.dat");
    char* line; snew(line,STRLEN+1);
    while(!feof(gnuin2)) { // Counting lines
      if (fgets(line,STRLEN+1,gnuin2)) pot->np_Surf++;    
    }
    pot->np_Surf--; // Exclude from the counting the line containing the coordinates of the geometric center
    rewind(gnuin2); sfree(line);
    snew(pot->Surf,pot->np_Surf);
    snew(pot->sgs->prop1,pot->np_Surf);
    fscanf(gnuin2,"%lf %lf %lf\n",&(pot->center[pot->dim0]),&(pot->center[pot->dim1]),&(pot->center[pot->dim2]));
    for (i=0;i<pot->np_Surf;i++) {
      int smurf;
      fscanf(gnuin2,"%lf",&(pot->Surf[i][pot->dim0]));
      fscanf(gnuin2,"%lf",&(pot->Surf[i][pot->dim1]));
      fscanf(gnuin2,"%lf",&(pot->Surf[i][pot->dim2]));
      fscanf(gnuin2,"%d",&smurf); 
      pot->sgs->prop1[i]= (gmx_bool)(smurf==1);
    }
    
    fprintf(stderr," done\n");
  }
  
  // Compute the "radius" of the surface
  // This will prevent calls to phi() (computational expensive) for atoms out of a ring between Surf_radius_int and Surf_radius
  real rrr; pot->Surf_radius=0.;pot->Surf_radius_int=9999.; 
  // We need the max and min of x- and y-coordinates of the points forming the embedding surface as well 
  pot->sgs->x_min = pot->Surf[0][pot->dim0];
  pot->sgs->x_max = pot->sgs->x_min;
  pot->sgs->y_min = pot->Surf[0][pot->dim1];
  pot->sgs->y_max = pot->sgs->y_min;
  for (i=0;i<pot->np_Surf;i++) {
    //EMY_CHANGE: Improved the computational efficiency
    //rrr = sqrt(pow(pot->Surf[i][pot->dim0],2)+pow(pot->Surf[i][pot->dim1],2));
    rrr = sqrt(pot->Surf[i][pot->dim0]*pot->Surf[i][pot->dim0] + pot->Surf[i][pot->dim1]*pot->Surf[i][pot->dim1]);
    if (rrr > pot->Surf_radius) pot->Surf_radius=rrr;
    if (rrr < pot->Surf_radius_int) pot->Surf_radius_int = rrr;
    if (pot->sgs->x_min>pot->Surf[i][pot->dim0]) pot->sgs->x_min=pot->Surf[i][pot->dim0];
    if (pot->sgs->x_max<pot->Surf[i][pot->dim0]) pot->sgs->x_max=pot->Surf[i][pot->dim0];
    if (pot->sgs->y_min>pot->Surf[i][pot->dim1]) pot->sgs->y_min=pot->Surf[i][pot->dim1];
    if (pot->sgs->y_max<pot->Surf[i][pot->dim1]) pot->sgs->y_max=pot->Surf[i][pot->dim1];
  }
  pot->sgs->z_min = pot->z_memb_b-pot->center[pot->dim2]-extradist;
  pot->sgs->z_max = pot->z_memb_t-pot->center[pot->dim2]+extradist;
  
  fprintf(stderr,"Surf_radius=%f\n",pot->Surf_radius);
  fprintf(stderr,"Surf_radius_int=%f\n",pot->Surf_radius_int);
  // exit(0);
  
  // Indexing surface points for a grid search
  fprintf(stderr,"Preparing surface points grid search index... \n");
  // EMY_CHECK: Where do those definitions come from? 
  pot->sgs->dz = max(pot->rcut,pot->smoothen*pot->vdw->rmax);
  pot->sgs->dy = max(pot->rcut,pot->r_protein) + (pot->smoothen-1)*pot->r_protein;
  pot->sgs->dx = max(pot->rcut,pot->r_protein) + (pot->smoothen-1)*pot->r_protein;
  
  build_index(pot,pot->sgs,pot->np_Surf,pot->Surf);
  
  if (!gnuin) {write_gnu_files(pot);}
  if (gnuin2) {fclose(gnuin2);}
  if (gnuin) {fclose(gnuin);}
  
}


// -------- distance and force calculation ------------------------------------------------


// types 1 and 3 
real compute_force_hsphere ( t_user_potential *pot,
			     rvec *x,
			     rvec *f,
			     real zp, // plane altitude (zp_t/b) with respect the geometric center (pot->center)
			     real r // sphere radius centered on the plane (r_t/b)
			   ) {
  
  real x0,x1,x2;
  //EMY: Coordinates of the atom with respect to the geometric center of the system (pot->center)
  //EMY: The origin of the hemisphere is not pot->center but its z coordinate corresponds to zp_t, which in turn is defined with respect the geometric center
  
  x0 = (*x)[pot->dim0]-pot->center[pot->dim0];
  x1 = (*x)[pot->dim1]-pot->center[pot->dim1];
  x2 = (*x)[pot->dim2]-pot->center[pot->dim2];
  
  //                  _______________________________
  //                 /                              /
  //   A        /   /        __.---.__             /
  //   |       /   /       _/         \_          /
  //   |      /   /       /             \        /
  //   |   yc- . /. . .  |. .  . +       |      /
  //   |    /   /        `----_______----'     /
  //   |   /   /               .              /
  //   |  /   /               .              /
  // zp| /   /______________________________/
  //   |/                   . 
  //   |--sym plane --------|---------------->
  //   |                   x_c
  
  // the points which require no calculations: 
  // under the plane (out of cutoff)
  // and above the hemisphere
  
  if ( x2 < zp - pot->rcut ) {
    clear_rvec(*f);
    return -0.5; // UNDER the plane (we're always out of the sphere)
  }
  
  if ( x2 > zp + r ) {
    clear_rvec(*f);
    return -1.; // ABOVE the hemisphere
  }
  
  // Computing the polar radius
  real radius = sqrt( x0*x0 + x1*x1 ); // Radius projected on the xy plane (distance from Z axis)
  
  // Above the plane, outside of cilinder containing the hemisphere the hemisphere	
  if ( x2 > zp && radius >= r )  {
    clear_rvec(*f);
    return -1.; // Outside the hemisphere
  }
  
  // Just under the plane
  if ( x2 >= zp - pot->rcut && radius >= r ) {
    (*f)[pot->dim0] = 0.;
    (*f)[pot->dim1] = 0.;
    (*f)[pot->dim2] = -user_force(pot,zp-x2);
    return zp - x2; // Distance from the membrane plane
  }
  
  // Inside the cilinder containing the hemisphere
  if ( x2 > zp ) { // no longer need to test the radius
    real rho = sqrt( x0*x0 + x1*x1 + (x2-zp)*(x2-zp) ); // rho = real 3d-radius centered on the plane (and not on the origin of the geometric center)
    if ( rho > r ) { // Outside the hemisphere
      clear_rvec(*f);
      return -1.;
    }
    else if ( rho < r-pot->rcut ) { // Inside the hemishere and over the cutoff (too much inside)
      clear_rvec(*f);
      return -0.5;
    }
    else {
      real dist = r - rho;
      real intens = user_force(pot,dist);
      (*f)[pot->dim0] = -intens*(x0/rho);
      (*f)[pot->dim1] = -intens*(x1/rho);
      (*f)[pot->dim2] = -intens*((x2-zp)/rho);
      return dist;
    }
  }
  
  // Only the inner cylinder left...
  // EMY_CHANGE: Logic simplified: removed useless "if" check. At this level radius < r is always true. 
  //  if ( radius < r ) { // SANDAL: x2 < zp is implicit (we already checked all cases where x2 >= zp)
  // if ( radius < r - pot->rcut) {
  if ( radius < r - pot->rcut) { // SANDAL: x2 < zp is implicit (we already checked all cases where x2 >= zp)
    clear_rvec(*f);
    return -0.5;
  }
  //    else {
  real dist = sqrt(pow(r-radius,2) + pow(x2-zp,2)); // EMY: real radius centered in the kink; in 3D this is the radius of the tube that forms the torus, and the center of the tube corresponds to the kink in the 2D representation of the picture at pag. 2 of doc_MMCG_Wpot.pdf document
  // Dist is the distance between the point x and the nearest point of intersection between the hemisphere and the membrane plane.
  if (dist > pot->rcut) {
    clear_rvec(*f);
    return -0.5;
  }
  else {
    real intens = user_force(pot,dist);
    (*f)[pot->dim0] = -intens*x0*(r-radius)/(radius*dist);
    (*f)[pot->dim1] = -intens*x1*(r-radius)/(radius*dist);
    // EMY_CHANGE: This formula is not minus the derivative with respect x2 of the potential in user_force
    //        (*f)[pot->dim2] = -intens*(r-radius)/dist;
    // Corrected formula:
    (*f)[pot->dim2] = intens*(x2-zp)/dist;
    return dist;
  }
  return -1.;
}


// used for the upper plane, when there is not the upper hemisphere

real compute_force_hsphere_H_upper ( t_user_potential *pot,
                             rvec *x,
                             rvec *f,
                             real zp, // plane altitude (zp_t/b) with respect the geometric center (pot->center)
                             real r // sphere radius centered on the plane (r_t/b)
                           ) {

  real x0,x1,x2;
  //EMY: Coordinates of the atom with respect to the geometric center of the system (pot->center)
  //EMY: The origin of the hemisphere is not pot->center but its z coordinate corresponds to zp_t, which in turn is defined with respect the geometric center

  x0 = (*x)[pot->dim0]-pot->center[pot->dim0];
  x1 = (*x)[pot->dim1]-pot->center[pot->dim1];
  x2 = (*x)[pot->dim2]-pot->center[pot->dim2];

  //                  _______________________________
  //                 /                              /
  //   A        /   /        __.---.__             /
  //   |       /   /       _/         \_          /
  //   |      /   /       /             \        /
  //   |   yc- . /. . .  |. .  . +       |      /
  //   |    /   /        `----_______----'     /
  //   |   /   /               .              /
  //   |  /   /               .              /
  // zp| /   /______________________________/
  //   |/                   .
  //   |--sym plane --------|---------------->
  //   |                   x_c

  // the points which require no calculations:
  // under the plane (out of cutoff)
  // and above the hemisphere

  if ( x2 < zp - pot->rcut ) {
    clear_rvec(*f);
    return -0.5; // UNDER the plane (we're always out of the sphere)
  }

/*  if ( x2 > zp + r ) {
    clear_rvec(*f);
    return -1.; // ABOVE the hemisphere
  }
  */

  // Computing the polar radius
  real radius = sqrt( x0*x0 + x1*x1 ); // Radius projected on the xy plane (distance from Z axis)

  // Above the plane, outside of cilinder containing the hemisphere the hemisphere
  if ( x2 > zp && radius >= r )  {
    clear_rvec(*f);
    return -1.; // Outside the hemisphere
  }

  // Just under the plane
  if ( x2 >= zp - pot->rcut && radius >= r ) {
    (*f)[pot->dim0] = 0.;
    (*f)[pot->dim1] = 0.;
    (*f)[pot->dim2] = -user_force(pot,zp-x2);
    return zp - x2; // Distance from the membrane plane
  }

  /*
  // Inside the cilinder containing the hemisphere
  if ( x2 > zp ) { // no longer need to test the radius
    real rho = sqrt( x0*x0 + x1*x1 + (x2-zp)*(x2-zp) ); // rho = real 3d-radius centered on the plane (and not on the origin of the geometric center)
    if ( rho > r ) { // Outside the hemisphere
      clear_rvec(*f);
      return -1.;
    }
    else if ( rho < r-pot->rcut ) { // Inside the hemishere and over the cutoff (too much inside)
      clear_rvec(*f);
      return -0.5;
    }
    else {
      real dist = r - rho;
      real intens = user_force(pot,dist);
      (*f)[pot->dim0] = -intens*(x0/rho);
      (*f)[pot->dim1] = -intens*(x1/rho);
      (*f)[pot->dim2] = -intens*((x2-zp)/rho);
      return dist;
    }
  }

  // Only the inner cylinder left...
  // EMY_CHANGE: Logic simplified: removed useless "if" check. At this level radius < r is always true.
  //  if ( radius < r ) { // SANDAL: x2 < zp is implicit (we already checked all cases where x2 >= zp)
  // if ( radius < r - pot->rcut) {
  if ( radius < r - pot->rcut) { // SANDAL: x2 < zp is implicit (we already checked all cases where x2 >= zp)
    clear_rvec(*f);
    return -0.5;
  }
  //    else {
  real dist = sqrt(pow(r-radius,2) + pow(x2-zp,2)); // EMY: real radius centered in the kink; in 3D this is the radius of the tube that forms the torus, and the center of the tube corresponds to the kink in the 2D representation of the picture at pag. 2 of doc_MMCG_Wpot.pdf document
  // Dist is the distance between the point x and the nearest point of intersection between the hemisphere and the membrane plane.
  if (dist > pot->rcut) {
    clear_rvec(*f);
    return -0.5;
  }
  else {
    real intens = user_force(pot,dist);
    (*f)[pot->dim0] = -intens*x0*(r-radius)/(radius*dist);
    (*f)[pot->dim1] = -intens*x1*(r-radius)/(radius*dist);
    // EMY_CHANGE: This formula is not minus the derivative with respect x2 of the potential in user_force
    //        (*f)[pot->dim2] = -intens*(r-radius)/dist;
    // Corrected formula:
    (*f)[pot->dim2] = intens*(x2-zp)/dist;
    return dist;
  }
  */

  return -1.;
}


// used for the lower plane and hemisphere, when H-AdResS is used


real compute_force_hsphere_H_lower ( t_user_potential *pot,
                             rvec *x,
                             rvec *f,
                             real zp, // plane altitude (zp_t/b) with respect the geometric center (pot->center)
                             real r // sphere radius centered on the plane (r_t/b)
                           ) {

  real x0,x1,x2;
  //EMY: Coordinates of the atom with respect to the geometric center of the system (pot->center)
  //EMY: The origin of the hemisphere is not pot->center but its z coordinate corresponds to zp_t, which in turn is defined with respect the geometric center

  x0 = (*x)[pot->dim0]-pot->center[pot->dim0];
  x1 = (*x)[pot->dim1]-pot->center[pot->dim1];
  x2 = (*x)[pot->dim2]-pot->center[pot->dim2];

  //                  _______________________________
  //                 /                              /
  //   A        /   /        __.---.__             /
  //   |       /   /       _/         \_          /
  //   |      /   /       /             \        /
  //   |   yc- . /. . .  |. .  . +       |      /
  //   |    /   /        `----_______----'     /
  //   |   /   /               .              /
  //   |  /   /               .              /
  // zp| /   /______________________________/
  //   |/                   .
  //   |--sym plane --------|---------------->
  //   |                   x_c

  // the points which require no calculations:
  // under the plane (out of cutoff)
  // and above the hemisphere

  if ( x2 < zp - pot->rcut ) {
    clear_rvec(*f);
    return -0.5; // UNDER the plane (we're always out of the sphere)
  }

/*  if ( x2 > zp + r ) {
    clear_rvec(*f);
    return -1.; // ABOVE the hemisphere
  }
  */

  // Computing the polar radius
  real radius = sqrt( x0*x0 + x1*x1 ); // Radius projected on the xy plane (distance from Z axis)

  // Above the plane, outside of cilinder containing the hemisphere the hemisphere
  if ( x2 > zp && radius >= r )  {
    clear_rvec(*f);
    return -1.; // Outside the hemisphere
  }

  // Just under the plane
  if ( x2 >= zp - pot->rcut && radius >= r ) {
    (*f)[pot->dim0] = 0.;
    (*f)[pot->dim1] = 0.;
    (*f)[pot->dim2] = -user_force(pot,zp-x2);
    return zp - x2; // Distance from the membrane plane
  }


  // Inside the cilinder containing the hemisphere
  if ( x2 > zp ) { // no longer need to test the radius
    real rho = sqrt( x0*x0 + x1*x1 + (x2-zp)*(x2-zp) ); // rho = real 3d-radius centered on the plane (and not on the origin of the geometric center)
    if ( rho > r+pot->rcut) { // Outside the hemisphere, outside the cutoff
      clear_rvec(*f);
      return -1.;
    }


    else if ( rho > r && rho < r+pot->rcut) { // Outside the hemisphere, within the cutoff
        real dist = r - rho;
        real intens = user_force(pot,dist);
        (*f)[pot->dim0] = intens*(x0/rho);
        (*f)[pot->dim1] = intens*(x1/rho);
        (*f)[pot->dim2] = intens*((x2-zp)/rho);
        return dist;
    }



    else if ( rho < r-pot->rcut ) { // Inside the hemishere and over the cutoff (too much inside)
      clear_rvec(*f);
      return -0.5;
    }
    else { // inside the hemisphere, within the cutoff
      real dist = r - rho;
      real intens = user_force(pot,dist);
      (*f)[pot->dim0] = -intens*(x0/rho);
      (*f)[pot->dim1] = -intens*(x1/rho);
      (*f)[pot->dim2] = -intens*((x2-zp)/rho);
      return dist;
    }
  }

  // Only the inner cylinder left...
  // EMY_CHANGE: Logic simplified: removed useless "if" check. At this level radius < r is always true.
  //  if ( radius < r ) { // SANDAL: x2 < zp is implicit (we already checked all cases where x2 >= zp)
  // if ( radius < r - pot->rcut) {
  if ( radius < r - pot->rcut) { // SANDAL: x2 < zp is implicit (we already checked all cases where x2 >= zp)
    clear_rvec(*f);
    return -0.5;
  }
  //    else {
  real dist = sqrt(pow(r-radius,2) + pow(x2-zp,2)); // EMY: real radius centered in the kink; in 3D this is the radius of the tube that forms the torus, and the center of the tube corresponds to the kink in the 2D representation of the picture at pag. 2 of doc_MMCG_Wpot.pdf document
  // Dist is the distance between the point x and the nearest point of intersection between the hemisphere and the membrane plane.
  if (dist > pot->rcut) {
    clear_rvec(*f);
    return -0.5;
  }
  else {
    real intens = user_force(pot,dist);
    (*f)[pot->dim0] = -intens*x0*(r-radius)/(radius*dist);
    (*f)[pot->dim1] = -intens*x1*(r-radius)/(radius*dist);
    // EMY_CHANGE: This formula is not minus the derivative with respect x2 of the potential in user_force
    //        (*f)[pot->dim2] = -intens*(r-radius)/dist;
    // Corrected formula:
    (*f)[pot->dim2] = intens*(x2-zp)/dist;
    return dist;
  }


  return -1.;
}







real compute_force_membrane ( t_user_potential *pot,
			      rvec *x,
		     	      rvec *f,
			      rvec f_init
			      ) {
  //
  //    Irregular surface embedding a protein
  //    described by the set of points in pot->Surf
  //    See compute_surface_protein  
  // 
  //            __________________________
  //           /        .----.           /
  //          /     ___/     |          /
  //         /     /        /          /
  //        /     |__       |         /___
  //       /         \______/        /   /
  //      /                         /   /
  //     /_________________________/   /
  // A      /     |_     /            /
  // |zm   /        \   /            /
  // |    /         |__/            /
  // V   /_________________________/
  //
  //
  
  // x position with respect to the geometric center
  rvec *xr;
  snew(xr,1);
  (*xr)[pot->dim0] = (*x)[pot->dim0]-pot->center[pot->dim0];
  (*xr)[pot->dim1] = (*x)[pot->dim1]-pot->center[pot->dim1];
  (*xr)[pot->dim2] = (*x)[pot->dim2]-pot->center[pot->dim2];
  
  // Vertical position of the membrane with respect to the geometric center
  real z_t = pot->z_memb_t-pot->center[pot->dim2];
  real z_b = pot->z_memb_b-pot->center[pot->dim2];
  
  // EMY: In the following, top and bottom are treated differently: this can be due to the fact that we want the water molecules only in the upper part (extracellular side). The water is faster than the other atoms and the walls should interact smoother with them in order not to have too fast bouncing molecules
  // Zone to exclude: too far from the membrane
  // EMY_CHANGE: Replaced pot->vdw->rmax with pot->rcut since the interaction is 1/r-like
  // EMY_CHECK: Should be this interaction replaced by the repulsive part of the vdw in order to have a coherent repulsion along the membrane?
  if ((*xr)[pot->dim2]>z_t+pot->rcut || (*xr)[pot->dim2]<z_b-pot->r_protein-0.5) {sfree(xr); (*f)[pot->dim0]=0.;(*f)[pot->dim1]=0.;(*f)[pot->dim2]=0.; return -0.5;}
  // Zone to exclude: inside the membrane, out of the radius
  real radius = sqrt((*xr)[pot->dim0]*(*xr)[pot->dim0]+(*xr)[pot->dim1]*(*xr)[pot->dim1]);
  if ((*xr)[pot->dim2]>z_b && (*xr)[pot->dim2]<z_t && radius>pot->Surf_radius) {sfree(xr); (*f)[pot->dim0]=0.;(*f)[pot->dim1]=0.;(*f)[pot->dim2]=0.; return -1.;}
  // Above the membrane out of the radius
  real dist;
  // EMY_CHANGE: Replaced pot->vdw->rmax with pot->rcut since the interaction is 1/r-like
  // EMY_CHECK: The condition (*xr)[pot->dim2]<z_t+pot->rcut is useless because it will be always verified if the code reaches this point.
  if ((*xr)[pot->dim2]>z_t && (*xr)[pot->dim2]<z_t+pot->rcut && radius>pot->Surf_radius) {
    dist = (*xr)[pot->dim2]-z_t;
    (*f)[pot->dim0] = 0.;
    (*f)[pot->dim1] = 0.;
    (*f)[pot->dim2] = user_force(pot,dist);
    sfree(xr);
    return dist;
  }
  // Under the membrane out of the radius
  // EMY_CHECK: The condition (*xr)[pot->dim2]>z_b-pot->r_protein-0.5 is useless because it will be always verified if the code this point.
  if ((*xr)[pot->dim2]<z_b && (*xr)[pot->dim2]>z_b-pot->r_protein-0.5 && radius>pot->Surf_radius) {
    dist = -(*xr)[pot->dim2]+z_b;
    (*f)[pot->dim0] = 0.;
    (*f)[pot->dim1] = 0.;
    (*f)[pot->dim2] = -user_force(pot,dist);
    sfree(xr);
    return dist;
  }
  
  int i,j,k;
  int ii,iii,iiii;
  int xbegin,xend,ybegin,yend,zbegin,zend;
  
  // Above the cavity:
  //       - inside the embedding surface + (smoothen - 1(*r_protein zone: no repulsive forces
  //       - outside the embedding surface: repulsive vdw-like in vertical direction
  //   Distance from the membrane plane used to calculate the repulsive forces
  
  // EMY: The vdw interaction acts on the protein atoms close to the membrane stabilizing them. Moreover, it acts on the entering water molecules, attracting them and possibly avoiding that they cross the membrane
  // EMY_CHANGES: Replaced pot->vdw->rmax with pot->rcut since the interaction is 1/r-like
  //              Removed useless condition
  //  if ((*xr)[pot->dim2]>=z_t && (*xr)[pot->dim2]<z_t+pot->rcut) {  //0.1*(z_t-z_b) && radius<=pot->Surf_radius) {
  if ((*xr)[pot->dim2]>=z_t) {
    real distmin=1337.e42; int indmin=0;
    
    find_gs_cell(xr,pot,pot->sgs,&xbegin,&xend,&ybegin,&yend,&zbegin,&zend);
    
    for(ii=ybegin;ii<yend;ii++) {
      for(iii=xbegin;iii<xend;iii++) {
        int xxbegin = pot->sgs->xi[pot->sgs->nz-1][ii][iii]; // from the indexes of the cell, get the index of the surface points from where to start (and to end) the search
        int xxend   = pot->sgs->xi[pot->sgs->nz-1][ii][iii+1];
        if (xxbegin==pot->np_Surf) xxbegin--;
        if (xxend==pot->np_Surf) xxend--;
	for (iiii=xxbegin;iiii<xxend;iiii++) {
	  // EMY_CHECK: distance2 gives the distance in 3D space and not the projection over the xy plane at z~=z_t+extradist
	  // Possible change: dist=sqrt(distance2(*xr,pot->Surf[iiii]) - xr[pot->dim2]*xr[pot->dim2]);
          dist=sqrt(distance2(*xr,pot->Surf[iiii]));
          if(distmin>dist) {distmin=dist;indmin=iiii;}
	}
      }}
    
    // EMY_CHECK: As it is, the force implemented in this block acts only on an interval (+/-(smoothen-1)*r_protein) around the top part of the embedding surface. So, if the mouth of the protein is narrower than the rest, it is a gap [surface+(smoothen-1)*r_protein,Surf_radius] without any repulsive interaction!
    if (distmin<pot->r_protein*(pot->smoothen-1.)) {
      (*f)[pot->dim0] = 0.;
      (*f)[pot->dim1] = 0.;
      // EMY_CHECK: replaced "pot->vdw->rmax" with "pot->r_protein" in order to have only a repulsive force that goes to zero at the border z_t + pot->r_protein
      if ((*xr)[pot->dim2]-z_t < pot->vdw->rmax) {(*f)[pot->dim2] = user_force_vdw(pot,(*xr)[pot->dim2]-z_t);}
      // if ((*xr)[pot->dim2]-z_t < pot->r_protein) {(*f)[pot->dim2] = user_force_vdw(pot,(*xr)[pot->dim2]-z_t)}
      else {(*f)[pot->dim2] = 0.;}
      real toto=(*xr)[pot->dim2];
      sfree(xr);
      // EMY_CHECK
      if (toto-z_t < pot->vdw->rmax) return toto-z_t;
      //if (toto-z_t < pot->r_protein) return toto-z_t;
      return -0.5;
    }
    
    real phii; 
    if (radius < pot->Surf_radius_int) phii = 1.;  
    else {
      phii = phi(pot,xr);
    }
    
    if (phii<0) { // outside the extended (smoothen) embedding surface
      (*f)[pot->dim0] = 0.;
      (*f)[pot->dim1] = 0.;
      // EMY_CHECK
      if ((*xr)[pot->dim2]-z_t < pot->vdw->rmax) {(*f)[pot->dim2] = user_force_vdw(pot,(*xr)[pot->dim2]-z_t);}
      //if ((*xr)[pot->dim2]-z_t < pot->r_protein) {(*f)[pot->dim2] = user_force_vdw(pot,(*xr)[pot->dim2]-z_t)}
      else {(*f)[pot->dim2] = 0.;}
      real toto=(*xr)[pot->dim2];
      sfree(xr);
      // EMY_CHECK
      if (toto-z_t < pot->vdw->rmax) return toto-z_t;
      //if (toto-z_t < pot->r_protein) return toto-z_t;
      return -0.5;
    } 
    else { // inside the extended (smoothen) embedding surface
      sfree(xr); (*f)[pot->dim0]=0.;(*f)[pot->dim1]=0.;(*f)[pot->dim2]=0.; return -0.5;
      
      /*	real toofar;
		toofar = pot->smoothen*pot->vdw->rmax;
		if (distmin>toofar) { // too far
		sfree(xr);
		(*f)[pot->dim0]=0.;(*f)[pot->dim1]=0.;(*f)[pot->dim2]=0.; return -0.5;
		}
		
		rvec *direction; snew(direction,1);
		rvec_sub(*xr,pot->Surf[indmin],*direction);
		unitv(*direction,*direction);
		real tmp;
		if (pot->sgs->prop1[indmin]) {tmp = pot->r_protein; pot->r_protein = pot->r_panchor;}
		svmul(user_force_vdw(pot,distmin-(pot->smoothen-1)*pot->r_protein),*direction,(*f));
		sfree(direction);sfree(xr);
		if (pot->sgs->prop1[indmin]) {pot->r_protein = tmp;}
		return distmin-(pot->smoothen-1)*pot->r_protein;
      */    }
  }
  
  // Under the cavity
  // EMY_CHECK: To Remove useless condition:
  //  if ((*xr)[pot->dim2]<=z_b {
  if ((*xr)[pot->dim2]<=z_b && (*xr)[pot->dim2]>z_b-pot->r_protein-0.5){ //0.1*(z_t-z_b) && radius<=pot->Surf_radius) {
    real distmin=1337.e42; int indmin=0;
    
    find_gs_cell(xr,pot,pot->sgs,&xbegin,&xend,&ybegin,&yend,&zbegin,&zend);
    
    for(ii=ybegin;ii<yend;ii++) {
      for(iii=xbegin;iii<xend;iii++) {
        int xxbegin = pot->sgs->xi[0][ii][iii];
        int xxend   = pot->sgs->xi[0][ii][iii+1];
        if (xxbegin==pot->np_Surf) xxbegin--;
        if (xxend==pot->np_Surf) xxend--;
	for (iiii=xxbegin;iiii<xxend;iiii++) {
          dist=sqrt(distance2(*xr,pot->Surf[iiii]));
          if(distmin>dist) {distmin=dist;indmin=iiii;}
	}
      }}
    
    if (distmin<pot->r_protein*(pot->smoothen-1.)) {
      (*f)[pot->dim0] = 0.;
      (*f)[pot->dim1] = 0.;
      // EMY_CHECK: To replace "pot->vdw->rmax" with "pot->r_protein"
      if (z_b-(*xr)[pot->dim2] < pot->vdw->rmax) {(*f)[pot->dim2] = -user_force_vdw(pot,z_b-(*xr)[pot->dim2]);}
      else {(*f)[pot->dim2] = 0.;}
      real toto=(*xr)[pot->dim2];
      sfree(xr);
      // EMY_CHECK: To replace "pot->vdw->rmax" with "pot->r_protein"
      if (z_b-toto < pot->vdw->rmax) return z_b-toto;
      return -0.5;
    }
    
    real phii; 
    if (radius < pot->Surf_radius_int) phii = 1.;  
    else {
      phii = phi(pot,xr);
    }
    
    if (phii<0) { // outside the extended (smoothen) embedding surface
      (*f)[pot->dim0] = 0.;
      (*f)[pot->dim1] = 0.;
      // EMY_CHECK: To replace "pot->vdw->rmax" with "pot->r_protein"
      if (z_b-(*xr)[pot->dim2] < pot->vdw->rmax) {(*f)[pot->dim2] = -user_force_vdw(pot,z_b-(*xr)[pot->dim2]);}
      else {(*f)[pot->dim2] = 0.;}
      real toto=(*xr)[pot->dim2];
      sfree(xr);
      // EMY_CHECK: To replace "pot->vdw->rmax" with "pot->r_protein"
      if (z_b-toto < pot->vdw->rmax) return z_b-toto;
      return -0.5;
    } else { // inside the extended (smoothen) embedding surface
      sfree(xr); (*f)[pot->dim0]=0.;(*f)[pot->dim1]=0.;(*f)[pot->dim2]=0.; return -0.5;
      
      /*	real toofar;
		toofar = pot->smoothen*pot->vdw->rmax; // 1.5 for the cutoff
		if (distmin>toofar) { // too far
		sfree(xr);
		(*f)[pot->dim0]=0.;(*f)[pot->dim1]=0.;(*f)[pot->dim2]=0.; return -0.5;
		}
		rvec *direction; snew(direction,1);
		rvec_sub(*xr,pot->Surf[indmin],*direction);
		unitv(*direction,*direction);
		real tmp;
		if (pot->sgs->prop1[indmin]) {tmp = pot->r_protein; pot->r_protein = pot->r_panchor;}
		svmul(user_force_vdw(pot,distmin-(pot->smoothen-1)*pot->r_protein),*direction,(*f));
		sfree(direction);sfree(xr);
		if (pot->sgs->prop1[indmin]) {pot->r_protein = tmp;}
		return distmin-(pot->smoothen-1)*pot->r_protein;
      */    }
  }
  
  // Inside the cavity
  if ((*xr)[pot->dim2]<z_t && (*xr)[pot->dim2]>z_b) {
    real distmin=1337; int indmin=0;  
    
    find_gs_cell(xr,pot,pot->sgs,&xbegin,&xend,&ybegin,&yend,&zbegin,&zend);
    
    for(i=zbegin;i<zend;i++) { // loop over z layers
      for(ii=ybegin;ii<yend;ii++) {
	for(iii=xbegin;iii<xend;iii++) {
	  int xxbegin = pot->sgs->xi[i][ii][iii];
	  int xxend   = pot->sgs->xi[i][ii][iii+1];
	  if (xxbegin==pot->np_Surf) xxbegin--;
	  if (xxend==pot->np_Surf) xxend--;
	  for (iiii=xxbegin;iiii<xxend;iiii++) {
	    dist=sqrt(distance2(*xr,pot->Surf[iiii]));
	    if(distmin>dist) {distmin=dist;indmin=iiii;}
	  }
	}}}
    if (distmin-(pot->smoothen-1)*pot->r_protein < 0 ){sfree(xr); (*f)[pot->dim0]=0.;(*f)[pot->dim1]=0.;(*f)[pot->dim2]=0.; return -1.0;}
    // EMY: vdw force goes to zero if distance > rmax. The following condition checks if the atom is above rmax to avoid to do useless calculations
    real toofar;
    toofar = pot->vdw->rmax;
    if (distmin-(pot->smoothen-1)*pot->r_protein > toofar){sfree(xr); (*f)[pot->dim0]=0.;(*f)[pot->dim1]=0.;(*f)[pot->dim2]=0.; return -0.5;}
    
    real phii; 
    if (radius < pot->Surf_radius_int) phii = 1.;  
    else {
      phii = phi(pot,xr);
    }
    if (phii<0) {
      sfree(xr);
      (*f)[pot->dim0] = 0.;
      (*f)[pot->dim1] = 0.;
      (*f)[pot->dim2] = 0.;
      return -1.;
    } else {
      // EMY_CHECK: replace "*direction" with "direction" in this block
      rvec *direction; snew(direction,1);
      rvec_sub(*xr,pot->Surf[indmin],*direction);
      unitv(*direction,*direction);
      real tmp;
      if (pot->sgs->prop1[indmin]) {tmp = pot->r_protein; pot->r_protein = pot->r_panchor; pot->c1 *= pot->c_anchor;}
      svmul(user_force_vdw(pot,distmin-(pot->smoothen-1)*pot->r_protein),*direction,(*f));
      sfree(direction);sfree(xr);
      if (pot->sgs->prop1[indmin]) {pot->r_protein = tmp; pot->c1 /= pot->c_anchor;}
      return distmin-(pot->smoothen-1)*pot->r_protein;
    }
  }
  // default
  sfree(xr);
  (*f)[pot->dim0] = 0.;
  (*f)[pot->dim1] = 0.;
  (*f)[pot->dim2] = 0.;
  return -0.5;
  
}

// Compute the force for one atom (or a position in the space in the case of force interpolation procedure)
// Distance from each surface is computed and the closest surface is identified

real compute_single_force ( t_user_potential *pot,
			    rvec *x,
		     	    rvec *f,
			    rvec f_init
			    ) {
  
  if (pot->type==wpt_SP) {
    real dist0=-1.,dist1=-1.;
    rvec *force0,*force1;
    rvec *mx; // Symetric image of x with respect to the plane parallel to the membrane anphid crossing the geometric center
    snew(force0,1);
    snew(force1,1);
    snew(mx,1);
    copy_rvec(*x,*mx);
    (*mx)[pot->dim2] -= 2*((*x)[pot->dim2]-pot->center[pot->dim2]); 
    dist0 = compute_force_hsphere(pot,x,force0,pot->zp_t,pot->r_t);
    // EMY_CHECK: As it is, the z coordinate of mx is respect the gc. However, compute_force_hsphere expects mx to be defined with respect to the box origin and then rescale the coordinates accordingly. Therefore, the following compute_force_hsphere will process the wrong value of z coordinate.
    // EMY_CHECK: Probably we should remove the definition of mx and replace mx with x in the next line
    dist1 = compute_force_hsphere(pot,mx,force1,-pot->zp_b,pot->r_b);
    
    sfree(mx);
    
    if (dist0 == -1.  || dist1 == -1.  ) { 
      clear_rvec(*f);
      sfree(force0);
      sfree(force1);
      return   -1.;
    }
    if (dist0 == -0.5 && dist1 == -0.5 ) { 
      clear_rvec(*f);
      sfree(force0);
      sfree(force1); 
      //      return dist0;
      return -0.5; // EMY_CHANGE: same as original above but more readable
    }
    if (dist0 == -0.5) { // EMY_CHECK: What is this block useful for? Why should I consider the symmetric point in place of x?? 
      copy_rvec(*force1,*f);
      (*f)[pot->dim2] *= -1.; // SANDAL: the z-coordinate of the force must be inverted since the force has been calculated by compute_force_hsphere function for the case of an atom in the z-positive hemisphere/plane
      sfree(force0);
      sfree(force1); 
      return dist1;
    }
    if (dist1 == -0.5 ) {
      copy_rvec(*force0,*f);
      sfree(force0);
      sfree(force1); 
      return dist0;
    } 
    if (dist0 < dist1 ) {
      copy_rvec(*force0,*f); 
      sfree(force0);
      sfree(force1); 
      return dist0;
    }
    else { // EMY_CHECK: What is this block useful for? Why should I consider the symmetric point in place of x?? 
      copy_rvec(*force1,*f);
      (*f)[pot->dim2] *= -1.;
      sfree(force0);
      sfree(force1); 
      return dist1; 
    }
    
  }
  
  if (pot->type==wpt_MB) {
    return compute_force_membrane (pot,x,f,f_init);
  }
  
  if (pot->type==wpt_SPMB) {
    // minimum distance between the two types
    real dist0=-1.,dist1=-1.;
    rvec *force0,*force1;
    snew(force0,1);
    snew(force1,1);
    pot->type = wpt_SP;
    dist0 = compute_single_force(pot,x,force0,f_init);
    
    pot->type = wpt_SPMB;
    dist1 = compute_force_membrane(pot,x,force1,f_init);
    
    if (dist0 == -1.  || dist1 == -1.  ) {
      clear_rvec(*f);
      sfree(force0);
      sfree(force1);
      return   -1.;
    }
    if (dist0 == -0.5 && dist1 == -0.5 ) {
      clear_rvec(*f);
      sfree(force0);
      sfree(force1);
      return dist0;
    }
    if (dist0 == -0.5 ) {
      copy_rvec(*force1,*f); 
      sfree(force0);
      sfree(force1);
      return dist1;
    }
    if (dist1 == -0.5 ) { 
      copy_rvec(*force0,*f); 
      sfree(force0);
      sfree(force1);
      return dist0;
    } //(SANDAL) Here it is where it decides what is shorter instead of summing the vector (type 3)
    /*    if (dist0 < dist1 ) { 
      copy_rvec(*force0,*f); 
      sfree(force0);
      sfree(force1);
      return dist0; 
    }
    else { 
      copy_rvec(*force1,*f); 
      sfree(force0);
      sfree(force1);
      return dist1; 
      } */

    else {
      (*f)[pot->dim0] = (*force0)[pot->dim0] + (*force1)[pot->dim0];
      (*f)[pot->dim1] = (*force0)[pot->dim1] + (*force1)[pot->dim1];
      (*f)[pot->dim2] = (*force0)[pot->dim2] + (*force1)[pot->dim2];
      sfree(force0);
      sfree(force1);
      return; 

  }
  }
  

  if (pot->type==wpt_SPH) {
    real dist0=-1.,dist1=-1.;
    rvec *force0,*force1;
    rvec *mx; // Symetric image of x with respect to the plane parallel to the membrane anphid crossing the geometric center
    snew(force0,1);
    snew(force1,1);
    snew(mx,1);
    copy_rvec(*x,*mx);
    (*mx)[pot->dim2] -= 2*((*x)[pot->dim2]-pot->center[pot->dim2]);
    dist0 = compute_force_hsphere_H_upper(pot,x,force0,pot->zp_t,pot->r_b);
    // EMY_CHECK: As it is, the z coordinate of mx is respect the gc. However, compute_force_hsphere expects mx to be defined with respect to the box origin and then rescale the coordinates accordingly. Therefore, the following compute_force_hsphere will process the wrong value of z coordinate.
    // EMY_CHECK: Probably we should remove the definition of mx and replace mx with x in the next line
    dist1 = compute_force_hsphere_H_lower(pot,mx,force1,-pot->zp_b,pot->r_b);

    sfree(mx);

    if (dist0 == -1.  || dist1 == -1.  ) {
      clear_rvec(*f);
      sfree(force0);
      sfree(force1);
      return   -1.;
    }
    if (dist0 == -0.5 && dist1 == -0.5 ) {
      clear_rvec(*f);
      sfree(force0);
      sfree(force1);
      //      return dist0;
      return -0.5; // EMY_CHANGE: same as original above but more readable
    }
    if (dist0 == -0.5) { // EMY_CHECK: What is this block useful for? Why should I consider the symmetric point in place of x??
      copy_rvec(*force1,*f);
      (*f)[pot->dim2] *= -1.; // SANDAL: the z-coordinate of the force must be inverted since the force has been calculated by compute_force_hsphere function for the case of an atom in the z-positive hemisphere/plane
      sfree(force0);
      sfree(force1);
      return dist1;
    }
    if (dist1 == -0.5 ) {
      copy_rvec(*force0,*f);
      sfree(force0);
      sfree(force1);
      return dist0;
    }
    if (dist0 < dist1 ) {
      copy_rvec(*force0,*f);
      sfree(force0);
      sfree(force1);
      return dist0;
    }
    else { // EMY_CHECK: What is this block useful for? Why should I consider the symmetric point in place of x??
      copy_rvec(*force1,*f);
      (*f)[pot->dim2] *= -1.;
      sfree(force0);
      sfree(force1);
      return dist1;
    }

  }





  if (pot->type==wpt_SPMBH) {
    // minimum distance between the two types
    real dist0=-1.,dist1=-1.;
    rvec *force0,*force1;
    snew(force0,1);
    snew(force1,1);
    pot->type = wpt_SPH;
    dist0 = compute_single_force(pot,x,force0,f_init);

    pot->type = wpt_SPMBH;
    dist1 = compute_force_membrane(pot,x,force1,f_init);

    if (dist0 == -1.  || dist1 == -1.  ) {
      clear_rvec(*f);
      sfree(force0);
      sfree(force1);
      return   -1.;
    }
    if (dist0 == -0.5 && dist1 == -0.5 ) {
      clear_rvec(*f);
      sfree(force0);
      sfree(force1);
      return dist0;
    }
    if (dist0 == -0.5 ) {
      copy_rvec(*force1,*f);
      sfree(force0);
      sfree(force1);
      return dist1;
    }
    if (dist1 == -0.5 ) {
      copy_rvec(*force0,*f);
      sfree(force0);
      sfree(force1);
      return dist0;
    } //(SANDAL) Here it is where it decides what is shorter instead of summing the vector (type 3)
    /*    if (dist0 < dist1 ) {
      copy_rvec(*force0,*f);
      sfree(force0);
      sfree(force1);
      return dist0;
    }
    else {
      copy_rvec(*force1,*f);
      sfree(force0);
      sfree(force1);
      return dist1;
      } */

    else {
      (*f)[pot->dim0] = (*force0)[pot->dim0] + (*force1)[pot->dim0];
      (*f)[pot->dim1] = (*force0)[pot->dim1] + (*force1)[pot->dim1];
      (*f)[pot->dim2] = (*force0)[pot->dim2] + (*force1)[pot->dim2];
      sfree(force0);
      sfree(force1);
      return;

  }
  }








  return 0.;
  
}


// ________________________________________________________________________________________________________

// Stores the forces on a regular grid all over the simulation box and order to interpolate them
// rather than doing the whole force computation at each step

void prepare_Wpot_force_field (t_user_potential *pot,
                               t_state *state,
                               t_commrec *cr) {

      int i,j,k,N=pot->Fint_N;
      if (MASTER(cr)) fprintf(stderr,"Storing forces for interpolation, this will require %8.1lf Mb of memory per node.\n",N*N*N*sizeof(real)/(real)(1024*1024));

      int *loads; snew(loads,cr->nnodes+1);
      int charge=0;
      loads[0]=0;
      for(i=1;i<cr->nnodes+1;i++) {
        charge += (N*N*N)/cr->nnodes;
        if (i<=(N*N*N)%cr->nnodes) charge++;
        loads[i]=charge;
      }

      pot->Fint_startx = pot->center[pot->dim0] - 0.5*(state->box[0][pot->dim0] + state->box[1][pot->dim0] + state->box[2][pot->dim0]);
      real endx = pot->center[pot->dim0] + 0.5*(state->box[0][pot->dim0] + state->box[1][pot->dim0] + state->box[2][pot->dim0]);
      pot->Fint_starty = pot->center[pot->dim1] - 0.5*(state->box[0][pot->dim1] + state->box[1][pot->dim1] + state->box[2][pot->dim1]);
      real endy = pot->center[pot->dim1] + 0.5*(state->box[0][pot->dim1] + state->box[1][pot->dim1] + state->box[2][pot->dim1]);
      pot->Fint_startz = pot->center[pot->dim2] - 0.5*(state->box[0][pot->dim2] + state->box[1][pot->dim2] + state->box[2][pot->dim2]);
      real endz = pot->center[pot->dim2] + 0.5*(state->box[0][pot->dim2] + state->box[1][pot->dim2] + state->box[2][pot->dim2]);

      pot->Fint_dx = (endx-pot->Fint_startx)/(real)N;
      pot->Fint_dy = (endy-pot->Fint_starty)/(real)N;
      pot->Fint_dz = (endz-pot->Fint_startz)/(real)N;

      snew(pot->Fint,pow(N,3));
      rvec *xt; snew(xt,1);
      rvec *zero; snew(zero,1); for(i=0;i<3;i++) (*zero)[i]=0.;
      for(i=0;i<N;i++){
      for(j=0;j<N;j++){
      for(k=0;k<N;k++){
      if (i*N*N+j*N+k >= loads[cr->sim_nodeid] && i*N*N+j*N+k < loads[cr->sim_nodeid+1] ){ 
        (*xt)[pot->dim0] = i*pot->Fint_dx+pot->Fint_startx;         
        (*xt)[pot->dim1] = j*pot->Fint_dy+pot->Fint_starty;         
        (*xt)[pot->dim2] = k*pot->Fint_dz+pot->Fint_startz;         
	// EMY_CHANGE: dist is not used in the cycle: removed
      // real dist = compute_single_force(pot,xt,&(pot->Fint[i*N*N+j*N+k]),*zero);
        compute_single_force(pot,xt,&(pot->Fint[i*N*N+j*N+k]),*zero);
      }}}}

      // broadcasts
      for(i=0;i<cr->nnodes;i++) {
      for(j=loads[i];j<loads[i+1];j++) { 
      for(k=0;k<3;k++) {
#ifdef GMX_DOUBLE
        MPI_Bcast(&(pot->Fint[j][k]),1,MPI_DOUBLE,i,cr->mpi_comm_mygroup);
#else
        MPI_Bcast(&(pot->Fint[j][k]),1,MPI_REAL,i,cr->mpi_comm_mygroup);
#endif
      MPI_Barrier(cr->mpi_comm_mygroup);
      }}
      }

}


// ______________ INITIALIZE _____________________________________________________________________________________

int
init_Wpotential (t_user_potential *pot, 
		 int nfile, const t_filenm fnm[],
		 t_state *state,
                 t_state *state_global,
		 gmx_mtop_t *top,
		 t_commrec *cr,
		 rvec *f
		 ) {
  
  
  char *wpotf, line[STRLEN+1], strtmp[STRLEN+1], *memb, *solvent, *anchor;
  FILE *inp;
  int i,j,k,l;
  snew(memb,STRLEN+1);snew(solvent,STRLEN+1),snew(anchor,STRLEN+1);
  
  if (MASTER(cr)) fprintf(stderr,"\n------------------------------------\n\nWall potential initialization\n");
  // wpotf is the pointer to the string of the filename provided through mdrun with the option -wpot (usually "wpot.dat")
  wpotf = ftp2fn(efWPO,nfile,fnm);
  
  // debug in parallel (with gdb : attach to given PIDs, up 2, then set waitdebug=1, continue)
  
  // int waitdebug = 0;
  // fprintf(stderr,"PID %d Waiting attached debugger\n",getpid());
  // while(!waitdebug) sleep(5);
  
  // reading input data file
  if (MASTER(cr)) {
    if((inp = fopen(wpotf,"r"))==NULL) {
      fprintf(stderr, "ERROR : Impossible to open the wall potential data file\n");
      return 1;
    }
    else {
      // EMY_CHANGE: Initialization of r_t and t_b
      pot->r_t=-1; pot->r_b=-1;
      
      int check;
      get_int_from_file(inp,&(pot->type),"type");pot->type--;
      get_char_from_file(inp,&(pot->orientation),"orientation");
      get_string_from_file(inp,memb,"membrane");
      get_string_from_file(inp,solvent,"solvent");
      if (pot->type==wpt_SP || pot->type==wpt_SPMB || pot->type==wpt_SPH || pot->type==wpt_SPMBH) {
	get_real_from_file(inp,&(pot->rplus),"rplus");
	get_real_from_file(inp,&(pot->zplus),"zplus");
	// EMY_CHANGE: Radii of the hemispheres to be given by input in order to avoid the program recalculates them at each restart.
        if (pot->type==wpt_SP || pot->type==wpt_SPMB) {
            get_real_from_file(inp,&(pot->r_t),"r_t");
           }
	get_real_from_file(inp,&(pot->r_b),"r_b");
      }
      pot->bCheck=FALSE;
      // EMY_CHANGE: Initialization of z_memb_t and z_memb_b
      pot->z_memb_t=99999; pot->z_memb_b=99999;
      if (pot->type==wpt_MB || pot->type==wpt_SPMB || pot->type==wpt_SPMBH ) {
	get_real_from_file(inp,&(pot->r_protein),"r_protein");
	get_real_from_file(inp,&(pot->r_panchor),"r_panchor");
	get_real_from_file(inp,&(pot->smoothen),"smoothen");
	get_real_from_file(inp,&(pot->z_memb_t),"z_memb_t");
	get_real_from_file(inp,&(pot->z_memb_b),"z_memb_b");
	if (pot->z_memb_t < pot->z_memb_b) { real temp;
	  temp = pot->z_memb_t;
	  pot->z_memb_t = pot->z_memb_b;
	  pot->z_memb_b = temp;
	}
	get_string_from_file(inp,anchor,"anchor");
	pot->c_anchor=1.;
	get_real_from_file(inp,&(pot->c_anchor),"c_anchor");
	get_int_from_file(inp,&(check),"check");
	pot->bCheck = (check!=0);
      }
      snew(pot->vdw,1);
      pot->vdw->n = 1;
      // EMY_CHECK: Do we want to put n_hs and VDW_reduc_hs in the wpot.dat?
      // EMY_CHANGE: Addition for vdw potential due to the hemisphere walls
      pot->vdw->n_hs = 1;
      get_int_from_file(inp,&(pot->vdw->n),"VDW_force");
      get_real_from_file(inp,&(pot->vdw->VDW_reduc),"VDW_reduc");
      // EMY_CHANGE: Addition for vdw potential due to the hemisphere walls
      pot->vdw->VDW_reduc_hs=pot->vdw->VDW_reduc;
      get_real_from_file(inp,&(pot->rcut),"cut-off");
      // EMY TO_ASK: pot->nsteps_Surf NOT INITIALIZED, NEVER READ, NEVER USED IN THE CODE... In include/types/inputrec.h is describes as "Steps between 2 computation of the surface"
      //	get_int_from_file(inp,&(pot->nsteps_Surf),"nsteps_Surf");
      get_real_from_file(inp,&(pot->c1),"c1");
      pot->bVtk_check = FALSE; check=0;
      get_int_from_file(inp,&check,"vtk_check");
      pot->bVtk_check = (check!=0);
      pot->nsteps_vtk = 10000;
      get_int_from_file(inp,&(pot->nsteps_vtk),"nsteps_vtk");
      pot->bFint=FALSE;
      pot->Fint_N = 100;
      get_int_from_file(inp,&(pot->Fint_N),"Fint");
      pot->bFint= (pot->Fint_N!=0);
      
      check=0; pot->bWpot_ener=FALSE;
      get_int_from_file(inp,&check,"ener");
      pot->bWpot_ener = (check!=0);
    }
    fclose(inp);
    fprintf(stderr, "Parameter file read\n");
  }
  
  // Broadcast input
  gmx_bcast(sizeof(pot->type),&(pot->type),cr);
  gmx_bcast(sizeof(pot->orientation),&(pot->orientation),cr);
  if (pot->orientation == 'Z') {pot->dim0 = 0; pot->dim1 = 1; pot->dim2 = 2;}
  if (pot->orientation == 'X') {pot->dim0 = 1; pot->dim1 = 2; pot->dim2 = 0;}
  if (pot->orientation == 'Y') {pot->dim0 = 2; pot->dim1 = 0; pot->dim2 = 1;}
  gmx_bcast(sizeof(char)*STRLEN+1,memb,cr);
  gmx_bcast(sizeof(char)*STRLEN+1,solvent,cr);
  if (pot->type==wpt_SP || pot->type==wpt_SPMB || pot->type==wpt_SPH || pot->type==wpt_SPMBH ) {
    gmx_bcast(sizeof(pot->rplus),&(pot->rplus),cr);
    gmx_bcast(sizeof(pot->zplus),&(pot->zplus),cr);
    // EMY_CHANGE: Broadcast pot->r_t/b
    if (pot->type==wpt_SP || pot->type==wpt_SPMB) {
    gmx_bcast(sizeof(pot->r_t),&(pot->r_t),cr);
  }
    gmx_bcast(sizeof(pot->r_b),&(pot->r_b),cr);
  }
  if (pot->type==wpt_MB || pot->type==wpt_SPMB || pot->type==wpt_SPMBH ) {
    gmx_bcast(sizeof(char)*STRLEN+1,anchor,cr);
    gmx_bcast(sizeof(pot->r_protein),&(pot->r_protein),cr);
    gmx_bcast(sizeof(pot->r_panchor),&(pot->r_panchor),cr);
    gmx_bcast(sizeof(pot->smoothen),&(pot->smoothen),cr);
    gmx_bcast(sizeof(pot->c_anchor),&(pot->c_anchor),cr);
    gmx_bcast(sizeof(pot->z_memb_t),&(pot->z_memb_t),cr);
    gmx_bcast(sizeof(pot->z_memb_b),&(pot->z_memb_b),cr);
  }
  if (!MASTER(cr)) snew(pot->vdw,1);
  gmx_bcast(sizeof(pot->vdw->VDW_reduc),&(pot->vdw->VDW_reduc),cr);
  gmx_bcast(sizeof(pot->vdw->n),&(pot->vdw->n),cr);
  // EMY_CHANGE: Additions for vdw potential due to the hemisphere walls
  gmx_bcast(sizeof(pot->vdw->VDW_reduc_hs),&(pot->vdw->VDW_reduc_hs),cr);
  gmx_bcast(sizeof(pot->vdw->n_hs),&(pot->vdw->n_hs),cr);      
  gmx_bcast(sizeof(pot->rcut),&(pot->rcut),cr);
  // EMY_CHANGE: pot->nsteps_Surf NOT INITIALIZED, NEVER READ, NEVER USED IN THE CODE... In include/types/inputrec.h is described as "Steps between 2 computation of the surface"
  // gmx_bcast(sizeof(pot->nsteps_Surf),&(pot->nsteps_Surf),cr);
  gmx_bcast(sizeof(pot->c1),&(pot->c1),cr);
  gmx_bcast(sizeof(pot->bVtk_check),&(pot->bVtk_check),cr);
  gmx_bcast(sizeof(pot->nsteps_vtk),&(pot->nsteps_vtk),cr);
  gmx_bcast(sizeof(pot->Fint_N),&(pot->Fint_N),cr);
  gmx_bcast(sizeof(pot->bFint),&(pot->bFint),cr);
  gmx_bcast(sizeof(pot->bWpot_ener),&(pot->bWpot_ener),cr);
  // EMY: Initialize the MMCG energy potential variable in each node 
  pot->Wpotener=0.; 
  // VDW-like stuff  
  // EMY: With the following definition of sigma, the minimum 
  //      of the VDW-like potential of the coating wall coincides with r_protein
  pot->vdw->sigma = pow(2.,-1/(real)pot->vdw->n)*pot->r_protein;
  // EMY_CHANGE: Additions for vdw potential due to the hemisphere walls
  // EMY: With the following definition of sigma_hs, the minimum
  //      of the VDW-like potential of the hemisphere wall is min_hs=0.2 nm
  // EMY_TO_DO: Many of these parameters should be put in the wpot.dat in order to be user-controlled
  pot->vdw->min_hs = 0.2;
  pot->vdw->sigma_hs = pow(2.,-1/(real)pot->vdw->n_hs)*pot->vdw->min_hs;
  // EMY_CHECK: WHY epsilon is function of sigma and n?
  pot->vdw->epsilon    = pot->c1*pot->vdw->sigma/(4.*pot->vdw->n);
  pot->vdw->epsilon_hs = pot->c1*pot->vdw->sigma_hs/(4.*pot->vdw->n_hs);
  // EMY_CHECK: WHERE does 0.25 come from?
  pot->vdw->rmax    = pot->r_protein+0.25;
  pot->vdw->rmax_hs = pot->vdw->min_hs+0.25;
  // Shifting polynomial coefficients
  pot->vdw->Vm = pot->vdw->VDW_reduc*4.*pot->vdw->epsilon*(pow(pot->vdw->sigma/pot->vdw->rmax,2.*pot->vdw->n)-pow(pot->vdw->sigma/pot->vdw->rmax,pot->vdw->n));
  pot->vdw->Fm = -1.*(pot->vdw->VDW_reduc*(4.*pot->vdw->n/pot->vdw->rmax*pot->vdw->epsilon*(2*pow(pot->vdw->sigma/pot->vdw->rmax,2*pot->vdw->n)-pow(pot->vdw->sigma/pot->vdw->rmax,pot->vdw->n))));
  pot->vdw->pm = pot->r_protein - pot->vdw->rmax;
  pot->vdw->A = -2.*pot->vdw->Vm - pot->vdw->pm*pot->vdw->Fm;
  pot->vdw->B = 3.*(pot->r_protein+pot->vdw->rmax)*pot->vdw->Vm + (2.*pot->r_protein+pot->vdw->rmax)*pot->vdw->pm*pot->vdw->Fm;
  pot->vdw->C = -6.*pot->r_protein*pot->vdw->rmax*pot->vdw->Vm - pot->r_protein*(pot->r_protein+2.*pot->vdw->rmax)*pot->vdw->pm*pot->vdw->Fm;
  pot->vdw->D = -pot->r_protein*pot->r_protein*(pot->r_protein-3.*pot->vdw->rmax)*pot->vdw->Vm + pot->r_protein*pot->r_protein*pot->vdw->rmax*pot->vdw->pm*pot->vdw->Fm;
  
  pot->vdw->Vm_hs = pot->vdw->VDW_reduc_hs*4.*pot->vdw->epsilon_hs*(pow(pot->vdw->sigma_hs/pot->vdw->rmax_hs,2.*pot->vdw->n_hs)-pow(pot->vdw->sigma_hs/pot->vdw->rmax_hs,pot->vdw->n_hs));
  pot->vdw->Fm_hs = -1.*(pot->vdw->VDW_reduc_hs*(4.*pot->vdw->n_hs/pot->vdw->rmax_hs*pot->vdw->epsilon_hs*(2*pow(pot->vdw->sigma_hs/pot->vdw->rmax_hs,2*pot->vdw->n_hs)-pow(pot->vdw->sigma_hs/pot->vdw->rmax_hs,pot->vdw->n_hs))));
  pot->vdw->pm_hs = pot->vdw->min_hs - pot->vdw->rmax_hs;
  pot->vdw->A_hs = -2.*pot->vdw->Vm_hs - pot->vdw->pm_hs*pot->vdw->Fm_hs;
  pot->vdw->B_hs = 3.*(pot->vdw->min_hs+pot->vdw->rmax_hs)*pot->vdw->Vm_hs + (2.*pot->vdw->min_hs+pot->vdw->rmax_hs)*pot->vdw->pm_hs*pot->vdw->Fm_hs;
  pot->vdw->C_hs = -6.*pot->vdw->min_hs*pot->vdw->rmax_hs*pot->vdw->Vm_hs - pot->vdw->min_hs*(pot->vdw->min_hs+2.*pot->vdw->rmax_hs)*pot->vdw->pm_hs*pot->vdw->Fm_hs;
  pot->vdw->D_hs = -pot->vdw->min_hs*pot->vdw->min_hs*(pot->vdw->min_hs-3.*pot->vdw->rmax_hs)*pot->vdw->Vm_hs + pot->vdw->min_hs*pot->vdw->min_hs*pot->vdw->rmax_hs*pot->vdw->pm_hs*pot->vdw->Fm_hs;
  
  if ( pot->bWpot_ener && pot->bFint ) {
    if (MASTER(cr)) fprintf(stderr,"Energy calculation and force interpolation are not compatible (yet?). Turning interpolation off.\n");
    pot->bFint=FALSE;
  }
  if(MASTER(cr)) snew(pot->dist,state_global->natoms);
  else snew(pot->dist,state->natoms);
  
  // saving residues names, solvent and lipids
  // to determine the max and min altitudes of protein and lipids atoms
  
  char* temp,*temp2;
  
  
  if (!solvent) {
    fprintf(stderr,"Solvent residue names are needed (if no solvent residues are present, insert '0' after 'solvent =' keyword)\n"); return 1; 
  }
  temp = solvent; temp2 = solvent;
  while (temp2[0]!=' ') temp2++;
  temp2[0] = '\0';
  pot->sols_nr=atoi(temp);
  temp2++;temp=temp2;
  snew(pot->sols,pot->sols_nr);
  for (i=0;i<pot->sols_nr;i++) {
    snew(pot->sols[i],STRLEN+1);
    while (temp2[0]!=' ' && temp2[0]!='\0') temp2++;
    temp2[0] = '\0';
    strcpy(pot->sols[i],temp);
    // EMY_CHANGE: If pot->sols_nr > number of residues mentioned then stop in order to avoid a crash
    if (pot->sols[i][0] == '\0') {
      fprintf(stderr,"In wpot.dat the solvent residues are less than declared\n"); 
      return 1;
    }
    temp2++;temp=temp2;
  }
  
  
  if (!memb) {
    fprintf(stderr,"Membrane residue names are needed (if no membrane residues are present, insert '0' after 'membrane =' keyword)\n"); return 1; 
  }
  temp = memb; temp2 = memb;
  while (temp2[0]!=' ') temp2++;
  temp2[0] = '\0';
  pot->lips_nr=atoi(temp);
  temp2++;temp=temp2;
  snew(pot->lips,pot->lips_nr);
  for (i=0;i<pot->lips_nr;i++) {
    snew(pot->lips[i],STRLEN+1);
    while (temp2[0]!=' ' && temp2[0]!='\0') temp2++;
    temp2[0] = '\0';
    strcpy(pot->lips[i],temp);
    // EMY_CHANGE: If pot->lips_nr > number of residues mentioned then stop in order to avoid a crash
    if (pot->lips[i][0] == '\0') {
      fprintf(stderr,"In wpot.dat the lipid residues are less than declared\n"); 
      return 1;
    }
    temp2++;temp=temp2;
  }
  
  
  // EMY_CHANGE: This control has been moved ouside because there is no reason to check for anchor in case 1
  if (pot->type==wpt_MB || pot->type==wpt_SPMB || pot->type==wpt_SPMBH ) {
    if(!anchor) {
      fprintf(stderr,"Anchor residue names are needed (if no anchor residues are present, insert '0' after 'anchor =' keyword)\n"); return 1; 
    }
    temp = anchor; temp2 = anchor;
    while (temp2[0]!=' ') temp2++;
    temp2[0] = '\0';
    pot->anchor_nr=atoi(temp);
    temp2++;temp=temp2; 
    snew(pot->anchor,pot->anchor_nr);
    for (i=0;i<pot->anchor_nr;i++) {
      snew(pot->anchor[i],STRLEN+1);
      while (temp2[0]!=' ' && temp2[0]!='\0') temp2++;
      temp2[0] = '\0';
      strcpy(pot->anchor[i],temp);
      temp2++;temp=temp2;
    }
  }
  
  sfree(memb);
  sfree(solvent);
  sfree(anchor);
  
  
  // Get min & max altitudes of lipids and protein residues 
  real alt_res_max=-1337., alt_lip_max=-1337.;
  real alt_res_min=1337., alt_lip_min=1337.;
  // Counters to check the absence of one type
  int resnr=0,lipnr=0;
  // Geometrical center will be computed as well
  rvec gc; clear_rvec(gc); 
  
  if (MASTER(cr)) {
      int index=0,j,k,l;
      for(j=0;j<top->nmolblock;j++) {					//run over all molecule types
	for(k=0;k<top->molblock[j].nmol;k++) { 				//run over all instances of each molecule type
	  for(l=0;l<top->moltype[top->molblock[j].type].atoms.nr;l++) {     //run over all atoms in each instance of each molecule type
	    char** atomname=top->moltype[top->molblock[j].type].atoms.atomname[l];
	    int resind = top->moltype[top->molblock[j].type].atoms.atom[l].resind;
	    char** resname=top->moltype[top->molblock[j].type].atoms.resinfo[resind].name;
	    
	    rvec_inc(gc,state_global->x[index]);
	    gmx_bool bSolvent=FALSE,bLipid=FALSE;
	    for(i=0;i<pot->sols_nr;i++) if (!strcmp(pot->sols[i],*resname)) bSolvent=TRUE;
	    for(i=0;i<pot->lips_nr;i++) if (!strcmp(pot->lips[i],*resname)) bLipid=TRUE;
	    
	    if (!bSolvent) { // check for non-waters atoms
	      if (bLipid) { // atom belonging to membrane residues
		if (alt_lip_max < state_global->x[index][pot->dim2]) {
		  alt_lip_max = state_global->x[index][pot->dim2];
		  lipnr++;
		}
		if (alt_lip_min > state_global->x[index][pot->dim2]) {
		  alt_lip_min = state_global->x[index][pot->dim2];
		  lipnr++;
		}
	      }
	      else { // atoms in protein residues
		if (alt_res_max < state_global->x[index][pot->dim2]) {
		  alt_res_max = state_global->x[index][pot->dim2];
		  resnr++;
		}
		if (alt_res_min > state_global->x[index][pot->dim2]) {
		  alt_res_min = state_global->x[index][pot->dim2];
		  resnr++;
		}
	      }
	    }
	    index++;
	  }}}
      svmul(1./(real)top->natoms,gc,gc);
      
      // EMY_CHANGE: Moved this instructions inside the if(MASTER(cr))
      // If the file containing the coordinates of the geometrical center already exists, then read them from it
      FILE *gnuin=fopen("surf_gnu.dat","r");
      if ((pot->type == wpt_MB || pot->type == wpt_SPMB || pot->type == wpt_SPMBH ) && gnuin) {
        fscanf(gnuin,"%lf %lf %lf",&(gc[pot->dim0]),&(gc[pot->dim1]),&(gc[pot->dim2]));
        fclose(gnuin);
      }
      
      // EMY_CHANGE: this section was moved inside the if(MASTER(cr)) and improved considering the possible z_memb_* values in the wpot.dat file
      // Default values of alt_lip_max/alt_lip_min and alt_res_max/alt_res_min in the case no lipid or no protein residues are present in the system
      if ( lipnr == 0 ) {
	if (pot->z_memb_t != 99999 && pot->z_memb_b != 99999) {
	  if ( pot->lips_nr ) printf("WARNING: No lipid residues found. Membrane position will be read from the wpot.dat file"); 
	  alt_lip_max = pot->z_memb_t; alt_lip_min = pot->z_memb_b;
	} else {
          if(pot->type==wpt_MB || pot->type==wpt_SPMB || pot->type == wpt_SPMBH ) {
	    fprintf(stderr,"No lipid residues found and no membrane position information in the wpot.dat file.");
	    return 1;
	  }
	  alt_lip_max = gc[pot->dim2]; alt_lip_min = gc[pot->dim2];
	}
      }
      if ( resnr == 0 ) { alt_res_max = gc[pot->dim2]; alt_res_min = gc[pot->dim2]; }
      
  }
  // BROADCAST
  // EMY_CHANGE: this barrier seems to be unnecessary
  //      gmx_barrier(cr);
  for(i=0;i<3;i++) gmx_bcast_sim(sizeof(gc[i]),&(gc[i]),cr);
  gmx_bcast_sim(sizeof(alt_res_max),&alt_res_max,cr);
  gmx_bcast_sim(sizeof(alt_lip_max),&alt_lip_max,cr);
  gmx_bcast_sim(sizeof(alt_res_min),&alt_res_min,cr);
  gmx_bcast_sim(sizeof(alt_lip_min),&alt_lip_min,cr);
  for (i=0;i<3;i++) gmx_bcast(sizeof(gc[i]),&(gc[i]),cr);
  
  snew(pot->center,3);
  for (i=0;i<3;i++) pot->center[i] = gc[i];
  
  // EMY_CHANGE: Put the alt_lip_max and alt_lip_min values (the membrane position in each case) in pot->z_memb_t and pot->z_memb_b respectively, in order to export them outside the init_Wpotential() function
  pot->z_memb_t = alt_lip_max;
  pot->z_memb_b = alt_lip_min;
  
  // types 2 & 3 : inner surface reconstruction
  // only the master can do this since it is the only node
  // which has the current positions of all atoms       EMY_CHECK: Is this comment true?
  snew(pot->sags,1);
  snew(pot->sgs,1);
  
  if(MASTER(cr) && (pot->type==wpt_MB || pot->type==wpt_SPMB || pot->type == wpt_SPMBH )) {
    fprintf(stderr,"Building the surface embedding the protein...\n");
    compute_surface_protein(pot,top,state_global,cr);
    fprintf(stderr,"Done !\n");
      }
  // BROADCAST
  if (pot->type == wpt_MB || pot->type==wpt_SPMB || pot->type == wpt_SPMBH ) {
    gmx_bcast(sizeof(pot->Surf_radius),&(pot->Surf_radius),cr);
    gmx_bcast(sizeof(pot->Surf_radius_int),&(pot->Surf_radius_int),cr);
    // surface points 
    gmx_bcast(sizeof(pot->np_Surf),&(pot->np_Surf),cr);
    if (!MASTER(cr)) {
      snew(pot->Surf,pot->np_Surf);
    }
    for(i=0;i<pot->np_Surf;i++) 
      for (j=0;j<3;j++)
	gmx_bcast(sizeof(real),&(pot->Surf[i][j]),cr);
    // atoms
    gmx_bcast(sizeof(pot->bAtoms_GS),&(pot->bAtoms_GS),cr);
    gmx_bcast(sizeof(pot->Surf_natoms),&(pot->Surf_natoms),cr);
    gmx_bcast(sizeof(pot->nindex),&(pot->nindex),cr);
    gmx_bcast(sizeof(pot->nindex2),&(pot->nindex2),cr);
    if (!MASTER(cr)) {
      snew(pot->Surf_atoms,pot->Surf_natoms);
      snew(pot->sags->prop1,pot->Surf_natoms);
      snew(pot->index,pot->nindex);
      snew(pot->index2,pot->nindex2);
    }
    for(i=0;i<pot->nindex;i++) { 
      gmx_bcast(sizeof(pot->index[i]),&(pot->index[i]),cr);
    }
    for(i=0;i<pot->nindex2;i++) { 
      gmx_bcast(sizeof(pot->index2[i]),&(pot->index2[i]),cr);
    }
    for(i=0;i<pot->Surf_natoms;i++) { 
      gmx_bcast(sizeof(gmx_bool),&(pot->sags->prop1[i]),cr);
      for (j=0;j<3;j++)
	gmx_bcast(sizeof(real),&(pot->Surf_atoms[i][j]),cr);
    }
      // indexes of atoms
    if (pot->bAtoms_GS) {
      gmx_bcast(sizeof(pot->sags->nz),&(pot->sags->nz),cr);
      gmx_bcast(sizeof(pot->sags->ny),&(pot->sags->ny),cr);
      gmx_bcast(sizeof(pot->sags->nx),&(pot->sags->nx),cr);
      if (!MASTER(cr)) {
	snew(pot->sags->zi,pot->sags->nz+1);
	snew(pot->sags->yi,pot->sags->nz);
	snew(pot->sags->xi,pot->sags->nz);
	for(i=0;i<pot->sags->nz;i++) {
	  snew(pot->sags->yi[i],pot->sags->ny+1);
	  snew(pot->sags->xi[i],pot->sags->ny);
	  for(j=0;j<pot->sags->ny;j++) {
	    snew(pot->sags->xi[i][j],pot->sags->nx+1);
	  }
	}
      }
      for(i=0;i<pot->sags->nz;i++) {
	gmx_bcast(sizeof(int),&(pot->sags->zi[i]),cr);
	for(j=0;j<pot->sags->ny;j++) {
	  gmx_bcast(sizeof(int),&(pot->sags->yi[i][j]),cr);
	  for(k=0;k<pot->sags->nx;k++) {
	    gmx_bcast(sizeof(int),&(pot->sags->xi[i][j][k]),cr);
	  }
	  gmx_bcast(sizeof(int),&(pot->sags->xi[i][j][pot->sags->nx]),cr);
	}
	gmx_bcast(sizeof(int),&(pot->sags->yi[i][pot->sags->ny]),cr);
      }
      gmx_bcast(sizeof(int),&(pot->sags->zi[pot->sags->nz]),cr);
      gmx_bcast(sizeof(real),&(pot->sags->x_min),cr);
      gmx_bcast(sizeof(real),&(pot->sags->y_min),cr);
      gmx_bcast(sizeof(real),&(pot->sags->z_min),cr);
    }
    // indexes of layers
    gmx_bcast(sizeof(pot->sgs->nz),&(pot->sgs->nz),cr);
    gmx_bcast(sizeof(pot->sgs->ny),&(pot->sgs->ny),cr);
    gmx_bcast(sizeof(pot->sgs->nx),&(pot->sgs->nx),cr);
    gmx_bcast(sizeof(pot->sgs->dz),&(pot->sgs->dz),cr);
    gmx_bcast(sizeof(pot->sgs->dy),&(pot->sgs->dy),cr);
    gmx_bcast(sizeof(pot->sgs->dx),&(pot->sgs->dx),cr);
    gmx_bcast(sizeof(real),&(pot->sgs->x_min),cr);
    gmx_bcast(sizeof(real),&(pot->sgs->x_max),cr);
    gmx_bcast(sizeof(real),&(pot->sgs->y_min),cr);
    gmx_bcast(sizeof(real),&(pot->sgs->y_max),cr);
    gmx_bcast(sizeof(real),&(pot->sgs->z_min),cr);
    gmx_bcast(sizeof(real),&(pot->sgs->z_max),cr);
    if (!MASTER(cr)) {
      snew(pot->sgs->zi,pot->sgs->nz+1);
      snew(pot->sgs->yi,pot->sgs->nz);
      snew(pot->sgs->xi,pot->sgs->nz);
      snew(pot->sgs->prop1,pot->np_Surf);
      for(i=0;i<pot->sgs->nz;i++) {
	snew(pot->sgs->yi[i],pot->sgs->ny+1);
	snew(pot->sgs->xi[i],pot->sgs->ny);
	for(j=0;j<pot->sgs->ny;j++) {
	  snew(pot->sgs->xi[i][j],pot->sgs->nx+1);
	}
      }
    }
    for(i=0;i<pot->sgs->nz;i++) {
      gmx_bcast(sizeof(int),&(pot->sgs->zi[i]),cr);
      for(j=0;j<pot->sgs->ny;j++) {
	gmx_bcast(sizeof(int),&(pot->sgs->yi[i][j]),cr);
	for(k=0;k<pot->sgs->nx;k++) {
	  gmx_bcast(sizeof(int),&(pot->sgs->xi[i][j][k]),cr);
	}
	gmx_bcast(sizeof(int),&(pot->sgs->xi[i][j][pot->sgs->nx]),cr);
      }
      gmx_bcast(sizeof(int),&(pot->sgs->yi[i][pot->sgs->ny]),cr);
    }
    gmx_bcast(sizeof(int),&(pot->sgs->zi[pot->sgs->nz]),cr);
  }
  // end surface setup for all nodes
  
  // types 1 & 3 setup
  if(pot->type==wpt_SP || pot->type==wpt_SPMB || pot->type==wpt_SPH || pot->type==wpt_SPMBH ) {
    pot->zplus = fabs(pot->zplus);
    pot->rplus = fabs(pot->rplus);
    // EMY CHANGES: This section as been moved before when alt_lip_* are defined
    /*  if ( pot->type == wpt_SPMB ) { 
	alt_lip_max = pot->z_memb_t; // EMY: In the case 3, alt_lip_* is defined in wpot.dat. In the case 1 and 2 alt_lip_* is defined in block after the comment "// Get min & max altitudes of lipids and protein residues"
	alt_lip_min = pot->z_memb_b;
	// Increase sphere radius ? too large here, waters have to stay above and under the protein to correctly soak the binding site 
	// if (pot->rplus < pot->Surf_radius + pot->rcut - (pot->smoothen-1)*pot->r_protein ) {
	//    if (MASTER(cr)) fprintf(stderr,"The radius of the half spheres has been increased by %lf to avoid perturbations.\n",
	//           pot->Surf_radius + pot->rcut - pot->rplus);
	//    pot->rplus = pot->Surf_radius + pot->rcut - (pot->smoothen-1.)*pot->r_protein;
	// }
	} */
    //SANDAL: zp_t=z of top plane (dashed line at Pag. 2 in doc_MMCG_Wpot, zp_b=z of botton plane with respect the geometrical center gc
    pot->zp_t = alt_lip_max - gc[pot->dim2] + pot->zplus;
    pot->zp_b = alt_lip_min - gc[pot->dim2] - pot->zplus;
    if (MASTER(cr) && pot->zp_t > 0.5*(state->box[0][pot->dim2] + state->box[1][pot->dim2] + state->box[2][pot->dim2])) {
      fprintf(stderr,"Warning : the upper plane is out of the box, this will cause\n");
      fprintf(stderr,"interferences with the periodic boundary conditions !\n");
      fprintf(stderr,"You should increase the box size along the\n");
      fprintf(stderr,"direction perpendicular to the membrane\n");
    }
    if (MASTER(cr) && pot->zp_b < -0.5*(state->box[0][pot->dim2] + state->box[1][pot->dim2] + state->box[2][pot->dim2])) {
      fprintf(stderr,"Warning : the lower plane is out of the box, this will cause\n");
      fprintf(stderr,"interferences with the periodic boundary conditions !\n");
      fprintf(stderr,"You should increase the box size along the\n");
      fprintf(stderr,"direction perpendicular to the membrane\n");
    }
    //SANDAL: The hemisphere is calculated centered on the plane of the (top/bottom) plane
    //r_t = radius of top hemisphere; r_b = radius of bottom hemisphere
    // EMY_CHANGE: Calculate pot->r_t and pot->r_b only if it is not defined in the wpot.dat file. This way we can avoid the problem of recalculation of them at any restart file
    if (pot->r_t == -1) pot->r_t = max( alt_res_max  + pot->rplus - fabs(pot->zp_t) - gc[pot->dim2], 0. ); // EMY: alt_res_max = max z of the protein atoms
    if (pot->r_b == -1) pot->r_b = max( gc[pot->dim2] - alt_res_min + pot->rplus - fabs(pot->zp_b), 0. );  // EMY: alt_res_min = min z of the protein atoms
    //EMY_CHECK: MMCG code works only with pbc = xyz (coulombtype = Cut-Off). Are PDBs essential? Can PDBs be removed?
    if (MASTER(cr) && (pot->zp_t + pot->r_t) > 0.5*(state->box[0][pot->dim2] + state->box[1][pot->dim2] + state->box[2][pot->dim2])) {
      fprintf(stderr,"Warning : the upper hemisphere is (partially) out of the box, this will cause\n");
      fprintf(stderr,"interferences with the periodic boundary conditions !\n");
      fprintf(stderr,"You should increase the box size along the\n");
      fprintf(stderr,"direction perpendicular to the membrane\n");
    }
    if (MASTER(cr) && (pot->zp_b - pot->r_b) < -0.5*(state->box[0][pot->dim2] + state->box[1][pot->dim2] + state->box[2][pot->dim2])) {
      fprintf(stderr,"Warning : the lower hemisphere is (partially) out of the box, this will cause\n");
      fprintf(stderr,"interferences with the periodic boundary conditions !\n");
      fprintf(stderr,"You should increase the box size along the\n");
      fprintf(stderr,"direction perpendicular to the membrane\n");
    }
  }
  
  // User console recap      
  if (MASTER(cr)) {
    fprintf(stderr,"\n---------------------------------\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"  Wall potential parameters : \n\n");
    if(pot->type==wpt_SP || pot->type==wpt_SPMB || pot->type==wpt_SPH || pot->type==wpt_SPMBH ) {
      fprintf(stderr,"    Total spacing between the planes       %7.3lf (nm)\n",pot->zp_t-pot->zp_b);
      if(pot->type==wpt_SP || pot->type==wpt_SPMB) {
      fprintf(stderr,"    Radius of upper hemisphere            %7.3lf (nm)\n",pot->r_t);
      }
      fprintf(stderr,"    Radius of lower hemisphere            %7.3lf (nm)\n",pot->r_b);
    }
    if(pot->type==wpt_MB || pot->type==wpt_SPMB || pot->type==wpt_SPMBH ) {
      fprintf(stderr,"    Membrane thickness                    %7.3lf (nm)\n",pot->z_memb_t-pot->z_memb_b);
      //SANDAL: Surf_radius and Surf_radius_int : Max and min radius of surface coating the protein from the axis of the coating to make calculations of potential faster (TO VERIFY)
      fprintf(stderr,"    Max radius of the inner surface       %7.3lf (nm)\n",pot->Surf_radius);
      fprintf(stderr,"    Min radius of the inner surface       %7.3lf (nm)\n",pot->Surf_radius_int);
      fprintf(stderr,"    Number of points defining the surface %7d \n",pot->np_Surf);
    }
    fprintf(stderr,"    Cut-off for force calculations        %7.3lf (nm)\n",pot->rcut);
    fprintf(stderr,"    Center                %7.3lf %7.3lf %7.3lf (nm)\n",gc[0],gc[1],gc[2]);
    fprintf(stderr,"\n");
    fprintf(stderr,"\n---------------------------------\n");
  }
  
  // Forces interpolation
  // EMY: Interpolation to calculate the forces in a space grid through a lookup table
  if (pot->bFint) {
    prepare_Wpot_force_field(pot,state,cr);
  }
  
  // Check the atoms position with respect the interacting surfaces. It will also firstly compute the distances for all atoms.
  if (MASTER(cr)) {
    rvec *junk;
    snew(junk,state_global->natoms);
    int compteur=0;
    for(i=0;i<state_global->natoms;i++) {
      // EMY: compute_single_force computes the distance between barrier and atom. If the returned value is positive than its meaning is the corresponding distance, otherwise it is a flag pointing out the reason why the position of the atom is not needed. 
      pot->dist[i] = compute_single_force(pot,&(state_global->x[i]),&(junk[i]),f[i]);
      if (pot->dist[i] < -0.75) compteur++;
    }
    if(compteur!=0) {fprintf(stderr,"There is (are) %d atom(s) outside of the interacting surfaces, most probably water molecules\n",compteur);
      fprintf(stderr,"Try larger values for r_plus and z_plus.\n");
      fprintf(stderr,"Files ff_init.vtk, surface.vtk and surface_sphere.vtk\n");
      fprintf(stderr,"will be written in order to help you to visualize them.\n\n");}
    
    FILE *vtkout= fopen("ff_init.vtk","w");
    rvec *zero; snew(zero,1); for(i=0;i<3;i++) (*zero)[i]=0.;
    write_field_and_scalar_vtk(vtkout,state_global->natoms,state_global->x,pot->dist,junk,zero);
    // EMY_CHANGE: Moved fclose and sfree instruction from later in the routine back to here for improving reading
    fclose(vtkout);
    sfree(junk);
    sfree(zero);
    
    int NN=100;
    srenew(pot->Surf,pot->np_Surf+2*NN*NN);
    int supppoints=0;
    real startx,starty,endx,endy,dx,dy;
    // EMY: pot->Surf is a matrix. Each line contains the positions of the points forming the coating surface + the membrane planes as they appear in a .gro file.
    // EMY: Redefine the origin of the coordinates in the middle of the box (z is defined with respect the geometric center (gc). pot->dim0 represents the x-coordinate. box[1][pot->dim0] is the x-coordinate of the second box vector.
    // EMY: surface.gro is the file that stores points on the coating surface that mimics the membrane around the z axis and points on the membrane planes as well.
    startx = -0.5*(state->box[0][pot->dim0] + state->box[1][pot->dim0] + state->box[2][pot->dim0]);
    endx = 0.5*(state->box[0][pot->dim0] + state->box[1][pot->dim0] + state->box[2][pot->dim0]);
    starty = -0.5*(state->box[0][pot->dim1] + state->box[1][pot->dim1] + state->box[2][pot->dim1]);
    endy = 0.5*(state->box[0][pot->dim1] + state->box[1][pot->dim1] + state->box[2][pot->dim1]);
    
    dx = (endx-startx)/(real)NN;
    dy = (endy-starty)/(real)NN;
    
    if(pot->type == wpt_MB || pot->type == wpt_SPMB || pot->type==wpt_SPMBH ) {
      for(i=0;i<NN;i++) {
	for(j=0;j<NN;j++) {
	  pot->Surf[pot->np_Surf+supppoints][pot->dim0] = i*dx+startx;
	  pot->Surf[pot->np_Surf+supppoints][pot->dim1] = j*dy+starty;
	  pot->Surf[pot->np_Surf+supppoints][pot->dim2] = pot->z_memb_t - pot->center[pot->dim2];
	  if(phi(pot,&(pot->Surf[pot->np_Surf+supppoints]))<=0) {supppoints++;}
        }}
      for(i=0;i<NN;i++) {
	for(j=0;j<NN;j++) {
	  pot->Surf[pot->np_Surf+supppoints][pot->dim0] = i*dx+startx;
	  pot->Surf[pot->np_Surf+supppoints][pot->dim1] = j*dy+starty;
	  pot->Surf[pot->np_Surf+supppoints][pot->dim2] = pot->z_memb_b - pot->center[pot->dim2];
	  if(phi(pot,&(pot->Surf[pot->np_Surf+supppoints]))<=0) {supppoints++;}
        }}
      
      vtkout = fopen("surface.vtk","w");
      write_points_vtk(vtkout,pot->np_Surf+supppoints,pot->Surf,pot->center);
      fclose(vtkout);
      FILE *grofile = fopen("surface.gro","w");
      write_gro_points(grofile,pot->np_Surf+supppoints,pot->Surf,pot->center);
      fclose(grofile);
      srenew(pot->Surf,pot->np_Surf);
      
      // finer check of the force field (uncomment to get good visuals of the ffield)
      /*
	NN = 100;
	vtkout=fopen("ff_init2.vtk","w");
	real startz = pot->z_memb_b - 0.2*(pot->z_memb_t-pot->z_memb_b);
	real endz = pot->z_memb_t + 0.2*(pot->z_memb_t-pot->z_memb_b);
	startx = pot->center[pot->dim0] - 0.5*(state->box[0][pot->dim0] + state->box[1][pot->dim0] + state->box[2][pot->dim0]);
	endx = pot->center[pot->dim0] + 0.5*(state->box[0][pot->dim0] + state->box[1][pot->dim0] + state->box[2][pot->dim0]);
	starty = pot->center[pot->dim1] - 0.5*(state->box[0][pot->dim1] + state->box[1][pot->dim1] + state->box[2][pot->dim1]);
	endy = pot->center[pot->dim1] + 0.5*(state->box[0][pot->dim1] + state->box[1][pot->dim1] + state->box[2][pot->dim1]);
	real dz = (endz-startz)/(real)NN;
	real dy = (endy-starty)/(real)NN;
	real dx = (endx-startx)/(real)NN;
	rvec *xt,*junk2; snew(xt,NN*NN*NN); snew(junk2,NN*NN*NN);
	real *dist2,*phhi; snew(dist2,NN*NN*NN);snew(phhi,NN*NN*NN);
	rvec *zero; snew(zero,1); for(i=0;i<3;i++) (*zero)[i]=0.;
	for(i=0;i<NN;i++){          
	for(j=0;j<NN;j++){          
	for(k=0;k<NN;k++){ 
	xt[i*NN*NN+j*NN+k][pot->dim0] = i*dx+startx;         
	xt[i*NN*NN+j*NN+k][pot->dim1] = j*dy+starty;         
	xt[i*NN*NN+j*NN+k][pot->dim2] = k*dz+startz;         
	dist2[i*NN*NN+j*NN+k] = compute_single_force(pot,&(xt[i*NN*NN+j*NN+k]),&(junk2[i*NN*NN+j*NN+k]),*zero);
	rvec_dec(xt[i*NN*NN+j*NN+k],pot->center);
	phhi[i*NN*NN+j*NN+k] = phi(pot,&(xt[i*NN*NN+j*NN+k]));
	rvec_inc(xt[i*NN*NN+j*NN+k],pot->center);
	}}}
	
	write_field_and_scalar_vtk(vtkout,NN*NN*NN,xt,dist2,junk2,zero);
	fclose(vtkout);
	vtkout = fopen("phi.vtk","w");
	write_scalar_vtk(vtkout,NN*NN*NN,xt,phhi,zero);
	fclose(vtkout);
	sfree(zero);
	sfree(xt);sfree(junk2);sfree(dist2);
	sfree(phhi);
        //
        */
      
      real r,fjunk;
      gmx_bool bWpot_ener_tmp=pot->bWpot_ener;
      pot->bWpot_ener=TRUE;
      FILE *datout;
      vtkout=fopen("vdw-like_pot.dat","w");
      datout=fopen("vdw-like_fce.dat","w");
      for(r=0.001;r<=pot->r_protein+0.6;r+=0.001) {
	pot->Wpotener=0;fjunk=user_force_vdw(pot,r);
	fprintf(vtkout,"%lf\t%lf\n",r,pot->Wpotener);
	fprintf(datout,"%lf\t%lf\n",r,fjunk);
      }
      fclose(vtkout);
      fclose(datout);
      vtkout=fopen("r_2_pot.dat","w");
      datout=fopen("r_2_fce.dat","w");
      for(r=0.001;r<=pot->rcut+0.1;r+=0.001) {
	pot->Wpotener=0;fjunk=user_force(pot,r);
	fprintf(vtkout,"%lf\t%lf\n",r,pot->Wpotener);
	fprintf(datout,"%lf\t%lf\n",r,fjunk);
      }
      fclose(vtkout);
      fclose(datout);
      pot->bWpot_ener=bWpot_ener_tmp;
    }
    
    // EMY: surface_sphere.gro is the file that stores points on the top and bottom planes and the hemispheres
    if(pot->type==wpt_SP || pot->type==wpt_SPMB ){
      
      startx = -0.5*(state->box[0][pot->dim0] + state->box[1][pot->dim0] + state->box[2][pot->dim0]);
      endx = 0.5*(state->box[0][pot->dim0] + state->box[1][pot->dim0] + state->box[2][pot->dim0]);
      starty = -0.5*(state->box[0][pot->dim1] + state->box[1][pot->dim1] + state->box[2][pot->dim1]);
      endy = 0.5*(state->box[0][pot->dim1] + state->box[1][pot->dim1] + state->box[2][pot->dim1]);
      dx = (endx-startx)/(real)NN;
        dy = (endy-starty)/(real)NN;
	
        rvec *xt;
	real radius;
        snew(xt,2*NN*NN);
        for(i=0;i<NN;i++) {
	  for(j=0;j<NN;j++) {
	    xt[i*NN+j][pot->dim0] = i*dx+startx;
	    xt[i*NN+j][pot->dim1] = j*dy+starty;
	    xt[i*NN+j][pot->dim2] = pot->zp_t;
	    // EMY_CHANGE: More efficient
	    // real radius = sqrt(pow(i*dx+startx,2)+pow(j*dy+starty,2));
	    radius = sqrt((i*dx+startx)*(i*dx+startx)+(j*dy+starty)*(j*dy+starty));
            if (radius<pot->r_t) xt[i*NN+j][pot->dim2] += sqrt(pot->r_t*pot->r_t - radius*radius);
	  }}
        for(i=0;i<NN;i++) {
	  for(j=0;j<NN;j++) {
	    xt[i*NN+j+NN*NN][pot->dim0] = i*dx+startx;
	    xt[i*NN+j+NN*NN][pot->dim1] = j*dy+starty;
	    xt[i*NN+j+NN*NN][pot->dim2] = pot->zp_b;
	    // EMY_CHANGE: More efficient
	    // real radius = sqrt(pow(i*dx+startx,2)+pow(j*dy+starty,2));
	    radius = sqrt((i*dx+startx)*(i*dx+startx)+(j*dy+starty)*(j*dy+starty));
            if (radius<pot->r_b) xt[i*NN+j+NN*NN][pot->dim2] -= sqrt(pot->r_b*pot->r_b - radius*radius);
	  }}
	
        vtkout = fopen("surface_sphere.vtk","w");
        write_points_vtk(vtkout,2*NN*NN,xt,pot->center);
        fclose(vtkout);     
        FILE *grofile = fopen("surface_sphere.gro","w");
        write_gro_points(grofile,2*NN*NN,xt,pot->center);
        fclose(grofile);
        sfree(xt);
    }
    



    if(pot->type==wpt_SPH || pot->type==wpt_SPMBH ){

      startx = -0.5*(state->box[0][pot->dim0] + state->box[1][pot->dim0] + state->box[2][pot->dim0]);
      endx = 0.5*(state->box[0][pot->dim0] + state->box[1][pot->dim0] + state->box[2][pot->dim0]);
      starty = -0.5*(state->box[0][pot->dim1] + state->box[1][pot->dim1] + state->box[2][pot->dim1]);
      endy = 0.5*(state->box[0][pot->dim1] + state->box[1][pot->dim1] + state->box[2][pot->dim1]);
      dx = (endx-startx)/(real)NN;
        dy = (endy-starty)/(real)NN;

        rvec *xt;
        real radius;
        snew(xt,2*NN*NN);
        for(i=0;i<NN;i++) {
          for(j=0;j<NN;j++) {
              radius = sqrt((i*dx+startx)*(i*dx+startx)+(j*dy+starty)*(j*dy+starty));
              if (radius>pot->r_b) {
            xt[i*NN+j][pot->dim0] = i*dx+startx;
            xt[i*NN+j][pot->dim1] = j*dy+starty;
            xt[i*NN+j][pot->dim2] = pot->zp_t;
            // EMY_CHANGE: More efficient
            // real radius = sqrt(pow(i*dx+startx,2)+pow(j*dy+starty,2));
/*	    radius = sqrt((i*dx+startx)*(i*dx+startx)+(j*dy+starty)*(j*dy+starty));
            if (radius<pot->r_b) xt[i*NN+j][pot->dim2] += sqrt(pot->r_t*pot->r_t - radius*radius); */
          }}}
        for(i=0;i<NN;i++) {
          for(j=0;j<NN;j++) {
            xt[i*NN+j+NN*NN][pot->dim0] = i*dx+startx;
            xt[i*NN+j+NN*NN][pot->dim1] = j*dy+starty;
            xt[i*NN+j+NN*NN][pot->dim2] = pot->zp_b;
            // EMY_CHANGE: More efficient
            // real radius = sqrt(pow(i*dx+startx,2)+pow(j*dy+starty,2));
            radius = sqrt((i*dx+startx)*(i*dx+startx)+(j*dy+starty)*(j*dy+starty));
            if (radius<pot->r_b) xt[i*NN+j+NN*NN][pot->dim2] -= sqrt(pot->r_b*pot->r_b - radius*radius);
          }}

        vtkout = fopen("surface_sphere.vtk","w");
        write_points_vtk(vtkout,2*NN*NN,xt,pot->center);
        fclose(vtkout);
        FILE *grofile = fopen("surface_sphere.gro","w");
        write_gro_points(grofile,2*NN*NN,xt,pot->center);
        fclose(grofile);
        sfree(xt);
    }







    fprintf(stderr,"Wall potential initialization done\n\n----------------------------------------\n");
    time_t  currtime;                                                    
    // EMY_CHANGE: charTime not used
    // char charTime[10] = {0};                                             
    
    time(&currtime);                                                     
    //strftime(charTime,sizeof(charTime)-1,"%Y-%m-%d ",localtime(&currtime)); 
    fprintf(stderr,"Began at %s\n",asctime(localtime(&currtime)));
  } 
  
  return 0;
  
}

// DO_FORCE

void
do_Wpotential(t_user_potential *pot,
	      t_inputrec *ir,
	      t_state *state,
	      rvec *f,
	      t_commrec *cr, // for debugging
	      gmx_large_int_t step,
	      gmx_mtop_t *top,
	      t_mdatoms *md) {
  
  
  int i,j;
  pot->Wpotener=0;
  rvec *junk,*junk2; snew(junk,1); snew(junk2,1);
  srenew(pot->dist,state->natoms);
  for(i=md->start;i<md->start+md->homenr;i++) {
    gmx_bool bdo=FALSE;
    if (pot->type == wpt_SP ) {
      bdo=TRUE;
    } else { 
      for(j=0;j<pot->nindex;j++) { 
        if (pot->index[j]==i)
          bdo=TRUE;
      }
    }
    if (bdo) {
      
      if(pot->bFint){
	int im = (int)floor((state->x[i][pot->dim0]-pot->Fint_startx)/(real)pot->Fint_dx);      
	int jm = (int)floor((state->x[i][pot->dim1]-pot->Fint_starty)/(real)pot->Fint_dy);      
	int km = (int)floor((state->x[i][pot->dim2]-pot->Fint_startz)/(real)pot->Fint_dz);      
	
	// should not happen, (beware of pressure coupling though->think about modifying starts and dxs
	if(im>=pot->Fint_N-1) im=pot->Fint_N-2; if(im<0) im=0;
	if(jm>=pot->Fint_N-1) jm=pot->Fint_N-2; if(jm<0) jm=0;
	if(km>=pot->Fint_N-1) km=pot->Fint_N-2; if(km<0) km=0;
	
	// back in a [0:1]^3 box
	real xxx = (state->x[i][pot->dim0]-pot->Fint_startx-im*pot->Fint_dx)/pot->Fint_dx;
	real yyy = (state->x[i][pot->dim1]-pot->Fint_starty-jm*pot->Fint_dy)/pot->Fint_dy;
	real zzz = (state->x[i][pot->dim2]-pot->Fint_startz-km*pot->Fint_dz)/pot->Fint_dz;
	
	clear_rvec(*junk);
	rvec_inc(*junk2,pot->Fint[im*pot->Fint_N*pot->Fint_N+jm*pot->Fint_N+km]);
	svmul((1.-xxx)*(1.-yyy)*(1.-zzz),*junk2,*junk2);
	rvec_inc(*junk,*junk2);
	clear_rvec(*junk2);
	rvec_inc(*junk2,pot->Fint[(im+1)*pot->Fint_N*pot->Fint_N+jm*pot->Fint_N+km]);
	svmul(xxx*(1.-yyy)*(1.-zzz),*junk2,*junk2);
	rvec_inc(*junk,*junk2);
	clear_rvec(*junk2);
	rvec_inc(*junk2,pot->Fint[(im+1)*pot->Fint_N*pot->Fint_N+(jm+1)*pot->Fint_N+km]);
	svmul(xxx*yyy*(1.-zzz),*junk2,*junk2);
	rvec_inc(*junk,*junk2);
	clear_rvec(*junk2);
	rvec_inc(*junk2,pot->Fint[im*pot->Fint_N*pot->Fint_N+(jm+1)*pot->Fint_N+km]);
	svmul((1.-xxx)*yyy*(1.-zzz),*junk2,*junk2);
	rvec_inc(*junk,*junk2);
	clear_rvec(*junk2);
	rvec_inc(*junk2,pot->Fint[im*pot->Fint_N*pot->Fint_N+jm*pot->Fint_N+km+1]);
	svmul((1.-xxx)*(1.-yyy)*zzz,*junk2,*junk2);
	rvec_inc(*junk,*junk2);
	clear_rvec(*junk2);
	rvec_inc(*junk2,pot->Fint[(im+1)*pot->Fint_N*pot->Fint_N+jm*pot->Fint_N+km+1]);
	svmul(xxx*(1.-yyy)*zzz,*junk2,*junk2);
	rvec_inc(*junk,*junk2);
	clear_rvec(*junk2);
	rvec_inc(*junk2,pot->Fint[(im+1)*pot->Fint_N*pot->Fint_N+(jm+1)*pot->Fint_N+km+1]);
	svmul(xxx*yyy*zzz,*junk2,*junk2);
	rvec_inc(*junk,*junk2);
	clear_rvec(*junk2);
	rvec_inc(*junk2,pot->Fint[im*pot->Fint_N*pot->Fint_N+(jm+1)*pot->Fint_N+km+1]);
	svmul((1.-xxx)*yyy*zzz,*junk2,*junk2);
	rvec_inc(*junk,*junk2);
	
	//if (MASTER(cr)) fprintf(stderr,"atom %d ijk %d %d %d xyz %f %f %f f %f %f %f \n",
	//                        i,im,jm,km,xxx,yyy,zzz,(*junk)[0],(*junk)[1],(*junk)[2]);
	
      }
      else {
	gmx_bool bdo2=FALSE;
	gmx_bool wpot_ener_temp = pot->bWpot_ener;
	if(pot->bWpot_ener) {
	  pot->bWpot_ener=FALSE;
	  for(j=0;j<pot->nindex2;j++) {
	    if (pot->index2[j]==i)
	      bdo2=TRUE;
	  }  
	}
	if(bdo2) pot->bWpot_ener=TRUE;
	
	pot->dist[i] = compute_single_force(pot,&(state->x[i]),junk,f[i]);
	//if (MASTER(cr)) fprintf(stderr,"atom %d f %f %f %f \n",
	//                       i,(*junk)[0],(*junk)[1],(*junk)[2]);
	pot->bWpot_ener = wpot_ener_temp;
      }
      rvec_inc(f[i],*junk);
    }
  }
  
  if (pot->bWpot_ener){
#ifdef GMX_DOUBLE
    MPI_Allreduce(&(pot->Wpotener),&(pot->Wpot_ener_tot),1,MPI_DOUBLE,MPI_SUM,cr->mpi_comm_mygroup);
#else
    MPI_Allreduce(&(pot->Wpotener),&(pot->Wpot_ener_tot),1,MPI_REAL,MPI_SUM,cr->mpi_comm_mygroup);
#endif
  }
  
  if (MASTER(cr) && pot->bWpot_ener && step%2000 == 0) {
    FILE* fener = fopen("Wpot_ener.dat","a");
    fprintf(fener,"%ld\t%lf\n",step,pot->Wpot_ener_tot);
    fclose(fener);
  }
  
  
  // vtk check - writes .vtk file readable by visIt, Paraview and more...
  if(pot->bVtk_check && step%pot->nsteps_vtk == 0) {
    
    FILE* vtkout; char outpname[STRLEN+1];
    char procletter = 'a';
    procletter += (char)cr->sim_nodeid;
    sprintf(outpname,"distance_%c%c%c_wpot00%ld.vtk",procletter,procletter,procletter,(long)step/pot->nsteps_vtk);
    vtkout = fopen(outpname,"w");
    
    srenew(junk,state->natoms);
    for(i=md->start;i<md->start+md->homenr;i++) {
      pot->dist[i] = compute_single_force(pot,&(state->x[i]),&(junk[i]),f[i]);
    }
    rvec *zero; snew(zero,1); for(i=0;i<3;i++) (*zero)[i]=0.;
    write_field_and_scalar_vtk(vtkout,md->homenr,state->x+md->start,pot->dist+md->start,junk,zero);
    sfree(zero);
  }
  sfree(junk);sfree(junk2);
}

void finalize_Wpot(t_user_potential *pot) {
  
  int i,j;
  
  sfree(pot->center);
  sfree(pot->sols);
  sfree(pot->lips);
  
  if (pot->type == wpt_MB || pot->type == wpt_SPMB || pot->type==wpt_SPMBH) {
    sfree(pot->Surf);
    sfree(pot->index);
    sfree(pot->index2);
    sfree(pot->Surf_atoms);
    sfree(pot->sgs->zi);  
    for (i=0;i<pot->sgs->nz;i++) {
      sfree(pot->sgs->yi[i]);
      for(j=0;j<pot->sgs->ny;j++) {
        sfree(pot->sgs->xi[i][j]);
      }
      sfree(pot->sgs->xi[i]);
    }
    sfree(pot->sgs->xi);
    sfree(pot->sgs->yi);
    if (pot->bAtoms_GS) {
      sfree(pot->sags->zi);  
      for (i=0;i<pot->sags->nz;i++) {
        sfree(pot->sags->yi[i]);
        for(j=0;j<pot->sags->ny;j++) {
	  sfree(pot->sags->xi[i][j]);
        }
        sfree(pot->sags->xi[i]);
      }
      sfree(pot->sags->xi);
      sfree(pot->sags->yi);
    }
  } 
  
  sfree(pot->dist);
  
}
