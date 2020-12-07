/** @file */
#ifndef _sbet_class_h_
#define _sbet_class_h_	
#ifdef __cplusplus

/**
Imports sbet files -- files associated with mobile-mapping platforms that record the location and attitude of the vehicle collecting the data.
   
At this time, it is only implemented to read a .txt file that lists the vehicle position at the times when a camera picture is taken.
There are 4 cameras as given in the second column.\n
   \n
Format 1:  \n
\n
lots of header lines\n
\n
[Index]\n
ImNum	Cam	CamImNm	Rotate	Error	GPSTime	Index	Filename	Latitude	Longitude	Altitude	Roll	Pitch	Heading\n
1	4	1	0	5	136022.777770	0	US51_11234_1A1_C4_000001.jpg	38.3122037913	-77.4764542829	-13.9976	-0.472439	-0.082086	213.096267	2456	2058	0	0\n
2	3	1	0	5	136022.777770	0	US51_11234_1A1_C3_000001.jpg	38.3122037913	-77.4764542829	-13.9976	-0.472439	-0.082086	213.096267	2456	2058	0	0\n


*/
class sbet_class:public vector_layer_class{
   
   private:
      int i_format;			// Format
	  int npts;				// No of points
	  int index_cur;		// Current position in time array for searches
      
	  float *heading_a;			// Points -- Vehicle heading (azimuth) angle in deg
      
      float time_delta;			// Time -- Time difference between samples (<0 if not regular samples)
      float time_first_data;		// Time -- Time of first data point
      float time_last_data;		// Time -- Time of last data point
      
      FILE *tiff_fd;			// Work
      
      // Private methods
      int read_data_format1();
	  int get_parms_at_time(double locsec, double &east, double &north, float &height, float &roll, float &pitch, float &az);
      
   public:
	sbet_class();
      ~sbet_class();
      
      int set_format(int i_format_in);
      int read_file(string sfilename) override;
	  int write_file(string sfilename) override;

	  int get_scan_parms(double x, double y, double z, double time, int sc1Flag, int scanAngEst, float &scanAng, float &scanRange);
};

#endif /* __cplusplus */
#endif /* _sbet_class_h_ */
