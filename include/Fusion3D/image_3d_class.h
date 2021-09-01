#ifndef _image_3d_class_h_
#define _image_3d_class_h_
#ifdef __cplusplus

/**
Virtual class that is the base class for all 3-d data.

*/
class image_3d_class: public base_jfd_class{
 protected:
   int n_hits_current;				///< Data storage -- hit-list -- no of hits
   short int *width_a;				///< Data storage -- hit-list -- geometry 
   short int *height_a;				///< Data storage -- hit-list -- geometry 
   float *range_a;					///< Data storage -- hit-list -- geometry 
   unsigned char *intens_a;			///< Data storage -- hit-list -- intensity for greyscale
   unsigned char *red_a;			///< Data storage -- hit-list -- intensity for color scale 
   unsigned char *grn_a;			///< Data storage -- hit-list -- intensity for color scale 
   unsigned char *blu_a;			///< Data storage -- hit-list -- intensity for color scale 
   int *smooth_flags;				///< Data storage -- hit-list -- per hit, smooth flags
   float *northa, *easta, *eleva;	///< Data storage -- hit-list -- (North,East,elev) relative to ref point
   
   int *n_hits_a;					///< Data storage -- output multi -- no of hits per image
   float **coords3_a;				///< Data storage -- output multi -- dewarped (x,y,z) for each vertex
   unsigned char **intens_a_display;///< Data storage -- output multi -- data amplitudes
   unsigned char **red_a_display;	///< Data storage -- output multi -- data color
   unsigned char **grn_a_display;	///< Data storage -- output multi -- data color
   unsigned char **blu_a_display;	///< Data storage -- output multi -- data color
   
   unsigned char *image_2d;			///< Data storage -- 2-D image (for multi, last image read)
   unsigned char *red_2d;			///< Data storage -- 2-D image (for multi, last image read)
   unsigned char *grn_2d;			///< Data storage -- 2-D image (for multi, last image read)
   unsigned char *blu_2d;			///< Data storage -- 2-D image (for multi, last image read)
   
   int reread_pending;				///< Image logic -- Change pending -- new image (from file/Epix/etc) (0/1 default)
   int recalc_pending;				///< Image logic -- Change pending -- new parms requiring recalc (0/1 default)
   int alloc_pending;				///< Image logic -- Change pending -- new size requiring realloc (0/1 default)
   int if_range_slices;				///< Image logic -- 1 iff amp-vs-range waveforms available
   int if_time_slices;				///< Image logic -- 1 iff amp-vs-time waveforms available
   int if_frame_slices;				///< Image logic -- 1 iff image-frame-vs-time waveforms available
   int if_2d_aux_images;			///< Image logic -- 1 iff aux 2D images available
   int if_diag_slices;				///< Image logic -- 1 iff diag waveforms available
   int seqno_frame;					///< Image logic -- Increment when frame display needs update
   int seqno_2d;					///< Image logic -- Increment when 2d image display needs update
   int iframe_current;				///< Image logic -- Current frame no.
   int frame_type;					///< Image logic -- 0=unreg, 1=reg
   int id_img;						///< Image logic -- id of image -- for looking up in index
   
   float xangle_delta, yangle_delta;		///< Single image parms -- Angle step sizes
   float xangle_center, yangle_center;		///< Single image parms -- Image center (for mirror offsets)
   int num_width, num_height, num_range_orig;///< Single image parms -- Size of data cube as read in
   int ntar_per_pixel;						///< Single image parms -- No of targets/hits per pixel
   int num_range_clip;						///< Single image parms -- Number of ranges at output
   int num_time;							///< Single image parms -- Number of time samples
   int coord_type_flag;						///< Single image parms -- 0=spherical(default), 1=cartesian
   int if_crop;								///< Single image parms -- 1 if input image to be cropped
   int iw1, iw2, ih1, ih2;					///< Single image parms -- Coords of cropped image
   int reg_frames_flag;						///< Single image parms -- reg frames -- 1 iff register
   int iwr1, iwr2, ihr1, ihr2;				///< Single image parms -- reg frames -- Coords of registration subimage
   int reg_frames_mflag;					///< Single image parms -- reg frames -- 1 iff med filter
   int reg_frames_mhalf;					///< Single image parms -- reg frames -- med filter halfwidth (pixels)
   int reg_frames_idel;						///< Single image parms -- reg frames -- max diff frame-to-frame
   int reg_frames_iref;						///< Single image parms -- reg frames -- ref image
   int reg_frames_idiag;					///< Single image parms -- reg frames -- diagnostic level (0=off)
   int write_flag;							///< Single image parms -- 0=no write, 1=write cropped
   int file_size;							///< Single image parms -- Size of file (single frame) in bytes

   int n_files;					///< Multi image parms -- No of files to be buffered
   int i_file;					///< Multi image parms -- Current file no
   int n_hits_max;				///< Multi image parms -- Max no of hits over all data files

