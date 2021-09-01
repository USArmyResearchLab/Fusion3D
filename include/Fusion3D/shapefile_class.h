/*
   Reads and write track points in Shapefile format
   
   Hardwired:
   	The dbf delete flags are ignored
	The fields for lat, long and MGRS are hardwired

*/
#ifndef _shapefile_class_h_
#define _shapefile_class_h_	
#ifdef __cplusplus

/// @cond		Exclude from Doxygen
typedef struct {
   int record_no;
   int content_len;
} record_header;

typedef struct {
   int offset;
   int len;
} index_record;

typedef struct {
   unsigned char version;
   unsigned char date_yy;
   unsigned char date_mm;
   unsigned char date_dd;
   int nrec;
   short int rec_pos;
   short int rec_len;
   char dum2[16];
   unsigned char table_flags;
   char dum3[3];
} dbf_file_header;

typedef struct {
   char name[11];
   char type;
   int displacement;
   unsigned char len;
   unsigned char ndec;
   char field_flag;
   int inc_next;
   unsigned char inc_step;
   char reserved[7];
   unsigned char index_field_flag;
} dbf_subrecord;
/// @endcond	End Doxygen exclusion

/**
Imports and exports data in Shapefile format -- mostly superceded by ogr_class which is more general and better tested.

This class was written before we adopted the GDAL/OGR library, which has superceded most of its functions.
The OGR implementation is more general and better maintained and tested so should be used for future work.\n
\n
The only application of this class at this time is to write LOS shadow boundaries --
a very complex shape that is difficult to validate.
Therefore the application to LOS was left in this class but should probably be moved to the ogr_class if that application
is ever reviewed.

*/
class shapefile_class:public vector_layer_class{
   
   private:
      int purpose_flag;		// Shapefiles so ill-defined, needs some general direction in dealing with
      						//	0 = unknown
							//	1 = track registration (copy as much as possible from input to output)
							//	2 = track digitization
							//	3 = LOS regions
      int ll_utm_flag;		// Location units 
      						// 	0 = internal UTM, file LL (default)
							// 	1 = internal UTM, file UTM
							//	2 = internal LL,  file LL
      int time_flag;		// Time -- 1 iff times defined

      record_header*	shp_rec_header;			// Shp record headers
      int shp_file_code, shp_file_len, shp_version;	// Shp header info
      int shp_shape_type;				// Shp header info
      double shp_bbminx, shp_bbminy, shp_bbminz;	// Shp header info
      double shp_bbmaxx, shp_bbmaxy, shp_bbmaxz;	// Shp header info
      double shp_bbminm, shp_bbmaxm;			// Shp header info
      int shp_rec_header_content_len;			// Shp record header -- Count in 16-bit words
      
      index_record*	shx_index_rec;			// Shx 
      int shx_file_len;					// Shx header info
      int shx_nrec;					// Shx -- file len in 16-bit words
      
      dbf_file_header*	dbf_header;			// Dbf -- Header
      dbf_subrecord**	dbf_subrec;			// Dbf -- Subrecord header
      int dbf_nrec;					// Dbf -- No of records
      short int dbf_rec_pos, dbf_rec_len;		// Dbf -- header info
      int dbf_displacement, dbf_inc_next;		// Dbf -- header info
      unsigned char dbf_subrec_len;			// Dbf -- header info
      int dbf_nfields;					// Dbf -- No of fields
      char** dbf_cfield;				// Dbf -- Field values
      int* dbf_len_field;				// Dbf -- Field lengths
      char* dbf_del_flags;				// Dbf -- Delete flag for each record (*=del)

