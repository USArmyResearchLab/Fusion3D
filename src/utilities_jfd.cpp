#include "internals.h"
#if defined(LIBS_QT) 
	#include <qthread.h>
#endif

// ********************************************************************************
/// Search for a file or directory in a given directory tree.
/// Does recursive search in the application directory (where executable is located).  
/// If the file not found repeats search in parent directory and iterates until file found.
/// Implemented only for Qt.
// ********************************************************************************
int find_file_in_exec_tree(string name, string &path)
{
#if defined(LIBS_QT)
	bool done = FALSE;
	QString qpath = QCoreApplication::applicationDirPath();
	QStringList filters;
	filters.append(QString::fromStdString(name));
	while (!done) {
		cout << "Search in dir " << qpath.toStdString() << endl;
		QDirIterator it(qpath, QStringList() << filters, QDir::Files|QDir::Dirs, QDirIterator::Subdirectories);
		if (it.hasNext()) {						// Found the file -- just return path
			QString name = it.next();
			path = name.toStdString();
			cout << "   Found file/dir " << path << endl;
			done = TRUE;
		}
		else {									// Not found -- go to parent dir
			int i = qpath.lastIndexOf('/');
			if (i == -1) return(0);				// Cant go deeper, search fails
			qpath.truncate(i);
		}
	}
	return(1);
#else
	return(0);
#endif
}

// ********************************************************************************
/// Recursively searches given directory and returns all files that match the given pattern.
/// Cross Platform utility.
/// @param dirname		-- name of directory to be searched
/// @param pattern		-- search pattern -- constrained only allowing wildcard '*' to match arbitrary number of characters
/// @param listNames	-- output list of filenames satisfying pattern
/// @param listTimes	-- output list of file create times in seconds since start of Epoch UTC (1970).
// ********************************************************************************
int find_all_files_matching_pattern(string dirname, string pattern, int recursive_flag, vector<string> &listNames, vector<time_t> &listTimes)
{
#if defined(LIBS_QT)
	QString qdirname = QString::fromStdString(dirname);
	QDir dir(qdirname);
	dir.path();
	QStringList filters;
	filters.append(QString::fromStdString(pattern));
	QDirIterator::IteratorFlags flags;
	if (recursive_flag) {
		flags = QDirIterator::Subdirectories;
	}
	else {
		flags = QDirIterator::NoIteratorFlags;
	}
	QDirIterator it(dir.path(), QStringList() << filters, QDir::NoFilter, flags);
	while (it.hasNext()) {
		QString name = it.next();
		listNames.push_back(name.toStdString());
		if (&listTimes != NULL) {
			QFileInfo qfi(name);
			float ttt = stof(QT_VERSION_STR);			// Gets major and minor versions ie 5.12.1 -> 5.119999
														//QDateTime qdt = qfi.birthTime();			// Qt version 5.8 -- good but relative recent Qt may not be available
			QDateTime qdt = qfi.lastModified();
			//time_t tim = qdt.toSecsSinceEpoch();		// Qt version 5.8 -- good but relative recent Qt may not be available
			qint64 msec = qdt.toMSecsSinceEpoch();
			time_t tim = msec / 1000.;
			listTimes.push_back(tim);
			//QTime qt = qdt.time();							// Alternate def -- seconds since start of day
			//int msSinceStartDay = qt.msecsSinceStartOfDay();
			//float sSinceStartDay = float(msSinceStartDay) / 1000.;
			//listTimes.push_back(sSinceStartDay);
		}
	}

#else
	string subdirname, filename;
	char *newname = new char[300];
	char *cpat = new char[300];
	struct _finddata_t pgm_file;
	struct tm loctime;
	long hFile;
	strcpy(cpat, pattern.c_str());

	sprintf(newname, "%s\\*", dirname.c_str());
	hFile = _findfirst(newname, &pgm_file);	// Always "."
	while (_findnext(hFile, &pgm_file) == 0) {
		if (strncmp(pgm_file.name, ".", 1) == 0) {	// Disregard "." and ".."
		}
		else if ((pgm_file.attrib & _A_SUBDIR)) {			// Search subdirs
			subdirname = dirname;
			subdirname.append("/");
			subdirname.append(pgm_file.name);
			find_all_files_matching_pattern(subdirname, pattern, recursive_flag, listNames, listTimes);
		}
		else if (match_with_asterisks(pgm_file.name, cpat)) {
			filename = dirname;
			filename.append("/");
			filename.append(pgm_file.name);
			listNames.push_back(filename);
			time_t file_create_time = pgm_file.time_write;	// This seems to give more reasonable times
			//time_t file_create_time = pgm_file.time_create;
			localtime_s(&loctime, &file_create_time);
			listTimes.push_back(file_create_time);
		}
	}
	_findclose(hFile);
	delete[] newname;
	delete[] cpat;

#endif
	return(1);
}