   int data_intensity_type;			///< Overall parms --	0 = unread, 5 = 8-bit grayscale, 6 = 24-bit rgb color
   int intens_thresh;				///< Overall parms --	Intensity threshold
   float xbb_min, xbb_max;			///< Overall parms -- Bounding box (after clipping)
   float ybb_min, ybb_max;			///< Overall parms -- Bounding box (after clipping)
   float zbb_min, zbb_max;			///< Overall parms -- Bounding box (after clipping)
   int if_clip_amp_user;			///< Overall parms -- 1 if clipping output amplitudes set by user
   float min_amp_user, max_amp_user;		///< Overall parms -- Min and max clipping amplitudes set by user
   float min_brt_vs_time, max_brt_vs_time;	///< Overall parms -- Min and max of amp-vs-time waveform
   float min_brt_vs_range, max_brt_vs_range;	///< Overall parms -- Min and max of amp-vs-range waveform
   float dheight, dwidth, drange;		///< Overall parms -- Size of pixel for data in cartesian coords
   float range_offset;				///< Overall parms -- Range corresponding to first data index
   int x_fft;					///< Overall parms -- Range resolution spoiling factor, exponent_of_factor_2
   int if_superres;				///< Overall parms -- 0=no superres, 1=superres (default)
   int version_no;				///< Overall parms -- 1=default
   int type_2d;					///< Overall parms -- How to calc 2-d image
   							///< 0 = use same peak as for 3-d image
   							///< 1 = use avg signal over chirp
   int type_peak_display;			///< Overall parms -- How to use peaks
   							///< 0 = use all peaks
   							///< 1 = use only largest peak within range bounds

   
   int diag_level;					///< 0 -- no diagnostics, 1 min diagnostics, 2 more, etc
   char *diag_filename;				///< Diagnostic output filename
   
   int if_know_sensor_pos;				///< Sensor loc -- 1 iff can get sensor loc from data
   float x_sensor, y_sensor, z_sensor;	///< Sensor loc -- loc in m in world coordinates
   
   float* aux_image;				///< Aux 2d -- image
   int aux_nx, aux_ny;				///< Aux 2d -- Image dimensions
   float aux_dx, aux_dy;			///< Aux 2d -- Image pixel dimensions
   int aux_imax;					///< Aux 2d -- Pixel no. of max signal
   float aux_fmax;					///< Aux 2d -- Max signal value
   
   float range_calc_min, range_calc_max;	///< Range -- lims for current calcs
   int range_calc_flag;						///< Range -- 1 iff lims have been set 
   float hxx[38];							///< False color -- red-to-blue hue array

   // Private methods
   int interp(int x[],float y[],int n, float cof[]);
   float getDerivative(int x[],float cof[]);
   int make_verts_cartesian(short int *wa, short int *ha, float *ra, float *c3a);
   int make_verts_spherical(short int *wa, short int *ha, float *ra, float *c3a);
   int hsv_to_rgb(float h, float s, float v, float &r, float &g, float &b);

 public:
   image_3d_class();
   virtual ~image_3d_class();			// Must be virtual so subclass destructor called by delete
   
   // Get data in hit-list format
   int get_n_hits_current();
   int get_hit_geom(short int* &width_aout, short int* &height_aout, float* &range_aout);
   int get_hit_xyz(float* &north, float* &east, float* &elev);
   int get_hit_intensity(unsigned char* &intens_aout);
   int get_hit_color(unsigned char* &red, unsigned char* &grn, unsigned char* &blu);
   int get_smooth_flags(int* &smooth);
   
   // Modify hit-list format
   int cull_hit_list(int *mask, int n_cull);
   
   // Get data in Inventor format
   int get_n_hits_dewarped(int i_file);
   int get_coords3_dewarped(int i_file, float* &coords_out);
   // int get_hit_geom_dewarped(int i_file, float* &coords_out);
   int get_hit_intensity_dewarped(int i_file, unsigned char* &intens_aout);
   int get_hit_color_dewarped(int i_file, unsigned char* &red, unsigned char* &grn, unsigned char* &blu);
   
   // Get 2-d image data
   unsigned char* get_image_2d();	// Gets last image read				   
   unsigned char* get_red_2d();		// Gets last image read				   
   unsigned char* get_grn_2d();		// Gets last image read				   
   unsigned char* get_blu_2d();		// Gets last image read				   
   
   // Get 2-d Aux image data
   float* get_2d_aux_image();		// Aux 2d -- Get image
   int get_2d_aux_nx();			// Aux 2d -- Get x dim
   int get_2d_aux_ny();			// Aux 2d -- Get y dim
   int get_2d_aux_imax();		// Aux 2d -- Get pixel no of max
   float get_2d_aux_fmax();		// Aux 2d -- Get signal max
   float get_2d_aux_dx();		// Aux 2d -- Get pixel size x
   float get_2d_aux_dy();		// Aux 2d -- Get pixel size y

