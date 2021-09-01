#ifndef _ladar_mm_manager_class_h_
#define _ladar_mm_manager_class_h_	
#ifdef __cplusplus

/**
Manages point clouds.

Manages reading, writing and displaying point clouds.
The class is primarily used for terrain mapping point clouds -- sensing terrain from the air.
Experiments were also done to demonstrate merge of airborne and mobile mapping lidar
but this feature has not been thoroughly developed or tested.

*/
class ladar_mm_manager_class:public atrlab_manager_class{
   protected:
	  int ndisplay_max;					///< Max no. of points to display
	  int npts_read;					///< After reading, total no. of points read (over all files)
	  int npts_filtered;				///< After filtering, total no. of points that pass filter
	  int nskip_display;				///< On display, skip to stay within display quota
	  int npts_display;					///< On display, no of points displayed

	  int nRough;						///< No of hits to be displayed as a point cloud
	  int npts_dmax_user;				///< 1 iff user overrides max no. of pts displayed
      float elev_datlo;					///< Elevation value that is reasonable estimate of the low range of the data
	  float elev_datmed;				///< Elevation value that is reasonable estimate of the mid range of the data
	  float elev_dathi;					///< Elevation value that is reasonable estimate of the hi range of the data
	  float xmin_all_files;				///< Easting min over all files
	  float xmax_all_files;				///< Easting max over all files
	  float ymin_all_files;				///< Northing min over all files
	  float ymax_all_files;				///< Northing max over all files
	  float zmin_all_files;				///< Elevation min over all files
	  float zmax_all_files;				///< Elevation max over all files

      float *hxx;						///< Array of hue values for rainbow colors
      float *natSat;					///< Array of saturation values for natural color scale
      float *natHue;					///< Array of hue values for natural color scale

	  int epsgCodeNo;					///< EPSG code number used for all point clouds
	  int *status_flags;				///< Per file -- status flags (1-bit=reading class defined and file read, 2-bit=file transfered into OIV object

	  int translate_flag;				///< Translations, rotations
	  int rotate_flag;					///< Translations, rotations
	  float rotate_az;					///< Translations, rotations
	  float rotate_el;					///< Translations, rotations
	  float rotate_roll;				///< Translations, rotations
	  double translate_x;				///< Translations, rotations -- may translate to dummy loc if no loc info in file
	  double translate_y;				///< Translations, rotations -- may translate to dummy loc if no loc info in file
	  float translate_z;				///< Translations, rotations -- may translate to dummy loc if no loc info in file

      float utm_rainbow_hmin;			///< Color -- elevation min for rainbow color scheme
      float utm_rainbow_hmax;			///< Color -- elevation max for rainbow color scheme
      int rainbow_hlims_flag;			///< Color -- 1 iff rainbow range limits entered by user
	  int rainbow_scale_flag;			///< Color -- 0 for natural scale/abs elev (default), 1 for blue-red scale/abs elev, 2 for natural scale/rel elev, 3 for blue-red scale/rel elev
	  int rainbow_scale_user_flag;		///< Color -- 1 iff user enters override for rainbow_scale_flag
   
      float brt0_rgb;					///< Min brightness for default color scale -- for data with rgb values
      float brt0_intensity;				///< Min brightness for default color scale -- for data with only intensity
      int nbands;						///< No of associated color/intensity data bands (0 for none, 1 for grayscale, 3 for color, 4 for color+alpha)
      int color_balance_flag;			///< 0 for no color balance, 1 for linear histogram eq, 2 for s-curve histogram eq.
      int amp_clip_user_flag;			///< User-input amp clipping -- 1 iff read
      float amp_clip_min;				///< User-input amp clipping -- Min clipping amp
      float amp_clip_max;				///< User-input amp clipping -- Max clipping amp

	  int cull_near_ground_flag;		///< Cull near-ground -- 1 iff cull
	  float cull_near_ground_thresh;	///< Cull near-ground -- cull if difference less than this threshold

