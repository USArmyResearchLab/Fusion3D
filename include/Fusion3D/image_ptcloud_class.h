#ifndef _image_ptcloud_class_h_
#define _image_ptcloud_class_h_
#ifdef __cplusplus

/**
Virtual class that is the base class for all point-cloud data.

Point clouds are unstructured, as opposed to data from the ARL lidars which are raster scanned and have the raster structure and are children of base class image_3d_class.


*/
class image_ptcloud_class: public base_jfd_class{
 protected:
   
   int data_intensity_type;			///< Metadata -- Type of intensity/color data --	0 = unread, 5 = 8-bit grayscale, 6 = 24-bit rgb color
   int tauFlag;						///< Metadata -- TAU -- 1 iff TAU (quality metric per hit) values in data
   int timeFlag;					///< Metadata -- Time -- 0 for no times, 1 for time GPS week time, 2 for time GPS standard time - 10**9
   double timeStart;				///< Metadata -- Time -- Start time
   double timeEnd;					///< Metadata -- Time -- End time (at end of data)
   double nmin, nmax;				///< Metadata -- Min and max of data array in UTM coords
   double emin, emax;				///< Metadata -- Min and max of data array in UTM coords
   float  zmin, zmax;				///< Metadata -- Min and max of data array in UTM coords (may be truncated for noisy data)
   int epsgTypeCode;				///< Metadata -- Code specifying the Projected Coordinate System
   int npts_file;					///< Metadata -- Total no. of points in file
   int bytes_per_point;				///< Metadata -- No. of bytes allocated per point

   int npts_read;					///< No. of points read
   int nskip;						///< Stride for reading in case memory constraints require decimation
   float amp_min;					///< Min amplitude mapped into output [0,255]
   float amp_max;					///< Min amplitude mapped into output [0,255]
   int amp_lims_user_flag;			///< 1 iff amplitude limits set by user
   
   float *coords3_a;				///< Data storage -- dewarped (x,y,z) for each vertex

   int diag_level;					///< 0 -- no diagnostics, 1 min diagnostics, 2 more, etc
   int swap_flag;					///< Image logic -- 1 iff byteswap
   int reread_pending;				///< Data must be reread from file and any calcs also
   int recalc_pending;				///< Data need not be reread but any calcs like amp scaling must be redone
   
   int clip_extent_flag;			///< Clip extent -- 1 iff clip
   double clip_extent_e;			///< Clip extent -- east boundary in UTM coordinates
   double clip_extent_w;			///< Clip extent -- west boundary in UTM coordinates
   double clip_extent_n;			///< Clip extent -- north boundary in UTM coordinates
   double clip_extent_s;			///< Clip extent -- south boundary in UTM coordinates

   double utm_cen_east, utm_cen_north;	///< Center of data

   // Private methods

public:
   image_ptcloud_class();
   virtual ~image_ptcloud_class();			// Must be virtual so subclass destructor called by delete
   
   // Metadata
   int set_nskip(int nskip_in);
   int set_amp_clip(float min_amp_user_in, float max_amp_user_in);
   int set_intensity_range(float intens_min, float intens_max);
   int set_clip_extent(double xmin, double xmax, double ymin, double ymax);

   int get_npts_file();	
   int get_npts_read();
   int get_bytes_per_point();
   double get_utm_cen_east();
   double get_utm_cen_north();
   int is_tau();
   int get_time_type();
   double get_time_start();
   double get_time_end();
   int get_coord_system_code();																		// For image_las_class / image_bpf_class
   int get_bounds(double &xmin_utm, double &xmax_utm, double &ymin_utm, double &ymax_utm, float &zmin_utm, float &zmax_utm);// For image_las_class / image_bpf_class
   int get_coords3(float* &coords_out);

   // Set/get image logic
   int set_reread_pending();
   int set_recalc_pending();
   int get_intensity_type();		// 5 for unsigned char data, 6 for 3-unsigned char rgb
   
   // Virtual
   virtual int get_type(char* type);
   virtual double get_x(int i);																				// For image_las_class / image_bpf_class
   virtual double get_y(int i);																				// For image_las_class / image_bpf_class
   virtual double get_z(int i);																				// For image_las_class / image_bpf_class
   virtual unsigned short* get_intensa();																	// For image_las_class / image_bpf_class
   virtual unsigned short* get_reda();																		// For image_las_class / image_bpf_class
   virtual unsigned short* get_grna();																		// For image_las_class / image_bpf_class
   virtual unsigned short* get_blua();																		// For image_las_class / image_bpf_class
   virtual unsigned char* get_tau();																		// For image_las_class / image_bpf_class

   virtual int get_z_at_percentiles(float percentile1, float percentile2, float percentile3, float &z1, float &z2, float &z3, int diag_flag);	// For image_las_class / image_bpf_class

};

#endif /* __cplusplus */
#endif /* _image_ptcloud_class_h_ */
