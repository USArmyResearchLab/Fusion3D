#include "internals.h"

// ***************************************
/// Constructor.
// ***************************************
jfd_transform::jfd_transform()

{
   int i, j;
   
   // Initialize unity matrix
   for (i=0; i<4; i++) {
      for (j=0; j<4; j++) {
         if (i == j) {
            identity[i] [j] = 1.;
            curmat  [i] [j] = 1.;
         }
         else {
            identity[i] [j] = 0.;
            curmat  [i] [j] = 0.;
         }
      }
   }
} 

// *************************************************************
/// Reset tranformation to unity transform.
// *************************************************************

int
jfd_transform::reset()
{
   int i, j;
   
   // Initialize unity matrix
   for (i=0; i<4; i++) {
      for (j=0; j<4; j++) {
         if (i == j) {
            identity[i] [j] = 1.;
            curmat  [i] [j] = 1.;
         }
         else {
            identity[i] [j] = 0.;
            curmat  [i] [j] = 0.;
         }
      }
   }
   return(1);
} 
   
// *************************************************************
/// Destructor.
// *************************************************************
   
jfd_transform::~jfd_transform()

{
}

// *************************************************************
/// Rotate ang degrees around the x-axis.
// *************************************************************
   
int jfd_transform::rotate_x(float ang)

{
   Matrix premat, newmat;
   float angrad, cosang, sinang;
   int i, j;
   
   angrad = 3.14159f * ang /180.f;
   cosang = cos(angrad);
   sinang = sin(angrad);
   
   for (i=0; i<4; i++) {
      for (j=0; j<4; j++) {
         premat[i] [j] = curmat[i] [j];
         newmat[i] [j] = 0.;
      }
   }
   newmat[1] [1] =  cosang;
   newmat[1] [2] =  sinang;
   newmat[2] [1] = -sinang;
   newmat[2] [2] =  cosang;
   newmat[0] [0] = 1.;
   newmat[3] [3] = 1.;
   
   curmat[0][0] = newmat[0][0]*premat[0][0] + newmat[0][1]*premat[1][0]
                + newmat[0][2]*premat[2][0] + newmat[0][3]*premat[3][0];
   curmat[0][1] = newmat[0][0]*premat[0][1] + newmat[0][1]*premat[1][1]
                + newmat[0][2]*premat[2][1] + newmat[0][3]*premat[3][1];
   curmat[0][2] = newmat[0][0]*premat[0][2] + newmat[0][1]*premat[1][2]
                + newmat[0][2]*premat[2][2] + newmat[0][3]*premat[3][2];
   curmat[0][3] = newmat[0][0]*premat[0][3] + newmat[0][1]*premat[1][3]
                + newmat[0][2]*premat[2][3] + newmat[0][3]*premat[3][3];
                
   curmat[1][0] = newmat[1][0]*premat[0][0] + newmat[1][1]*premat[1][0]
                + newmat[1][2]*premat[2][0] + newmat[1][3]*premat[3][0];
   curmat[1][1] = newmat[1][0]*premat[0][1] + newmat[1][1]*premat[1][1]
                + newmat[1][2]*premat[2][1] + newmat[1][3]*premat[3][1];
   curmat[1][2] = newmat[1][0]*premat[0][2] + newmat[1][1]*premat[1][2]
                + newmat[1][2]*premat[2][2] + newmat[1][3]*premat[3][2];
   curmat[1][3] = newmat[1][0]*premat[0][3] + newmat[1][1]*premat[1][3]
                + newmat[1][2]*premat[2][3] + newmat[1][3]*premat[3][3];

   curmat[2][0] = newmat[2][0]*premat[0][0] + newmat[2][1]*premat[1][0]
                + newmat[2][2]*premat[2][0] + newmat[2][3]*premat[3][0];
   curmat[2][1] = newmat[2][0]*premat[0][1] + newmat[2][1]*premat[1][1]
                + newmat[2][2]*premat[2][1] + newmat[2][3]*premat[3][1];
   curmat[2][2] = newmat[2][0]*premat[0][2] + newmat[2][1]*premat[1][2]
                + newmat[2][2]*premat[2][2] + newmat[2][3]*premat[3][2];
   curmat[2][3] = newmat[2][0]*premat[0][3] + newmat[2][1]*premat[1][3]
                + newmat[2][2]*premat[2][3] + newmat[2][3]*premat[3][3];

   curmat[3][0] = newmat[3][0]*premat[0][0] + newmat[3][1]*premat[1][0]
                + newmat[3][2]*premat[2][0] + newmat[3][3]*premat[3][0];
   curmat[3][1] = newmat[3][0]*premat[0][1] + newmat[3][1]*premat[1][1]
                + newmat[3][2]*premat[2][1] + newmat[3][3]*premat[3][1];
   curmat[3][2] = newmat[3][0]*premat[0][2] + newmat[3][1]*premat[1][2]
                + newmat[3][2]*premat[2][2] + newmat[3][3]*premat[3][2];
   curmat[3][3] = newmat[3][0]*premat[0][3] + newmat[3][1]*premat[1][3]
                + newmat[3][2]*premat[2][3] + newmat[3][3]*premat[3][3];

   return(1);               
}