// ******************************************
/// Match a filename with a pattern using the '*' char indicating an arbitrary number of characters. 
// ******************************************
int match_with_asterisks(char *file, char* pat)
{
	char *ptrp1, *ptrp2, *ptrf1, *ptrf2;
	char mstr[100];
	int nchar;
	
	ptrp1 = pat;
	ptrf1 = file;
	do {
		while (*ptrp1 == '*') ptrp1++;
		if (ptrp1 == NULL) return(1);
		strcpy(mstr, ptrp1);
		ptrp2 = strstr(mstr, "*");
		if (ptrp2 != NULL) *ptrp2 = '\0';
		ptrf2 = strstr(ptrf1, mstr);
		if (ptrf2 == NULL) return(0);
		if (ptrp1 == pat && ptrf2 != file) {	// If pattern doesnt start with *, make sure start of pattern matches start of filename
			return(0);
		}
		nchar = strlen(mstr);
		ptrf1 = ptrf2 + nchar;
		if (ptrp2 != NULL) {
			ptrp1 = ptrp2 + 1;
		}
	} while (ptrp2 != NULL);
   if (strcmp(ptrf1, "") != 0) return(0);	// Dont want to accept files like 'a.tif.kml'
   
   return(1);
}


// ********************************************************************************
/// Sleep.
/// Cross Platform utility.
/// @param msec Sleep time in msec
// ********************************************************************************
int cross_sleep(int msec)
{
#if defined(LIBS_QT)
	QThread::msleep(msec);
#else
	Sleep(msec);
#endif
	return(1);
}

// ********************************************************************************
/// Check that given file exists.
/// Cross Platform utility.
// ********************************************************************************
int check_file_exists(string filename)
{
#if defined(LIBS_QT) 
	QFile tt(QString::fromStdString(filename));
	if (!tt.exists()) {
		return(0);
	}
#else
	DWORD dwAttrs = GetFileAttributesA(filename.c_str());
	if (dwAttrs == INVALID_FILE_ATTRIBUTES) {
		return(0);
	}
#endif
	return(1);
}

// ********************************************************************************
/// Check that given directory exists.
/// Cross Platform utility.
/// @param		dirname	Input directory name 
/// @return		returns 0 if dir does not exist or if dirname empty, 1 if dir exists
// ********************************************************************************
int check_dir_exists(string dirname)
{
	if (dirname.empty()) return(0);

#if defined(LIBS_QT) 
	QDir tt(QString::fromStdString(dirname));	// Uses working dir if string empty
	if (!tt.exists()) {
		return(0);
	}
#else
	DWORD dwAttrs = GetFileAttributesA(dirname.c_str());
	if (dwAttrs == INVALID_FILE_ATTRIBUTES) {
		return(0);
	}
#endif
	return(1);
}

