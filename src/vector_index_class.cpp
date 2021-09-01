#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
vector_index_class::vector_index_class(int nmax)
	:base_jfd_class()
{
	update_flags = new int[20];
	clear_all();
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
vector_index_class::~vector_index_class()
{
   int i;
   delete[] update_flags;
}

// ********************************************************************************
/// Reset class.
// ********************************************************************************
int vector_index_class::clear_all()
{
	n_files = 0;
	visFlags.clear();
	typeFlags.clear();
	filenameS.clear();
	red.clear();
	grn.clear();
	blu.clear();
	elevOffset.clear();
	DefaultAltitudeMode.clear();
	DefaultFillMode.clear();
	asAnnotationFlag.clear();
	renderedFlag.clear();

   for (int i=0; i<20; i++) {
      update_flags[i] = 0;
   }
   return(1);
}

// ********************************************************************************
/// Add a file to the index.
/// Sets flag that file has not yet been rendered.
/// Deletes any entries with same name as the file to be added.
/// @param name		name of file to be added
/// @param type		Type of file (see above)		
/// @param redi		Color of overlay -- red [0,1]	
/// @param grni		Color of overlay -- green [0,1]
/// @param blui		Color of overlay -- blue [0,1]
/// @param hoff		Elevation offset in m
/// @param altMode	Altitude mode:  0=clamp-to-ground, 1=rel-to-ground, 2=absolute, 3=rel-to-file-attribute
/// @param fillMode	Polygon fill mode:  0=wireframe, 1=fill
/// @param annFlag	Annotation flag: 0=SoSeparator (mix with map), 1=SoAnnotation (on top of all map elements)
/// @return			Total number of files after addition
// ********************************************************************************
int vector_index_class::add_file(string name, int type, float redi, float grni, float blui, float hoff, int altMode, int fillMode, int annFlag)
{
   filenameS.push_back(name);
   typeFlags.push_back(type);
   visFlags.push_back(1);
   red.push_back(redi);
   grn.push_back(grni);
   blu.push_back(blui);
   elevOffset.push_back(hoff);
   DefaultAltitudeMode.push_back(altMode);
   DefaultFillMode.push_back(fillMode);
   asAnnotationFlag.push_back(annFlag);
   renderedFlag.push_back(0);
   n_files++;

   // Delete any previous files with same name as current entry
   for (int i = 0; i < n_files - 1; i++) {
	   if (name.compare(filenameS[i]) == 0) del_file(i);
   }
   return n_files;
}

// ********************************************************************************
/// Delete a file.
/// Sets the type to 99 effectively deleting it.
// ********************************************************************************
int vector_index_class::del_file(int ifile)
{
	typeFlags[ifile] = 99;
	return(1);
}

// ********************************************************************************
/// Set the visibility of file ifile.
/// Set visibility (0=invisible, 1=visible).
// ********************************************************************************
int vector_index_class::set_vis(int ifile, int val)
{
	visFlags[ifile] = val;
	return(1);
}

// ********************************************************************************
/// Set the rendered flag of file ifile.
/// @param ifile	File number
/// @param val		0 for file not rendered yet, 1 for file rendered (transfered to local manager storage)
// ********************************************************************************
int vector_index_class::set_rendered_flag(int ifile, int val)
{
	renderedFlag[ifile] = val;
	return(1);
}

// ********************************************************************************
/// Set the update flag. 
/// @param type	Type of data to be updated
/// @param val	1 if needs update, 0 if no update needed.
// ********************************************************************************
int vector_index_class::set_update_flag(int type, int val)
{
   update_flags[type] = val;
   return(1);
}

// ********************************************************************************
/// Get the update flag. 
/// If the update flag for a particular type of vector overlay is set, then all overlays of this type will be updated
/// @param type	Type of data to be updated
/// @return 1 if this type of overlay needs updating, 0 if not.
// ********************************************************************************
int vector_index_class::get_update_flag(int type)
{
   return update_flags[type];
}

// ********************************************************************************
/// Get the total number of files in the index.
// ********************************************************************************
int vector_index_class::get_n()
{
   return n_files;
}

// ********************************************************************************
///  Get the pathname for the given index. 
// ********************************************************************************
const char* vector_index_class::get_name(int ifile)
{
   return filenameS[ifile].c_str();
}

// ********************************************************************************
///  Get the filename without path for the given index. 
// ********************************************************************************
const char* vector_index_class::get_name_nopath(int ifile)
{
   int ich, i, nch;
   nch = strlen(filenameS[ifile].c_str());
   ich = 0;
   for (i=0; i<nch; i++) {
      if (strncmp(&filenameS[ifile].c_str()[i],"/",1) == 0) ich = i+1;
      if (strncmp(&filenameS[ifile].c_str()[i],"\\",1) == 0) ich = i+1;
   }
   return &filenameS[ifile].c_str()[ich];
}

// ********************************************************************************
/// Get the type for the given index. 
// ********************************************************************************
int vector_index_class::get_type(int ifile)
{
   return typeFlags[ifile];
}

// ********************************************************************************
/// Get the visibility for the given index. 
/// Returns 1 for visible, 0 for not visible.
// ********************************************************************************
int vector_index_class::get_vis(int ifile)
{
   return visFlags[ifile];
}

// ********************************************************************************
/// Get the color for the given index.
// ********************************************************************************
int vector_index_class::get_rgb(int ifile, float &redi, float &grni, float &blui)
{
   redi = red[ifile];;
   grni = grn[ifile];;
   blui = blu[ifile];;
   return(1);
}

// ********************************************************************************
/// Get the elevation offset in m for the given index.
// ********************************************************************************
float vector_index_class::get_elev_offset(int ifile)
{
	return elevOffset[ifile];
}

// ********************************************************************************
/// Get the default altitude mode for the given index.
/// @return	Altitude mode:  0=clamp-to-ground, 1=rel-to-ground, 2=absolute, 3=rel-to-file-attribute
// ********************************************************************************
int vector_index_class::get_default_alt_mode(int ifile)
{
	return DefaultAltitudeMode[ifile];
}

// ********************************************************************************
/// Get the default fill mode for the given index.
/// @return Default fill mode:  0=wireframe, 1=fill.
// ********************************************************************************
int vector_index_class::get_default_fill_mode(int ifile)
{
	return DefaultFillMode[ifile];
}

// ********************************************************************************
/// Get the annotation flag for the given index.
/// @return Annotation flag: 0=SoSeparator (mix with map), 1=SoAnnotation (on top of all map elements).
// ********************************************************************************
int vector_index_class::get_annotation_flag(int ifile)
{
	return asAnnotationFlag[ifile];
}

// ********************************************************************************
/// Get the rendered flag for the given index.
/// @return Annotation flag: 0=not rendered, 1=rendered (transfered to local manager storage).
// ********************************************************************************
int vector_index_class::get_rendered_flag(int ifile)
{
	return renderedFlag[ifile];
}

// **********************************************
/// Read file in tagged ascii format.
/// Read in operational parameters from the standard tagged ascii format.
// **********************************************
int vector_index_class::read_tagged(const char* filename)
{
	char tiff_tag[240], tiff_junk[240], name[300], type[30], place[30];
	char *cptr;
	FILE *tiff_fd;
	int ntiff, nread, n, DefaultAltitudeModet, DefaultFillModet, asAnnotationFlagt;
	float redt, grnt, blut, hoff, thoff=1.0;
	float primeCols[24] = { 1,0,0,  1,1,0,  0,1,0,  0,1,1,   0,0,1,  1,0,1,  0.5,0.5,0.5,  1,0.5,0.5 };
	vector<string>tname;
	string sname, sdirname;

	// ******************************
	// Read-tagged from file
	// ******************************
	if (!(tiff_fd = fopen(filename, "r"))) {
		cout << "vector_index_class::read_tagged:  unable to open input setup file " << filename << endl;
		return (0);
	}

	do {
		/* Read tag */
		ntiff = fscanf(tiff_fd, "%s", tiff_tag);


		/* If cant read any more (EOF), do nothing */
		if (ntiff != 1) {
		}
		else if (strcmp(tiff_tag, "Vector-Over-File") == 0) {
			nread = fscanf(tiff_fd, "%30s %30s %f %f %f", place, type, &redt, &grnt, &blut);
			read_string_with_spaces(tiff_fd, sname);
			convert_name_to_absolute(filename, sname, sdirname);
			if (check_file_exists(sname)) {
				cptr = strstr(place, "RenderCodes");
				if (cptr != NULL) {
					hoff = atof(place);
					char *ctt = cptr + 11;
					int itt = atoi(ctt);
					DefaultAltitudeModet = itt / 100;
					DefaultFillModet = (itt % 100) / 10;
					asAnnotationFlagt = itt % 10;
				}
				else {
					hoff = 0.;
					DefaultAltitudeModet = 0;
					DefaultFillModet = 0;
					asAnnotationFlagt = 1;
				}
				red.push_back(redt);
				grn.push_back(grnt);
				blu.push_back(blut);
				elevOffset.push_back(hoff);
				DefaultAltitudeMode.push_back(DefaultAltitudeModet);
				DefaultFillMode.push_back(DefaultFillModet);
				asAnnotationFlag.push_back(asAnnotationFlagt);
				renderedFlag.push_back(0);
				visFlags.push_back(1);
				if (strcmp(type, "display") == 0) {
					typeFlags.push_back(1);
				}
				else if (strcmp(type, "bookmarks") == 0) {
					typeFlags.push_back(2);
				}
				else if (strcmp(type, "mti") == 0) {
					typeFlags.push_back(3);
				}
				else if (strcmp(type, "track") == 0) {
					typeFlags.push_back(4);
				}
				else if (strcmp(type, "sensorml") == 0) {
					typeFlags.push_back(5);
				}
				else if (strcmp(type, "om") == 0) {
					typeFlags.push_back(6);
				}
				else if (strcmp(type, "draw") == 0) {
					typeFlags.push_back(7);
				}
				else if (strcmp(type, "cad") == 0) {
					typeFlags.push_back(8);
				}
				else {
					typeFlags.push_back(0);
				}

				n = strlen(place);
				n = strlen(name);
				filenameS.push_back(sname);
				n_files++;
			}
			else {
				warning_s("Input vector file from Project file does not exist: ", sname);
			}
		}

		// This tag depricated -- only for compatiblity with old Parm files
		else if (strcmp(tiff_tag, "Shape-Input-File") == 0) {
			fscanf(tiff_fd, "%f %f %f %300s", &red[n_files], &grn[n_files], &blu[n_files], name);
			visFlags.push_back(1);
			typeFlags.push_back(1);
			n = strlen(name);
			filenameS.push_back(name);
			n_files++;
		}

		// This tag depricated -- only for compatiblity with old Parm files
		else if (strcmp(tiff_tag, "Track-Above-Ground") == 0) {
			fscanf(tiff_fd, "%f", &thoff);
		}
		else if (strcmp(tiff_tag, "Track-Input-File") == 0) {
			read_string_with_spaces(tiff_fd, sname);
			convert_name_to_absolute(filename, sname, sdirname);
			tname.push_back(sname);
		}
		else {
			fgets(tiff_junk, 240, tiff_fd);
		}
	} while (ntiff == 1);

	fclose(tiff_fd);

	// Must wait until any possible modifiers have been read
	for (int i = 0; i < tname.size(); i++) {
		visFlags.push_back(1);
		typeFlags.push_back(4);
		filenameS.push_back(tname[i]);
		red.push_back(0.0);
		grn.push_back(0.0);
		blu.push_back(1.0);
		elevOffset.push_back(thoff);
		DefaultAltitudeMode.push_back(2);					// Absolute??
		DefaultFillMode.push_back(0);
		asAnnotationFlag.push_back(1);
		renderedFlag.push_back(0);
		n_files++;
	}
	return(1);
}

// **********************************************
/// Write current vector overlay filenames to output Project File.
/// Method designed not to be called directly but to be called by manager for each particular file type.
// **********************************************
int vector_index_class::write_parms(FILE *out_fd, int type)
{
	char ctype[30], name[100];
	for (int i = 0; i<n_files; i++) {
		if (typeFlags[i] != type) continue;	// Bookmarks will be covered by 'Jump-To' tags from the Bookmark manager

		sprintf(name, "%9.2fRenderCodes%d%d%d", elevOffset[i], DefaultAltitudeMode[i], DefaultFillMode[i], asAnnotationFlag[i]);
		if (typeFlags[i] == 1) {
			strcpy(ctype, "display");
		}
		else if (typeFlags[i] == 2) {
			strcpy(ctype, "bookmarks");
		}
		else if (typeFlags[i] == 3) {
			strcpy(ctype, "mti");
		}
		else if (typeFlags[i] == 4) {
			strcpy(ctype, "track  ");
		}
		else if (typeFlags[i] == 5) {
			strcpy(ctype, "om");
		}
		else if (typeFlags[i] == 6) {
			strcpy(ctype, "sensorml");
		}
		else if (typeFlags[i] == 7) {
			strcpy(ctype, "draw");
		}
		else if (typeFlags[i] == 8) {
			strcpy(ctype, "cad");
		}
		if (strstr(filenameS[i].c_str(), " ") != NULL) {
			fprintf(out_fd, "Vector-Over-File %s %s %f %f %f \"%s\"\n", name, ctype, red[i], grn[i], blu[i], filenameS[i].c_str());	// Space if filename, so quote
		}
		else {
			fprintf(out_fd, "Vector-Over-File %s %s %f %f %f %s\n", name, ctype, red[i], grn[i], blu[i], filenameS[i].c_str());		// No space in filename
		}
	}
	return(1);
}