// *************************************************************
/// Rotate ang degrees around the y-axis.
// *************************************************************
   
int jfd_transform::rotate_y(float ang)

{
   Matrix premat, newmat;
   float angrad, cosang, sinang;
   int i, j;
   
   angrad = 3.14159f * ang /180.f;
   cosang = cos(angrad);
   sinang = sin(angrad);
   
   for (i=0; i<4; i++) {
      for (j=0; j<4; j++) {
         premat[i] [j] = curmat[i] [j];
         newmat[i] [j] = 0.;
      }
   }
   newmat[0] [0] =  cosang;
   newmat[0] [2] = -sinang;
   newmat[2] [0] =  sinang;
   newmat[2] [2] =  cosang;
   newmat[1] [1] = 1.;
   newmat[3] [3] = 1.;
   
   curmat[0][0] = newmat[0][0]*premat[0][0] + newmat[0][1]*premat[1][0]
                + newmat[0][2]*premat[2][0] + newmat[0][3]*premat[3][0];
   curmat[0][1] = newmat[0][0]*premat[0][1] + newmat[0][1]*premat[1][1]
                + newmat[0][2]*premat[2][1] + newmat[0][3]*premat[3][1];
   curmat[0][2] = newmat[0][0]*premat[0][2] + newmat[0][1]*premat[1][2]
                + newmat[0][2]*premat[2][2] + newmat[0][3]*premat[3][2];
   curmat[0][3] = newmat[0][0]*premat[0][3] + newmat[0][1]*premat[1][3]
                + newmat[0][2]*premat[2][3] + newmat[0][3]*premat[3][3];
                
   curmat[1][0] = newmat[1][0]*premat[0][0] + newmat[1][1]*premat[1][0]
                + newmat[1][2]*premat[2][0] + newmat[1][3]*premat[3][0];
   curmat[1][1] = newmat[1][0]*premat[0][1] + newmat[1][1]*premat[1][1]
                + newmat[1][2]*premat[2][1] + newmat[1][3]*premat[3][1];
   curmat[1][2] = newmat[1][0]*premat[0][2] + newmat[1][1]*premat[1][2]
                + newmat[1][2]*premat[2][2] + newmat[1][3]*premat[3][2];
   curmat[1][3] = newmat[1][0]*premat[0][3] + newmat[1][1]*premat[1][3]
                + newmat[1][2]*premat[2][3] + newmat[1][3]*premat[3][3];

   curmat[2][0] = newmat[2][0]*premat[0][0] + newmat[2][1]*premat[1][0]
                + newmat[2][2]*premat[2][0] + newmat[2][3]*premat[3][0];
   curmat[2][1] = newmat[2][0]*premat[0][1] + newmat[2][1]*premat[1][1]
                + newmat[2][2]*premat[2][1] + newmat[2][3]*premat[3][1];
   curmat[2][2] = newmat[2][0]*premat[0][2] + newmat[2][1]*premat[1][2]
                + newmat[2][2]*premat[2][2] + newmat[2][3]*premat[3][2];
   curmat[2][3] = newmat[2][0]*premat[0][3] + newmat[2][1]*premat[1][3]
                + newmat[2][2]*premat[2][3] + newmat[2][3]*premat[3][3];

   curmat[3][0] = newmat[3][0]*premat[0][0] + newmat[3][1]*premat[1][0]
                + newmat[3][2]*premat[2][0] + newmat[3][3]*premat[3][0];
   curmat[3][1] = newmat[3][0]*premat[0][1] + newmat[3][1]*premat[1][1]
                + newmat[3][2]*premat[2][1] + newmat[3][3]*premat[3][1];
   curmat[3][2] = newmat[3][0]*premat[0][2] + newmat[3][1]*premat[1][2]
                + newmat[3][2]*premat[2][2] + newmat[3][3]*premat[3][2];
   curmat[3][3] = newmat[3][0]*premat[0][3] + newmat[3][1]*premat[1][3]
                + newmat[3][2]*premat[2][3] + newmat[3][3]*premat[3][3];

   return(1);               
}

