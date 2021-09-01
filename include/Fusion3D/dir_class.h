#ifndef _dir_class_h_
#define _dir_class_h_
#ifdef __cplusplus

/**
Stores the set of files for a map including DSMs, grayscale, color and MrSID textures, point clouds and KML extents.
Stores the set of files for a map including last-hit DEM, first-hit DEM, grayscale intensity texture, color texture, MrSID high-res texture,
point clouds and KML extents.\n
Either finds all the files in a given directory that match the given pattern or can store files individually where filenames dont follow a pattern.
\n
The class can search arbitrarily deep in a directory tree.
\n
The class stores the filenames that it finds and can be used for arbitrary sets of files.
However, the class is specially configured for map tiles following the Buckeye/Haloe/Alirt naming conventions.
The class includes storage for the primary ('a2') DEM files and other commonly associated files for lidar elevation map tiles.
By this convention, each subsection or tile of the map includes files like the following:\n
\n
dem_1m_a2_tile1.tif		base file containing last hit (a2) elevation data that is searched for\n
dem_1m_a1_tile1.tif		associated file containing first hit (a1) elevation data (derived from a2 file by '_a2_' -> '_a1_')\n
mrg_1m_a1_tile1.tif		associated file containing rgb texture data (derived from a2 file by '_a2_' -> '_a1_' and 'dem_' -> 'mrg_')\n
int_1m_a1_tile1.tif		associated file containing intensity texture data (derived from a2 file by '_a2_' -> '_a1_' and 'dem_' -> 'int_')\n
otrhophoto_tile1.sid	associated MrSID file containing high-resolution orthophoto (arbitrary filename)


*/
class dir_class:public base_jfd_class{
 private:
   int nfiles_a2;			///< No of a2 DEM files in dir satisfying inclusion criteria
   int nfiles_a1;			///< No of a1 DEM files in dir satisfying inclusion criteria
   int nfiles_mrg;			///< No of rgb texture files in dir satisfying inclusion criteria
   int nfiles_int;			///< No of intensity texture files in dir satisfying inclusion criteria
   int nfiles_mrsid;		///< No of MrSID files in dir satisfying inclusion criteria
   int nfiles_kml;			///< No of KML files in dir satisfying inclusion criteria
   int nfiles_ptcloud;		///< No of point-cloud files in dir satisfying inclusion criteria
   int nfiles_mask;			///< No of mask files in dir satisfying inclusion criteria
   int nfiles_low_tex;		///< No of low-res texture files in dir satisfying inclusion criteria

   string sdirname;			///< Name of base dir to be searched
   char *pattern_kml;		///< Match pattern for KML files
   char *pattern_a2i;		///< Match pattern for a2 files
   char *pattern_mrsidi;	///< Match pattern for MrSID files
   char *match_pat;			///< Working storage for patterns
   
   std::vector<string>a2name;		///< Per file -- a2 DEM pathname
   std::vector<string>a1name;		///< Per file -- a1 DEM pathname
   std::vector<string>mrgname;		///< Per file -- rgb texture pathname
   std::vector<string>intname;		///< Per file -- gray texture pathname
   std::vector<string>mrsidname;	///< Per file -- MrSID pathname
   std::vector<string>kmlname;		///< Per file -- KML pathname
   std::vector<string>ptcloudname;	///< Per file -- point cloud pathname
   std::vector<string>maskname;		///< Per file -- mask pathname
   std::vector<string>lowtname;		///< Per file -- low-res texture pathname

   int fileset_defined_flag;///< 0 if not defined, 1 if defined (filenames defined by search or add tile)
   int texture_filetype;	///< 1='int', 2='mrg', 0=mrsid only, -99=unknown
   int sensor_type;			///< 0=unknown, 1=Buckeye, 2=Haloe, 3=ALIRT
   int search_kml_flag;		///< 0=dont search, 1=include search for KML files
   
 public:
   dir_class();
   ~dir_class();

   int find_by_sensor(const char *pattern_a2, const char *pattern_mrsid, int recursive_flag);
   int find_buckeye  (const char *pattern_a2, const char *pattern_mrsid, int recursive_flag);
   int find_haloe    (const char *pattern_a2, const char *pattern_mrsid, int recursive_flag);
   int find_alirt    (const char *pattern_a2, const char *pattern_mrsid, int recursive_flag);
   
   int set_dem_dir(string dir);
   int set_dem_fileset_defined();
   int set_search_kml_flag();
   int add_file(string name_in, int type);
   int find_all_with_pattern(const char *match_pattern, int file_type, int recursive_flag);	// Get names -- filenames in format *<prefix>*<suffix>
   int check_valid_demset();
   int check_valid_pcset();
   int clear_all() override;
   int clear_demset();

   int get_nfiles_a2();
   int get_nfiles_a1();
   int get_nfiles_texture_rgb();
   int get_nfiles_texture_gray();
   int get_nfiles_mrsid();
   int get_nfiles_kml();
   int get_nfiles_ptcloud();
   int get_nfiles_mask();
   int get_nfiles_low_tex();

   string get_a2_name(int itile);
   string get_a1_name(int itile);
   string get_rgb_texture_name(int itile);
   string get_gray_texture_name(int itile);
   string get_mrsid_name(int itile);
   string get_kml_name(int itile);
   string get_ptcloud_name(int itile);
   string get_mask_name(int itile);
   string get_low_tex_name();

   int copy_a2_name(int itile, char *name);
   char *get_a2_pattern();
   char *get_mrsid_pattern();
   char *get_kml_pattern();
   string get_dirname();
   int get_texture_filetype();
   int get_sensor_type();
   int is_fileset_defined();
};

#endif /* __cplusplus */
#endif /* _dir_class_h_ */
