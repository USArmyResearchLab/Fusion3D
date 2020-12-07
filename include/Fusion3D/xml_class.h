#ifndef _xml_class_h_
#define _xml_class_h_	
#ifdef __cplusplus

/**
Imports XML files, but is implemented specifically to read UGS files.

This class is a local special-purpose implementation of the xML standard.
It was written to do specific tasks associated with UGS sensor files.

*/
class xml_class:public vector_layer_class{
   
   private:
	  int file_type;					///< Type of xml file -- 0=unknown, 1=SensorML SML, 2=SensorML O&M, 3=OSUS (both status and O&M)
	  double sensor_lat;				///< Sensor -- loc
	  double sensor_lon;				///< Sensor -- loc
	  double sensor_north;				///< Sensor -- loc
	  double sensor_east;				///< Sensor -- loc
	  float sensor_elev;				///< Sensor -- loc
	  int sensor_camera_flag;			///< Sensor -- 1 iff sensor includes camera
	  int sensor_bearing_flag;			///< Sensor -- 1 iff sensor may output a bearing
	  int sensor_stationary_flag;		///< Sensor -- 1 iff sensor is stationary and has a fixed loc
	  double sensor_offset_lat;			///< Sensor -- Latitude  offset to translate sensor location to different map
	  double sensor_offset_lon;			///< Sensor -- Longitude offset to translate sensor location to different map
	  float sensor_offset_elev;			///< Sensor -- Elevation offset to translate sensor location to different map

	  int sensor_mode_flag;				///< O&M -- modality flag 0=no data, 1=seismic, 2=magnetic, 3=acoustic
	  int sensor_az_flag;				///< O&M -- 1 iff azimuth angle defined for this sensor
	  int sensor_el_flag;				///< O&M -- 1 iff elevation angle defined for this sensor
	  float sensor_az;					///< O&M -- azimuth angle in deg cw from North (defined like a compass heading)
	  float sensor_el;					///< O&M -- elevation angle in deg up from horizontal
	  int sensor_image_flag;			///< O&M image -- 0=no image, 1=jpeg, 2=jpeg2000, 3=png, 4=geoTiff
	  int sensor_jpeg_flag;				///< O&M image -- flag indicating whether image has been converted to jpeg
	  unsigned char* sensor_image_buf;	///< O&M image data array
	  int sensor_image_nx;				///< O&M image size
	  int sensor_image_ny;				///< O&M image size
	  int sensor_image_psize;			///< O&M image bits per pixel

	  double pointing_north;			///< O&M Mobile -- pointing location
	  double pointing_east;				///< O&M Mobile -- pointing location

	  int wami_armed_flag;				///< O&M wami -- 0=dont look for response to WAMI request, 1=look for response
	  std::string wami_sensor_name;		///< O&M wami -- name of sensor from which WAMI is requested
	  float wami_arm_time;				///< O&M wami -- time of file must be greater than arm time (when WAMI was requested) before response is accepted
	  int wami_response_flag;			///< O&M wami -- 0=no wami response within file, 1 if there is
	  std::string wami_addr;			///< O&M wami -- addr of wami stream within response

	  std::string sensor_simage_base64;	///< O&M image base64 encoded
	  std::string sensor_simage_jpeg;	///< O&M image as jpeg string
	  std::string sensor_stime;			///< O&Mtime string in format 2015-06-10T17:07:01Z
	  std::string sensor_sid;			///< Sensor/O&M id
	  std::string sensor_sname;			///< short name
	  std::string sensor_sloc;			///< Working

	  std::string level0;		///< Working -- Stores entire file with comments erased (parsers may get bogus info from comments)
	  std::string level1;		///< Working -- Substring
	  std::string level2;		///< Working -- Substring
	  std::string level3;		///< Working -- Substring
	  std::string level4;		///< Working -- Substring
      std::string base64_chars;	///< Working -- For converting base64 to jpeg
      std::string stemp;		///< Working -- 
      std::ifstream logstr;		///< Working --  OSUS log --  input stream

	  std::vector<string>valid_id;					///< Valid sensor type -- Unique portion of sensor name that will ID this type of sensor
	  std::vector<int>valid_camera_flag;			///< Valid sensor type -- 1 iff sensor has camera
	  std::vector<int>valid_bearing_flag;			///< Valid sensor type -- 1 iff sensor produces line of bearing
	  std::vector<int>valid_stationary_flag;		///< Valid sensor type -- 1 iff sensor stationary

	  int n_warn;

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
      xml_class();
      ~xml_class();

	  int set_type_osus();
	  int set_type_sensorml();
	  int set_type_oandm();
	  int set_sensor_offset(double lat, double lon, float elev);
	  int add_valid_sensor_type(string unique_id_string, int camera_flag, int bearing_flag, int stationary_flag);
	  int set_wami_flag(string sensor_name, float arm_time);
      
      int read_file(string sfilename) override;
	  int set_base64(std::string const& encoded_string, int image_type);
	  int convert_image();
	  int write_sensor_image_jpeg(char *filename);
      
	  double get_sensor_lat();
	  double get_sensor_lon();
      double get_sensor_north();
      double get_sensor_east();
      float get_sensor_elev();
	  float get_sensor_az();
	  float get_sensor_el();
	  int get_sensor_modality();
      string get_sensor_id();
      string get_sensor_short_name();
      string get_sensor_time();
	  string get_sensor_image_base64();
	  string get_sensor_image_jpeg();
	  int get_sensor_image_nx();
	  int get_sensor_image_ny();
	  int get_sensor_image_psize();
	  unsigned char* get_sensor_image_data();
	  int get_wami_request(string &wami_addr);

	  double get_pointing_north();
	  double get_pointing_east();

	  int get_sensor_az_flag();
	  int get_sensor_el_flag();
	  int get_sensor_image_flag();
	  int get_sml_camera_flag();
	  int get_sml_bearing_flag();
};

#endif /* __cplusplus */
#endif /* _xml_class_h_ */