      char** dbf_wname;					// Dbf write -- name of field
      char** dbf_wentry;				// Dbf write -- entry in constant field
      char* dbf_wtype;					// Dbf write -- type of entry
      int* dbf_wvar_flag;				// Dbf write -- 0=constant for all recs, 1=var ints, 2=var chars
      int* dbf_wlen;					// Dbf write -- length of entry
      int* dbf_wdec;					// Dbf write -- length of record
      int** dbf_wflag_ptr;				// Dbf write -- ptr to array of flags
      char*** dbf_wchars;				// Dbf write -- variable character entries
      char* dbf_wwork;					// Dbf write -- work string
      
      double poly_xmin;			// Polyline -- 
      double poly_ymin;			// Polyline -- 
      double poly_xmax;			// Polyline -- 
      double poly_ymax;			// Polyline -- 
      int poly_num_parts;		// Polyline --
      int* poly_parts;			// Polyline --
      
      int nparts;
      int ipt_current;
      int *part_start, *part_end;
      
      double *part_xmin, *part_xmax;	// Extents -- 
      double *part_ymin, *part_ymax;	// Extents -- 
      double shape_xmin, shape_xmax;	// Extents --
      double shape_ymin, shape_ymax;	// Extents --

      int npts;				// Points --
      double* xpt;			// Points --
      double* ypt;			// Points --
      int* iframe_pt;			// Points -- Per point, frame no
      int* flag_pt;			// Points -- Per point, interpolation flag (0=no, 1=yes)
      float *time_pt;			// Points -- Per point, time
      
      int *out_rec_size;		// Output -- shp/shx record sizes
      char *begin_time_full_string;	// Time string including date for first data point
	  float end_time;			// Seconds after midnight
      
      int input_fd, output_fd;		// Work
      int bend_sgi_flag;		// Work -- 1 iff computer is big-endian (SGI), 0 if little endian (PC)
      int n_files_read;			// No of files read in
      int output_shape_type;		// 1=points, 3=polyline (default=1)
      float interp_dist;		// If no interpolation flags, use this dist to dummy these flags
      int ifield_interp_flag;		// <=0 iff interpolation flags in data (set to field no)
      
      char *name_mod_dbf;		// File Mod -- Name of input file
      char *name_mod_shx;		// File Mod -- Name of input file
      char *name_mod_prj;		// File Mod -- Name of input file
      
      char *name_attr_copy_from;	// If not NULL, copy output attributes from this file	
      
      // Private methods
      int read_point(int irec);
      int read_polyline();
      int read_polygon();
      int read_shp_header();
      int read_shx_header();
      int read_dbf();
      int write_shx();
      int write_shp();
      int copy_shx();
      int copy_prj();
      int copy_dbf(char *filename);
      int transfer_dbf(char *filename);
      int write_shp_shx_header(int file_len);
      int write_dbf_los();
      int write_polyline(int ipart);
      int calc_output_lims();
      int calc_extents();
      int local_free();
      int make_interp_flags();
      
   public:
      shapefile_class();
      ~shapefile_class();
      
      int set_purpose_flag(int flag);
      int set_n_points(int npts_in);
      int set_parts(int n_poly, int* poly_end);
      int set_x(int i, double x);
      int set_y(int i, double y);
      int set_output_shape_type(int type);
      int set_ll_or_utm(int flag);
      int set_dbf_parm_const_numeric(const char* field_name, int len, int dec, const char* entry);
      int set_dbf_parm_var_ints(const char* field_name, int* entry, int entry_len);
      int set_dbf_parm_var_chars(const char* field_name, char** entry, int entry_len);
      int set_interp_dist(float dist);
      int set_begin_time_full_string(const char *ctime);
      int set_end_time(float time);
	  int copy_attr_from_file(const char *filename);
      
      int read_file(string sfilename) override;
      int write_file(string sfilename) override;
      
      int get_n_points(int ipart);
      double* get_x();
      double* get_y();
      int get_time_flag();
      float* get_time();
      int* get_iframe();
      int* get_interpolation_flag();
      char *get_begin_time_full_string();
      int get_end_time();
};

#endif /* __cplusplus */
#endif /* _shapefile_class_h_ */