// *************************************************************
/// Rotate ang degrees around the z-axis.
// *************************************************************
   
int jfd_transform::rotate_z(float ang)

{
   Matrix premat, newmat;
   float angrad, cosang, sinang;
   int i, j;
   
   angrad = 3.14159f * ang /180.f;
   cosang = cos(angrad);
   sinang = sin(angrad);
   
   for (i=0; i<4; i++) {
      for (j=0; j<4; j++) {
         premat[i] [j] = curmat[i] [j];
         newmat[i] [j] = 0.;
      }
   }
   newmat[0] [0] =  cosang;
   newmat[0] [1] =  sinang;
   newmat[1] [0] = -sinang;
   newmat[1] [1] =  cosang;
   newmat[2] [2] = 1.;
   newmat[3] [3] = 1.;
   
   curmat[0][0] = newmat[0][0]*premat[0][0] + newmat[0][1]*premat[1][0]
                + newmat[0][2]*premat[2][0] + newmat[0][3]*premat[3][0];
   curmat[0][1] = newmat[0][0]*premat[0][1] + newmat[0][1]*premat[1][1]
                + newmat[0][2]*premat[2][1] + newmat[0][3]*premat[3][1];
   curmat[0][2] = newmat[0][0]*premat[0][2] + newmat[0][1]*premat[1][2]
                + newmat[0][2]*premat[2][2] + newmat[0][3]*premat[3][2];
   curmat[0][3] = newmat[0][0]*premat[0][3] + newmat[0][1]*premat[1][3]
                + newmat[0][2]*premat[2][3] + newmat[0][3]*premat[3][3];
                
   curmat[1][0] = newmat[1][0]*premat[0][0] + newmat[1][1]*premat[1][0]
                + newmat[1][2]*premat[2][0] + newmat[1][3]*premat[3][0];
   curmat[1][1] = newmat[1][0]*premat[0][1] + newmat[1][1]*premat[1][1]
                + newmat[1][2]*premat[2][1] + newmat[1][3]*premat[3][1];
   curmat[1][2] = newmat[1][0]*premat[0][2] + newmat[1][1]*premat[1][2]
                + newmat[1][2]*premat[2][2] + newmat[1][3]*premat[3][2];
   curmat[1][3] = newmat[1][0]*premat[0][3] + newmat[1][1]*premat[1][3]
                + newmat[1][2]*premat[2][3] + newmat[1][3]*premat[3][3];

   curmat[2][0] = newmat[2][0]*premat[0][0] + newmat[2][1]*premat[1][0]
                + newmat[2][2]*premat[2][0] + newmat[2][3]*premat[3][0];
   curmat[2][1] = newmat[2][0]*premat[0][1] + newmat[2][1]*premat[1][1]
                + newmat[2][2]*premat[2][1] + newmat[2][3]*premat[3][1];
   curmat[2][2] = newmat[2][0]*premat[0][2] + newmat[2][1]*premat[1][2]
                + newmat[2][2]*premat[2][2] + newmat[2][3]*premat[3][2];
   curmat[2][3] = newmat[2][0]*premat[0][3] + newmat[2][1]*premat[1][3]
                + newmat[2][2]*premat[2][3] + newmat[2][3]*premat[3][3];

   curmat[3][0] = newmat[3][0]*premat[0][0] + newmat[3][1]*premat[1][0]
                + newmat[3][2]*premat[2][0] + newmat[3][3]*premat[3][0];
   curmat[3][1] = newmat[3][0]*premat[0][1] + newmat[3][1]*premat[1][1]
                + newmat[3][2]*premat[2][1] + newmat[3][3]*premat[3][1];
   curmat[3][2] = newmat[3][0]*premat[0][2] + newmat[3][1]*premat[1][2]
                + newmat[3][2]*premat[2][2] + newmat[3][3]*premat[3][2];
   curmat[3][3] = newmat[3][0]*premat[0][3] + newmat[3][1]*premat[1][3]
                + newmat[3][2]*premat[2][3] + newmat[3][3]*premat[3][3];

   return(1);               
}

// *************************************************************
/// Translate.
// *************************************************************
   