	  int clip_extent_flag;				///< Clip extent -- 1 iff clip
	  double clip_extent_e;				///< Clip extent -- east boundary
	  double clip_extent_w;				///< Clip extent -- west boundary
	  double clip_extent_n;				///< Clip extent -- north boundary
	  double clip_extent_s;				///< Clip extent -- south boundary

	  int decimation_n;					///< Decimation ratio -- =1 for no decimation, =n would keep only 1 point in n)

      int diag_level;					///< Output diagnostic level

      string outname;					///< Write -- filename
	  int write_flag;					///< Write -- 1 iff file to be written

	  int filter_type_flag;				///< Filter -- 0 for no filtering, 1 for FINE, 2 for abs elevation, 3 for rel elevation
	  int filter_nbins_max;				///< Filter -- Max no. of filter bins -- no. of separators allocated, default filter [0,filter_nbins_max-1]
	  int filter_mindisp;				///< Filter -- Span of bins that can be displayed [filter_mindisp, filter_maxdisp] -- subset of bins defined by menu limits (requires remake)
	  int filter_maxdisp;				///< Filter -- Span of bins that can be displayed [filter_mindisp, filter_maxdisp] -- subset of bins defined by menu limits (requires remake)
	  int filter_mindisp_slider;		///< Filter -- Span of bins that currently displayed [filter_mindisp_slider, filter_maxdisp]-- sub-subset defined by slider (no remake)
	  unsigned char **filter_bin;		///< Filter -- per point, bin number of that point
	  int **filter_bin_n;				///< Filter -- per bin, no. of points in that bin
	  int filter_bin_is_ptr_flag;		///< Filter -- 1 iff filter_bin[ifile] is a ptr rather than alloc within this class

	  int filt_lims_abs_flag;			///< Filter -- Elevation -- limits:  0=from data (default), 1=Project file override, 2=menu override -- abs
	  int filt_lims_rel_flag;			///< Filter -- Elevation -- limits:  0=from data (default), 1=Project file override, 2=menu override -- rel
	  float filt_zmin_abs,filt_zmax_abs;///< Filter -- Elevation -- limits for absolute el
	  float filt_zmin_rel,filt_zmax_rel;///< Filter -- Elevation -- limits for relative el

	  int csv_icx, csv_icy, csv_icz;	///< CSV format -- Col nos for x,y,z
	  int csv_ici;						///< CSV format -- Col no for intensity
	  int csv_icr, csv_icg, csv_icb;	///< CSV format -- Col nos for r,g,b color
	  int csv_nheader;					///< CSV format -- No. of header lines
	  char csv_delim;					///< CSV format -- Delimiter between data cols

	  int dummy_loc_flag;				///< Dummy loc -- 1 iff translate to dummy geo loc for file that has no geo 
	  int dum_epsg;						///< Dummy loc -- EPSG no
	  double dum_north, dum_east;		///< Dummy loc -- Northing, easting

      // Classes to read different ladar data formats that are subclasses of image_3d_class
      image_las_class**		image_las;			///< Helper classes -- To handle LAS images
	  image_bpf_class**		image_bpf;			///< Helper classes --  To handle BPF images
	  image_pccsv_class**	image_pccsv;		///< Helper classes --  To handle BPF images
	  image_ptcloud_class**	image_ptcloud;		///< Helper classes --  To handle parent class of BPF and LAS images

      int if_label;								///< Data label -- flag 1 iff display label
      float label_east, label_north;			///< Data label -- location
      char label_text[300];						///< Data label -- text
      
      // Private methods
      int read_input_image(int ifile, string filename, int read_header_only_flag, int diag_flag_local);
	  int set_pc_ref_loc(image_ptcloud_class* ptc);
	  int init_filter(int ifile);
	  int bin_elevation(int ifile, int diag_flag);
	  int bin_tau(int ifile, int diag_flag);
	  int reset_all();
      
   public:
      ladar_mm_manager_class(int n_data_max_in);
      virtual ~ladar_mm_manager_class();
      
      int read_tagged(const char* filename);
	  int write_parms(FILE *out_fd) override;
};

#endif /* __cplusplus */
#endif /* _ladar_mm_manager_class_h_ */
