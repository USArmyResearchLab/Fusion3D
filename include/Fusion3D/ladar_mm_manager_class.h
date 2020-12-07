#ifndef _ladar_mm_manager_class_h_
#define _ladar_mm_manager_class_h_	
#ifdef __cplusplus

/**
Manages mobile-mapping lidars.

Merges ground-based lidars with airborne lidar map imagery.
This class was written to demonstrate merge of airborne and mobile mapping lidar
and has not been thoroughly developed or tested.

*/
class ladar_mm_manager_class:public atrlab_manager_class{
   protected:
	  int ndisplay_max;					///< Max no. of points to display
	  int nskip_read;					///< On reading, skip to stay within max memory quota
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
	  float translate_x;				///< Translations, rotations
	  float translate_y;				///< Translations, rotations
	  float translate_z;				///< Translations, rotations

      float utm_rainbow_hmin;			///< Color -- elevation min for rainbow color scheme
      float utm_rainbow_hmax;			///< Color -- elevation max for rainbow color scheme
      int rainbow_hlims_flag;			///< Color -- 1 iff rainbow range limits entered by user
	  int rainbow_scale_flag;			///< Color -- 0 for natural scale/abs elev (default), 1 for blue-red scale/abs elev, 2 for natural scale/rel elev, 3 for blue-red scale/rel elev
	  int rainbow_scale_user_flag;		///< Color -- 1 iff user enters override for rainbow_scale_flag
   
      float brt0_rgb;					///< Min brightness for default color scale -- for data with rgb values
      float brt0_intensity;				///< Min brightness for default color scale -- for data with only intensity
      int intensity_type;				///< 5=8-bit greyscale, 6=24-bit color
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

	  int dec_ang_reg_n;				///< Decimation angular regions -- number of regions defined (default 0)
	  int *dec_ang_reg_a1;				///< Decimation angular regions -- lower bound angle in deg (45deg up, 135deg down)
	  int *dec_ang_reg_a2;				///< Decimation angular regions -- upper bound angle in deg (45deg up, 135deg down)
	  int *dec_ang_reg_d1;				///< Decimation angular regions -- decimation factor at lower bound
	  int *dec_ang_reg_d2;				///< Decimation angular regions -- decimation factor at upper bound

      int diag_level;					///< Output diagnostic level

      string outname;					///< Write -- filename
	  int write_flag;					///< Write -- 1 iff file to be written

	  int filter_flag;					///< Filter -- 0 for no filtering, 1 for FINE, 2 for abs elevation, 3 for rel elevation
	  int filter_nbins_max;				///< Filter -- Max no. of filter bins -- no. of separators allocated, default filter [0,filter_nbins_max-1]
	  int filter_mindisp;				///< Filter -- Span of bins that can be displayed [filter_mindisp, filter_maxdisp] -- subset of bins defined by menu limits (requires remake)
	  int filter_maxdisp;				///< Filter -- Span of bins that can be displayed [filter_mindisp, filter_maxdisp] -- subset of bins defined by menu limits (requires remake)
	  int filter_mindisp_slider;		///< Filter -- Span of bins that currently displayed [filter_mindisp_slider, filter_maxdisp]-- sub-subset defined by slider (no remake)
	  unsigned char **filter_bin;		///< Filter -- per point, bin number of that point
	  int **filter_bin_n;				///< Filter -- per bin, no. of points in that bin

	  int *fine_flags;					///< Filter -- Fine -- 1 iff data contains FINE algorithm TAU metric values
	  unsigned char **fine_tau;			///< Filter -- Fine -- Per point, tau value
	  int **fine_n;						///< Filter -- Fine -- No of pts per TAU value bin
	  float **fine_raw;					///< Filter -- Fine -- Stores raw TAU (Chi) values associated with each TAU bin

	  int filt_zbin_type;				///< Filter -- Elevation -- 0=not filtered, 1=filtered on abs el, 2=filtered on rel el
	  int filt_lims_abs_flag;			///< Filter -- Elevation -- 1 iff user defines filter limits -- abs
	  int filt_lims_rel_flag;			///< Filter -- Elevation -- 1 iff user defines filter limits -- rel
	  float filt_zmin_abs,filt_zmax_abs;///< Filter -- Elevation -- limits for absolute el
	  float filt_zmin_rel,filt_zmax_rel;///< Filter -- Elevation -- limits for relative el
	  float filt_dz;					///< Filter -- Elevation -- size of each bin
	  int filt_nzbin;					///< Filter -- Elevation -- No of elevation bins
	  unsigned char **filt_zbin;		///< Filter -- Elevation -- per point, elevation bin
	  int **filt_zbin_n;				///< Filter -- Elevation -- per bin, no. of points

      // Classes to read different ladar data formats that are subclasses of image_3d_class
      image_las_class**		image_las;			// To handle LAS images
      image_bpf_class**		image_bpf;			// To handle BPF images
      image_ptcloud_class**	image_ptcloud;		// To handle parent class of BPF and LAS images

      int if_label;
      float label_east, label_north;			// Offsets
      char label_text[300];
      
      // Private methods
      int read_input_image(int ifile, string filename, int read_header_only_flag, int nskip_loc, int diag_flag_local);
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
