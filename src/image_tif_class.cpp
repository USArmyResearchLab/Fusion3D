#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
image_tif_class::image_tif_class(gps_calc_class *gps_calc_in)
        :image_geo_class()
{
	if (gps_calc_in == NULL) {
		exit_safe(1, "image_tif_class::image_tif_class:  NULL gps_calc_class");
	}
	gps_calc = gps_calc_in;
	useGdalReadFlag = 1;				// Default to read with GDAL rather than my parser
	compressionType = 1;			// Default to no compression

	nstrips = 0;
   nrows_per_strip = 1;
   xres_offset 			= 0;
   yres_offset 			= 0;
   ties_offset 			= 0;
   strip_offsets_base 	= 0;
   scale_offset 		= 0;
   geokeys_offset		= 0;
   modelt_offset		= 0;
   geoascii_offset		= 0;
   nodata_offset		= 0;
   nnodata				= 0;
   geog_type_flag		= 0;
   areaOrPointFlag	= 2;			// Apparently, tie-point is center of pixel has historically been default
   nodataValue = -9999.;			// Seems to be most used value
   data = new unsigned char*[1];	// Assume only single frame
   type_write 		= -99;
   tile_flag		= 0;			// Default to strips rather than tiles
   zunits_to_m		= 1.;				// Assume z in m
   xyunits_to_m		= 1.;				// Assume xy in m
   zunits_code		= 9001;
   xyunits_code		= 9001;
   xres				= 1.;
   yres				= 1.;

   strip_offset		= NULL;
   tile_offset		= NULL;
   uctile			= NULL;
   ftile			= NULL;
   data[0]			= NULL;

   input_fd = NULL;
#if defined(LIBS_GDAL)
   poDataset = NULL;
#endif
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_tif_class::~image_tif_class()
{
   if (strip_offset != NULL) delete[] strip_offset;
   if (tile_offset  != NULL) delete[] tile_offset;
   if (uctile       != NULL) delete[] uctile;
   if (ftile        != NULL) delete[] ftile;
}

// ******************************************
/// Return the nodata value (defaulted to -9999 is no value specified in the file).
// ******************************************
double image_tif_class::get_nodata_value()
{
	return(nodataValue);
}

// ******************************************
/// Read image.
// ******************************************
int image_tif_class::read_file(string sfilename)
{
	if (!read_file_open(sfilename)) return(0);
	if (!read_file_header()) return(0);
	if (!read_file_data()) return(0);
	read_file_close();
	return(1);
}

// ******************************************
/// Open image.
// ******************************************
int image_tif_class::read_file_open(string sfilename)
{
	if (useGdalReadFlag) {
		// *****************************************
		// Open file with GDAL 
		// *****************************************
#if defined(LIBS_GDAL)
		GDALAllRegister();
		CPLSetConfigOption("CPL_MAX_ERROR_REPORTS", "10");	// Default is 1000 -- can be annoying for big datasets
		if (poDataset == NULL) {
			poDataset = (GDALDataset *)GDALOpen(sfilename.c_str(), GA_ReadOnly);
			if (poDataset == NULL) {
				warning_s("image_tif_class::read_file_open: cant open image file", sfilename);
				return(0);
			}
		}
#endif
	}

	// *****************************************
	// Open file for my parser 
	// *****************************************
	else {
		if ((input_fd = open(sfilename.c_str(), O_RDONLY | O_BINARY)) < 0) {
			warning_s("image_tif_class::read_file_open:  unable to open input file ", sfilename);
			return FALSE;
		}
	}
   filename_save = sfilename.c_str();
   return(1);
}

   
// ******************************************
/// Read the data from an open image into a Region Of Interest (ROI).
/// Output may be downsampled (assumes an integer downsampling ratio).
/// If error reading image, return 0.
/// If the ROI does not intersect the MrSID image, return 1.
/// If there is an intersection, copy the MrSID data to the portion of the ROI that intersects --
/// a ROI may intersect multiple images or a portion of it may not intersect any.
/// @param	roi_w	left boundary of ROI in UTM m
/// @param	roi_n	top  boundary of ROI in UTM m
/// @param	roi_e	right boundary of ROI in UTM m
/// @param	roi_s	bottom boundary of ROI in UTM m
/// @param	nxout	Size of output (maybe) downsampled ROI in pixels in x
/// @param	nyout	Size of output (maybe) downsampled ROI in pixels in y
/// @param	data_out Output data array for ROI image (allocated outside this class)
/// @param	ninter	Input/output -- current number of images that have intersected ROI (on input, previous intersecions; on output, incremented if current image intersects)
// ******************************************
int image_tif_class::read_into_roi(double roi_w, double roi_n, double roi_e, double roi_s, int nxout, int nyout, unsigned char *data_out, int &ninter)
{
	int ip, ih, iw, iin, iout, nxcrop, nycrop, ndown=1, nspp;
	int loc_w1, loc_h1;						// Crop area -- starting pixels in input array for partial overlap
	int partial_flag=1;						// Crop area -- 1 if partial overlap, 0 if crop area completely enclosed in MrSID image

   if      (roi_w > lrx || roi_e < ulx || roi_n < lry || roi_s > uly) {			// No intersection
		return(1);
	}

	// ****************************************
	// Init downsampling option
	// ****************************************
	if (downsample_flag) {
		ndown = int(downsample_ratio + 0.1);
		nxcrop = ndown * nxout;
		nycrop = ndown * nyout;
	}
	else {
		nxcrop = nxout;
		nycrop = nyout;
	}

	// ***************************************
	// Calc crop indices
	// ***************************************
	if (roi_w > ulx && roi_e < lrx && roi_n < uly && roi_s > lry) {	// ROI fully enclosed
		  iw1 = int((roi_w - ulx) / dwidth + 0.5);
		  iw2 = iw1 + nxcrop;
		  ih1 = int((uly - roi_n) / dheight+ 0.5);
		  ih2 = ih1 + nycrop;
		  crop_ncols = nxcrop;
		  crop_nrows = nycrop;
		  partial_flag = 0;
	}
	else {																	// Partial intersection
	  if (roi_w >= ulx && roi_e <= lrx) { // In this dimension, entirely within
		  iw1 = int((roi_w - ulx) / dwidth + 0.5);
		  iw2 = iw1 + nxcrop;
		  loc_w1 = 0;
	  }
	  else if (roi_w < ulx) {
		  iw1 = 0;
		  iw2 = int((roi_e - ulx) / dwidth + 0.5);
		  loc_w1 = nxcrop - (iw2 -iw1);
	  }
	  else {
		  iw1 = int((roi_w - ulx) / dwidth + 0.5);
		  iw2  = ncols;
		  loc_w1 = 0;
	  }
	  crop_ncols = iw2 - iw1;
      
	  if (roi_s >= lry && roi_n <= uly) { // In this dimension, entirely within
		  ih1 = int((uly - roi_n) / dheight + 0.5);
		  ih2 = ih1 + nycrop;
		  loc_h1 = 0;
	  }
	  else if (roi_n > uly) {
		  ih1 = 0;
		  ih2 = int((uly - roi_s) / dheight + 0.5);
		  loc_h1 = nycrop - (ih2 - ih1);
	  }
	  else {
		  ih1 = int((uly - roi_n) / dheight + 0.5);
		  ih2  = nrows;
		  loc_h1 = 0;
	  }
	  crop_nrows = ih2 - ih1;
	}
	crop_flag = 2;		// 2=Crop using crop limits

	if (crop_ncols <= 0 || crop_nrows <= 0) {	// For downsampled, may not have samples even tho areas intersect
		return(1);
	}

	// ***************************************
	// Read cropped portion of image
	// ***************************************
	if (!read_file_data()) {
	   cout << "image_tif_class::read_into_roi: Cant read image " << endl;
	   return(0);
	}
	if (data_type == 5){
		nspp = 1;
	} 
	else {
		nspp = 3;
	}


	// ***************************************
	// Full intersection, no downsampling, color/grayscale
	// ***************************************
	if (!partial_flag && !downsample_flag) {
		for (ip=0; ip<nspp * crop_nrows*crop_ncols; ip++) {
			data_out[ip] = data[0][ip];
		}
	}

	// ****************************************
	// Full intersection, downsample, color
	// ****************************************
	else if (!partial_flag && downsample_flag && data_type == 6) {
		for (ih=0, iout=0; ih<nyout; ih++) {
			iin = ndown * ih * nxcrop;
			for (iw=0; iw<nxout; iw++, iout++, iin=iin+ndown) {
				data_out[3*iout  ] = data[0][3*iin  ];
				data_out[3*iout+1] = data[0][3*iin+1];
				data_out[3*iout+2] = data[0][3*iin+2];
			}
		}
	}

	// ****************************************
	// Full intersection, downsample, grayscale
	// ****************************************
	else if (!partial_flag && downsample_flag) {
		for (ih=0, iout=0; ih<nyout; ih++) {
			iin = ndown * ih * nxcrop;
			for (iw=0; iw<nxout; iw++, iout++, iin=iin+ndown) {
				data_out[3*iout  ] = data[0][iin];
				data_out[3*iout+1] = data[0][iin];
				data_out[3*iout+2] = data[0][iin];
			}
		}
	}

	// ****************************************
	// Partial intersection, no downsample, color
	// ****************************************
	else if (partial_flag && !downsample_flag && data_type == 6) {
		for (ih=0, iin=0; ih<crop_nrows; ih++) {
			iout = (loc_h1 + ih) * nxcrop + loc_w1;
			for (iw=0; iw<crop_ncols; iw++,iin++,iout++) {
				data_out[3*iout  ] = data[0][3*iin  ];
				data_out[3*iout+1] = data[0][3*iin+1];
				data_out[3*iout+2] = data[0][3*iin+2];
			}
		}
	}

	// ****************************************
	// Partial intersection, no downsample, grayscale
	// ****************************************
	else if (partial_flag && !downsample_flag) {
		for (ih=0, iin=0; ih<crop_nrows; ih++) {
			iout = (loc_h1 + ih) * nxcrop + loc_w1;
			for (iw=0; iw<crop_ncols; iw++,iin++,iout++) {
				data_out[3*iout  ] = data[0][iin];
				data_out[3*iout+1] = data[0][iin];
				data_out[3*iout+2] = data[0][iin];
			}
		}
	}


	// ****************************************
	// Partial intersection, downsample, color/grayscale
	// ****************************************
	else if (partial_flag && downsample_flag) {
		int loc_w1f = (loc_w1 / ndown) * ndown + (ndown / 2);				// First sample in full-scale output image
		if (loc_w1f < loc_w1) loc_w1f = loc_w1f + ndown;
		int loc_w2f = ((loc_w1+crop_ncols) / ndown) * ndown + (ndown / 2);	// Last sample in full-scale output image
		if (loc_w2f < loc_w1+crop_ncols) loc_w2f = loc_w2f + ndown;
		int iw1r = loc_w1f - loc_w1;											// First sample in read image
		int cropwd = loc_w2f - loc_w1f;
		int crop_ncols_down = cropwd / ndown;								// Width in output image
		int iw1o = loc_w1f / ndown;											// First sample in output image

		int loc_h1f = (loc_h1 / ndown) * ndown + (ndown / 2);				// First sample in full-scale output image
		if (loc_h1f < loc_h1) loc_h1f = loc_h1f + ndown;
		int loc_h2f = ((loc_h1+crop_nrows) / ndown) * ndown + (ndown / 2);	// Last sample in full-scale output image
		if (loc_h2f < loc_h1+crop_nrows) loc_h2f = loc_h2f + ndown;
		int ih1r = loc_h1f - loc_h1;											// First sample in read image
		int crophd = loc_h2f - loc_h1f;
		int crop_nrows_down = crophd / ndown;								// Height in output image
		int ih1o = loc_h1f / ndown;											// First sample in output image

		if (data_type == 6) {	// Color
			for (ih=0; ih<crop_nrows_down; ih++) {
				iin  = (ndown * ih + ih1r) * crop_ncols + iw1r;
				iout = (ih1o + ih) * nxout + iw1o;
				for (iw=0; iw<crop_ncols_down; iw++, iin=iin+ndown, iout++) {
					data_out[3*iout  ] = data[0][3*iin  ];
					data_out[3*iout+1] = data[0][3*iin+1];
					data_out[3*iout+2] = data[0][3*iin+2];
				}
			}
		}
		else {					// Grayscale
			for (ih=0; ih<crop_nrows_down; ih++) {
				iin  = (ndown * ih + ih1r) * crop_ncols + iw1r;
				iout = (ih1o + ih) * nxout + iw1o;
				for (iw=0; iw<crop_ncols_down; iw++, iin=iin+ndown, iout++) {
					data_out[3*iout  ] = data[0][iin];
					data_out[3*iout+1] = data[0][iin];
					data_out[3*iout+2] = data[0][iin];
				}
			}
		}
	}
	ninter++;
	return(1);
}

// ******************************************
/// Read the header from an open image.
/// First try GDAL which should read almost all files
/// There are some DEMs made early at ARL that have a faulty tag that GDAL cant read so dont show a EPSG
/// For these files (and others?) try home-grown parser that is bulk of this method.
// ******************************************
int image_tif_class::read_file_header()
{
	short int version;		// Must be either 42=classicTiff or 43=BigTiff
	int n_read, itag, offsetFlag;
   char *word = new char[5];
   strcpy(word, "  ");
   unsigned long ulong;
   off_t ifd1_offset;
   unsigned long long ntags=0;
   short int tagtyp;
   unsigned short int tagid;
   short int ishort;
   int nifd=0, bits_per_sample, iSampleFormat=1;
   char ctemp[300];

   unsigned long long outNvals, outOffset, outInt;
   float outFloat;
   double outDouble;
   char outAscii[300];

   // **********************************************
   // First try GDAL which should read almost all files
   // There are some DEMs made early at ARL that have a faulty tag that GDAL cant read so dont show a EPSG
   // For these files (and others?) try home-grown parser below.
   // **********************************************
   if (useGdalReadFlag) {
	   if (!read_header_by_gdal()) {
		   useGdalReadFlag = 0;
		   read_file_close();
		   read_file_open(filename_save);
	   }
	   else {
		   return(1);
	   }
   }

   // **********************************************
   // Read file header
   // **********************************************
   // Byte order
   n_read = read(input_fd, word, 2);
   if (strcmp(word, "II") == 0) {			// Indicates data little-endian, default architecture little-endian
      byte_order_flag = 0;
#ifdef BBIG_ENDIAN
	  swap_flag = 1;
#else
	  swap_flag = 0;
#endif
   }
   else if (strcmp(word, "MM") == 0) {		// Indicates data big-endian, default architecture little-endian
      byte_order_flag = 1;
#ifdef BBIG_ENDIAN
	  swap_flag = 0;
#else
	  swap_flag = 1;
#endif
   }
   else {
      cerr << "image_tif_class::read_file_header;  bad byte order flag " << word << endl;
      exit_safe(1, "image_tif_class::read_file_header;  bad byte order flag ");
   }
   
   // Version #
   n_read = read(input_fd, &version, 2);
   if (swap_flag) byteswap_i2(&version, 1);
   if (n_read != 2 || (version != 42 && version != 43)) {
      exit_safe_s("image_tif_class::read_file_header;  bad version no.", std::to_string(version));
   }
   
   // Classic Tiff
   if (version == 42) {
	   bigTiffFlag = 0;
	   // Offset to first IFD -- all offsets from file beginning
	   n_read = read(input_fd, &ulong, 4);
	   if (swap_flag) byteswap_ulong(&ulong, 4, 4);
	   ifd1_offset = ulong;
   }

   // BigTiff
   else {
	   bigTiffFlag = 1;
	   exit_safe(1, "Homegrown parser unable to read bigTiff");
   }

   // Go to first IFD
   if (ifd1_offset > 0) {
	   off_t result = lseek(input_fd, ifd1_offset, SEEK_SET);
	   if (result != ifd1_offset) {
		   cerr << "image_tif_class::read_file_header;  cant skip " << version << endl;
		   exit_safe(1, "image_tif_class::read_file_header;  cant seek ");
	   }
   }

   // **********************************************
   // Loop over IFDs
   // **********************************************
   do {
      nifd++;
      if (nifd > 1) {
         cerr << "Only implemented for single IFD -- exit " << endl;
	     exit_safe(1, "Only implemented for single IFD -- exit ");
      }
      
	  // **********************************************
	  // Read IFD header
	  // **********************************************
	  n_read = read(input_fd, &ishort, 2);
	  if (swap_flag) byteswap_i2(&ishort, 1);
	  ntags = ishort;
	  if (diag_flag > 1) cout << "Tiff header with ntags=" << ntags << endl;

	  // **********************************************
	  // Read tags within IFD
	  // **********************************************
	  for (itag=0; itag<ntags; itag++) {
		  read_tag(tagid, tagtyp, outNvals, outAscii, outInt, outFloat, outDouble, offsetFlag, outOffset);
	 
	 // NewSubfileType 
	 if (tagid == 254) {
	    int subfile_type = outInt;
	    if (diag_flag>1) cout << "Tag " << tagid << "   NewSubfileType " << subfile_type << endl;
	 }
	 
	 // Image width
	 else if (tagid == 256) {
	    ncols = outInt;
	    if (diag_flag > 1) cout << "Tag " << tagid << "   Ncols " << ncols << endl;
	 }
	 
	 // Image length
	 else if (tagid == 257) {
	    nrows = outInt;
	    if (diag_flag > 1) cout << "Tag " << tagid << "   Nrows " << nrows << endl;
	 }
	 
	 // Bits per sample -- Always short
	 else if (tagid == 258) {
	    bits_per_sample = outInt;
	    if (diag_flag>1) cout << "Tag " << tagid << "   BitsPerSample " << bits_per_sample << endl;
	 }
	 
	 // Compression 
	 else if (tagid == 259) {
		 compressionType = outInt;
		 if (compressionType == 1) {
			 if (diag_flag > 1) cout << "Tag " << tagid << "   No compression" << endl;
		 }
		 else {
			 useGdalReadFlag = 1;
			 if (diag_flag > 1) cout << "Tag " << tagid << "   Compression type " << outInt << endl;
		 }
	 }
	 
	 // Photometric interpretation -- ignore 
	 else if (tagid == 262) {
	    if (diag_flag>1) cout << "Tag " << tagid << "   PhotometricInterpretation -- Ignored -- 1=greyscale, 2=RGB " << outInt <<endl;
	 }
	 
	 // ImageDescription-- ignore 
	 else if (tagid == 270) {
	    if (diag_flag>1) cout << "Tag " << tagid << "   ImageDescription (ascii string) -- ignore" << endl;
	 }
	 
	 // Strip offsets
	 else if (tagid == 273) {
		 nstrips = outNvals;
		 strip_offsets_base = outOffset;
	     if (diag_flag > 1) cout << "Tag " << tagid << "   Strips " << nstrips << " at base offset " << strip_offsets_base << endl;
	 }
	 
	 // Orientation-- ignore 
	 else if (tagid == 274) {
	    if (diag_flag>1) cout << "Tag " << tagid << "   Orientation -- ignore" << endl;
	 }
	 
	 // Samples per pixel
	 else if (tagid == 277) {
	       if (diag_flag > 1) cout << "Tag " << tagid << "   Samples per pixel " << outInt << endl;
		   nbands = outInt;
	       if (outInt == 1) {
	          data_type = 5;
	       }
	       else {
	          data_type = 6;
	       }
	 }
	 
	 // RowsPerStrip
	 else if (tagid == 278) {
		nrows_per_strip = outInt;
	    if (diag_flag>1) cout << "Tag " << tagid << "   RowsPerStrip " << nrows_per_strip << endl;
	 }
	 
	 // StripByteCounts -- ignore 
	 else if (tagid == 279) {
	    if (diag_flag>1) cout << "Tag " << tagid << "   StripByteCounts -- ignore" << endl;
	 }
	 
	 // XResolution
	 else if (tagid == 282) {
		 if (offsetFlag) {
			 xres_offset = outOffset;
			 if (diag_flag>1) cout << "Tag " << tagid << "   XResolution at offset " << xres_offset << endl;
		 }
		 else {
			dwidth = outFloat;
			if (diag_flag>1) cout << "Tag " << tagid << "   XResolution = " << dwidth << endl;
		 }
	 }
	 

	 // YResolution
	 else if (tagid == 283) {
		 if (offsetFlag) {
			 yres_offset = outOffset;
			 if (diag_flag>1) cout << "Tag " << tagid << "   YResolution at offset " << yres_offset << endl;
		 }
		 else {
			 dheight = outFloat;
			 if (diag_flag>1) cout << "Tag " << tagid << "   YResolution = " << dheight << endl;
		 }
	 }
	 

	 // PlanarConfiguration -- ignore 
	 else if (tagid == 284) {
	    if (outInt != 2) {
	       if (diag_flag>1) cout << "Tag " << tagid << "   PlanarConfiguration = 1 (all vals for pixel contiguous)" << endl;
	    }
	    else {
	       if (diag_flag>1) cout << "   PlanarConfiguration -- implemented only for 1 (RGBRGBRGB)" << endl;
	       if (outInt != 1) {
	          cerr << "image_tif_class::read_file_header: PlanarConfiguration == 2 not supported" << outInt << endl;
	          exit_safe(1, "image_tif_class::read_file_header: PlanarConfiguration == 2  not supported");
	       }
	    }
	 }
	 
	 // XPosition 
	 else if (tagid == 286){
	    if (diag_flag>1) cout << "Tag " << tagid << "  XPosition -- ignore" << endl;
	 }
	 
	 // YPosition 
	 else if (tagid == 287){
	    if (diag_flag>1) cout << "Tag " << tagid << "  YPosition -- ignore" << endl;
	 }
	 
	 // ResolutionUnit 
	 else if (tagid == 296){
	    if (diag_flag>1) cout << "Tag " << tagid << "  ResolutionUnit -- ignore" << endl;
	 }
	 
	 // Software 
	 else if (tagid == 305){
	    if (diag_flag>1) cout << "Tag " << tagid << "  Software -- ignore" << endl;
	 }
	 
	 // TileWidth
	 else if (tagid == 322) {
	    tile_width = outInt;
	    tiles_across = (ncols + tile_width - 1) / tile_width;
	    if (diag_flag>1) cout << "Tag " << tagid << "   TileWidth " << tile_width  << " nTiles across " << tiles_across << endl;
	 }
	 

	 // TileLength
	 else if (tagid == 323) {
	    tile_length = outInt;
	    tiles_down = (nrows + tile_length - 1) / tile_length;
	    tiles_per_image = tiles_down * tiles_across;
	    tile_flag = 1;
	    if (diag_flag>1) cout << "Tag " << tagid << "   TileLength " << tile_length << " nTiles down " << tiles_down << endl;
	 }
	 

	 // TileOffsets
	 else if (tagid == 324) {
	       if (tiles_per_image != outNvals) {
	          cerr << "image_tif_class::read_file_header: error with tag type " << tagid << endl;
	          exit_safe_s("image_tif_class::read_file_header: error with tag type ", std::to_string(tagid));
	       }
		   tile_offsets_base = outOffset;
	       if (diag_flag > 1) cout << "Tag " << tagid << "   Tiles " << tiles_per_image << " at base offset " << tile_offsets_base << endl;
	 }
	 
	 // TileByteCounts 
	 else if (tagid == 325){
	    if (diag_flag>1) cout << "Tag " << tagid << "  TileByteCounts -- ignore" << endl;
	 }
	 
	 // SampleFormat
	 else if (tagid == 339) {
	    iSampleFormat = outInt;
	    if (iSampleFormat == 1) {
	       if (diag_flag > 1) cout << "Tag " << tagid << "   SampleFormat -- unsigned int" << endl;
	    }
		else if (iSampleFormat == 3) {
			if (diag_flag > 1) cout << "Tag " << tagid << "   SampleFormat -- float" << endl;
		}
		else  if (iSampleFormat == 0) {
			iSampleFormat = 1;	// There are many 1-band data files with 0 in this field
		}
		else {
			cerr << "image_tif_class::read_file_header: WARNING unimplemented SampleFormat(1=uchar,2=int,3=float) assume uchar" << iSampleFormat << endl;
			iSampleFormat = 1;	// There may be garbage in this field
		}
	 }
	 
	 // GeoTiff -- 33432 Copyright?-- ignore 
	 else if (tagid == 33432) {
	    if (diag_flag>1) cout << "Tag " << tagid << "  copyright -- ignore" << endl; 
	    ngeoascii = outNvals;
		geoascii_offset = outOffset;
	 }
	 
	 // GeoTiff -- 33550 ModelPixelScaleTag 
	 else if (tagid == 33550) {
	    if (diag_flag>1) cout << "Tag " << tagid << "   ModelPixelScaleTag" << endl; 
		if (offsetFlag) {
			scale_offset = outOffset;
		}
		else if (outDouble != 0.) {
			xres = outDouble;
			yres = outDouble;
		}
	    if (diag_flag>1) cout << "Tag " << tagid << "   Scale at base offset " << scale_offset << endl;
	 }
	 
	 // GeoTiff -- 33922 ModelTiepointTag  -- must use 33922 ModelTiepointTag or 34264  ModelTransformationTag but not both
	 else if (tagid == 33922) {
	    if (diag_flag>1) cout << "Tag " << tagid << "   ModelTiepointTag" << endl; 
	    nties = outNvals / 6;
		if (offsetFlag) {
			ties_offset = outOffset;
		}
	    if (diag_flag > 1) cout << "   Tiepoint " << nties << " at base offset " << ties_offset << endl;
	 }
	 
	 // GeoTiff -- 34264  ModelTransformationTag -- must use 33922 ModelTiepointTag or 34264  ModelTransformationTag but not both
	 else if (tagid == 34264) {
	    if (diag_flag>1) cout << "Tag " << tagid << "   GeoKeyDirectoryTag" << endl; 
	    nmodelt = outNvals / 4;
		if (offsetFlag) {
			modelt_offset = outOffset;
		}
	 }
	 
	 // GeoTiff -- 34735  GeoKeyDirectoryTag
	 else if (tagid == 34735) {
	    if (diag_flag>1) cout << "Tag " << tagid << "   GeoKeyDirectoryTag" << endl; 
	    ngeokeys = outNvals / 4;
		if (offsetFlag) {
			geokeys_offset = outOffset;
		}
	 }
	 
	 // GeoTiff -- 34737 GeoAsciiParamsTag -- ignore 
	 else if (tagid == 34737) {
	    if (diag_flag>1) cout << "Tag " << tagid << "  GeoAsciiParamsTag -- ignore" << endl; 
	    ngeoascii = outNvals;
		if (offsetFlag) {
			geoascii_offset = outOffset;
		}
	 }
	 
	 // GeoTiff -- 42113 GDAL_NODATA -- partially implemented -- 
	 // Non-standard tag initiated by GDAL.  Included in ALIRT, Haloe but not Buckeye.  Not getting meaningful values for either ALIRT or Haloe
	 else if (tagid == 42113) {
	    nnodata = outNvals;
		if (offsetFlag) {
			nodata_offset = outOffset;
		}
		if (diag_flag>1) cout << "Tag " << tagid << "  GDAL_NODATA " << " type " << tagtyp << " nvals " << nnodata << endl;
	 }
	 
	 // Dont care tag 
	 else {
	    if (diag_flag>1) cout << "Tag " << tagid << "  Unknown tag -- ignore" << endl;
	 }
      }
      
	  // Offset to next IFD -- 0 if no more IFDs
	  n_read = read(input_fd, &ulong, 4);
	  if (swap_flag) byteswap_ulong(&ulong, 4, 4);
	  ifd1_offset = ulong;

      
      // **********************************************
      // Read stuff from offsets
      // **********************************************
      if (diag_flag > 1) cout << "Calculate stuff from offsets ********" << endl;
      if (nodata_offset  > 0 && nnodata > 0 && nnodata < 100) read_nodata();		// Have had trouble with this tag
      if (xres_offset    > 0) read_rational(xres_offset, dwidth);
      if (yres_offset    > 0) read_rational(yres_offset, dheight);
      if (scale_offset   > 0) read_scale();
      if (ties_offset    > 0) read_ties();
      if (modelt_offset  > 0) read_model_transformation();
      if (geokeys_offset > 0) read_geokeys();
      if (geoascii_offset> 0) read_geoascii();
      if (strip_offsets_base  > 0) read_strip_offsets();
      if (tile_flag) read_tile_offsets();
      
	  // **********************************************
	  // If global coord system not defined (first file presented with coord system) then define
	  // **********************************************
	  if (!gps_calc->is_coord_system_defined() && localCoordFlag <= 0) return(0); // No global coords yet and none in this file
		  if (!gps_calc->is_coord_system_defined()) {
			  gps_calc->init_from_epsg_code_number(localCoordFlag);
	  }

	  // **********************************************
      // Calc basic parms that depend on several others
      // **********************************************
	  // If header data for read_scale(), use that.  Otherwise use read_rational
	  if (geog_type_flag == 0) {			// Coord system is projected
	     if (scale_offset   >= 0) {
            dwidth  = xyunits_to_m * xres;
            dheight = xyunits_to_m * yres;
	     }
		 uly = xyunits_to_m * yTiepoint + iyTiepoint * dheight;		// Upper edge of first pixel for RasterPixelIsArea
		 if (areaOrPointFlag == 2) uly = uly + 0.5 * dheight;		// If RasterPixelIsPoint, tie-point refers to center of first pixel
		 lry = uly - float(nrows) * dheight;
		 utm_cen_north[0] = uly - (float(nrows)/2.) * dheight;
		 cen_utm_north = utm_cen_north[0];

		 ulx = xyunits_to_m * xTiepoint - ixTiepoint * dwidth;		// Left edge of first pixel for RasterPixelIsArea
		 if (areaOrPointFlag == 2) ulx = ulx - 0.5 * dwidth;			// If RasterPixelIsPoint, tie-point refers to center of first pixel
		 lrx = ulx + float(ncols) * dwidth;

		 transform_to_global_coord_system(localCoordFlag);		// Transform ulx,uly,lry,lrx from local to global coord system if necessary
		 utm_cen_east[0]  = ulx + (float(ncols)/2.) * dwidth;
		 cen_utm_east = utm_cen_east[0];
	  }
	  else if (geog_type_flag > 0) {		// Coord system is geographic
		  init_geog_type();
	  }

      if (diag_flag > 0) cout << "   Image center at East " << utm_cen_east[0] << " North " << utm_cen_north[0] << endl;
      if (diag_flag > 0) cout << "         pixel size x = " << dwidth << " y = " << dheight << endl;
 
	  // **********************************************
      // Sometimes junk in iSampleFormat, so set it for unsigned short int
      // **********************************************
      iCharIntFloatFlag = iSampleFormat;
      if (bits_per_sample == 16 && iSampleFormat == 1) iCharIntFloatFlag = 2;
	  if (oCharIntFloatFlag == 0) oCharIntFloatFlag = iCharIntFloatFlag;
	  if (iCharIntFloatFlag != oCharIntFloatFlag) useGdalReadFlag = 1;
   } while (ifd1_offset > 0);
   return(1);
}

// ******************************************
/// Uses GDAL to read header.
/// Checked out for BuckEye, Haloe, Alirt and several files from GRiD
/// Checked out for BigTiff using APG dataset with very large EO files
// ******************************************
int image_tif_class::read_header_by_gdal()
{
	double adfGeoTransform[6];

#if defined(LIBS_GDAL)
	ncols = poDataset->GetRasterXSize();
	nrows = poDataset->GetRasterYSize();
	nbands = poDataset->GetRasterCount();
	if (poDataset->GetGeoTransform(adfGeoTransform) != CE_None) {
		warning_s("No transform with data origin -- cant read file", filename_save);
		return(0);
	}
	dwidth  = adfGeoTransform[1];
	dheight = -adfGeoTransform[5];
	ulx = adfGeoTransform[0];
	uly = adfGeoTransform[3];

	// **********************************************
	// Parms from GDALRasterBand
	// ****************** GETTING NO-DATA EXPERIMENTAL,  NOT FULLY IMPLEMENTED ************
	// **********************************************
	GDALRasterBand *rasterBand = poDataset->GetRasterBand(1);
	if (rasterBand != NULL) {
		int pbSuccess;
		double nod = rasterBand->GetNoDataValue(&pbSuccess);
		if (pbSuccess) nodataValue = nod;
	}

	// **********************************************
	// Parms from OGRSpatialReference from poDataset
	// **********************************************
	const char *cRef = poDataset->GetProjectionRef();
	if (strcmp(cRef, "") == 0) {
		char ctemp[400];
		if (diag_flag > 0) fprintf(stdout, "WARNING:  GDAL finds no projection info -- try alternate parser  %s", filename_save.c_str());
		//warning(1, ctemp);
		return(0);
	}
	OGRSpatialReference oSRS(cRef);	
	//oSRS.importFromWkt(cRef);		// This should work in later revs but not defined in this
	if (oSRS.IsProjected()) {
		geog_type_flag = 0;
	}
	else if (oSRS.IsGeographic()) {
		geog_type_flag = 1;
	}
	//else {					// GDAL can create DEMs that are good but dont identify as either proj or geo??
	//	char ctemp[400];
	//	sprintf(ctemp, "No projection/geo info -- cant read file  %s", filename_save.c_str());
	//	warning(1, ctemp);
	//	return(0);
	//}
	bool isGeo = oSRS.IsGeographic();
	bool isPrj = oSRS.IsProjected();
	//epsgTypeCode = oSRS.GetEPSGGeogCS();	// Not what we want -- associated geographic coord system?
	const char *acode = oSRS.GetAuthorityCode(NULL);
	if (acode == NULL) {
		char ctemp[400];
		if (diag_flag > 0) fprintf(stdout, "WARNING:  GDAL finds no EPSG -- try alternate parser  %s", filename_save.c_str());
		//warning(1, ctemp);
		return(0);
	}
	localCoordFlag = atoi(acode);							// EPSG for local coord system -- may be different from global
	double lineUnits = oSRS.GetLinearUnits();
	//OGRSpatialReference::DestroySpatialReference(&oSRS);
	//delete &oSRS;

	// **********************************************
	// If global coord system not defined (first file presented with coord system) then define
	// **********************************************
	if (!gps_calc->is_coord_system_defined()) {
		gps_calc->init_from_epsg_code_number(localCoordFlag);
	}

	// **********************************************
	// Parms from Metadata from poDataset
	// **********************************************
	char ** meta = poDataset->GetMetadata();
	int i = 0;
	areaOrPointFlag = 2;
	while (meta[i] != NULL) {
		if (strstr(meta[i], "AREA_OR_POINT") != NULL) {
			if (strstr(meta[i], "=Point") != NULL) {
				areaOrPointFlag = 1;
			}
		}
		//fprintf(stdout, "%s \n", meta[i]);
		i++;
	}

	// **********************************************
	// Parms from rasterBand from poDataset
	// **********************************************
	//GDALRasterBand *b = poDataset->GetRasterBand(1);	// Works but may have high overhead and not really useful so far
	// nodatatemp = b->GetNoDataValue();

	// **********************************************
	// Secondary parms
	// **********************************************
	if (nbands == 1) {
		data_type = 5;
	}
	else if (nbands == 3) {
		data_type = 6;
	}
	else {
		warning_s("GDAL implementation can only deal with 1 or 3 bands -- cant read file", filename_save);
		return(0);
	}
	//if (areaOrPointFlag == 2) uly = uly + 0.5 * dheight;		// If RasterPixelIsPoint, tie-point refers to center of first pixel
	lry = uly - float(nrows) * dheight;
	utm_cen_north[0] = uly - (float(nrows) / 2.) * dheight;
	cen_utm_north = utm_cen_north[0];

	//if (areaOrPointFlag == 2) ulx = ulx - 0.5 * dwidth;			// If RasterPixelIsPoint, tie-point refers to center of first pixel
	lrx = ulx + float(ncols) * dwidth;

	transform_to_global_coord_system(localCoordFlag);		// Transform ulx,uly,lry,lrx from local to global coord system if necessary
	utm_cen_east[0] = ulx + (float(ncols) / 2.) * dwidth;
	cen_utm_east = utm_cen_east[0];

	if (geog_type_flag > 0) {		// Coord system is geographic
		init_geog_type();
	}

	  if (diag_flag > 0) cout << "   Image center at East " << utm_cen_east[0] << " North " << utm_cen_north[0] << endl;
	  if (diag_flag > 0) cout << "         pixel size x = " << dwidth << " y = " << dheight << endl;

	  // **********************************************
	  // Input data type -- not easy to get from GDAL -- should not be required using GDAL which should just need the output data type
	  // **********************************************
	  //iCharIntFloatFlag = iSampleFormat;
	  //if (bits_per_sample == 16 && iSampleFormat == 1) iCharIntFloatFlag = 2;
	  //if (oCharIntFloatFlag == 0) oCharIntFloatFlag = iCharIntFloatFlag;
	  //if (iCharIntFloatFlag != iCharIntFloatFlag) useGdalFlag = 1;
#endif
	  return(1);
}

// ******************************************
/// For geographic coordinate systems, translate header parms in latlon to output in expected projected system.
/// Not fully implemented -- works for hyperspectral rgb images generated here by ENVI.
/// There are still lots of shortcuts,
// ******************************************
int image_tif_class::init_geog_type()
{
	double lat, lon, north, east, north2, east2;
	lat = yTiepoint;
	lon = xTiepoint;
	gps_calc_class *gps_calct = new gps_calc_class();
	gps_calct->init_wgs84_from_ll(lat, lon);
	int utm_lon_zone = gps_calct->get_utm_lon_zone_ref();
	localCoordFlag = utm_lon_zone + 32600;
	// **********************************************
	// If global coord system not defined (first file presented with coord system) then define
	// **********************************************
	if (!gps_calc->is_coord_system_defined()) {
		gps_calc->init_from_epsg_code_number(localCoordFlag);
	}

	gps_calct->ll_to_proj(lat, lon, north, east);
	gps_calct->ll_to_proj(lat + yres, lon + xres, north2, east2);
	dheight = north2 - north;
	dwidth = east2 - east;

	uly = north - iyTiepoint * dheight;		// For hyperspectral, tiepoint is top of image
	lry = uly - float(nrows) * dheight;
	utm_cen_north[0] = uly - (float(nrows) / 2.) * dheight;
	cen_utm_north = utm_cen_north[0];

	ulx = east + ixTiepoint * dwidth;		// For hyperspectral, tiepoint is image left
	lrx = ulx + float(ncols) * dwidth;

	transform_to_global_coord_system(localCoordFlag);		// Transform ulx,uly,lry,lrx from local to global coord system if necessary
	utm_cen_east[0] = ulx + (float(ncols) / 2.) * dwidth;
	cen_utm_east = utm_cen_east[0];
	return(1);
}

// ******************************************
/// Read the data from an open image -- internally allocated storage.
// ******************************************
int image_tif_class::read_file_data()
{
   if (!header_only_flag) {
	   if (useGdalReadFlag) {
		   if (input_fd != NULL) {		// Left with my parser open -- close this and open GDAL
			   close(input_fd);
			   input_fd = NULL;
			   read_file_open(filename_save);
		   }
		   read_data_by_gdal(external_alloc_flag);
	   }
	   else if (tile_flag) {
		   read_data_by_tiles(external_alloc_flag);
	   }
	   else {
         read_data_by_strips(external_alloc_flag);
      }
   }
   return(1);
}

// ******************************************
/// Read the data from an open image into externally allocated storaget.
// ******************************************
int image_tif_class::get_data_uchar(unsigned char* udata)
{
   if (!header_only_flag) {
      data[0] = udata;
	  if (tile_flag) {
         read_data_by_tiles(1);
      }
      else {
         read_data_by_strips(1);
      }
   }
   return(1);
}

// ******************************************
/// Close the image.
// ******************************************
int image_tif_class::read_file_close()
{
	// Opened with GDAL
#if defined(LIBS_GDAL)
	if (poDataset != NULL) {
		GDALClose(poDataset);
		poDataset = NULL;
	}
#endif

	// Opened for my parser
	if (input_fd != NULL) {
		close(input_fd);
		input_fd = NULL;
	}
	return(1);
}

// ******************************************
/// Read data from strips -- Private.
// ******************************************
int image_tif_class::read_data_by_strips(int external_alloc_flag)
{
   int i, j, n_read, nspp, istrip, irow_in_strip, ncols_crop, nrows_crop;
   off_t mod_offset;
   double left_full, left_crop, top_full, top_crop;
   unsigned char* uptr;
   float* fptr;
   
   // *****************************************
   // If UTM crop, calculate indices
   // *****************************************
   if (crop_flag == 1) {
      left_full = utm_cen_east[0]  - dwidth * float(     ncols)/2.;
      left_crop = crop_cen_east - dwidth * float(crop_ncols)/2.;
      iw1 = (left_crop - left_full + 0.5) / dwidth;
      if (iw1 < 0) iw1 = 0;
      iw2 = iw1 + crop_ncols;

      top_full = utm_cen_north[0]  + dheight * float(     nrows)/2.;
      top_crop = crop_cen_north + dheight * float(crop_nrows)/2.;
      ih1 = (top_full - top_crop + 0.5) / dheight;
      if (ih1 < 0) ih1 = 0;
      ih2 = ih1 + crop_nrows;
      
      utm_cen_east[0] = crop_cen_east;
      utm_cen_north[0] = crop_cen_north;
   }
   
   // *****************************************
   // If crop indices, calc UTM
   // *****************************************
   else if (crop_flag == 2) {
      left_full = utm_cen_east[0]  - dwidth * float(     ncols)/2.;
      left_crop = left_full + dwidth * float(iw1);
      utm_cen_east[0] = left_crop + 0.5 * dwidth * float(iw2 - iw1);
      
      top_full = utm_cen_north[0]  + dheight * float(     nrows)/2.;
      top_crop = top_full - dheight * float(ih1);
      utm_cen_north[0] = top_crop - 0.5 * dheight * float(ih2 - ih1);
   }
   
   // *****************************************
   // Adjust for crop
   // *****************************************
   else if (crop_flag == 0) {
      ih1 = 0;
      ih2 = nrows;
      iw1 = 0;
      iw2 = ncols;
   }

    nrows_crop = ih2 - ih1;
    ncols_crop = iw2 - iw1;
   
   // *****************************************
   // Read data -- float
   // *****************************************
   if (iCharIntFloatFlag == 3) {
	  if (!external_alloc_flag) {
		 if (falloc_flag) delete[] fdata;
         fdata = new float[nrows_crop*ncols_crop];
         falloc_flag = nrows_crop*ncols_crop;
	  }

      for (i=ih1; i<ih2; i++) {
         istrip = i / nrows_per_strip;
         irow_in_strip = i - istrip * nrows_per_strip;
         mod_offset = strip_offset[istrip] + irow_in_strip*sizeof(float)*ncols  + sizeof(float)*iw1;
		 if (lseek(input_fd, mod_offset, SEEK_SET) != mod_offset){
            cerr << "image_tif_class::read_data_by_strips;  cant seek for strip " << i << " istrip " << istrip << " float file " << filename_save << endl;
            exit_safe(1, "image_tif_class::read_data_by_strips;  cant seek for strip ");
         }
         fptr = fdata + (i-ih1) * ncols_crop;
         n_read = read(input_fd, fptr, ncols_crop*sizeof(float));
         if (n_read != ncols_crop*sizeof(float)) {
            cerr << "tif::read_by_strips; " << filename_save << " Noread Fstrip " << i << " at " << mod_offset << endl;
	        cerr << "  n_read " << n_read << " ih1 " << ih1 << " iw1 " << iw1 << " ncols " << ncols_crop<< endl;
            for (j=0; j<ncols_crop; j++) {
	           fdata[(i-ih1) * ncols_crop + j] = 0.;
	        }
         }
      }
      if (swap_flag) byteswap((int*)fdata, sizeof(float)*nrows_crop*ncols_crop, sizeof(float));

	  // If elevation not in m, scale it
      if (zunits_to_m != 1.) {
         for (i=0; i<nrows_crop*ncols_crop; i++) {
            fdata[i] = zunits_to_m * fdata[i];
         }
      }
   }

   // *****************************************
   // Read data -- 16-byte unsigned int
   // *****************************************
   else if (iCharIntFloatFlag == 2) {
	  if (!external_alloc_flag) {
         if (falloc_flag) delete[] fdata;
         fdata = new float[nrows_crop*ncols_crop];
         falloc_flag = nrows_crop*ncols_crop;
	  }
      unsigned short int *ishort = new unsigned short int[ncols_crop];
      float zmin = 862.37097168;		// Hardwired for Lubbock data
      float zmax = 1060.62304688;		// Hardwired for Lubbock data
      float dz = zmax - zmin;

      for (i=ih1; i<ih2; i++) {
         istrip = i / nrows_per_strip;
         irow_in_strip = i - istrip * nrows_per_strip;
         mod_offset = strip_offset[istrip] + irow_in_strip*sizeof(unsigned short int)*ncols  + sizeof(unsigned short int)*iw1;
		 if (lseek(input_fd, mod_offset, SEEK_SET) != mod_offset){
			cerr << "image_tif_class::read_data_by_strips;  cant seek for strip " << i << " istrip " << istrip << " 16-b uns file " << filename_save << endl;
			exit_safe(1, "image_tif_class::read_data_by_strips;  cant seek for strip ");
         }
         fptr = fdata + (i-ih1) * ncols_crop;
         n_read = read(input_fd, ishort, ncols_crop*sizeof(unsigned short int));
         if (n_read != ncols_crop*sizeof(unsigned short int)) {
            cerr << "tif::read_by_strips; " << filename_save << " Noread Fstrip " << i << " at " << mod_offset << endl;
            cerr << "  n_read " << n_read << " ih1 " << ih1 << " iw1 " << iw1 << " ncols " << ncols_crop<< endl;
            for (j=0; j<ncols_crop; j++) {
	       fdata[(i-ih1) * ncols_crop + j] = 0.;
	    }
         }
	 else {
	    if (swap_flag) byteswap_i2((short*)ishort, ncols_crop);
	    for (j=0; j<ncols_crop; j++) {
	       fptr[j] = zmin + (ishort[j] / 65535.0f) * dz;
	    }
	 }
      }
      delete[] ishort;
   }

   // *****************************************
   // Read data -- uchar
   // *****************************************
   else {
      if (data_type == 6) {
       nspp = 3;
      }
      else {
         nspp = 1;
      }
      
	  if (!external_alloc_flag) {
         if (ualloc_flag) delete[] data[0];
         data[0] = new unsigned char[nspp*nrows_crop*ncols_crop];
         ualloc_flag = nspp*nrows_crop*ncols_crop;
	  }
      
      for (i=ih1; i<ih2; i++) {
         if (i >= nrows) {		// For some Lubbock edge tiles, elev files may have 1 more row than satellite images ??
			 if (diag_flag > 0) cout << "tif::read_by_strips: " << filename_save << " Warning -- i=" << i << " >= nrows=" << nrows << endl;
             for (j=0; j<nspp*ncols_crop; j++) {
                data[0][nspp * (i-ih1) * ncols_crop + j] = 0.;
            }
			continue;
		 }

		 istrip = i / nrows_per_strip;
         irow_in_strip = i - istrip * nrows_per_strip;
         mod_offset = strip_offset[istrip] + irow_in_strip*nspp*ncols + nspp*iw1;
		 if (lseek(input_fd, mod_offset, SEEK_SET) != mod_offset){
			 cerr << "image_tif_class::read_data_by_strips;  cant seek for strip " << i << " istrip " << istrip << " uchar file " << filename_save << endl;
            exit_safe(1, "image_tif_class::read_data_by_strips;  cant seek for strip ");
         }
         uptr = data[0] + nspp * (i-ih1) * ncols_crop;
         n_read = read(input_fd, uptr, nspp*ncols_crop);
         if (n_read != nspp*ncols_crop) {
            cerr << "tif::read_by_strips; " << filename_save << " noread Ustrip " << i << " at " << mod_offset << endl;
            cerr << "  n_read " << n_read << " ih1 " << ih1 << " iw1 " << iw1 << " ncols " << ncols_crop<< endl;
            for (j=0; j<nspp*ncols_crop; j++) {
               data[0][nspp * (i-ih1) * ncols_crop + j] = 0.;
            }
         }
      }
   }
   
   crop_flag = 3;
   return(1);
}

// ******************************************
/// Read data by tiles -- Private.
// ******************************************
int image_tif_class::read_data_by_tiles(int external_alloc_flag)
{
	int ity, itx, itile, n_read, nspp, ncols_crop, nrows_crop, nc_tile;
	double left_full, left_crop, top_full, top_crop;
	int iyi, iyi1, iyi2, ixi, ixi1, ixi2, ipi;
	int iyo, iyo1, iyo2, ixo1, ixo2, ipo;
	int itile_x1, itile_x2, itile_y1, itile_y2;

	// *****************************************
	// If UTM crop, calculate indices
	// *****************************************
	if (crop_flag == 1) {
		left_full = utm_cen_east[0] - dwidth * float(ncols) / 2.;
		left_crop = crop_cen_east - dwidth * float(crop_ncols) / 2.;
		iw1 = (left_crop - left_full + 0.5) / dwidth;
		if (iw1 < 0) iw1 = 0;
		iw2 = iw1 + crop_ncols;

		top_full = utm_cen_north[0] + dheight * float(nrows) / 2.;
		top_crop = crop_cen_north + dheight * float(crop_nrows) / 2.;
		ih1 = (top_full - top_crop + 0.5) / dheight;
		if (ih1 < 0) ih1 = 0;
		ih2 = ih1 + crop_nrows;

		utm_cen_east[0] = crop_cen_east;
		utm_cen_north[0] = crop_cen_north;
	}

	// *****************************************
	// If crop indices, calc UTM
	// *****************************************
	else if (crop_flag == 2) {
		left_full = utm_cen_east[0] - dwidth * float(ncols) / 2.;
		left_crop = left_full + dwidth * float(iw1);
		utm_cen_east[0] = left_crop + 0.5 * dwidth * float(iw2 - iw1);

		top_full = utm_cen_north[0] + dheight * float(nrows) / 2.;
		top_crop = top_full - dheight * float(ih1);
		utm_cen_north[0] = top_crop - 0.5 * dheight * float(ih2 - ih1);
	}

	// *****************************************
	// Adjust for crop
	// *****************************************
	else if (crop_flag == 0) {
		ih1 = 0;
		ih2 = nrows;
		iw1 = 0;
		iw2 = ncols;
	}

	nrows_crop = ih2 - ih1;
	ncols_crop = iw2 - iw1;

	itile_x1 = iw1 / tile_width;
	itile_x2 = (iw2 - 1) / tile_width;
	itile_y1 = ih1 / tile_length;
	itile_y2 = (ih2 - 1) / tile_length;

	// *****************************************
	// Alloc
	// *****************************************
	if (iCharIntFloatFlag == 1) {
		if (data_type == 6) {
			nspp = 3;
		}
		else {
			nspp = 1;
		}

		if (!external_alloc_flag) {
			if (ualloc_flag) delete[] data[0];
			data[0] = new unsigned char[nspp*nrows_crop*ncols_crop];
			ualloc_flag = nspp*nrows_crop*ncols_crop;
		}

		nc_tile = nspp * tile_length * tile_width;
		if (uctile == NULL) uctile = new unsigned char[nc_tile];

	}

	else if (iCharIntFloatFlag == 3) {
		if (!external_alloc_flag) {
			if (falloc_flag) delete[] fdata;
			fdata = new float[nrows_crop*ncols_crop];
			falloc_flag = nrows_crop*ncols_crop;
		}

		nc_tile = 4 * tile_length * tile_width;
		if (ftile == NULL) ftile = new float[tile_length*tile_width];
	}


	else {
		cout << "image_tif_class::read_data_by_tiles:  Unimplemented data type -- exit" << iCharIntFloatFlag << endl;
		exit_safe_s("image_tif_class::read_data_by_tiles:  Unimplemented data type -- exit", std::to_string(iCharIntFloatFlag));
	}

	// *****************************************
	// Read data
	// *****************************************
	for (ity = itile_y1; ity <= itile_y2; ity++) {
		for (itx = itile_x1; itx <= itile_x2; itx++) {
			itile = ity * tiles_across + itx;

			iyi1 = ih1 - ity * tile_length;
			if (iyi1 < 0) iyi1 = 0;
			iyi2 = ih2 - ity * tile_length;
			if (iyi2 > tile_length) iyi2 = tile_length;

			iyo1 = ity * tile_length - ih1;
			if (iyo1 < 0) iyo1 = 0;
			iyo2 = (ity + 1) * tile_length - ih1;
			if (iyo2 > tile_length) iyo2 = tile_length;

			ixi1 = iw1 - itx * tile_width;
			if (ixi1 < 0) ixi1 = 0;
			ixi2 = iw2 - itx * tile_width;
			if (ixi2 > tile_width) ixi2 = tile_width;

			ixo1 = itx * tile_width - iw1;
			if (ixo1 < 0) ixo1 = 0;
			ixo2 = (itx + 1) * tile_width - iw1;
			if (ixo2 > tile_width) ixo2 = tile_width;

			if (lseek(input_fd, tile_offset[itile], SEEK_SET) != tile_offset[itile]) {
				cerr << "image_tif_class::read_data_by_tiles;  cant seek for tile " << itile << endl;
				exit_safe(1, "image_tif_class::read_data_by_tiles;  cant seek for tile ");
			}

			// *********************************
			// Read uchar
			// *********************************
			if (iCharIntFloatFlag == 1) {
				n_read = read(input_fd, uctile, nc_tile);
				if (n_read != nc_tile) {
					cerr << "tif::read_by_tiles; " << filename_save << " noread tile " << itile << endl;
					exit_safe(1, "tif::read_by_tiles: cant read tile");
				}

				for (iyi = iyi1, iyo = iyo1; iyi<iyi2; iyi++, iyo++) {
					ipi = iyi * tile_width + ixi1;
					ipo = iyo * ncols_crop + ixo1;
					if (data_type == 6) {
						for (ixi = ixi1; ixi<ixi2; ixi++, ipi++, ipo++) {
							data[0][3 * ipo] = uctile[3 * ipi];
							data[0][3 * ipo + 1] = uctile[3 * ipi + 1];
							data[0][3 * ipo + 2] = uctile[3 * ipi + 2];
						}
					}
					else {
						for (ixi = ixi1; ixi<ixi2; ixi++, ipi++, ipo++) {
							data[0][ipo] = uctile[ipi];
						}
					}
				}
			}

			// *********************************
			// Read float
			// *********************************
			else {
				n_read = read(input_fd, ftile, nc_tile);
				if (n_read != nc_tile) {
					cerr << "tif::read_by_tiles; " << filename_save << " noread tile " << itile << endl;
					exit_safe(1, "tif::read_by_tiles;  cant read tile ");
				}

				for (iyi = iyi1, iyo = iyo1; iyi<iyi2; iyi++, iyo++) {
					ipi = iyi * tile_width + ixi1;
					ipo = iyo * ncols_crop + ixo1;
					for (ixi = ixi1; ixi<ixi2; ixi++, ipi++, ipo++) {
						fdata[ipo] = zunits_to_m * ftile[ipi];
					}
				}
			}
		}
	}

	crop_flag = 3;
	return(1);
}

// ******************************************
/// Read data by tiles -- Private.
// ******************************************
int image_tif_class::read_data_by_gdal(int external_alloc_flag)
{
	int nspp, ncols_crop, nrows_crop;
	double left_full, left_crop, top_full, top_crop;

	// *****************************************
	// If UTM crop, calculate indices
	// *****************************************
	if (crop_flag == 1) {
		left_full = utm_cen_east[0] - dwidth * float(ncols) / 2.;
		left_crop = crop_cen_east - dwidth * float(crop_ncols) / 2.;
		iw1 = (left_crop - left_full + 0.5) / dwidth;
		if (iw1 < 0) iw1 = 0;
		iw2 = iw1 + crop_ncols;

		top_full = utm_cen_north[0] + dheight * float(nrows) / 2.;
		top_crop = crop_cen_north + dheight * float(crop_nrows) / 2.;
		ih1 = (top_full - top_crop + 0.5) / dheight;
		if (ih1 < 0) ih1 = 0;
		ih2 = ih1 + crop_nrows;

		utm_cen_east[0] = crop_cen_east;
		utm_cen_north[0] = crop_cen_north;
	}

	// *****************************************
	// If crop indices, calc UTM
	// *****************************************
	else if (crop_flag == 2) {
		left_full = utm_cen_east[0] - dwidth * float(ncols) / 2.;
		left_crop = left_full + dwidth * float(iw1);
		utm_cen_east[0] = left_crop + 0.5 * dwidth * float(iw2 - iw1);

		top_full = utm_cen_north[0] + dheight * float(nrows) / 2.;
		top_crop = top_full - dheight * float(ih1);
		utm_cen_north[0] = top_crop - 0.5 * dheight * float(ih2 - ih1);
	}

	// *****************************************
	// Adjust for crop
	// *****************************************
	else if (crop_flag == 0) {
		ih1 = 0;
		ih2 = nrows;
		iw1 = 0;
		iw2 = ncols;
	}

	nrows_crop = ih2 - ih1;
	ncols_crop = iw2 - iw1;

	// *****************************************
	// Alloc
	// *****************************************
	if (oCharIntFloatFlag == 1) {
		if (data_type == 6) {
			nspp = 3;
			nbands = 3;
		}
		else {
			nspp = 1;
		}

		if (!external_alloc_flag) {
			if (ualloc_flag) delete[] data[0];
			data[0] = new unsigned char[nbands*nrows_crop*ncols_crop];
			ualloc_flag = nbands*nrows_crop*ncols_crop;
		}
	}

	else if (oCharIntFloatFlag == 3) {
		if (!external_alloc_flag) {
			if (falloc_flag) delete[] fdata;
			fdata = new float[nrows_crop*ncols_crop];
			falloc_flag = nrows_crop*ncols_crop;
		}
	}


	else {
		cout << "image_tif_class::read_data_by_tiles:  Unimplemented data type -- exit" << iCharIntFloatFlag << endl;
		exit_safe_s("image_tif_class::read_data_by_tiles:  Unimplemented data type -- exit", std::to_string(iCharIntFloatFlag));
	}

	// *****************************************
	// Read with GDAL 
	// *****************************************
#if defined(LIBS_GDAL)
	GDALRasterBand *poBand;

	// *********************************
	// Read uchar -- 8-bit unsigned int, GDT_Byte
	// *********************************
	if (oCharIntFloatFlag == 1) {
		//poDataset->RasterIO(GF_Read, iw1, ih1, ncols_crop, nrows_crop, data[0], ncols_crop, nrows_crop, GDT_Byte, 3, NULL, 0, 0, 0);	\\ Garbled image

		int iband, iy, ix, isamp;
		unsigned char *buf = new unsigned char[nrows_crop*ncols_crop];
		for (iband = 0; iband < nbands; iband++) {
			poBand = poDataset->GetRasterBand(iband + 1);
			poBand->RasterIO(GF_Read, iw1, ih1, ncols_crop, nrows_crop, buf, ncols_crop, nrows_crop, GDT_Byte, 0, 0);
			for (iy = 0; iy < nrows_crop; iy++) {
				isamp = iy * ncols_crop;	
				for (ix = 0; ix < ncols_crop; ix++, isamp++) {
					data[0][nbands*isamp + iband] = buf[isamp];
				}
			}
		}
		delete[] buf;
	}

	// *****************************************
	// Read unsigned short -- 16-byte unsigned int, GDT_UInt16 -- no data to test with, so just error message rather than possible crash
	// *****************************************
	else if (oCharIntFloatFlag == 2) {
		warning_s("image_tif_class::read_data: cant read compressed data for file", filename_save);
		return(0);
	}
	
	// *****************************************
	// Read data -- float, GDT_Float32
	// *****************************************
	else if (oCharIntFloatFlag == 3) {
		poBand = poDataset->GetRasterBand(1);
		poBand->RasterIO(GF_Read, iw1, ih1, ncols_crop, nrows_crop, fdata, ncols_crop, nrows_crop, GDT_Float32, 0, 0);
	}
#else
	warning_s("image_tif_class::read_data: cant read compressed data for file", filename_save);
	return(0);
#endif
	return(1);
}

// ******************************************
/// Read offsets to beginning of each strip of data -- Private.
// ******************************************
int image_tif_class::read_strip_offsets()
{
	int i, n_read;
	unsigned long ulong;
	unsigned long long ulonglong;
	strip_offset = new unsigned long long[nstrips];

	if (lseek(input_fd, strip_offsets_base, SEEK_SET) != strip_offsets_base){
		cerr << "image_tif_class::read_strip_offsets;  cant skip " << strip_offsets_base << endl;
		exit_safe(1, "image_tif_class::read_strip_offsets;  cant skip ");
	}
	for (i=0; i<nstrips; i++) {
		n_read = read(input_fd, &ulong, 4);
		if (n_read != 4) {
			cerr << "image_tif_class::read_strip_offsets;  cant read " << endl;
			exit_safe(1, "image_tif_class::read_strip_offsets;  cant read ");
		}
		//if (swap_flag) byteswap(&ival, 4, 4);
		strip_offset[i] = ulong;
	}
	return(1);
}

// ******************************************
/// Read offsets to beginning of each tile -- Private.
// ******************************************
int image_tif_class::read_tile_offsets()
{
	int i, n_read;
	unsigned long ulong;
	unsigned long long ulonglong;
	tile_offset = new unsigned long long int[tiles_per_image];
	if (tiles_per_image == 1) {					// For single tile, info fits into single word, so dont need offsets
		tile_offset[0] = tile_offsets_base;
	}
	else {										// For multiple tiles, must use offsets
		if (lseek(input_fd, tile_offsets_base, SEEK_SET) != tile_offsets_base){
			cerr << "image_tif_class::read_tile_offsets;  cant skip " << strip_offsets_base << endl;
			exit_safe(1, "image_tif_class::read_tile_offsets;  cant skip ");
		}
		for (i = 0; i < tiles_per_image; i++) {
			n_read = read(input_fd, &ulong, 4);
			if (n_read != 4) {
				cerr << "image_tif_class::read_tile_offsets;  cant read " << endl;
				exit_safe(1, "image_tif_class::read_tile_offsets;  cant read ");
			}
			//if (swap_flag) byteswap(&ival, 4, 4);
			tile_offset[i] = ulong;
		}
	}
   return(1);
}

// ******************************************
/// Read no-data value -- Private.
// ******************************************
int image_tif_class::read_nodata()
{
	if (lseek(input_fd, nodata_offset, SEEK_SET) != nodata_offset){
	   warning_s("image_tif_class cant read nodata value for file", filename_save);
	   return(0);
   }
   char* ascii_parms = new char[nnodata];
   
   if (read(input_fd, ascii_parms, nnodata) != nnodata) {
	   warning_s("image_tif_class cant read nodata value for file", filename_save);
	   return(0);
   }
   nodataValue = atof(ascii_parms);
   // bool tt = std::isinf(no_data);		// false --  diagnostic only -- only works in vs2017
   // bool tt1 = std::isinf(-no_data);		// false --  diagnostic only -- only works in vs2017
   // bool tt2 = std::isfinite(no_data);	// true --  diagnostic only -- only works in vs2017
   cout << "GeoTiff GDAL tag no-data value = " << nodataValue << endl;
   return(1);
}

// ******************************************
/// Read tie info -- Private.
// ******************************************
int image_tif_class::read_ties()
{
   int i, n_read;
   float k, zd;
   double dval;
   
   if (lseek(input_fd, ties_offset, SEEK_SET) != ties_offset){
         cerr << "image_tif_class::read_ties;  cant skip " << ties_offset << endl;
         exit_safe(1, "image_tif_class::read_ties;  cant skip ");
   }
   for (i=0; i<nties; i++) {
      n_read = read(input_fd, &dval, 8);
      if (swap_flag) byteswap_double(&dval, 1);
      ixTiepoint = dval;
      n_read = read(input_fd, &dval, 8);
      if (swap_flag) byteswap_double(&dval, 1);
      iyTiepoint = dval;
      n_read = read(input_fd, &dval, 8);
      if (swap_flag) byteswap_double(&dval, 1);
      k = dval;
      
      n_read = read(input_fd, &dval, 8);
      if (swap_flag) byteswap_double(&dval, 1);
      xTiepoint = dval;
      n_read = read(input_fd, &dval, 8);
      if (swap_flag) byteswap_double(&dval, 1);
      yTiepoint = dval;
      n_read = read(input_fd, &dval, 8);
      if (n_read != 8){
         cerr << "image_tif_class::read_ties;  cant read " << endl;
         exit_safe(1, "image_tif_class::read_ties;  cant read " );
      }
      if (swap_flag) byteswap_double(&dval, 1);
      zd = dval;
      
      if (diag_flag > 0) cout << "   Tiepoint ix " << ixTiepoint << " iy " << iyTiepoint << " iz " << k  << endl;
      if (diag_flag > 0) cout << "   Tiepoint x " << xTiepoint << " y " << yTiepoint << " z " << zd << endl;
      //if (iy > 0) {
      //   cout << "   Kluged mrg to make it agree with int and dem files ************* " << endl;
      //   yd = yd - 1.0;
      //   cout << "   Tiepoint xd " << xd << " yd " << yd << " zd " << zd << endl;
      //}
      utm_flag = 1;

   }
   return(1);
}

// ******************************************
/// Read modelTransformationTag info -- Private.
/// Assumes no rotations.
/// I cant find a complete definition of this tag anywhere, so this a partial implementation using best guesses.
// ******************************************
int image_tif_class::read_model_transformation()
{
	int i, n_read;
	double dval[16];
   
	if (lseek(input_fd, modelt_offset, SEEK_SET) != modelt_offset){
		cerr << "image_tif_class::read_model_transformation;  cant skip " << modelt_offset << endl;
		exit_safe(1, "image_tif_class::read_model_transformation;  cant skip ");
	}
	for (i=0; i<8; i++) {	// Should be 16 doubles, but only use 8
		n_read = read(input_fd, &dval[i], 8);
		if (swap_flag) byteswap_double(&dval[i], 1);
	}
	ixTiepoint = 0;
	iyTiepoint = 0;
	xTiepoint = dval[3];
	yTiepoint = dval[7];
	dwidth = dval[0];
	dheight = fabs(dval[5]);
	if (diag_flag > 0) cout << "   Tiepoint ix " << ixTiepoint << " iy " << iyTiepoint << endl;
	if (diag_flag > 0) cout << "   Tiepoint x  " << xTiepoint  << " y  " << yTiepoint  << endl;
	utm_flag = 1;
	return(1);
}

// ******************************************
/// Read scale info -- Private.
// ******************************************
int image_tif_class::read_scale()
{
   int n_read;
   double dval;
   
   if (lseek(input_fd, scale_offset, SEEK_SET) != scale_offset){
         cerr << "image_tif_class::read_scale;  cant skip " << scale_offset << endl;
         exit_safe(1, "image_tif_class::read_scale;  cant skip ");
   }
   n_read = read(input_fd, &dval, 8);
   if (swap_flag) byteswap_double(&dval, 1);
   xres = dval;
   n_read = read(input_fd, &dval, 8);
   if (swap_flag) byteswap_double(&dval, 1);
   yres = dval;
   n_read = read(input_fd, &dval, 8);
   if (n_read != 8){
      cerr << "image_tif_class::read_scale;  cant read " << endl;
      exit_safe(1, "image_tif_class::read_scale;  cant read ");
   }
   if (swap_flag) byteswap_double(&dval, 1);
   // scalez = dval;
   
   if (diag_flag > 0) cout << "   Scale in native units -- dx " << xres << " dy " << yres << endl;
   return(1);
}

// ******************************************
/// Read a rational value -- Private.
// ******************************************
int image_tif_class::read_rational(_int64 offset, float &val)
{
   int n_read;
   unsigned long denom, numer;
   
   if (lseek(input_fd, offset, SEEK_SET) != offset){
         cerr << "image_tif_class::read_rational;  cant skip " << offset << endl;
         exit_safe(1, "image_tif_class::read_rational;  cant skip ");
   }
   n_read = read(input_fd, &numer, 4);
   if (swap_flag) byteswap_ulong(&numer, 4, 4);
   n_read = read(input_fd, &denom, 4);
   if (n_read != 4){
      cerr << "image_tif_class::read_rational;  cant read " << endl;
      exit_safe(1, "image_tif_class::read_rational;  cant read ");
   }
   if (swap_flag) byteswap_ulong(&denom, 4, 4);
   val = float(numer) / float(denom);
   return(1);
}

// ******************************************
/// Read GeoKeys -- Private.
// ******************************************
int image_tif_class::read_geokeys()
{
   int i, n_read;
   unsigned short int ishort1, ishort2, ishort3, ishort4;
   int keyid, GTModelTypeGeoKeyVal=1, GeographicTypeGeoKeyVal=0;
   
   if (lseek(input_fd, geokeys_offset, SEEK_SET) != geokeys_offset){
         cerr << "image_tif_class::read_goekeys;  cant skip " << geokeys_offset << endl;
         exit_safe(1, "image_tif_class::read_goekeys;  cant skip ");
   }
   // ************************
   // Read header -- 4 shorts
   // ************************
   n_read = read(input_fd, &ishort1, 2);
   if (swap_flag) byteswap_ui2(&ishort1, 1);	// Version no.
   n_read = read(input_fd, &ishort2, 2);
   if (swap_flag) byteswap_ui2(&ishort2, 1);	// Rev major no
   n_read = read(input_fd, &ishort3, 2);
   if (swap_flag) byteswap_ui2(&ishort3, 1);	// Rev minor no
   n_read = read(input_fd, &ishort4, 2);
   if (swap_flag) byteswap_ui2(&ishort4, 1);	// No of keys
   ngeokeys = ishort4;
   
   // ************************
   // Read keys
   // ************************
   for (i=0; i<ngeokeys; i++) {
      n_read = read(input_fd, &ishort1, 2);
      if (swap_flag) byteswap_ui2(&ishort1, 1);
      n_read = read(input_fd, &ishort2, 2);
      if (swap_flag) byteswap_ui2(&ishort2, 1);
      n_read = read(input_fd, &ishort3, 2);
      if (swap_flag) byteswap_ui2(&ishort3, 1);
      n_read = read(input_fd, &ishort4, 2);
      if (n_read != 2){
         cerr << "image_tif_class::read_goekeys;  cant read " << endl;
         exit_safe(1, "image_tif_class::read_goekeys;  cant read ");
      }
      if (swap_flag) byteswap_ui2(&ishort4, 1);

      keyid = ishort1;
      
      if      (keyid == 1024) {
        if (diag_flag > 0) cout << "  keyid=1024 GTModelTypeGeoKey     " << ishort2 << " " << ishort3 << " " << ishort4
		 << "1=Projected system, 2=Geographic" << endl;
		GTModelTypeGeoKeyVal = ishort4;
      }
      else if (keyid == 1025) {
		 if (ishort4 == 1) {
			areaOrPointFlag = 1;
            if (diag_flag > 0) cout << "  keyid=1025 GTRasterTypeGeoKey4=1->RasterPixelIsArea" << endl;
		 }
		 else if (ishort4 == 2) {
			areaOrPointFlag = 2;
            if (diag_flag > 0) cout << "  keyid=1025 GTRasterTypeGeoKey4==2->RasterPixelIsPoint " << endl;
		 }
      }
      else if (keyid == 1026) {
        if (diag_flag > 0) cout << "  keyid=1026 GTCitationGeoKey      " << ishort2 << " " << ishort3 << " " << ishort4 << endl;
      }
      else if (keyid == 2048) {
        if (diag_flag > 0) cout << "  keyid=1026 GeographicTypeGeoKey      " << ishort2 << " " << ishort3 << " " << ishort4 << endl;
		GeographicTypeGeoKeyVal = ishort4;
      }
      else if (keyid == 2052) {
        if (diag_flag > 0) cout << "  keyid=2052 GeogLinearUnitsGeoKey " << ishort2 << " " << ishort3 << " " << ishort4 << endl;
        if (ishort4 == 9001) {
           if (diag_flag > 0) cout << "     x-y units = Linear_Meter " << endl;
        }
        else if (ishort4 == 9002) {
           if (diag_flag > 0) cout << "     x-y units = Linear_Foot " << endl;
           xyunits_to_m = .3048f;
        }
        else if (ishort4 == 9003) {
           if (diag_flag > 0) cout << "     x-y units = Linear_Foot_US_Survey " << endl;
           xyunits_to_m = 1200.0f /3937.0f;
        }
		else {
           if (diag_flag > 0) cout << "     x-y units unknown -- assume Linear_Meter " << endl;
        }
      }
      else if (keyid == 2054) {				// NOT YET IMPLEMENTED
        if (diag_flag > 0) cout << "  keyid=2054 Angular units " << ishort2 << " " << ishort3 << " " << ishort4 << endl;
        if (ishort4 == 9101) {
           if (diag_flag > 0) cout << "     angular units = Radians " << endl;
        }
        else if (ishort4 == 9102) {
           if (diag_flag > 0) cout << "     angular units = Degrees " << endl;
           //xyunits_to_m = .3048f;
        }
		else {
           if (diag_flag > 0) cout << "     angular units unknown -- assume Linear_Meter " << endl;
        }
      }
      else if (keyid == 3076) {
        if (diag_flag > 0) cout << "  keyid=3076 ProjLinearUnitsGeoKey " << ishort2 << " " << ishort3 << " " << ishort4 << endl;
        if (ishort4 == 9001) {
           if (diag_flag > 0) cout << "     x-y units = Linear_Meter " << endl;
        }
        else if (ishort4 == 9002) {
           if (diag_flag > 0) cout << "     x-y units = Linear_Foot " << endl;
           xyunits_to_m  = .3048f;
        }
        else if (ishort4 == 9003) {
           if (diag_flag > 0) cout << "     x-y units = Linear_Foot_US_Survey " << endl;
           xyunits_to_m  = 1200.0f /3937.0f;
        }
		else {
           if (diag_flag > 0) cout << "     x-y units unknown -- assume Linear_Meter " << endl;
        }
      }
      else if (keyid == 3072) {
        if (diag_flag > 0) cout << "  keyid=3072 ProjectedCSTypeGeoKey " << ishort2 << " " << ishort3 << " " << ishort4 << endl;
		localCoordFlag = ishort4;		// This should uniquely specify the Projected Coordinate System of the data
		if (localCoordFlag == 32767) {
			char ctemp[200];
			sprintf(ctemp, "Cant process EPSG coordinate code %d -- set to 3346 (Lithuania) so may be wrong", localCoordFlag);
			localCoordFlag = 3346;
		}
      }
      else if (keyid == 4099) {
        if (ishort4 == 9001) {
           if (diag_flag > 0) cout << "     z units = Linear_Meter " << endl;
        }
        else if (ishort4 == 9002) {
           if (diag_flag > 0) cout << "     z units = Linear_Foot " << endl;
           zunits_to_m  = .3048f;
        }
        else if (ishort4 == 9003) {
           if (diag_flag > 0) cout << "     z units = Linear_Foot_US_Survey " << endl;
           zunits_to_m  = 1200.0f /3937.0f;
        }
		else {
           if (diag_flag > 0) cout << "     z units unknown -- assume Linear_Meter " << endl;
        }
      }
      else {
        if (diag_flag > 0) cout << "  keyid=" << keyid << " " << ishort2 << " " << ishort3 << " " << ishort4 << endl;
      }
   }

   // If this is a Geographic system rather than the usual Projected system and there is a ID for that system
   if (GTModelTypeGeoKeyVal == 2) 		geog_type_flag = GeographicTypeGeoKeyVal;
   return(1);
}

// ******************************************
/// Read GeoKeys -- Private.
// ******************************************
int image_tif_class::read_geoascii()
{
	if (lseek(input_fd, geoascii_offset, SEEK_SET) != geoascii_offset) {
		cerr << "image_tif_class::read_geoascii;  cant skip " << geoascii_offset << endl;
		exit_safe(1, "image_tif_class::read_geoascii;  cant skip ");
	}
	char* ascii_parms = new char[ngeoascii];

	read(input_fd, ascii_parms, ngeoascii);
	if (diag_flag > 0) cout << "   ascii keys " << ascii_parms << endl;
	return(1);
}


// ******************************************
/// Read next tiff tag -- Private.
/// Only implemented for tag types that are used, other types are are read so as not to lose the place but discarded.
/// No output indicates which type of data is present and set to output -- it is assumed that outputs are interpreted differently for each tag externally.
/// Byte swapping is not implemented.
///
/// @param tagid	 -- output short int tag ID
/// @param tagtyp	 -- output short int tag type (1=byte, 2=ascii, 3=short int, 4=long, 5=rational, 11=float, 12=double, 16=unsigned 8-byte, 17=signed 8-byte)
/// @param nvals	 -- output unsigned long long no. of data values within tag structure
/// @param outAscii -- output	ascii string
/// @param outInt	 -- output unsigned long long int
/// @param outFloat	 -- output float
/// @param outDouble -- output	double
/// @param offsetFlag	 -- output 1 iff value of tag is an offset
/// @param outOffset 	 -- output unsigned long long offset if present
// ******************************************
int image_tif_class::read_tag(unsigned short &tagid, short &tagtyp, unsigned long long &nvals, char* outAscii, unsigned long long &outInt, 
	float &outFloat, double &outDouble, int &offsetFlag, unsigned long long &outOffset)
{
	int n_read;
	//int ilong;
	long long int ilonglong;
	unsigned short int ushort;
	unsigned long int ulong, numerator, denominator;
	unsigned long long int ulonglong;
	outInt = 0;
	outFloat = 0.;
	outDouble = 0.;
	offsetFlag = 0;
	outOffset = 0;

	n_read = read(input_fd, &tagid, 2);
	if (swap_flag) byteswap_ui2(&tagid, 1);
	n_read = read(input_fd, &tagtyp, 2);	// 1=byte, 2=ascii, 3=short int, 4=long, 5=rational, 11=float, 12=double, 16=unsigned 8-byte, 17=signed 8-byte
	if (swap_flag) byteswap_i2(&tagtyp, 1);

	// *******************************************************
	// Classic Tiff
	// *******************************************************
	n_read = read(input_fd, &ulong, 4);
	nvals = ulong;

	if (nvals > 1) {										// I think all cases of interest are either single val within tag or multiple vals accessed thru offset
		n_read = read(input_fd, &ulong, 4);
		//if (swap_flag) byteswap(&ilong, 4, 4);
		offsetFlag = 1;
		outOffset = ulong;
	}
	else if (tagtyp == 1) {										// Byte -- ignore for now
		n_read = read(input_fd, &ulong, 4); // ignore
	}
	else if (tagtyp == 2) {									// Ascii -- ignore for now
		n_read = read(input_fd, &ulong, 4); // ignore
	}
	else if (tagtyp == 3) {									// Short 16-bit unsigned int
		n_read = read(input_fd, &ushort, 2);
		//if (swap_flag) byteswap_i2(&(short)ushort, 1);
		outInt = ushort;
		n_read = read(input_fd, &ushort, 2); // Eat 2 junk bytes
	}
	else if (tagtyp == 4) {									// Long 32-bit unsigned int
		n_read = read(input_fd, &ulong, 4);
		//if (swap_flag) byteswap(&ulong, 4, 4);
		outInt = ulong;
	}
	else if (tagtyp == 5) {									// Rational -- 2 32-bit unsigned int -- must use offset
		n_read = read(input_fd, &ulong, 4);
		//if (swap_flag) byteswap(&ilong, 4, 4);
		offsetFlag = 1;
		outOffset = ulong;
	}
	else if (tagtyp == 11) {								// Float -- ignore for now
		n_read = read(input_fd, &ulong, 4); // ignore
	}
	else if (tagtyp == 12) {								// Double -- doesn't fit so must use offset
		n_read = read(input_fd, &ulong, 4);
		//if (swap_flag) byteswap(&ilong, 4, 4);
		offsetFlag = 1;
		outOffset = ulong;
	}
	else {													// Unknown -- ignore
		n_read = read(input_fd, &ulong, 4); // ignore
	}
	return(1);
}


// ******************************************
/// Set GeoTiff code for vertical units.
/// @param code		9001 for meters, 9002 for feet, 9003 for US Survey Feet
// ******************************************
int image_tif_class::set_zunits_code(int code)
{
   zunits_code = code;
   return(1);
}

// ******************************************
/// Set GeoTiff code for horizontal units.
/// @param code		9001 for meters, 9002 for feet, 9003 for US Survey Feet
// ******************************************
int image_tif_class::set_xyunits_code(int code)
{
   xyunits_code = code;
   return(1);
}

// ******************************************
/// Set output image tiepoint at north-west corner of image.
/// This tiepoint typically used by GDAL to set the projected coordinates of the output map.
// ******************************************
int image_tif_class::set_tiepoint(double north_bound, double west_bound)
{
	ulx = west_bound;
	uly = north_bound;
	return(1);
}

// ******************************************
/// Set image pixel size in m for export.
// ******************************************
int image_tif_class::set_data_res(float dheight_in, float dwidth_in)
{
   dheight = dheight_in;
   dwidth  = dwidth_in;
   return(1);
}

// ******************************************
/// Set image size in pixels for export.
// ******************************************
int image_tif_class::set_data_size(int nrows_in, int ncols_in)
{
   nrows = nrows_in;
   ncols = ncols_in;
   return(1);
}

// ******************************************
/// Set image data in floating point for export.
// ******************************************
int image_tif_class::set_data_array_float(float *fdata_in)
{
   fdata = fdata_in;
   oCharIntFloatFlag = 3;
   nbands = 1;
   type_write = 1;
   return(1);
}

// ******************************************
/// Set image data in unsigned char for export.
// ******************************************
int image_tif_class::set_data_array_uchar(unsigned char *uchar_in)
{
   data[0] = uchar_in;
   oCharIntFloatFlag = 1;
   nbands = 1;
   type_write = 0;
   return(1);
}

// ******************************************
/// Set image data in unsigned char rbg (3 bytes per pixel) for export.
// ******************************************
int image_tif_class::set_data_array_rgb(unsigned char *uchar_in)
{
   data[0] = uchar_in;
   oCharIntFloatFlag = 1;
   nbands = 3;
   type_write = 2;
   return(1);
}

// ******************************************
/// Write current data to a file.
/// Uses GDAL and imports coord system from the gps_calc_class.
// ******************************************
int image_tif_class::write_file(string sfilename)
{
	if (gps_calc == NULL) {					// Cant write unless coord system defined to pass to output
		exit_safe_s("image_tif_class::write_file: No coord system defined", sfilename);
	}

#if defined(LIBS_GDAL)
	GDALDriver *poDriver;
	char ** papszMetadata;
	GDALDataset *poDstDS;
	char **papszOptions = NULL;
	poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	papszMetadata = poDriver->GetMetadata();
	if (CSLFetchBoolean(papszMetadata, GDAL_DCAP_CREATE, FALSE)) printf("Driver supports create method ");

	double adfGeoTransform[6];
	GDALRasterBand *poBand;
	OGRSpatialReference* poSRS = gps_calc->get_spatial_ref();
	char *pszSRS_WKT;
	pszSRS_WKT = NULL;
	poSRS->exportToWkt(&pszSRS_WKT);

	// Set transform
	if (ulx == 0.) {
		ulx = utm_cen_east[0]  - float(ncols) * dwidth / 2.;
		uly = utm_cen_north[0] + float(nrows) * dheight / 2.;
		lrx = utm_cen_east[0]  + float(ncols) * dwidth / 2.;
		lry = utm_cen_north[0] - float(nrows) * dheight / 2.;
	}
	adfGeoTransform[0] = ulx;			// top left x
	adfGeoTransform[1] = dwidth;		// x pixel res
	adfGeoTransform[2] = 0;				// zero
	adfGeoTransform[3] = uly;			// top left y
	adfGeoTransform[4] = 0;				// zero
	adfGeoTransform[5] = -dheight;		// y pixel res (negative value)

	if (oCharIntFloatFlag == 3) {								// Write float
		poDstDS = poDriver->Create(sfilename.c_str(), ncols, nrows, 1, GDT_Float32, papszOptions);
		poDstDS->SetMetadataItem("AREA_OR_POINT", "POINT");
		poDstDS->SetGeoTransform(adfGeoTransform);
		poDstDS->SetProjection(pszSRS_WKT);
		poBand = poDstDS->GetRasterBand(1);
		poBand->RasterIO(GF_Write, 0, 0, ncols, nrows, fdata, ncols, nrows, GDT_Float32, 0, 0);
	}
	else if (oCharIntFloatFlag == 1 && nbands == 1) {								// Write unsigned int gray
		poDstDS = poDriver->Create(sfilename.c_str(), ncols, nrows, 1, GDT_Byte, papszOptions);
		poDstDS->SetMetadataItem("AREA_OR_POINT", "POINT");
		poDstDS->SetGeoTransform(adfGeoTransform);
		poDstDS->SetProjection(pszSRS_WKT);
		poBand = poDstDS->GetRasterBand(1);
		poBand->RasterIO(GF_Write, 0, 0, ncols, nrows, data[0], ncols, nrows, GDT_Byte, 0, 0);
	}
	else if (oCharIntFloatFlag == 1 && nbands == 3) {								// Write unsigned int rgb
		papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "PIXEL");				// Looks to be default -- doesnt change anything
		//papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "BAND");				// Definitely wrong
		poDstDS = poDriver->Create(sfilename.c_str(), ncols, nrows, 3, GDT_Byte, papszOptions);
		poDstDS->SetMetadataItem("AREA_OR_POINT", "POINT");
		poDstDS->SetGeoTransform(adfGeoTransform);
		poDstDS->SetProjection(pszSRS_WKT);
		// Last 3 parms address data order which is assumed to be interleaved by pixel  (eg. r1,g1,b1,r2,g2,b2,...)
		//   they are bytes between beginning of each pixel, bytes between scanlines, and bytes between bands (zeros indicate default)
		poDstDS->RasterIO(GF_Write, 0, 0, ncols, nrows, data[0], ncols, nrows, GDT_Byte, 3, NULL, 0, 0, 0);	// Complex -- may be wrong
	}
	else {
		warning(1, "Write option not implemented");
		return(0);
	}
	GDALClose((GDALDatasetH)poDstDS);
	return(1);
#else
	return(0);
#endif
}
