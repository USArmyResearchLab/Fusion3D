/** @file */
#ifndef _csv_class_h_
#define _csv_class_h_	
#ifdef __cplusplus

/**
Imports and exports files in comma separated values (csv) format.
   
Since this format is primitive, it is only implemented to import files in the following 2 specificformats:
   
Format 1:  My special format for writing a user-digitized vector consisting of a single line (used for outlining major features like rivers for user context):

Ref-UTM-M	 3720500.000000 233500.000000 0.000000\n
Place-Points		 195\n
0000    9287.08  -10132.50     980.00 waypoint\n
0001    9272.49   -8505.04     980.00 waypoint\n
...

Format 3:  Files for Bluegrass truthing in following format:

TimeUTC(s),Time(Local),Time(UTC),Latitude(deg),Longitude(deg),Altitude(feet-MSL),Velocity(m/s)\n
49530.000000,27-Sep-2007 08:45:30,27-Sep-2007 13:45:30,33.5350585,-101.9238997,3309.654531,1.000000\n
49535.000000,27-Sep-2007 08:45:35,27-Sep-2007 13:45:35,33.5350585,-101.9238997,3256.051836,1.000000\n
...


*/
class csv_class:public vector_layer_class{
   
   private:
      int i_format;			// Format
	  int npts;				// No of points
      
	  float *rolla;				///< Add to parent class -- roll angle in radians
	  float *pitcha;			///< Add to parent class -- pitch angle in radians
	  float *yawa;				///< Add to parent class -- yaw angle in radians
      
      float time_delta;			// Time -- Time difference between samples (<0 if not regular samples)
      float time_first_data;	// Time -- Time of first data point
      float time_last_data;		// Time -- Time of last data point
      
      FILE *tiff_fd;			// Work
      
      // Private methods
      int read_data_format1();
      int read_data_format3();
      int read_data_format4();
      
   public:
	csv_class();
      ~csv_class();
      
      int set_format(int i_format_in);
	  int read_file(string sfilename) override;
	  int write_file(string sfilename) override;

	  int get_angle_arrays(float* &roll, float* &pitch, float* &yaw);
};

#endif /* __cplusplus */
#endif /* _csv_class_h_ */
