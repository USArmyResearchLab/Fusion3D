#include "internals.h"

// *******************************************
/// Constructor.
// ********************************************
image_transformation_class :: image_transformation_class()
{
}

// *******************************************
/// Destructor.
// ********************************************
image_transformation_class :: ~image_transformation_class()
{
}

// *******************************************
/// Flip the image in the y dimension.
// ********************************************
int image_transformation_class :: flip_vertical(int n_rows, 
						int n_cols,
						unsigned char *buf)
{
    unsigned char *top_buf_ptr;
    unsigned char *bottom_buf_ptr;
    unsigned char temp_buf;
    int i,j;

    int last_row = n_rows/2;
    for (i=0; i<last_row; i++) {
       top_buf_ptr = buf + i * n_cols;
       bottom_buf_ptr = buf + (n_rows - i - 1) * n_cols;
       for (j=0; j<n_cols; j++, top_buf_ptr++, bottom_buf_ptr++) {
	  temp_buf = *top_buf_ptr;
	  *top_buf_ptr = *bottom_buf_ptr;
	  *bottom_buf_ptr = temp_buf;
       }
    }

    return TRUE;
}


// *******************************************
/// Flip the image in the y dimension.
// ********************************************
int image_transformation_class :: flip_vertical(int n_rows, 
						int n_cols,
						unsigned short *buf)
{
    unsigned short *top_buf_ptr;
    unsigned short *bottom_buf_ptr;
    unsigned short temp_buf;
    int i,j;

    int last_row = n_rows/2;
    for (i=0; i<last_row; i++) {
       top_buf_ptr = buf + i * n_cols;
       bottom_buf_ptr = buf + (n_rows - i - 1) * n_cols;
       for (j=0; j<n_cols; j++, top_buf_ptr++, bottom_buf_ptr++) {
	  temp_buf = *top_buf_ptr;
	  *top_buf_ptr = *bottom_buf_ptr;
	  *bottom_buf_ptr = temp_buf;
       }
    }

    return TRUE;
}


// *******************************************
/// Flip the image in the y dimension.
// ********************************************
int image_transformation_class :: flip_vertical(int n_rows, 
						int n_cols,
						int *buf)
{
    int *top_buf_ptr;
    int *bottom_buf_ptr;
    int temp_buf;
    int i,j;

    int last_row = n_rows/2;
    for (i=0; i<last_row; i++) {
       top_buf_ptr = buf + i * n_cols;
       bottom_buf_ptr = buf + (n_rows - i - 1) * n_cols;
       for (j=0; j<n_cols; j++, top_buf_ptr++, bottom_buf_ptr++) {
	  temp_buf = *top_buf_ptr;
	  *top_buf_ptr = *bottom_buf_ptr;
	  *bottom_buf_ptr = temp_buf;
       }
    }

    return TRUE;
}


// *******************************************
/// Flip the image in the y dimension.
// ********************************************
int image_transformation_class :: flip_vertical(int n_rows, 
						int n_cols,
						float *buf)
{
    float *top_buf_ptr;
    float *bottom_buf_ptr;
    float temp_buf;
    int i,j;

    int last_row = n_rows/2;
    for (i=0; i<last_row; i++) {
       top_buf_ptr = buf + i * n_cols;
       bottom_buf_ptr = buf + (n_rows - i - 1) * n_cols;
       for (j=0; j<n_cols; j++, top_buf_ptr++, bottom_buf_ptr++) {
	  temp_buf = *top_buf_ptr;
	  *top_buf_ptr = *bottom_buf_ptr;
	  *bottom_buf_ptr = temp_buf;
       }
    }

    return TRUE;
}



// *******************************************
/// Flip the image in the x dimension.
// ********************************************
int image_transformation_class :: flip_horizontal(int n_rows,
						  int n_cols,
						  unsigned char *buf)
{
    unsigned char *right_buf_ptr;
    unsigned char *left_buf_ptr;
    unsigned char temp_buf;

    int i, j;
    int last_col = n_cols/2;
    for (i=0; i<n_rows; i++) {
       right_buf_ptr = buf + i * n_cols;
       left_buf_ptr  = right_buf_ptr + n_cols - 1;
       for (j=0; j<last_col; j++, right_buf_ptr++, left_buf_ptr--) {
	   temp_buf = *right_buf_ptr;
	   *right_buf_ptr = *left_buf_ptr;
	   *left_buf_ptr = temp_buf;
       }
    }

    return TRUE;

}