int jfd_transform::transl(float xt, float yt, float zt)
{
   Matrix premat, newmat;
   int i, j;
   
   for (i=0; i<4; i++) {
      for (j=0; j<4; j++) {
         premat[i] [j] = curmat[i] [j];
         newmat[i] [j] = 0.;
      }
   }
   newmat[3] [0] = xt;
   newmat[3] [1] = yt;
   newmat[3] [2] = zt;
   newmat[0] [0] = 1.;
   newmat[1] [1] = 1.;
   newmat[2] [2] = 1.;
   newmat[3] [3] = 1.;
   
   curmat[0][0] = newmat[0][0]*premat[0][0] + newmat[0][1]*premat[1][0]
                + newmat[0][2]*premat[2][0] + newmat[0][3]*premat[3][0];
   curmat[0][1] = newmat[0][0]*premat[0][1] + newmat[0][1]*premat[1][1]
                + newmat[0][2]*premat[2][1] + newmat[0][3]*premat[3][1];
   curmat[0][2] = newmat[0][0]*premat[0][2] + newmat[0][1]*premat[1][2]
                + newmat[0][2]*premat[2][2] + newmat[0][3]*premat[3][2];
   curmat[0][3] = newmat[0][0]*premat[0][3] + newmat[0][1]*premat[1][3]
                + newmat[0][2]*premat[2][3] + newmat[0][3]*premat[3][3];
                
   curmat[1][0] = newmat[1][0]*premat[0][0] + newmat[1][1]*premat[1][0]
                + newmat[1][2]*premat[2][0] + newmat[1][3]*premat[3][0];
   curmat[1][1] = newmat[1][0]*premat[0][1] + newmat[1][1]*premat[1][1]
                + newmat[1][2]*premat[2][1] + newmat[1][3]*premat[3][1];
   curmat[1][2] = newmat[1][0]*premat[0][2] + newmat[1][1]*premat[1][2]
                + newmat[1][2]*premat[2][2] + newmat[1][3]*premat[3][2];
   curmat[1][3] = newmat[1][0]*premat[0][3] + newmat[1][1]*premat[1][3]
                + newmat[1][2]*premat[2][3] + newmat[1][3]*premat[3][3];

   curmat[2][0] = newmat[2][0]*premat[0][0] + newmat[2][1]*premat[1][0]
                + newmat[2][2]*premat[2][0] + newmat[2][3]*premat[3][0];
   curmat[2][1] = newmat[2][0]*premat[0][1] + newmat[2][1]*premat[1][1]
                + newmat[2][2]*premat[2][1] + newmat[2][3]*premat[3][1];
   curmat[2][2] = newmat[2][0]*premat[0][2] + newmat[2][1]*premat[1][2]
                + newmat[2][2]*premat[2][2] + newmat[2][3]*premat[3][2];
   curmat[2][3] = newmat[2][0]*premat[0][3] + newmat[2][1]*premat[1][3]
                + newmat[2][2]*premat[2][3] + newmat[2][3]*premat[3][3];

   curmat[3][0] = newmat[3][0]*premat[0][0] + newmat[3][1]*premat[1][0]
                + newmat[3][2]*premat[2][0] + newmat[3][3]*premat[3][0];
   curmat[3][1] = newmat[3][0]*premat[0][1] + newmat[3][1]*premat[1][1]
                + newmat[3][2]*premat[2][1] + newmat[3][3]*premat[3][1];
   curmat[3][2] = newmat[3][0]*premat[0][2] + newmat[3][1]*premat[1][2]
                + newmat[3][2]*premat[2][2] + newmat[3][3]*premat[3][2];
   curmat[3][3] = newmat[3][0]*premat[0][3] + newmat[3][1]*premat[1][3]
                + newmat[3][2]*premat[2][3] + newmat[3][3]*premat[3][3];

   return(1);
}


// *************************************************************
/// Scale.
// *************************************************************
   
