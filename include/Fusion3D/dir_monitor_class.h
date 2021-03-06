#ifndef _dir_monitor_class_h_
#define _dir_monitor_class_h_
#ifdef __cplusplus

/**
Stores the set of files in a given directory that match a given pattern and, if desired, that were last written after a given time.
Can also store files individually where filenames dont follow a pattern.
The class can search arbitrarily deep in a directory tree.
\n
*/
class dir_monitor_class:public base_jfd_class{
 private:
   int nfiles;				///< No of files in dir satisfying inclusion criteria
   char *dirname;			///< Name of base dir to be searched
   char *pattern;			///< Match pattern for files
   char *match_pat;			///< Working storage for patterns

   std::vector<string> filename;	///< Per file -- pathname
   std::vector<time_t> timecreate;	///< Per file -- creation time

   time_t max_create_time;			///< Accept only files created after this time -- max time of accepted files
   time_t min_create_time;			///< Accept only files created after this time -- min time in UTC to accept (default 0 accepts all files)
   int use_modify_time_flag;		///< 0=use file create time (default), 1=use file modify time
   
 public:
   dir_monitor_class();
   ~dir_monitor_class();

   int set_dir(const char *dir);
   int set_min_create_time(time_t mintime);
   int set_use_modify_time();
   int find_all_with_pattern(const char *match_pattern, int recursive_flag);	// Get names -- filenames in format *<prefix>*<suffix>

   int get_nfiles();
   time_t get_max_create_time();
   string get_name(int ifile);
   time_t get_create_time(int ifile);
   int copy_name(int ifile, char *name);
   char *get_pattern();
   char *get_dirname();
};

#endif /* __cplusplus */
#endif /* _dir_monitor_class_h_ */
