#ifndef _image_pnm_class_h_
#define _image_pnm_class_h_
#ifdef __cplusplus

/**
Does import and export of 2-d images in .pnm format.

This class handles either gray-scale images (1 byte per pixel, internal image type = 5) or rgb images (3 bytes per pixel, internal image type = 6).
*/
class image_pnm_class:public image_2d_inv_class{
 private:
   FILE * input_fd;
   int nspp;			// No of samples per pixel
   int data_offset;		// Loc in chars of first data
   
 public:
   image_pnm_class();
   ~image_pnm_class();
   
   int read_file(string sfilename) override;
   int read_file_open(string sfilename) override;
   int read_file_header() override;
   int read_file_data() override;
   int read_file_close() override;
   
   int set_data(unsigned char *data, int num_rows_in, int num_cols_in, int type_in);
   int write_file(string sfilename) override;
};

#endif /* __cplusplus */
#endif /* _image_pnm_class_h_ */