// ********************************************************************************
/// Read file 'CustomParms.txt' that contains default parameters for Fusion3D.
/// Contains critical parms like the map size and max size of point clouds and initial stereo setting.
// ********************************************************************************
int read_defaults(int &nlowx, int &nlowy, int &nmed, int &nhi, int &stereoType, int &PCMaxD, int &elMax)
{
	char tiff_tag[240], tiff_junk[240];
	FILE *tiff_fd;
	int ntiff, it1;
	base_jfd_class* oops = new base_jfd_class();
	string spath;

	// ***********************************
	// Open file that stores defaults
	// ***********************************
#if defined(LIBS_QT) 
	if (!find_file_in_exec_tree("CustomParms.txt", spath)) {
		oops->warning(1, "Default parameters file does not exist in executable directory tree");
		return(0);
	}

#else			// Windows only
	char tname[400];
	int nbytes = GetModuleFileNameA(NULL, tname, 400);
	if (strstr(tname, "bin") != NULL) {				// Not from Visual Studio
		char *pdest = strrchr(tname, '\\');
		*pdest = '\0';
		strcat(tname, "\\CustomParms.txt");
	}
	else {
		strcpy(tname, "C:\\Fusion3d\\bin\\CustomParms.txt");
	}
	spath = tname;
#endif

	if (!check_file_exists(spath)) {
		oops->warning_s("Default parameters file does not exist: ", spath);
		return(0);
	}

	if (!(tiff_fd = fopen(spath.c_str(), "r"))) {
		oops->warning_s("Default parameters file cant be opened for read: ", spath);
		return(0);
	}

	// ***********************************
	// Read defaults
	// ***********************************
	do {
		/* Read tag */
		ntiff = fscanf(tiff_fd, "%s", tiff_tag);

		/* If cant read any more (EOF), do nothing */
		if (ntiff != 1) {
		}
		else if (strcmp(tiff_tag, "Tiles-N") == 0) {
			fscanf(tiff_fd, "%d %d %d %d", &nlowx, &nlowy, &nmed, &nhi);					// Defaults for Map dimensions
		}
		else if (strcmp(tiff_tag, "Map3d-Color-Scale") == 0) {
			fscanf(tiff_fd, "%d", &it1);
		}
		else if (strcmp(tiff_tag, "Stereo-On-Off") == 0) {
			fscanf(tiff_fd, "%s", tiff_junk);
			if (strcmp(tiff_junk, "on") == 0 || strcmp(tiff_junk, "On") == 0) {
				stereoType = 1;
			}
			else if (strcmp(tiff_junk, "anaglyph") == 0 || strcmp(tiff_junk, "Anaglyph") == 0) {
				stereoType = 2;
			}
			else {
				stereoType = 0;
			}
		}
		else if (strcmp(tiff_tag, "Mobmap-Max-NDisplay") == 0) {
			fscanf(tiff_fd, "%d", &PCMaxD);
		}
		else if (strcmp(tiff_tag, "Camera-El-Max") == 0) {
			fscanf(tiff_fd, "%d", &elMax);
		}
		else {
			fgets(tiff_junk, 240, tiff_fd);
		}
	} while (ntiff == 1);

	fclose(tiff_fd);
	return(1);
}
// ********************************************************************************
/// Set stereo mode using current value of OIV Global variable GL_stereo_on.
// ********************************************************************************
#if defined(LIBS_COIN) 
int set_stereo_mode(fusion3d_viewer_class *myViewer)
{
	SoSFInt32*  GL_stereo_on    = (SoSFInt32*)  SoDB::getGlobalField("Stereo-On");
	int stereo_flag = GL_stereo_on->getValue();
	cout << "Stereo set to (0=mono, 1=raw, 2=anaglyph):  " << stereo_flag << endl;

#if defined(LIBS_SOQT)
	SbViewportRegion myRegion =   myViewer->getViewportRegion();
	//SoCamera *myCamera = myViewer->getCamera();
	//myCamera->setStereoMode(SoCamera::QUAD_BUFFER);
	bool status;
	if (stereo_flag == 0) {
		status = myViewer->setStereoType(SoQtViewer::STEREO_NONE);
		//myViewer->setStereoViewing(FALSE);				// Superceded -- should use setStereoType
	}
	else if (stereo_flag == 1) {
		status = myViewer->setStereoType(SoQtViewer::STEREO_QUADBUFFER);
		//myViewer->setStereoViewing(TRUE);				// Superceded -- should use setStereoType
	}
	else if (stereo_flag == 2) {
		status = myViewer->setStereoType(SoQtViewer::STEREO_ANAGLYPH);
		//myViewer->setStereoViewing(TRUE);				// Superceded -- should use setStereoType
	}
	else if (stereo_flag == 3) {
		status = myViewer->setStereoType(SoQtViewer::STEREO_INTERLEAVED_ROWS);
	}
	else if (stereo_flag == 4) {
		status = myViewer->setStereoType(SoQtViewer::STEREO_INTERLEAVED_COLUMNS);
	}
	if (status == FALSE) cout << "Setting stereo type not successful ********" << endl;
#elif defined (LIBS_SOWIN)
	if(stereo_flag == 0) {
		myViewer->setStereoType(SoWinViewer::STEREO_NONE);
	}
	else if(stereo_flag == 1) {
		myViewer->setStereoType(SoWinViewer::STEREO_QUADBUFFER);
	}
	else if(stereo_flag == 2) {
		myViewer->setStereoType(SoWinViewer::STEREO_ANAGLYPH);
	}
	else if(stereo_flag == 3) {
		myViewer->setStereoType(SoWinViewer::STEREO_INTERLEAVED_ROWS);
	}
	else if(stereo_flag == 4) {
		myViewer->setStereoType(SoWinViewer::STEREO_INTERLEAVED_COLUMNS);
	}
#endif
	return 1;
}
#endif

