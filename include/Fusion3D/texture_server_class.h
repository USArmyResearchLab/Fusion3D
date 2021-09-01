#ifndef _texture_server_class_h_
#define _texture_server_class_h_
#ifdef __cplusplus

class map3d_index_class;

/**
Gets best texture image for each tile and also gets general texture parameters.


*/
class texture_server_class:public base_jfd_class{
 private:
	int nx_map;				///< Basic Map Parms -- No of tiles and stored arrays along x
	int ny_map;				///< Basic Map Parms -- No of tiles and stored arrays along y
	double map_e, map_w;	///< Basic Map Parms -- Map boundaries
	double map_n, map_s;	///< Basic Map Parms -- Map boundaries
	float dx_map, dy_map;	///< Basic Map Parms -- Size of tile in m in x and y

	int n_reader_groups;	///< No. of groups of readers
	int nfiles_tex;			///< No. of texture files
    int mrsid_readerror_flag;///<	Flag so write out only 1 warning message
	int res_type;			///< Resolution of stored data -- 1=hi, 2=med, 3=low
	int all_gray_flag;		///< 1 if ALL texture files are grayscale

	float resx_a2;			///< DEM -- res in x (from first A2 file but assumed to be constant)
	float resy_a2;			///< DEM -- res in x (from first A2 file but assumed to be constant)
	int nx_tile, ny_tile;	///< DEM -- Size of tile in pixels in x and y
	int n_cushion;			///< DEM -- difference in pixels between tile size and read size -- read-size = tile-size+2*n_cushion

	float *resxa;			///< Per texture file -- resolution (pixel size) in x in m
	float *resya;			///< Per texture file -- resolution (pixel size) in y in m
	double *tex_e, *tex_w;	///< Per texture file -- Per mask file -- bounding box
	double *tex_s, *tex_n;	///< Per texture file -- Per mask file -- bounding box
	int *data_typea;		///< Per texture file -- Data type -- 5=gray-scale (1 byte per pixel), 6=color (3 bytes per pixel).
	int hires_flag;			///< 1 iff at least 1 texture file is higher res than the DEM files
	int medres_flag;		///< 1 iff any files present -- always use
	int lowres_flag;		///< 1 iff any files present -- always use
	int *id_sort_by_res;	///< Texture file ids sorted highest-res to lowest

	int down_ratio_med;		///< Downsampling ratio for medium-res imagery
	int down_ratio_low;		///< Downsampling ratio for low-res imagery
	int down_ratio_low_flag;///< Downsampling ratio for low-res imagery -- 0 for auto calculation (default), 1 for user-entered
	float resx_hi;			///< Res in x -- highest over all files
	float resy_hi;			///< Res in y -- highest over all files
	float resx_med;			///< Res in x -- medium
	float resy_med;			///< Res in y -- medium
	float resx_low;			///< Res in x -- low
	float resy_low;			///< Res in y -- low
	int nx_hi;				///< Size in pixels -- high
	int ny_hi;				///< Size in pixels -- high
	int nx_med;				///< Size in pixels -- medium
	int ny_med;				///< Size in pixels -- medium
	int nx_low;				///< Size in pixels -- low
	int ny_low;				///< Size in pixels -- low

	float *hxx;				///< Array of hue values for rainbow color scale
	float *natSat;			///< Array of saturation values for natural color scale
	float *natHue;			///< Array of hue values for natural color scale

	char *filename_change;				///< Change detection -- 
	image_tif_class *image_tif_change;	///< Change detection -- for diff image
	float *elev_change;					///< Change detection -- 
	int nx_change, ny_change;			///< Change detection -- 
	float thresh_change;				///< Change detection -- 
	float color_max_change;				///< Change detection -- 
      
