#ifndef _base_jfd_class_h_
#define _base_jfd_class_h_	
#ifdef __cplusplus

/**
Base class for many classes.

Includes a reference location -- latitude, longitude , UTM coordinates and zone and reference name (e.g. "Lubbock")
and methods to set this location.
Includes basic methods to display basic warning and exit messages.
Includes method to get the name of the class (e.g. "KML"

*/
class base_jfd_class{
   protected:
      int diag_flag;				///< Diagnostic level -- 0=none, 1=little, 2=moderate, 3=heavy
      int opsys_flag;				///< Operating system -- 0=unix, 1=windows, 2=linux
      char *class_type;				///< Unique name for each child class

	  int localCoordFlag;			///< Coord systems -- -1=unknown local coord system, 0=local same as global, Positive=EPSG where known local coord different from global 
	  gps_calc_class *gps_calc;		///< Coord systems -- Worker class to define global coordinate systems and transformations

   public:
      base_jfd_class();
      virtual ~base_jfd_class();
      
      int set_diag_flag(int flag);
	  int register_coord_system(gps_calc_class *gps_calc_in);
      const char *get_type();
      
	  int warning(int oiv_window_flag, const char* message);
	  int warning_s(string cmessage, string smessage);
	  int exit_safe(int oiv_window_flag, const char* message);
	  int exit_safe_s(string message, string smessage);

	  virtual int clear_all();
	  virtual int read_file(string sfilename);
	  virtual int read_file_open(string sfilename);
	  virtual int read_file_header();
	  virtual int read_file_data();
	  virtual int read_file_close();
	  virtual int write_file(string sfilename);
	  virtual int write_parms(FILE *tiff_fd);
};

#endif /* __cplusplus */
#endif /* _base_jfd_class_h_ */
