#include "internals.h"

// *******************************************
/// Constructor.
/// Constructor with no storage allocated
// *******************************************
cad_class::cad_class()
	:vector_layer_class()
{
   strcpy(class_type, "cad");
}

// *************************************************************
/// Destructor.
// *************************************************************

cad_class::~cad_class()
{
}

// ********************************************************************************
/// Returns true if this class can handle the format associated with the input filename, false if not.
/// This method checks for the most common formats read by Assimp and returns true for these.
/// If format is less common, it must use an Assimp importer object which takes some time, so may be relatively slow.
// ********************************************************************************
bool cad_class::canHandleThisFormat(string filename)
{
	int i = filename.find_last_of(".");
	string suffix = filename.substr(i);

#if defined(LIBS_ASSIMP) 
	if (suffix.compare(".obj") == 0 || suffix.compare(".dae") == 0 || suffix.compare(".ply") == 0 || suffix.compare(".dxf") == 0) {	// Save some time for most common formats
		return TRUE;
	}
	else {
		Assimp::Importer importer;
		size_t iFormat = importer.GetImporterIndex(suffix.c_str());
		if (iFormat == -1) {
			return FALSE;
		}
		else {
			return TRUE;
		}
	}
#else
	return FALSE;
#endif
}

// ********************************************************************************
/// Write a file -- Not implemented.
/// Assimp can write files but does not have methods to populate a scene if you want to make your own.
/// The write is intended to format conversion -- reading into a scene in one format and writing the fully formed scene in another format.
// ********************************************************************************
int cad_class::write_file(string sfilename)
{
	cout << "WARNING -- cad_class::write_file not implemented -- do nothing" << endl;
	return(1);
}

// ********************************************************************************
/// Read file.
/// Read file from file filename.
// ********************************************************************************
int cad_class::read_file(string sfilename)
{
	if (!check_file_exists(sfilename)) {
		warning_s("cad_class::read_file:  CAD file does not exist: ", sfilename);
		return(0);
	}
	else {
		draw_data->assimpFilename = sfilename;
	}
	return(1);
}