// *******************************************
/// Flip the image in the x dimension.
// ********************************************
int image_transformation_class :: flip_horizontal(int n_rows,
						  int n_cols,
						  unsigned short *buf)
{
    unsigned short *right_buf_ptr;
    unsigned short *left_buf_ptr;
    unsigned short temp_buf;

    int i, j;
    int last_col = n_cols/2;
    for (i=0; i<n_rows; i++) {
       right_buf_ptr = buf + i * n_cols;
       left_buf_ptr  = right_buf_ptr + n_cols - 1;
       for (j=0; j<last_col; j++, right_buf_ptr++, left_buf_ptr--) {
	   temp_buf = *right_buf_ptr;
	   *right_buf_ptr = *left_buf_ptr;
	   *left_buf_ptr = temp_buf;
       }
    }

    return TRUE;

}



// *******************************************
/// Flip the image in the x dimension.
// ********************************************
int image_transformation_class :: flip_horizontal(int n_rows,
						  int n_cols,
						  int *buf)
{
    int *right_buf_ptr;
    int *left_buf_ptr;
    int temp_buf;

    int i, j;
    int last_col = n_cols/2;
    for (i=0; i<n_rows; i++) {
       right_buf_ptr = buf + i * n_cols;
       left_buf_ptr  = right_buf_ptr + n_cols - 1;
       for (j=0; j<last_col; j++, right_buf_ptr++, left_buf_ptr--) {
	   temp_buf = *right_buf_ptr;
	   *right_buf_ptr = *left_buf_ptr;
	   *left_buf_ptr = temp_buf;
       }
    }

    return TRUE;

}



// *******************************************
/// Flip the image in the x dimension.
// ********************************************
int image_transformation_class :: flip_horizontal(int n_rows,
						  int n_cols,
						  float *buf)
{
    float *right_buf_ptr;
    float *left_buf_ptr;
    float temp_buf;

    int i, j;
    int last_col = n_cols/2;
    for (i=0; i<n_rows; i++) {
       right_buf_ptr = buf + i * n_cols;
       left_buf_ptr  = right_buf_ptr + n_cols - 1;
       for (j=0; j<last_col; j++, right_buf_ptr++, left_buf_ptr--) {
	   temp_buf = *right_buf_ptr;
	   *right_buf_ptr = *left_buf_ptr;
	   *left_buf_ptr = temp_buf;
       }
    }

    return TRUE;

}


// *******************************************
/// Rotate the image 90 deg ccw.
// ********************************************
int image_transformation_class :: rotate_ccw(int n_rows_orig,
					     int n_cols_orig,
					     unsigned char *buf)

{

    unsigned char *temp_buf = new unsigned char[n_rows_orig * n_cols_orig];

    int i,j;

    unsigned char *orig_buf_ptr = buf;
    for (i=0; i< n_rows_orig; i++) {
      for (j=0; j < n_cols_orig; j++, orig_buf_ptr++) {
	 temp_buf[(n_cols_orig-1-j) * n_rows_orig + i] = *orig_buf_ptr;
      }
    }

    unsigned char *new_buf_ptr = temp_buf;
    orig_buf_ptr = buf;
    for (i=0; i < n_rows_orig; i++) {
      for (j=0; j < n_cols_orig; j++, new_buf_ptr++, orig_buf_ptr++) {
	 *orig_buf_ptr = *new_buf_ptr;
      }
    }

    delete[] temp_buf;

    return TRUE;

}

// *******************************************
/// Rotate the image 90 deg ccw.
// ********************************************
int image_transformation_class :: rotate_ccw(int n_rows_orig,
					     int n_cols_orig,
					     unsigned short *buf)

{

    unsigned short *temp_buf = new unsigned short[n_rows_orig * n_cols_orig];

    int i,j;

    unsigned short *orig_buf_ptr = buf;
    for (i=0; i< n_rows_orig; i++) {
      for (j=0; j < n_cols_orig; j++, orig_buf_ptr++) {
	 temp_buf[(n_cols_orig-1-j) * n_rows_orig + i] = *orig_buf_ptr;
      }
    }

    unsigned short *new_buf_ptr = temp_buf;
    orig_buf_ptr = buf;
    for (i=0; i < n_rows_orig; i++) {
      for (j=0; j < n_cols_orig; j++, new_buf_ptr++, orig_buf_ptr++) {
	 *orig_buf_ptr = *new_buf_ptr;
      }
    }

    delete[] temp_buf;
    return TRUE;
}

// *******************************************
/// Rotate the image 90 deg ccw.
// ********************************************
int image_transformation_class :: rotate_ccw(int n_rows_orig,
					     int n_cols_orig,
					     int *buf)

