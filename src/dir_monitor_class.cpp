#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
dir_monitor_class::dir_monitor_class()
	:base_jfd_class()
{
   nfiles	= 0;
   max_create_time = 0;
   min_create_time = 0;
   use_modify_time_flag = 0;
   pattern	= new char[100];
   dirname 		= new char[300];
   match_pat	= new char[100];
   strcpy(pattern, "");
   strcpy(dirname, "");
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
dir_monitor_class::~dir_monitor_class()
{
   delete[] dirname;
   delete[] pattern;
   delete[] match_pat;
}

// ******************************************
/// Set the pathname for the directory to be searched.
// ******************************************
int dir_monitor_class::set_dir(const char *dirx)
{
   strcpy(dirname, dirx);
   return(1);
}

// ******************************************
/// Set flag to only accept files written after the specified write time.
/// @param	mintime		Time in UTC (long integer seconds after specified date)
// ******************************************
int dir_monitor_class::set_min_create_time(time_t mintime)
{
   min_create_time = mintime;
   max_create_time = mintime;
   return(1);
}

// ******************************************
/// Set flag to use file modify time instead of file create time to order files and test for new files.
// ******************************************
int dir_monitor_class::set_use_modify_time()
{
   use_modify_time_flag = 1;
   return(1);
}

// ******************************************
/// Get the latest write time of all accepted files from previous search.
// ******************************************
time_t dir_monitor_class::get_max_create_time()
{
   return max_create_time;
}

// ******************************************
/// Get the number of files that have been found.
// ******************************************
int dir_monitor_class::get_nfiles()
{
   return nfiles;
}

// ******************************************
/// Copy the pathname for the file to a string for the given index.
/// The string must be allocated outside this class.
// ******************************************
int dir_monitor_class::copy_name(int itile, char *name_out)
{
   strcpy(name_out, filename[itile].c_str());
   return(1);
}

// ******************************************
/// Get the pattern used to search for files.
// ******************************************
char* dir_monitor_class::get_pattern()
{
   return(pattern);
}

// ******************************************
/// Get the name of the directory that is being searched.
// ******************************************
char* dir_monitor_class::get_dirname()
{
   return(dirname);
}

// ******************************************
/// Get the file pathname string for the given index.
// ******************************************
string dir_monitor_class::get_name(int itile)
{
	if (itile >= nfiles) {
		return NULL;
	}
	else {
		return filename[itile];
	}
}

// ******************************************
/// Get the file pathname string for the given index.
// ******************************************
time_t dir_monitor_class::get_create_time(int itile)
{
	if (itile >= nfiles) {
		return NULL;
	}
	else {
		return timecreate[itile];
	}
}

// ******************************************
/// Find all files in the given directory that match the given pattern.
/// The method is now implemented to search arbitrarily deep into the directory tree.
/// Only accepts files with create times after previously-read files
/// @param match_pattern	Pattern is constrained only allowing wildcard '*' to match arbitrary number of characters
/// @param recursive_flag	1 to search all subdirectories down to arbitrary depth, 0 to search only the given directory
// ******************************************
int dir_monitor_class::find_all_with_pattern(const char *match_pattern, int recursive_flag)
{
	string sdirname = dirname;
	string spattern = match_pattern;
	vector<string> listNames;
	vector<time_t> listTimes;
	find_all_files_matching_pattern(sdirname, spattern, recursive_flag, listNames, listTimes);
	int n = listNames.size();
	for (int i = 0; i < n; i++) {
		time_t file_create_time = listTimes[i];
		if (max_create_time < file_create_time) {
			max_create_time = file_create_time;
			if (diag_flag > 0)  cout << "Add " << listNames[i] << " create time " << file_create_time << endl;
			filename.push_back(listNames[i]);
			timecreate.push_back(file_create_time);
			nfiles++;
		}
		else {
			if (diag_flag > 0) cout << "Skip " << listNames[i] << " create time " << file_create_time << endl;
		}
	}
	return(1);
}
