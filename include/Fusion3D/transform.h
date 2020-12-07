#ifndef _jfd_transform_h_
#define _jfd_transform_h_
#ifdef __cplusplus


/**

Does coordinate transformations SGI GL style.

   Coordinate transformations\n
   \n
   Author:  J.F. Dammann\n
   Revision 2.0\n
   Date:  94/03/11\n
   \n
   Transformations are SGI GL style. Homogeneous coordinates are used and
   a single 4x4 transformation matrix is maintained.  Compositing matrices is
   done by pre-multiplying the new matrix onto the existing matrix:\n
   \n
   	[ new ] [ pre-existing] = [ result ]\n
   	\n
   This matrix is used to generate a new point (x',y',z') by\n
   \n
   	[x,y,z,w] = [ 4x4 matrix[row][col] ] = [x',y',z',w']\n
   	\n
    xt = matrix[0][0]*x[i] + matrix[1][0]*y[i] + matrix[2][0]*z[i] + matrix[3][0];\n
    yt = matrix[0][1]*x[i] + matrix[1][1]*y[i] + matrix[2][1]*z[i] + matrix[3][1];\n
    zt = matrix[0][2]*x[i] + matrix[1][2]*y[i] + matrix[2][2]*z[i] + matrix[3][2];\n
   \n
   See Appendix C of Graphics Library Programming Guide for trans. matrices\n
   \n
   Notes	
   	scale, translate and rotate should all composit properly\n
   	The SGI matrix stack has not yet been implemented.\n
   
*/
class jfd_transform{
   private:
      Matrix identity;
      Matrix curmat;
   
   public:
      jfd_transform();
      ~jfd_transform();
      int reset();
      int rotate_x(float ang);	// Angle in deg
      int rotate_y(float ang); 	// Angle in deg
      int rotate_z(float ang); 	// Angle in deg
      int transl(float xt, float yt, float zt);
      int scale(float xs, float ys, float zs);
      int get(Matrix& out_matrix);
      
      // Set transform matrix to sar rotation
      int sar_rotation(float azim, float elev, float sqint);
};

#endif /* __cplusplus */
#endif /* _transform_h_ */
