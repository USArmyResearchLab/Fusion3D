#ifndef _sensor_read_osus_class_h_
#define _sensor_read_osus_class_h_	
#ifdef __cplusplus

/**
Imports XML files, but is implemented specifically to read UGS files.

This class is a local special-purpose implementation of the xML standard.
It was written to do specific tasks associated with UGS sensor files.

*/
class sensor_read_osus_class:public sensor_read_class{
   
   private:
	  int sensor_az_flag;				///< O&M -- 1 iff azimuth angle defined for this sensor
	  int sensor_el_flag;				///< O&M -- 1 iff elevation angle defined for this sensor
	  float sensor_az;					///< O&M -- azimuth angle in deg cw from North (defined like a compass heading)
	  float sensor_el;					///< O&M -- elevation angle in deg up from horizontal

	  int sensor_image_flag;			///< O&M image -- 0=no image, 1=jpeg, 2=jpeg2000, 3=png, 4=geoTiff
	  unsigned char* sensor_image_buf;	///< O&M image data array
	  int sensor_image_nx;				///< O&M image size
	  int sensor_image_ny;				///< O&M image size
	  int sensor_image_psize;			///< O&M image bits per pixel

	  std::string sensor_simage_base64;	///< O&M image base64 encoded
	  std::string sensor_simage_jpeg;	///< O&M image as jpeg string
	  std::string sensor_stime;			///< O&Mtime string in format 2015-06-10T17:07:01Z
	  std::string sensor_sid;			///< Sensor/O&M id
	  std::string sensor_sloc;			///< Working

	  std::string level0;		///< Working -- Stores entire file with comments erased (parsers may get bogus info from comments)
	  std::string level1;		///< Working -- Substring
	  std::string level2;		///< Working -- Substring
	  std::string level3;		///< Working -- Substring
	  std::string level4;		///< Working -- Substring
      std::string base64_chars;	///< Working -- For converting base64 to jpeg
      std::string stemp;		///< Working -- 
      std::ifstream logstr;		///< Working --  OSUS log --  input stream

	  // Private methods
	  int clear();
	  int parse_osus(std::string level0t, const char* filename);
      int find_lat(const char *filename, std::string stringIn, double &lat);
      int find_lon(const char *filename, std::string stringIn, double &lon);
      int find_time(const char *filename, std::string stringIn, string &time);
      int find_substring_within(std::string stringIn, std::string &stringOut, std::string stringBeg, std::string stringEnd, int &n_pos);
	  int get_string_two_levels(std::string beg1, std::string end1, std::string beg2, std::string end2, std::string &time);
	  int if_string_two_levels(std::string beg1, std::string end1, std::string desired);
	  int find_substring(std::string stringIn, std::string &stringOut, std::string stringBeg, std::string stringEnd, int &n_pos);
	  std::string base64_decode(std::string const& encoded_string);
	  int convert_image_3types(int type);
	  int convert_geotiff();

      static inline bool is_base64(unsigned char c) {
             return (isalnum(c) || (c == '+') || (c == '/'));
      }

   public:
      sensor_read_osus_class();
      ~sensor_read_osus_class();

      int read_file(string sfilename) override;
	  int set_base64(std::string const& encoded_string, int image_type);
	  int convert_image();
	  int write_sensor_image_jpeg(char *filename);
      
	  float get_sensor_az();
	  float get_sensor_el();
	  string get_sensor_image_base64();
	  string get_sensor_image_jpeg();
	  int get_sensor_image_nx();
	  int get_sensor_image_ny();
	  int get_sensor_image_psize();
	  unsigned char* get_sensor_image_data();

	  int get_sensor_az_flag();
	  int get_sensor_el_flag();
	  int get_sensor_image_flag();

	  int get_current_image(int &nx, int &ny, int &nb, unsigned char *&data);
};

#endif /* __cplusplus */
#endif /* _sensor_read_osus_class_h_ */