	int nmask;							///< Mask -- no. of mask images
	int imask_current;					///< Mask -- no. of current mask images
	int mask_combine_flag;				///< Mask -- 0=no, 1=and
	int mask_apply_flag;				///< Mask -- 0=darken texture where masked, 1=substitute mask where nonzero
	std::vector<int>mask_nx;			///< Mask -- Per mask file -- no. of pixels in x
	std::vector<int>mask_ny;			///< Mask -- Per mask file -- no. of pixels in y
	std::vector<float>mask_dx;			///< Mask -- Per mask file -- resolution (pixel size) in x in m
	std::vector<float>mask_dy;			///< Mask -- Per mask file -- resolution (pixel size) in y in m
	std::vector<int>mask_typea;			///< Mask -- Per mask file -- Data type -- 5=gray-scale (1 byte per pixel), 6=color (3 bytes per pixel).
	std::vector<double>mask_e;			///< Mask -- Per mask file -- bounding box
	std::vector<double>mask_w;			///< Mask -- Per mask file -- bounding box
	std::vector<double>mask_s;			///< Mask -- Per mask file -- bounding box
	std::vector<double>mask_n;			///< Mask -- Per mask file -- bounding box
	std::vector<string>mask_name;		///< Mask -- Per mask file -- name
	std::vector<unsigned char*>mask_data;///< Mask -- Per mask file -- data
	image_geo_class** image_geo_mask;	///< Mask -- Per mask file -- Helper class to read data

	double mrsid_north_translate;		///< Translate -- Translate MrSID imagery -- from DEM coord to MrSID coord system
	double mrsid_east_translate;		///< Translate -- Translate MrSID imagery -- from DEM coord to MrSID coord system

	image_geo_class*** image_geo;			///< Helper class -- Array of reader parent classes -- over reader groups and files within each group
	map3d_index_class *map3d_index;			///< Helper class -- gets parms
	map3d_lowres_class*	  map3d_lowres;		///< Helper class -- does low-res calculations
	dir_class *dir;							///< Helper class -- for map filenames
	tiles_rtv_class *tiles_rtv;				///< Helper class -- gets parms

	// Private methods
	int apply_masks(double roi_w, double roi_n, double roi_e, double roi_s, int nxcrop, int nycrop, float dxtex, float dytex, unsigned char *data);
	int apply_masks_add(double roi_w, double roi_n, double roi_e, double roi_s, int nxcrop, int nycrop, float dxtex, float dytex, unsigned char *data);
	int apply_masks_substitute(double roi_w, double roi_n, double roi_e, double roi_s, int nxcrop, int nycrop, float dxtex, float dytex, unsigned char *data);
	int hsv_to_rgb(float h, float s, float v, float &r, float &g, float &b);

public:
	texture_server_class();
	~texture_server_class();
   
	int register_map3d_lowres(map3d_lowres_class*	map3d_lowres_in);
	int set_change_det(float thresh, float color_max);
	int is_all_gray();
	int get_hires_flag();
	int get_medres_flag();
	int get_lowres_flag();
	int get_nx_hi();
	int get_ny_hi();
	int get_nx_med();
	int get_ny_med();
	int get_nx_low();
	int get_ny_low();
	float get_resx_hi();
	float get_resy_hi();
	float get_resx_med();
	float get_resy_med();
	float get_resx_low();
	float get_resy_low();
	image_geo_class *get_image_geo_class(int i_reader_group, int ifile);

	int read_tagged(const char* filename);
	int write_parms(FILE *out_fd) override;
	int init(map3d_index_class *map3d_index_in, int n_reader_groups_in, int n_cushion);
	int add_mask_file(string filename);
	int clear_all_masks();
	int write_mask(string filename);
	int set_mask_combine_flag(int iflag);
	int combine_masks();
	int combine_masks_alt();
	int get_texture(double north_cen, double east_cen, float height_tile, float width_tile, int res_flag, int i_mrsid, unsigned char *data);
	int make_falsecolor_texture(double ncen, double ecen, float *data_elev, unsigned char* data_tex, int res_flag, int color_flag, float rainbow_rmin, float rainbow_rmax, float brt0);
	int mod_for_change_det(double north_cen, double east_cen, float height_tile, float width_tile, unsigned char* data);
	int reset_all();
   
};

#endif /* __cplusplus */
#endif /* _texture_server_class_h_ */
