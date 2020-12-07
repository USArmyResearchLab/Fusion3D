#ifndef _image_transformation_class_h_
#define _image_transformation_class_h_
#ifdef __cplusplus

/**
Does simple image transformations like flipping and rotating by multiples of 90 deg.

These transformations are simple ones that do not need resampling.

*/
class image_transformation_class {

 public:
  image_transformation_class(void);
  ~image_transformation_class(void);
  int flip_vertical(int n_rows, int n_cols, unsigned char *buf);
  int flip_vertical(int n_rows, int n_cols, unsigned short *buf);
  int flip_vertical(int n_rows, int n_cols, int *buf);
  int flip_vertical(int n_rows, int n_cols, float *buf);

  int flip_horizontal(int n_rows, int n_cols, unsigned char *buf);
  int flip_horizontal(int n_rows, int n_cols, unsigned short *buf);
  int flip_horizontal(int n_rows, int n_cols, int *buf);
  int flip_horizontal(int n_rows, int n_cols, float *buf);

  int rotate_ccw(int n_rows_orig, int n_cols_orig, unsigned char *buf);
  int rotate_ccw(int n_rows_orig, int n_cols_orig, unsigned short *buf);
  int rotate_ccw(int n_rows_orig, int n_cols_orig, int *buf);
  int rotate_ccw(int n_rows_orig, int n_cols_orig, float *buf);


  int rotate_cw(int n_rows_orig, int n_cols_orig, unsigned char *buf);
  int rotate_cw(int n_rows_orig, int n_cols_orig, unsigned short *buf);
  int rotate_cw(int n_rows_orig, int n_cols_orig, int *buf);
  int rotate_cw(int n_rows_orig, int n_cols_orig, float *buf);


  int rotate_180(int n_rows, int n_cols, unsigned char *buf);
  int rotate_180(int n_rows, int n_cols, unsigned short *buf);
  int rotate_180(int n_rows, int n_cols, int *buf);
  int rotate_180(int n_rows, int n_cols, float *buf);
};

#endif /* __cplusplus */
#endif /* _image_transformation_class_h_ */