// ********************************************************************************
/// When reading from a text file, translate a relative pathname (relative to the reading-file ) to absolute.
/// If the path is already absolute, does not modify the name.
/// Cross Platform utility.
/// @param name					Complete pathname of input file (path+filename -- must have both).
/// @param name					Output filename (no path)
/// @param dirname				Output directory name (no filename)
// ********************************************************************************
int parse_filepath(string name_in, string &filename, string &dirname)
{
#if defined(LIBS_QT)
	QFileInfo qfi(QString::fromStdString(name_in));
	filename = qfi.fileName().toStdString();
	dirname  = qfi.path().toStdString();
#else
	size_t pos = name_in.find_last_of("/\\");		// Should handle both forward and backward slash conventions
	if (pos == string::npos) {
		base_jfd_class oops;
		oops.warning(1, "parse_filepath:  Illegal input filename");
		return(0);
	}
	else {
		filename = name_in.substr(pos + 1);
		dirname = name_in.substr(0, pos);
	}
#endif
	return(1);
}

// ********************************************************************************
/// When reading from a text file, translate a relative pathname (relative to the reading-file ) to absolute.
/// If the path is already absolute, does not modify the name.
/// Cross Platform utility.
/// @param reading_from_name	Complete pathname of the file that you are reading from.
/// @param name					Input filename (either absolute or relative) / Output absolute filename
/// @param dirname				Output directory name
// ********************************************************************************
int convert_name_to_absolute(string reading_from_name, string &name, string &dirname)
{
#if defined(LIBS_QT)
	QString tt;
	tt = QString::fromStdString(name);
	QFileInfo qfi(tt);
	if (qfi.isAbsolute()) {
		dirname = qfi.absolutePath().toStdString();
		return(1);
	}
	else {
		// Find directory of Project File -- File name is relative to this directory
		QFileInfo readFrom(QString::fromStdString(reading_from_name));
		dirname = readFrom.absolutePath().toStdString();

		// Derive absolute filename
		QFileInfo qfi2(QString::fromStdString(dirname), QString::fromStdString(name));
		QString qname = qfi2.absoluteFilePath();
		name = qname.toStdString();
		return(1);
	}
#else
	int pos;
	DWORD retval = 0;
	LPCSTR tname = name.c_str();
	LPSTR tabsname = new CHAR[300];
	LPSTR tfilename = new CHAR[100];
	dirname = "";
	retval = GetFullPathNameA(tname, 300, tabsname, &tfilename);// Appears this gets name relative to current dir
	if (retval <= name.size()) {								// Input name no shorter absolute name, so input name is absolute 
		pos = name.rfind('\\');
		if (pos != string::npos) {
			dirname = reading_from_name.substr(0, pos);
		}
	}
	else {														// Input name different size, so input relative
		pos = reading_from_name.rfind('\\');
		if (pos != string::npos) {
			dirname = reading_from_name.substr(0, pos);
		}
		name = tabsname;
		//name.insert(0, "\\");
		//name.insert(0, dirname);
	}
	return(1);
#endif
}

