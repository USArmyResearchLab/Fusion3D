#ifndef _image_gdal_class_h_
#define _image_gdal_class_h_
#ifdef __cplusplus

/**
Does import/export of 2-d images in many formats.  It has been tested in bmp and png formats.
Reads and writes .bmp files.  Reads .png format but does not write it.

The class implements the read  using GDAL.
This class handles either gray-scale images (1 byte per pixel, internal image type = 5) or rgb images (3 bytes per pixel, internal image type = 6).
*/
class image_gdal_class:public image_2d_inv_class{
 private:
   FILE * input_fd;
   //int nrows;				///< No. of rows from parent class
   //int ncols;				///< No. of cols from parent class
   //int nbands;			///< No of samples per pixel from parent class
   unsigned char *dataOut;	///< Pointer to output unsigned char data
   
 public:
   image_gdal_class();
   ~image_gdal_class();
   
   int read_file(string sfilename) override;
   int read_file_open(string sfilename) override;
   int read_file_header() override;
   int read_file_data() override;
   int read_file_close() override;
   
   int set_data(unsigned char *data, int num_rows_in, int num_cols_in, int type_in);
   int write_file(string sfilename) override;
};

#endif /* __cplusplus */
#endif /* _image_gdal_class_h_ */