int jfd_transform::scale(float xs, float ys, float zs)
{
   Matrix premat, newmat;
   int i, j;
   
   for (i=0; i<4; i++) {
      for (j=0; j<4; j++) {
         premat[i] [j] = curmat[i] [j];
         newmat[i] [j] = 0.;
      }
   }
   newmat[0] [0] = xs;
   newmat[1] [1] = ys;
   newmat[2] [2] = zs;
   newmat[3] [3] = 1.;
   
   curmat[0][0] = newmat[0][0]*premat[0][0] + newmat[0][1]*premat[1][0]
                + newmat[0][2]*premat[2][0] + newmat[0][3]*premat[3][0];
   curmat[0][1] = newmat[0][0]*premat[0][1] + newmat[0][1]*premat[1][1]
                + newmat[0][2]*premat[2][1] + newmat[0][3]*premat[3][1];
   curmat[0][2] = newmat[0][0]*premat[0][2] + newmat[0][1]*premat[1][2]
                + newmat[0][2]*premat[2][2] + newmat[0][3]*premat[3][2];
   curmat[0][3] = newmat[0][0]*premat[0][3] + newmat[0][1]*premat[1][3]
                + newmat[0][2]*premat[2][3] + newmat[0][3]*premat[3][3];
                
   curmat[1][0] = newmat[1][0]*premat[0][0] + newmat[1][1]*premat[1][0]
                + newmat[1][2]*premat[2][0] + newmat[1][3]*premat[3][0];
   curmat[1][1] = newmat[1][0]*premat[0][1] + newmat[1][1]*premat[1][1]
                + newmat[1][2]*premat[2][1] + newmat[1][3]*premat[3][1];
   curmat[1][2] = newmat[1][0]*premat[0][2] + newmat[1][1]*premat[1][2]
                + newmat[1][2]*premat[2][2] + newmat[1][3]*premat[3][2];
   curmat[1][3] = newmat[1][0]*premat[0][3] + newmat[1][1]*premat[1][3]
                + newmat[1][2]*premat[2][3] + newmat[1][3]*premat[3][3];

   curmat[2][0] = newmat[2][0]*premat[0][0] + newmat[2][1]*premat[1][0]
                + newmat[2][2]*premat[2][0] + newmat[2][3]*premat[3][0];
   curmat[2][1] = newmat[2][0]*premat[0][1] + newmat[2][1]*premat[1][1]
                + newmat[2][2]*premat[2][1] + newmat[2][3]*premat[3][1];
   curmat[2][2] = newmat[2][0]*premat[0][2] + newmat[2][1]*premat[1][2]
                + newmat[2][2]*premat[2][2] + newmat[2][3]*premat[3][2];
   curmat[2][3] = newmat[2][0]*premat[0][3] + newmat[2][1]*premat[1][3]
                + newmat[2][2]*premat[2][3] + newmat[2][3]*premat[3][3];

   curmat[3][0] = newmat[3][0]*premat[0][0] + newmat[3][1]*premat[1][0]
                + newmat[3][2]*premat[2][0] + newmat[3][3]*premat[3][0];
   curmat[3][1] = newmat[3][0]*premat[0][1] + newmat[3][1]*premat[1][1]
                + newmat[3][2]*premat[2][1] + newmat[3][3]*premat[3][1];
   curmat[3][2] = newmat[3][0]*premat[0][2] + newmat[3][1]*premat[1][2]
                + newmat[3][2]*premat[2][2] + newmat[3][3]*premat[3][2];
   curmat[3][3] = newmat[3][0]*premat[0][3] + newmat[3][1]*premat[1][3]
                + newmat[3][2]*premat[2][3] + newmat[3][3]*premat[3][3];

   return(1);
}

// *************************************************************
/// Get the resulting composite matrix
// *************************************************************
   
int jfd_transform::get(Matrix& out_matrix)

{
   int i, j;
   
   for (i=0; i<4; i++) {
      for (j=0; j<4; j++) {
         out_matrix[i] [j] = curmat[i] [j];
      }
   }
   return(1);
}

// *************************************************************
///  Set matrix for a SAR rotation -- shortcut.
// *************************************************************
int jfd_transform::sar_rotation(float azim, float elev, float sqint)

{
   
   float tanelv, sinsqi, brerad, bre;
   float coselv, cossqi, conrad, con, pocrot;
   float azt;
   Matrix modmat;
   
   azt = azim + sqint - 90.f;
   tanelv = tan(3.14159f*elev/180.f);
   sinsqi = sin(3.14159f*sqint/180.f);
   brerad = atan2(tanelv,sinsqi);
   bre    = 180.f*brerad/3.14159f;
   coselv = cos(3.14159f*elev/180.f);
   cossqi = cos(3.14159f*sqint/180.f);
   conrad = acos(cossqi*coselv);
   con    = 180.f*conrad/3.14159f;
   pocrot = con - 90.f;
   
   cout << "movie_xform_class::sar_rotation  az " << azim << ", el" << elev <<
          " squint " << sqint << " ss " << con << endl;
   reset();
   rotate_z(180.);
   rotate_z(pocrot);
   rotate_y(bre);
   rotate_z(-azt);
   get(modmat);
   return(1);
}
 