{

    int *temp_buf = new int[n_rows_orig * n_cols_orig];

    int i,j;

    int *orig_buf_ptr = buf;
    for (i=0; i< n_rows_orig; i++) {
      for (j=0; j < n_cols_orig; j++, orig_buf_ptr++) {
	 temp_buf[(n_cols_orig-1-j) * n_rows_orig + i] = *orig_buf_ptr;
      }
    }

    int *new_buf_ptr = temp_buf;
    orig_buf_ptr = buf;
    for (i=0; i < n_rows_orig; i++) {
      for (j=0; j < n_cols_orig; j++, new_buf_ptr++, orig_buf_ptr++) {
	 *orig_buf_ptr = *new_buf_ptr;
      }
    }

    delete[] temp_buf;
    return TRUE;
}

// *******************************************
/// Rotate the image 90 deg ccw.
// ********************************************
int image_transformation_class :: rotate_ccw(int n_rows_orig,
					     int n_cols_orig,
					     float *buf)

{

    float *temp_buf = new float[n_rows_orig * n_cols_orig];

    int i,j;

    float *orig_buf_ptr = buf;
    for (i=0; i< n_rows_orig; i++) {
      for (j=0; j < n_cols_orig; j++, orig_buf_ptr++) {
	 temp_buf[(n_cols_orig-1-j) * n_rows_orig + i] = *orig_buf_ptr;
      }
    }

    float *new_buf_ptr = temp_buf;
    orig_buf_ptr = buf;
    for (i=0; i < n_rows_orig; i++) {
      for (j=0; j < n_cols_orig; j++, new_buf_ptr++, orig_buf_ptr++) {
	 *orig_buf_ptr = *new_buf_ptr;
      }
    }

    delete[] temp_buf;
    return TRUE;
}


// *******************************************
/// Rotate the image 90 deg cw.
// ********************************************
int image_transformation_class :: rotate_cw(int n_rows_orig,
					    int n_cols_orig,
					    unsigned char *buf)

{

    unsigned char *temp_buf = new unsigned char[n_rows_orig * n_cols_orig];

    int i,j;

    unsigned char *orig_buf_ptr = buf;
    for (i=0; i< n_rows_orig; i++) {
      for (j=0; j < n_cols_orig; j++, orig_buf_ptr++) {
	 temp_buf[j * n_rows_orig + n_rows_orig - 1 - i] = *orig_buf_ptr;
      }
    }

    unsigned char *new_buf_ptr = temp_buf;
    orig_buf_ptr = buf;
    for (i=0; i < n_rows_orig; i++) {
      for (j=0; j < n_cols_orig; j++, new_buf_ptr++, orig_buf_ptr++) {
	 *orig_buf_ptr = *new_buf_ptr;
      }
    }

    delete[] temp_buf;

    return TRUE;

}

// *******************************************
/// Rotate the image 90 deg cw.
// ********************************************
int image_transformation_class :: rotate_cw(int n_rows_orig,
					    int n_cols_orig,
					    unsigned short *buf)

{

    unsigned short *temp_buf = new unsigned short[n_rows_orig * n_cols_orig];

    int i,j;

    unsigned short *orig_buf_ptr = buf;
    for (i=0; i< n_rows_orig; i++) {
      for (j=0; j < n_cols_orig; j++, orig_buf_ptr++) {
	 temp_buf[j * n_rows_orig + n_rows_orig - 1 - i] = *orig_buf_ptr;
      }
    }

    unsigned short *new_buf_ptr = temp_buf;
    orig_buf_ptr = buf;
    for (i=0; i < n_rows_orig; i++) {
      for (j=0; j < n_cols_orig; j++, new_buf_ptr++, orig_buf_ptr++) {
	 *orig_buf_ptr = *new_buf_ptr;
      }
    }

    delete[] temp_buf;
    return TRUE;
}

// *******************************************
/// Rotate the image 90 deg cw.
// ********************************************
int image_transformation_class :: rotate_cw(int n_rows_orig,
					    int n_cols_orig,
					    int *buf)

{

    int *temp_buf = new int[n_rows_orig * n_cols_orig];

    int i,j;

    int *orig_buf_ptr = buf;
    for (i=0; i< n_rows_orig; i++) {
      for (j=0; j < n_cols_orig; j++, orig_buf_ptr++) {
	 temp_buf[j * n_rows_orig + n_rows_orig - 1 - i] = *orig_buf_ptr;
      }
    }

    int *new_buf_ptr = temp_buf;
    orig_buf_ptr = buf;
    for (i=0; i < n_rows_orig; i++) {
      for (j=0; j < n_cols_orig; j++, new_buf_ptr++, orig_buf_ptr++) {
	 *orig_buf_ptr = *new_buf_ptr;
      }
    }

    delete[] temp_buf;
    return TRUE;
}

