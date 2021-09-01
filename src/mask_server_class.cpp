#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
mask_server_class::mask_server_class()
	:base_jfd_class()
{
	mask_tex = NULL;
	clear_all();
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
mask_server_class::~mask_server_class()
{
	clear_all();
}

// ********************************************************************************
/// Reset class.
// ********************************************************************************
int mask_server_class::clear_all()
{
	if (mask_tex != NULL) delete[] mask_tex;
	mask_tex = NULL;
	mask_tex_flag = 0;
	return(1);
}

// ********************************************************************************
/// Read a texture mask file.
// ********************************************************************************
int mask_server_class::read_file(string sfilename)
{
	image_tif_class *image_tif = new image_tif_class(gps_calc);
	image_tif->read_file_open(sfilename);
	image_tif->read_file_header();
	ny_tex = image_tif->get_n_rows();
	nx_tex = image_tif->get_n_cols();
	dx_tex = image_tif->get_dwidth();
	dy_tex = image_tif->get_dheight();
	north_tex = image_tif->get_ulcorner_north();
	west_tex = image_tif->get_ulcorner_west();

	if (mask_tex != NULL) delete[] mask_tex;
	mask_tex = new unsigned char[ny_tex * nx_tex];
	image_tif->get_data_all_1band_uchar_to_external(mask_tex);
	image_tif->read_file_close();
	mask_tex_flag = 1;
	return(1);
}

// ********************************************************************************
/// Write a mask file -- currently implemented only for texture mask.
/// Scale mask from [0,1] to [0,255] so that it will show up well.
// ********************************************************************************
int mask_server_class::write_file(string sfilename)
{
	if (mask_tex == NULL) {													// Cant save since no LOS defined yet
		warning(1, "Cant save LOS -- LOS mask not defined -- no action");
		return(1);
	}
	image_tif_class *image_tif = new image_tif_class(gps_calc);
	image_tif->set_tiepoint(north_tex, west_tex);
	image_tif->set_data_res(dy_tex, dx_tex);
	image_tif->set_data_size(ny_tex, nx_tex);
	unsigned char *maskr = new unsigned char[ny_tex * nx_tex];
	for (int i = 0; i<ny_tex * nx_tex; i++) {		// Scale up to [0,255] if necessary
		maskr[i] = mask_tex[i];
		if (maskr[i] == 1) maskr[i] = 255;
	}
	image_tif->set_data_array_uchar(maskr);
	image_tif->write_file(sfilename);
	delete image_tif;
	delete[] maskr;
	return(1);
}

// ********************************************************************************
/// Register a texture mask, setting location and size and allocating storage internally for the mask.
/// @param north	ul corner of the mask (tiepoint)
/// @param west		ul corner of the mask (tiepoint)
/// @param dx		pixel size
/// @param dy		pixel size
/// @param nx		size of mask in pixels
/// @param ny		size of mask in pixels
// ********************************************************************************
int mask_server_class::register_mask_tex(double north, double west, float dx, float dy, int nx, int ny)
{
	north_tex = north;			///< Mask-- mask bounds
	west_tex = west;			///< Mask -- mask bounds
	nx_tex = nx;				///< Mask -- no. of pixels in mask
	ny_tex = ny;				///< Mask -- no. of pixels in mask
	dx_tex = dx;				///< Mask -- pixel size
	dy_tex = dy;				///< Mask -- pixel size
	if (mask_tex != NULL) delete[] mask_tex;
	mask_tex = new unsigned char[ny_tex * ny_tex];
	mask_tex_flag = 1;
	return(1);
}

// ********************************************************************************
/// Get texture mask parameters.
/// @param north	ul corner of the mask (tiepoint)
/// @param west		ul corner of the mask (tiepoint)
/// @param dx		pixel size
/// @param dy		pixel size
/// @param nx		size of mask in pixels
/// @param ny		size of mask in pixels
// ********************************************************************************
int mask_server_class::get_mask_parms_tex(double &north, double &west, float &dx, float &dy, int &nx, int &ny)
{
	north = north_tex;			///< Mask -- map bounds
	west = west_tex;			///< Mask -- map bounds
	nx = nx_tex;				///< Mask -- no. of pixels in mosaic image
	ny = ny_tex;				///< Mask -- no. of pixels in mosaic image
	dx = dx_tex;				///< Mask -- pixel size
	dy = dy_tex;				///< Mask -- pixel size
	return(1);
}

// ********************************************************************************
/// Get the texture mask array.
// ********************************************************************************
unsigned char* mask_server_class::get_mask_tex()
{
	return(mask_tex);
}

// ********************************************************************************
/// Apply texture mask to the specified texture data.
/// Where the mask is 0, the texture is unchanged, where it is 1 or 255 the texture is given a red tinge.
/// @param data			unsigned char texture data that will be modified with the texture mask
/// @param north_data	ul corner of the input data (tiepoint)
/// @param west_data	ul corner of the input data (tiepoint)
/// @param dx_data		pixel size of the input data
/// @param dy_data		pixel size of the input data
/// @param nx_data		size of input data in pixels
/// @param ny_data		size of input data in pixels
// ********************************************************************************
int mask_server_class::apply_mask_tex(unsigned char* data, double north_data, double west_data, float dx_data, float dy_data, int nx_data, int ny_data)
{
	double north, east;
	int ixm, iym, ipm, ipr;
	int ix, iy, ix1, ix2, iy1, iy2;

	if (!mask_tex_flag) return(0);											// Return if mask not active
																			
	double east_data  = west_data  + nx_data * dx_data;
	double south_data = north_data - ny_data * dy_data;
	double south_tex = north_tex - ny_tex * dy_tex;
	double east_tex = west_tex + nx_tex * dx_tex;
	if (east_data  <= west_tex  || west_data  >= east_tex ) return(0);		// Return if no intersection
	if (north_data <= south_tex || south_data >= north_tex) return(0);

	ix1 = 0;
	if (west_data < west_tex) ix1 = (west_tex - west_data) / dx_data;
	ix2 = nx_data;
	if (east_data > east_tex) ix2 = (east_tex - west_data) / dx_data;
	iy1 = 0;
	if (north_data > north_tex) iy1 = (north_data - north_tex) / dy_data;
	iy2 = ny_data;
	if (south_data < south_tex) iy2 = (north_data - south_tex) / dy_data;

	for (iy = iy1; iy<iy2; iy++) {
		north = north_data - iy * dy_data;
		if (north > north_tex || north <= south_tex) continue;
		iym = (north_tex - north) / dy_tex;
		if (iym >= ny_tex) {
			iym = ny_tex - 1;
		}
		for (ix = ix1; ix<ix2; ix++) {
			east = west_data + ix * dx_data;
			if (east >= east_tex || east < west_tex) continue;
			ixm = (east - west_tex) / dx_tex;
			ipm = iym * nx_tex + ixm;
			ipr = iy * nx_data + ix;
			if (mask_tex[ipm] == 2) {
				data[3 * ipr] = 0;
				data[3 * ipr + 1] = 0;
				data[3 * ipr + 2] = 255;
			}
			else if (mask_tex[ipm] > 0) {	// Simple masks are 1 as generated, 255 as read in from file
											//data_loc[3 * ipr] = data_loc[3 * ipr] / 2;			//Original scheme mimics shadows and tries not to crush terrain detail
											//data_loc[3 * ipr + 1] = data_loc[3 * ipr + 1] / 2;
											//data_loc[3 * ipr + 2] = 80;
				float amp = 0.33 * (data[3 * ipr] + data[3 * ipr + 1] + data[3 * ipr + 2]);
				data[3 * ipr] = amp;									// Military likes shadowed regions to be reddish
				data[3 * ipr + 1] = 0;
				data[3 * ipr + 2] = 0;
			}
		}
	}
	return(1);
}

// ********************************************************************************
/// Read file in tagged ascii format.
/// Read in operational parameters from the standard tagged ascii format.
// ********************************************************************************
int mask_server_class::read_tagged(const char* filename)
{
   
	char tiff_tag[240], tiff_junk[240];
	FILE *tiff_fd;
	int ntiff, n_tags_read = 1;

	// ******************************
	// Read-tagged defaults
	// ******************************
	if (strcmp(filename, "default") == 0) {
		return(1);
	}

	// ******************************
	// Read-tagged from file
	// ******************************
	tiff_fd= fopen(filename,"r");
	do {
		/* Read tag */
		ntiff = fscanf(tiff_fd,"%s",tiff_tag);
		n_tags_read += ntiff;

		/* If cant read any more (EOF), do nothing */
		if (ntiff != 1) {
		}
		//else if (strcmp(tiff_tag, "Map3d-Bright-Min") == 0) {
		//	fscanf(tiff_fd, "%f", &brt0_tex);
		//}
		else {
			fgets(tiff_junk,240,tiff_fd);
		}
	} while (ntiff == 1);

	fclose(tiff_fd);
	return(1);
}

 // *******************************************
/// Write parameters to the currently opened ASCII file -- for saving all relevent parameters to an output Parameter file.
/// Assumes that the file has already been opened and will be closed outside this class.
// *******************************************
int mask_server_class::write_parms(FILE *out_fd)
{
	fprintf(out_fd, "# ##################################################\n");
	fprintf(out_fd, "# Mask tags\n");
	fprintf(out_fd, "# ##################################################\n");
	return(1);
}