// ********************************************************************************
/// Read a name that may have embedded spaces into a string -- names with spaces must be enclosed in quotes.
/// Should handle no-space names, no-space names in quotes or names-with-spaces within quotes.
/// @param tiff_fd	FILE handle of already opened file.
/// @param name		Output string name
// ********************************************************************************
int read_string_with_spaces(FILE *fd, string &name)
{
	char temp[300];
	fscanf(fd, "%s", temp);
	name = temp;
	if (name.find('\"') == 0) {	// Quotes indicating string with spaces
		name.erase(0, 1);			// 
		while (name.find("\"") == string::npos) {
			fscanf(fd, "%s", temp);
			name.append(" ");
			name.append(temp);
		}
		name.erase(name.size() - 1, 1);
	}
	return 1;
}

/*
 *  FILE NAME:		byteswap
 *
 *  AUTHORS:        Greg Donohoe
 *
 *  DATE:           10/11/89
 *
 *  DESCRIPTION:    converts Sun format to Vax or vice versa by swapping
 *                  the byte order.
 *
 *  ARGUMENTS:      *iarray   array of integers to be converted
 *                  int alan  size of the array in bytes
 *                  int       isize size of the integer in bytes
 */
void byteswap_i2(short int *ia, int n)
{
   short int j;
   char *jptr = (char *)(&j);
   char *xptr = (char *)(&j) + 1;

   for (int k=0; k<n; k++) {
      char *iptr = (char *)(&ia[k]);
      char *tptr = (char *)(&ia[k]) + 1;
      *jptr = *tptr;
      *xptr = *iptr;
      ia[k] = j;
   }
}

   
      
void byteswap_ui2(unsigned short int *ia, int n)
{
   unsigned short int j;
   char *jptr = (char *)(&j);
   char *xptr = (char *)(&j) + 1;

   for (int k=0; k<n; k++) {
      char *iptr = (char *)(&ia[k]);
      char *tptr = (char *)(&ia[k]) + 1;
      *jptr = *tptr;
      *xptr = *iptr;
      ia[k] = j;
   }
}

   
      
void byteswap(int *iarray, int alen, int isize)
{
   register int i,j;
   int temp;
   char *tptr, *iptr;

   for (j=0; j<alen/isize; j++) {
         tptr = (char *)(&temp) + 3;		/* Point to end of temp */
         iptr = (char *)(&(iarray[j]));		/* Point to begin. of integer */

         for (i=0; i<4; i++) 
            *tptr-- = *iptr++;

         iarray[j] = temp;
      }
}

void byteswap_ulong(unsigned long *iarray, int alen, int isize)
{
   register int i,j;
   int temp;
   char *tptr, *iptr;

   for (j=0; j<alen/isize; j++) {
         tptr = (char *)(&temp) + 3;		/* Point to end of temp */
         iptr = (char *)(&(iarray[j]));		/* Point to begin. of integer */

         for (i=0; i<4; i++) 
            *tptr-- = *iptr++;

         iarray[j] = temp;
      }
}

void byteswap_double(double *iarray, int alen)
{
   register int i,j;
   double temp;
   char *tptr, *iptr;

   for (j=0; j<alen; j++) {
         tptr = (char *)(&temp) + 7;		/* Point to end of temp */
         iptr = (char *)(&(iarray[j]));		/* Point to begin. of integer */

         for (i=0; i<8; i++) 
            *tptr-- = *iptr++;

         iarray[j] = temp;
      }
}

// ********************************************************************************
// Compare 2 numbers for sort -- Private
// ********************************************************************************
int comp_nums(const void *a, const void *b)
{
  return ( *(int*)a - *(int*)b );
}
