#ifndef _image_mrsid_class_h_
#define _image_mrsid_class_h_
#ifdef __cplusplus

/**
Does import of 2-d images in MrSID format.

This class can extract a chip of a MrSID image into a buffer and also exports data in GeoTIFF format.\n
\n
The default data ordering for MrSID is BSQ (r,r,...,r,g,g,...,g,b,b,...,b).
However, the default for our codes is BIP (r,g,b,r,g,b,...,r,g,b).
For example, OIV textures use BIP ordering.
Therefore, a data reordering is done unless the BSQ flag is set with method set_bsq.


*/
class image_mrsid_class:public image_geo_class {
 private:
   int bsq_flag;
   double xRes, yRes;						// Full-scale res -- yRes is negative, so not equal to dheight
   unsigned char *membuf, *membuf2;
   int wCrop;								// Crop area -- width in pixels
   int hCrop;								// Crop area -- height in pixels
   int w1Crop;								// Crop area -- left edge in pixels
   int h1Crop;								// Crop area -- top edge in pixels
   int w2Crop;								// Crop area -- right edge in pixels
   int h2Crop;								// Crop area -- bottom edge in pixels
   int loc_w1, loc_h1;						// Crop area -- starting pixels in input array for partial overlap
   int partial_flag;						// Crop area -- 1 if partial overlap, 0 if crop area completely enclosed in MrSID image

   string storeName;						///< Storage for filename

#if defined(LIBS_MRSID) 
   MrSIDImageReader *reader;
   LTIScene *scene;
   LTICropFilter *cropFilter;
   LTIMultiResFilter *downsampleFilter;
#endif
   
   int get_epsg_by_searching_wkt(const char *wkt, int &code);
   int get_epsg_using_gdal(const char *wkt, int &code);

 public:
   image_mrsid_class(gps_calc_class *gps_calc_in);
   ~image_mrsid_class();
   
   int set_bsq();
   
   int read_file(string sfilename) override;
   int write_file(string sfilename) override;
   int read_file_open(string sfilename) override;
   int read_file_header() override;
   int read_file_data() override;
   int read_file_close() override;

   int read_into_roi(double roi_w, double roi_n, double roi_e, double roi_s, int nxcrop, int nycrop, unsigned char *data, int &ninter);

};

#endif /* __cplusplus */
#endif /* _image_mrsid_class_h_ */
