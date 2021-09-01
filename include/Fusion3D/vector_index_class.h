#ifndef _vector_index_class_h_
#define _vector_index_class_h_
#ifdef __cplusplus

/**
Maintains the central index for vector overlay files.

Allows filenames and pathnames for vector overlay file to be fetched that meet certain criteria.
These criteria include the type of data, and the location (the name of the map they are associated with).\n
\n
	type_flags\n
		0 = "map" -- KML from DEM fileset --  display but do not write\n
		1 = "display" -- display only, cant edit or animate\n
		2 = "bookmarks" -- bookmarks (can edit, jump to)
		3 = "mti" -- GMTI -- adds special processing for GMTI data\n
		4 = "track" -- adds special processing for time/motion and measuring distance\n
		5 = "sensorml" -- adds special processing for ugs SensorML\n
		6 = "om" -- adds special processing for ugs OM\n
		7 = 'draw" -- files for drawing, typically military scenarios\n
		8 = 'cad" -- files for CAD models\n
		99 = deleted\n
\n
	vis_flags	1 if file is currently visible, 0 if invisible.\n
\n
	update_flags	Per type, 1 if all overlays of that type need update, 0 if not.\n
\n

*/
class vector_index_class:public base_jfd_class{
 private:
   int n_files;					///< No of files read
   
   vector<int>visFlags;			///< For each vector overlay -- Vis (per file)
   vector<int>typeFlags;		///< For each vector overlay -- File usage types 
								///<	0=KML from Map fileset -- display but dont write
								///<	1=display
								///<    2=bookmarks
   								///<	3=mti		-- adds special processing for GMTI data
   								///<	4=track		-- adds special processing for time/motion and measuring distance
   								///<	5=sensorml	-- adds special processing for ugs SensorML
								///<	6=om		-- adds special processing for ugs OM
								///<	7=draw		-- adds special processing for drawing scenarios
								///<	8=cad		-- adds special processing for CAD models
								///<	99=deleted	-- 
   vector<string>filenameS;		///< For each vector overlay -- Filename
   vector<float>red;			///< For each vector overlay -- (rgb) for drawing vectors
   vector<float>grn;			///< For each vector overlay -- (rgb) for drawing vectors
   vector<float>blu;			///< For each vector overlay -- (rgb) for drawing vectors
   vector<float>elevOffset;		///< For each vector overlay -- Offset elevation
   vector<int>DefaultAltitudeMode;///< For each vector overlay -- 0=clamp-to-ground, 1=rel-to-grnd, 2=absolute, 3=rel to attribute (default if not explicitly specified)
   vector<int>DefaultFillMode;	///< For each vector overlay -- Polygon fill:  0=wireframe, 1=filled (default if not explicitly specified)
   vector<int>asAnnotationFlag;	///< For each vector overlay -- 0=SoSeparator (mix with map), 1=SoAnnotate 
   vector<int>renderedFlag;		///< For each vector overlay -- 0=not yet rendered, 1=rendered (transfered to local manager storage) 

   int *update_flags;			///< Per filetype, does it need updating

 public:
   vector_index_class(int nmax);
   ~vector_index_class();
   
   int add_file(string name, int type, float redi, float grni, float blui, float hoff, int altMode, int fillMode, int annFlag);
   int del_file(int ifile);
   int set_update_flag(int type, int val);
   int set_vis(int ifile, int val);
   int set_rendered_flag(int ifile, int val);
   int clear_all() override;
   
   int read_tagged(const char *filename);
   int write_parms(FILE *out_fd, int type);
   
   int get_update_flag(int type);
   int get_n();
   const char* get_name(int ifile);
   const char* get_name_nopath(int ifile);
   int get_type(int ifile);
   int get_vis(int ifile);
   int get_rgb(int ifile, float &redo, float &grno, float &bluo);
   float get_elev_offset(int ifile);
   int get_default_alt_mode(int ifile);
   int get_default_fill_mode(int ifile);
   int get_annotation_flag(int ifile);
   int get_rendered_flag(int ifile);
};
#endif /* __cplusplus */
#endif /* _vector_index_class_h_ */