// *******************************************
/// Rotate the image 90 deg cw.
// ********************************************
int image_transformation_class :: rotate_cw(int n_rows_orig,
					    int n_cols_orig,
					    float *buf)

{

    float *temp_buf = new float[n_rows_orig * n_cols_orig];

    int i,j;

    float *orig_buf_ptr = buf;
    for (i=0; i< n_rows_orig; i++) {
      for (j=0; j < n_cols_orig; j++, orig_buf_ptr++) {
	 temp_buf[j * n_rows_orig + n_rows_orig - 1 - i] = *orig_buf_ptr;
      }
    }

    float *new_buf_ptr = temp_buf;
    orig_buf_ptr = buf;
    for (i=0; i < n_rows_orig; i++) {
      for (j=0; j < n_cols_orig; j++, new_buf_ptr++, orig_buf_ptr++) {
	 *orig_buf_ptr = *new_buf_ptr;
      }
    }

    delete[] temp_buf;
    return TRUE;
}



// *******************************************
/// Rotate the image 180 deg.
// ********************************************
int image_transformation_class :: rotate_180(int n_rows, 
					     int n_cols,
					     unsigned char *buf)
{
    unsigned char *top_buf_ptr;
    unsigned char *bottom_buf_ptr;
    unsigned char temp_buf;
    int i,j;

    int last_row = n_rows/2;
    for (i=0; i<last_row; i++) {
       top_buf_ptr = buf + i * n_cols;
       bottom_buf_ptr = buf + (n_rows - i - 1) * n_cols + n_cols - 1;
       for (j=0; j<n_cols; j++, top_buf_ptr++, bottom_buf_ptr--) {
	  temp_buf = *top_buf_ptr;
	  *top_buf_ptr = *bottom_buf_ptr;
	  *bottom_buf_ptr = temp_buf;
       }
    }

    return TRUE;
}


// *******************************************
/// Rotate the image 180 deg.
// ********************************************
int image_transformation_class :: rotate_180(int n_rows, 
					     int n_cols,
					     unsigned short *buf)
{
    unsigned short *top_buf_ptr;
    unsigned short *bottom_buf_ptr;
    unsigned short temp_buf;
    int i,j;

    int last_row = n_rows/2;
    for (i=0; i<last_row; i++) {
       top_buf_ptr = buf + i * n_cols;
       bottom_buf_ptr = buf + (n_rows - i - 1) * n_cols + n_cols - 1;
       for (j=0; j<n_cols; j++, top_buf_ptr++, bottom_buf_ptr--) {
	  temp_buf = *top_buf_ptr;
	  *top_buf_ptr = *bottom_buf_ptr;
	  *bottom_buf_ptr = temp_buf;
       }
    }

    return TRUE;
}


// *******************************************
/// Rotate the image 180 deg.
// ********************************************
int image_transformation_class :: rotate_180(int n_rows, 
					     int n_cols,
					     int *buf)
{
    int *top_buf_ptr;
    int *bottom_buf_ptr;
    int temp_buf;
    int i,j;

    int last_row = n_rows/2;
    for (i=0; i<last_row; i++) {
       top_buf_ptr = buf + i * n_cols;
       bottom_buf_ptr = buf + (n_rows - i - 1) * n_cols + n_cols - 1;
       for (j=0; j<n_cols; j++, top_buf_ptr++, bottom_buf_ptr--) {
	  temp_buf = *top_buf_ptr;
	  *top_buf_ptr = *bottom_buf_ptr;
	  *bottom_buf_ptr = temp_buf;
       }
    }

    return TRUE;
}


// *******************************************
/// Rotate the image 180 deg.
// ********************************************
int image_transformation_class :: rotate_180(int n_rows, 
					     int n_cols,
					     float *buf)
{
    float *top_buf_ptr;
    float *bottom_buf_ptr;
    float temp_buf;
    int i,j;

    int last_row = n_rows/2;
    for (i=0; i<last_row; i++) {
       top_buf_ptr = buf + i * n_cols;
       bottom_buf_ptr = buf + (n_rows - i - 1) * n_cols + n_cols - 1;
       for (j=0; j<n_cols; j++, top_buf_ptr++, bottom_buf_ptr--) {
	  temp_buf = *top_buf_ptr;
	  *top_buf_ptr = *bottom_buf_ptr;
	  *bottom_buf_ptr = temp_buf;
       }
    }

    return TRUE;
}

