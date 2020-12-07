#ifndef _nato4607_class_h_
#define _nato4607_class_h_	
#ifdef __cplusplus

/// @cond		Exclude from Doxygen
typedef struct {
   double existence_mask;
   short int revisit_index;
   short int dwell_index;
   unsigned char last_dwell_of_revisit;
   short int target_report_count;
   int dwell_time;
   int sensor_lat;
   int sensor_lon;
   int sensor_alt;
   int lat_scale;
   
   int lon_scale;
   int uncertain_along;
   int uncertain_cross;
   short int uncertain_alt;
   short int sensor_track;
   int sensor_speed;
   unsigned char sensor_vertical_vel;
   unsigned char sensor_track_uncertainty;
   short int sensor_speed_uncertainty;
   short int sensor_vertical_velocity_uncertainty;
   
   short int platform_heading;
   short int platform_pitch;
   short int platform_roll;
   float area_lat;
   float area_lon;
   short int area_range;
   short int area_angle;
   short int sensor_heading;
   short int sensor_pitch;
   short int sensor_roll;
   
   unsigned char min_velocity;
} nato4607_dwell_header;
/// @endcond	End Doxygen exclusion

/**
   Imports GMTI data in NATO 4607 format.

   The class allows you to retrieve data within a given time window -- from the current time backward with a user-defined time window.
   It has been tested with LSRS data. 
   
*/
class nato4607_class:public roi_meta_class{
   
   private:
      nato4607_dwell_header*	dwell_header;			// Dwell Segment header
      
      int packet_size;			// Size of current packet
      int n_packets;			// No of packets read
      int segment_size;			// Size of current segment
      int segment_type;			// Type of current segment (2=dwell-segment)
      
      int idwell_start, idwell_end;	// Dwells -- Beginning and end of output interval
      
      float xmine, xmaxe, ymine, ymaxe;		// Extent of data
	  float footprint_verts[8];				// 4 points defining footprint (x1, y1, x2, y2, x3, y3, x4, y4)
      
      int segment_end;				// Location of end of current segment
      int nskip_treport;			// No of chars to skip at end of target report

      int maskd2 , maskd3 , maskd4 , maskd5 , maskd6 , maskd7 , maskd8 ;	// Dwell masks
      int maskd9 , maskd10, maskd11, maskd12, maskd13, maskd14, maskd15;
      int maskd16, maskd17, maskd18, maskd19, maskd20, maskd21, maskd22;
      int maskd23, maskd24, maskd25, maskd26, maskd27, maskd28, maskd29;
      int maskd30, maskd31;
      int rmask1, rmask2, rmask3, rmask4, rmask5, rmask6, rmask7, rmask8;
      int rmask9, rmask10, rmask11, rmask12, rmask13, rmask14, rmask15, rmask16, rmask17;
      
      int input_fd;			// Work
      char* junk;			// Work
      
      // Private methods
      int read_packet_header();
      int read_segment_header();
      int read_dwell();
	  int read_job_segment();
      int get_masks(unsigned char *maska);
      int make_dummy();
	  void byteswap_oni_ui(unsigned int &ui);
      
   public:
      nato4607_class();
      ~nato4607_class();
      
      int read_file(string sfilename) override;
      int calc_dwell_current(int &idwell);
      int calc_dwell_history(int &idwell);
};

#endif /* __cplusplus */
#endif /* _nato4607_class_h_ */
