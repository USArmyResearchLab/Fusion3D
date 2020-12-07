#ifndef _image_tif_class_h_
#define _image_tif_class_h_
#ifdef __cplusplus

/**
Does import and export of 2-d images in TIFF format.

This class has 2 basic implementations for reading: a parser developed internally by JFD and a parser based on GDAL.
Both have limitations.  My parser deals well with BuckEye, Haloe, Alirt and most commercial data and also handles BigTiff files.
It does not well with the GRiD data repository with the specific limitations:\n
	1.  It cant read compressed data which is the default for the GRiD data repository.\n
	2.  It cant deal with float texture data which I have seen from the GRiD data repository.\n
The GDAL can deal with the GRID issues but has the following important limitation with BuckEye data:\n
	1.  It does not find the ESPG code for certain Buckeye elevation files (which may have been modified at ARL)\n
\n
The current approach is to default to the internal parser, but to switch to GDAL whenever the either of the 2 issues described above
is encountered.\n
\n

Write Geo Keys:\n
	keyid = 1024;			// GTModelTypeGeoKey			1=ModelTypeProjected only\n
	keyid = 1025;			// GTRasterTypeGeoKey			1=RasterPixelIsArea only\n
	keyid = 3072;			// ProjectedCSTypeGeoKey		ESPG coordinate system code\n
	keyid = 3076;			// ProjLinearUnitsGeoKey		9001=meter/9002=feet/9003=USSurvey Feet only\n
	keyid = 4099;			// VerticalUnitsGeoKey			9001=meter/9002=feet/9003=USSurvey Feet only\n

*/
class image_tif_class:public image_geo_class {
 private:
	 bool bigTiffFlag;			// Must be either 42=classicTiff or 43=BigTiff
	 bool useGdalReadFlag;		// 1 iff read using GDAL, 0 for my parser
	 int compressionType;		///< Follows Tiff compression tag numbers -- 1 for no compression, greater for compression (8=Adobe Deflate used by GRiD database)
	 int byte_order_flag;		// 0=PC, 1=SGI

   int nstrips;				// Horizontal bands indexed separately
   int nrows_per_strip;		// No of rows per strip
   float zunits_to_m;       // Convert from internal units to m for elevation
   float xyunits_to_m;       // Convert from internal units to m for x and y
   int zunits_code;				// Units for vertical:    9001 for meters, 9002 for feet, 9003 for US Survey Feet
   int xyunits_code;			// Units for horizontal:  9001 for meters, 9002 for feet, 9003 for US Survey Feet
   float xTiepoint, yTiepoint;	// Tiepoint -- coordinates
   int ixTiepoint, iyTiepoint;	// Tiepoint -- pixel loc
   int areaOrPointFlag;			// Tiepoint -- 1 if RasterPixelIsArea, 2 if RasterPixelIsPoint (GTRasterTypeGeoKey values)
   float xres, yres;			// Resolution in native units
   int geog_type_flag;			// 0 when header data for projected coord system,  >0 for geographic coord system ID (only implemented for 4326=WGS84)
   double nodataValue;			// Value entered when there is no data for a particular pixel but place holder is necessary
   
   int input_fd, output_fd;	//
   
   unsigned long long int xres_offset;			// Offsets -- XResolution
   unsigned long long int yres_offset;			// Offsets -- YResolution
   unsigned long long int ties_offset;			// Offsets -- pixel-geoloc ties
   unsigned long long int strip_offsets_base;	// Offsets -- base for array of strip offsets
   unsigned long long int scale_offset;			// Offsets -- Pixel size
   unsigned long long int tile_offsets_base;	// Offsets -- Base for array of tile offsets
   unsigned long long int geokeys_offset;		// Offsets -- Geokeys
   unsigned long long int geoascii_offset;		// Offsets -- GeoAscii 
   unsigned long long int modelt_offset;		// Offsets -- ModelTransformationTag
   unsigned long long int nodata_offset;		// Offsets -- GDAL_NODATA
   unsigned long long int *tile_offset;			// Offsets -- Tiles --  For each tile, offset to first element
   unsigned long long int *strip_offset;		// Offsets -- Strips -- For each strip, offset to first element

   int tile_flag;			// Tiles -- 1 iff tiling
   int tile_width;			// Tiles -- Width in pixels
   int tile_length;			// Tiles -- Length in pixels
   int tiles_across;		// Tiles -- No of tiles in x
   int tiles_down;			// Tiles -- No of tiles in y
   int tiles_per_image;		// Tiles -- No of tiles per image
   unsigned char *uctile;	// Tiles -- Temp storage for a single tile
   float *ftile;			// Tiles -- Temp storage for a single tile

   int nties;				// No of pixel-geoloc ties
   
   int ngeokeys;			// Geokeys -- No
   int ngeoascii;			// GeoAscii -- No
   int nnodata;				// GDAL_NODATA -- No

   int nmodelt;				// ModelTransformationTag
   
   int offset_write;        // Write
   int offset_data;         // Write -- beginning of data
   int type_write;          // Write -- 0 for unsigned char intensity -- 1 per pixel
                            //          1 for float
                            //          2 for unsigned char rgb -- 3 per pixel
   
#if defined(LIBS_GDAL)
   GDALDataset *poDataset;
#endif

   // Private methods
   int init_geog_type();
   int read_data_by_strips(int external_alloc_flag);
   int read_nodata();
   int read_ties();
   int read_model_transformation();
   int read_scale();
   int read_geokeys();
   int read_geoascii();
   int read_rational(_int64 offset, float &val);
   int read_strip_offsets();
   int read_tag(unsigned short &tagid, short &tagtyp, unsigned long long &nvals, char* outAscii, unsigned long long &outInt,
	   float &outFloat, double &outDouble, int &offset_flag, unsigned long long &outOffset);

   int read_data_by_gdal(int external_alloc_flag);
   int read_data_by_tiles(int external_alloc_flag);
   int read_header_by_gdal();
   int read_tile_offsets();

 public:
   image_tif_class(gps_calc_class *gps_calc_in);
   ~image_tif_class();
   
   int read_file(string sfilename) override;
   int read_file_open(string sfilename) override;
   int read_file_header() override;
   int read_file_data() override;
   int read_file_close() override;
   int write_file(string sfilename) override;

   int read_into_roi(double roi_w, double roi_n, double roi_e, double roi_s, int nxcrop, int nycrop, unsigned char *data, int &ninter);
   
   int set_zunits_code(int code);
   int set_xyunits_code(int code);
   int set_tiepoint(double north_bound, double west_bound);
   int set_data_res(float dheight_in, float dwidth_in);
   int set_data_size(int nrows_in, int ncols_in);
   int set_data_array_float(float *fdata_in);
   int set_data_array_uchar(unsigned char *uchar_in);
   int set_data_array_rgb(unsigned char *uchar_in);

   int get_data_uchar(unsigned char* udata);
   double get_nodata_value();

};

#endif /* __cplusplus */
#endif /* _image_tif_class_h_ */