   // Revised range
   int set_range_calc(float min, float max);
   float get_range_calc_min();
   float get_range_calc_max();
   
   // Set single-image hit-list parms
   int set_xangle_delta(float xangle_delta_in);
   int set_yangle_delta(float yangle_delta_in);
   int set_xangle_center(float angle);
   int set_yangle_center(float angle);
   int set_coord_type_flag(int flag);
   int set_reg_frames(int iw1_in, int ih1_in, int iw2_in, int ih2_in,
   	int mflag, int mhalf, int idel, int iref, int idiag);
   int set_write_flag(int write_flag);

   // Get single-image hit-list parms
   float get_xangle_delta();
   float get_yangle_delta();
   float get_xangle_center();
   float get_yangle_center();
   int get_coord_type_flag();

   // Set multi-image parms
   int set_movie_mode(int n_files_in);
   int set_i_file(int i_file_in);
				   
   // Get multi-image parms
   int get_n_hits_max();
   int get_bb(float &xmin, float &xmax, float &ymin, float &ymax, float &zmin, float &zmax);

   // Set overall parms
   int set_inten_threshold(int intens_thresh_in);
   int set_amp_clip(float min_amp_user_in, float max_amp_user_in);
   int set_res_spoiling_factor(int exponent_of_factor_2);
   int set_version(int version_no_in);
   int set_superres(int if_superres_in);
   int set_type_peak_display(int type_peak_display_in);

   // Get/set overall parms
   int get_height();
   int get_width();
   int get_ntar_per_pixel();
   int get_voxel_size(float &dheight_out, float &dwidth_out, float &drange_out);
   int set_intensity_type(int type);	// 5 for unsigned char data, 6 for 3-unsigned char rgb
   int get_intensity_type();		// 5 for unsigned char data, 6 for 3-unsigned char rgb
   int get_inten_threshold();
   float get_min_brt_vs_range();
   float get_max_brt_vs_range();
   float get_min_brt_vs_time();
   float get_max_brt_vs_time();
   float get_amp_clip_min();
   float get_amp_clip_max();
   int   set_range_offset(float range);
   float get_range_offset();
   
   // Set/get image logic
   int set_reread_pending();
   int set_recalc_pending();
   int get_if_range_slices();
   int get_if_time_slices();
   int get_if_frame_slices();
   int get_if_2d_aux_images();
   int get_if_diag_slices();
   int get_seqno_frame();
   int get_seqno_2d();
   int get_iframe_current();
   int set_frame_type(int frame_type_in);
   int set_id(int id);

   int get_depth_orig();
   int get_depth_clip();
   int get_num_time();
   int get_file_size();
   
   // Get position of sensor (return 0 if unknown)
   int get_sensor_pos(float &x, float &y, float &z);
   
   // Export
   int export_geom_obj(char *filename);
   int export_2d_pnm(char *filename);
   
   // Virtual
   virtual int get_type(char* type);
   virtual int copy_frame_from_buffer(char* inBuf);															// For image_botladar_class
   virtual int proc_frame();																				// For image_botladar_class
   virtual int get_n();																						// For image_las_class / image_bpf_class
   virtual unsigned short* get_intensa();																	// For image_las_class / image_bpf_class
   virtual unsigned short* get_reda();																		// For image_las_class / image_bpf_class
   virtual unsigned short* get_grna();																		// For image_las_class / image_bpf_class
   virtual unsigned short* get_blua();																		// For image_las_class / image_bpf_class
   virtual double get_x(int i);																				// For image_las_class / image_bpf_class
   virtual double get_y(int i);																				// For image_las_class / image_bpf_class
   virtual double get_z(int i);																				// For image_las_class / image_bpf_class
   virtual int get_bounds(double &xmin, double &xmax, double &ymin, double &ymax, float &zmin, float &zmax);// For image_las_class / image_bpf_class
   virtual int get_coord_system_code();																		// For image_las_class / image_bpf_class
   virtual int make_dummy_scene(char* name_header);
   virtual int get_vals_at_index(int ix, int iy, int itar, float &range, float &amplitude);					// For image_botladar_class

   virtual int make_oiv();
   virtual int get_range_slice(float w_norm, float h_norm_in, float* slice_range,
   	float* slice_hit, float* slice_th, int &n_slice,
	float* slice_time, int &n_slice_time);
   virtual int get_frame_slice(int i_slice, unsigned char* frame);
   virtual int override_to_color();
   virtual int get_elev_at_locxy(float x, float y, float &z);
   virtual int set_type_2d(int type_2d_in);
   virtual int set_crop(int iw1_in, int ih1_in, int iw2_in, int ih2_in);

};

#endif /* __cplusplus */
#endif /* _image_3d_class_h_ */
