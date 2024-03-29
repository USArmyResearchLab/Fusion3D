#include "internals.h"
#
// ********************************************************************************
/// Constructor.
// ********************************************************************************
image_mrsid_class::image_mrsid_class(gps_calc_class *gps_calc_in)
        :image_geo_class()
{
	if (gps_calc_in == NULL) {
		exit_safe(1, "image_mrsid_class::image_mrsid_class:  NULL gps_calc_class");
	}
	class_type = "sid";
	gps_calc = gps_calc_in;
	bsq_flag = 0;
   data_type = 6;			// Default color -- can recognize RGB and Grayscale
   iCharIntFloatFlag = 1;	// 
   membuf = NULL;
   membuf2 = NULL;
#if defined(LIBS_MRSID)
   reader = NULL;
   scene = NULL;
   cropFilter = NULL;
   downsampleFilter = NULL;
#endif
   // diag_flag = 99;				// Uncomment this to get lots of diag
   data = new unsigned char*[1];		// Assume only single frame
   data[0] = NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_mrsid_class::~image_mrsid_class()
{
   if (membuf != NULL) delete[] membuf;
   if (membuf != NULL) membuf = NULL;
   if (membuf2 != NULL) delete[] membuf2;
   if (membuf2 != NULL) membuf2 = NULL;
   delete data;
}

// ******************************************
/// Set the output data ordering to BSQ (default is BIP).
/// The default data ordering for MrSID is BSQ (r,r,...,r,g,g,...,g,b,b,...,b).
/// However, the default for our codes is BIP (r,g,b,r,g,b,...,r,g,b).
/// For example, OIV textures use BIP ordering.
// ******************************************
int image_mrsid_class::set_bsq()
{
	bsq_flag = 1;
	return(1);
}

// ******************************************
/// Read image.
// ******************************************
int image_mrsid_class::read_file(string sfilename)
{
   read_file_open(sfilename);
   read_file_header();
   read_file_data();
   read_file_close();
   return(1);
}

// ******************************************
/// Open image.
/// This opens the file and also initializes the MrSID reader.
// ******************************************
int image_mrsid_class::read_file_open(string sfilename)
{
#if defined(LIBS_MRSID)
	LT_STATUS sts = LT_STS_Uninit;

	const LTFileSpec fileSpec(sfilename.c_str());
	reader = MrSIDImageReader::create();
	if (reader == NULL) {
		warning_s("image_mrsid_class::read_file_open: Cant open image ", sfilename);
		return(0);
	}
	sts = reader->initialize(fileSpec);
	if (sts != LT_STS_Success) {
		warning_s("image_mrsid_class::read_file_open: Cant initialize from ", sfilename);
		return(0);
	}
#endif
	return(1);
}

// ******************************************
/// Read the header from an open image.
/// This information has already been read by MrSID on file opening, but is transfered to local memory here.
/// Gets EPSG if it can using the WKT (This looks to be the only way available).
/// Tries GDAL first, then tries to read the EPSG explicitly from the WKT (havent tested this option because dont have any sample files)
/// In some files, there is no WKT in which case the EPSG is left as the default 0.
/// In some files, the EPSG is defined explicitly in the WKT in which case GDAL seems to work (eg BuckEye Belgium).
/// In some files, the EPSG is not defined explititly, so the code attempts to use GDAL to convert the WKT to an EPSG (eg BuckEye Nangahar).
/// If neither of these techniques work, the EPSG is left as the default 0.

// ******************************************
int image_mrsid_class::read_file_header()
{
	int code;
#if defined(LIBS_MRSID)
   // we will decode the whole image at  full resolution
   LT_STATUS sts = LT_STS_Uninit;
   lt_uint32 wAll=0, hAll=0;

   sts = reader->getDimsAtMag(1.0, wAll, hAll);

   const double pixelWidth = reader->getWidth();	// Width of image in pixels
   const double pixelHeight = reader->getHeight();	// Height of image in pixels
   const LTIGeoCoord& geo = reader->getGeoCoord();

   // *****************************************
   // Get EPSG code
   /// I have not found a very robust way of finding the local coord system -- I think thru the WKT is the only way to do it.
   // ******************************************
   const char* wkt = geo.getWKT();
   if (wkt == NULL) {		// If no wkt, just give up and hope it has the same coordinates as the DEM
	   cerr << "image_mrsid finds no WKT so cant get EPSG " << endl;
   }

   // Try using GDAL
   else if (get_epsg_using_gdal(wkt, code)) {
	   localCoordFlag = code;
   }

   // Search in the wkt for 'EPSG'
   else if (get_epsg_by_searching_wkt(wkt, code)) {
	   localCoordFlag = code;
   }
   else {											// Cant parse the WKT so just leave it at default
	   cerr << "image_mrsid cant parse WKT so cant get EPSG " << endl;
   }

   // Below should be a better way to do this, but with test files (Montreal, generated by Digital Globe) this gives the wrong answer
   // There are 2 different entries that give different answers.
   // So this was abandoned -- there is maybe a best way, but with no documentation I was not able to find it
   /*
   const LTIGeoCoord& geo = reader->getGeoCoord();
   const LTIMetadataDatabase& dbx = reader->getMetadata();
   const lt_uint32 numRecs = dbx.getIndexCount();
   unsigned short zz;
   for (lt_uint32 i = 0; i < numRecs; i++)
   {
   const LTIMetadataRecord* rec = NULL;
   dbx.getDataByIndex(i, rec);
   const char *yy = rec->getTagName();
   if (strstr(yy, "ProjectedCSTypeGeoKey") != NULL) {
   zz = (unsigned short)rec->getScalarData();
   cout << "tag name " << yy << " tag val " << zz << endl;
   }
   else {
   cout << "tag name " << yy << endl;
   }
   }
   */

   // *****************************************
   // Get image location and dimensions
   // ******************************************
   ulx = geo.getX();
   uly = geo.getY();
   xRes = geo.getXRes();
   yRes = geo.getYRes();
   lrx = ulx + xRes * pixelWidth;			// Local coords
   lry = uly + yRes * pixelHeight;
   if (!gps_calc->is_coord_system_defined()) {
	   gps_calc->init_from_epsg_code_number(localCoordFlag);
   }
   else {
	   transform_to_global_coord_system(localCoordFlag);		// Translate to global coord system if different
   }
   ncols = reader->getWidth();
   nrows = reader->getHeight();
   dwidth  =  (float)xRes;
   dheight = -(float)yRes;	// MrSID yRes is negative, ours is positive
   cen_utm_east  = ulx + xRes * ncols / 2.0;
   cen_utm_north = uly + yRes * nrows / 2.0;
   if (reader->getColorSpace() == LTI_COLORSPACE_RGB) {
      data_type = 6;
   }
   else if (reader->getColorSpace() == LTI_COLORSPACE_GRAYSCALE) {
      data_type = 5;
   }
   else {
	   cout << "MrSID unknown color-space -- assume RGB" << endl;
   }

#endif
   return(1);
}

// ******************************************
/// Read the data from an open image.
/// The data for a window is read into a buffer.
/// The default data ordering is BSQ, but if the flag is not set the data is converted to BIP ordering
// ******************************************
int image_mrsid_class::read_file_data()
{
#if defined(LIBS_MRSID)
   LT_STATUS sts = LT_STS_Uninit;

   if (crop_flag == 0) {
	   w1Crop = 0;
	   h1Crop = 0;
	   wCrop = ncols;
	   hCrop = nrows;
   }

   else if (crop_flag == 1) {
	   wCrop = crop_ncols;
	   hCrop = crop_nrows;
	   double ulxCrop = crop_cen_east - 0.5 * dwidth  * crop_ncols;
	   double ulyCrop = crop_cen_north + 0.5 * dheight * crop_nrows;
	   double dw = ulxCrop - ulx;
	   double dh = uly - ulyCrop;
	   double fw1 = dw / dwidth;
	   double fh1 = dh / dheight;
	   w1Crop = int(fw1);
	   h1Crop = int(fh1);
   }

   else if (crop_flag == 2) {
	   wCrop = iw2 - iw1;
	   hCrop = ih2 - ih1;
	   w1Crop = iw1;
	   h1Crop = ih1;
   }

   // decode the whole image at full resolution
   scene = new LTIScene(w1Crop, h1Crop, wCrop, hCrop, 1.0);
   
   // construct the buffer we're decoding into
   // note we choose to allocate our own buffer, rather than let
   // LTISceneBuffer implicitly allocate one for us
   const lt_uint32 siz = wCrop * hCrop * 1;
   if (membuf != NULL) delete[] membuf;
   membuf = new lt_uint8[siz*3];
   void* bufs[3] = { membuf+siz*0, membuf+siz*1, membuf+siz*2 };
   LTISceneBuffer bufData(reader->getPixelProps(), wCrop, hCrop, bufs);
   
   // perform the decode
   sts = reader->read(*scene, bufData);
   if (sts != LT_STS_Success) {
	   cout << "image_mrsid_class::read_image: Cant read image " << endl;
	   return(0);
   }

   // perform the decode
   if (bsq_flag) {
      data[0] = membuf;
   }
   else {
      if (membuf2 != NULL) delete[] membuf2;
	  membuf2 = new lt_uint8[siz*3];
      sts = bufData.exportDataBIP((void*&)membuf2);
      if (sts != LT_STS_Success) {
	     cout << "image_mrsid_class::read_image: Cant read image " << endl;
	     return(0);
      }
      data[0] = membuf2;
	  delete[] membuf;
	  membuf = NULL;
   }
#endif
  return(1);
}

// ******************************************
/// Read the data from an open image -- read entire image at downsampled resolution.
/// MrSID images are highly compressed and often the entire image cant be stored at max resolution.
/// This class enables reading at downsampled resolution without creating the entire full-res image.
/// It takes advantage of capability of MrSID to read lower resolutions directly.
/// The data for a window is read into a buffer.
/// The default data ordering is BSQ, but if the flag is not set the data is converted to BIP ordering
///
/// @param downsample ratio -- Must be power of 2 (what MrSID natively supports) or results will probably be wrong
///
// ******************************************
int image_mrsid_class::read_file_data_down(int downsample_ratio)
{
#if defined(LIBS_MRSID)
	LT_STATUS sts = LT_STS_Uninit;

	int nc = ncols / downsample_ratio;
	int nr = nrows / downsample_ratio;
	double magnification = 1.0 / double(downsample_ratio);

	// decode the whole image at full resolution
	scene = new LTIScene(0, 0, nc, nr, magnification);

	// construct the buffer we're decoding into
	// note we choose to allocate our own buffer, rather than let
	// LTISceneBuffer implicitly allocate one for us
	const lt_uint32 siz = nc * nr * 1;
	if (membuf != NULL) delete[] membuf;
	membuf = new lt_uint8[siz * 3];
	void* bufs[3] = { membuf + siz * 0, membuf + siz * 1, membuf + siz * 2 };
	LTISceneBuffer bufData(reader->getPixelProps(), nc, nr, bufs);

	// perform the decode
	sts = reader->read(*scene, bufData);
	if (sts != LT_STS_Success) {
		cout << "image_mrsid_class::read_image: Cant read image " << endl;
		return(0);
	}

	// perform the decode
	if (bsq_flag) {
		data[0] = membuf;
	}
	else {
		if (membuf2 != NULL) delete[] membuf2;
		membuf2 = new lt_uint8[siz * 3];
		sts = bufData.exportDataBIP((void*&)membuf2);
		if (sts != LT_STS_Success) {
			cout << "image_mrsid_class::read_image: Cant read image " << endl;
			return(0);
		}
		data[0] = membuf2;
		delete[] membuf;
		membuf = NULL;
	}
#endif
	return(1);
}

// ******************************************
/// Read the data from an open image into a Region Of Interest (ROI).
/// If error reading image, return 0.
/// If the ROI does not intersect the MrSID image, return 1.
/// If there is an intersection, copy the MrSID data to the portion of the ROI that intersects --
/// a ROI may intersect multiple MrSID image or a portion of it may not intersect any.
/// The default data ordering is BSQ, so the data is converted to BIP ordering as required for OIV textures.
/// @param	roi_w	left boundary of ROI in UTM m
/// @param	roi_n	top  boundary of ROI in UTM m
/// @param	roi_e	right boundary of ROI in UTM m
/// @param	roi_s	bottom boundary of ROI in UTM m
/// @param	nxcrop	Size of output cropped and (maybe) downsampled ROI in pixels in x
/// @param	nycrop	Size of output cropped and (maybe) downsampled ROI in pixels in y
/// @param	data	Output data array for ROI image (allocated outside this class
/// @param	ninter	Input/output -- current number of images that have intersected ROI (on input, previous intersecions; on output, incremented if current image intersects)
// ******************************************
int image_mrsid_class::read_into_roi(double roi_w, double roi_n, double roi_e, double roi_s, int nxcrop, int nycrop, unsigned char *data, int &ninter)
{
#if defined(LIBS_MRSID)
	int ih, iw, iin, iout;
	double roi_w_local, roi_e_local, roi_n_local, roi_s_local;
	LT_STATUS sts = LT_STS_Uninit;
	
	partial_flag = 1;
	float dwidth_mag = dwidth / mag;
	float dheight_mag = dheight / mag;
	int ncols_mag = ncols * mag;
	int nrows_mag = nrows * mag;

	roi_n_local = roi_n + north_translate;
	roi_s_local = roi_s + north_translate;
	roi_e_local = roi_e + east_translate;
	roi_w_local = roi_w + east_translate;

	if      (roi_w_local > lrx || roi_e_local < ulx || roi_n_local < lry || roi_s_local > uly) {			// No intersection
		return(1);
	}
	else if (roi_w_local > ulx && roi_e_local < lrx && roi_n_local < uly && roi_s_local > lry) {	// ROI fully enclosed
		  w1Crop = int((roi_w_local - ulx) / dwidth_mag + 0.5);
		  w2Crop = w1Crop + nxcrop;
		  h1Crop = int((uly - roi_n_local) / dheight_mag + 0.5);
		  h2Crop = h1Crop + nycrop;
		  wCrop = nxcrop;
		  hCrop = nycrop;
		  partial_flag = 0;
	}
	else {																	// Partial intersection
	  if (roi_w_local >= ulx && roi_e_local <= lrx) { // In this dimension, entirely within
		  w1Crop = int((roi_w_local - ulx) / dwidth_mag + 0.5);
		  w2Crop = w1Crop + nxcrop;
		  loc_w1 = 0;
	  }
	  else if (roi_w_local < ulx) {
		  w1Crop = 0;
		  w2Crop = int((roi_e_local - ulx) / dwidth_mag + 0.5);
		  loc_w1 = nxcrop - (w2Crop -w1Crop);
	  }
	  else {
		  w1Crop = int((roi_w_local - ulx) / dwidth_mag + 0.5);
		  w2Crop  = ncols_mag;
		  loc_w1 = 0;
	  }
	  wCrop = w2Crop - w1Crop;
      
	  if (roi_s_local >= lry && roi_n_local <= uly) { // In this dimension, entirely within
		  h1Crop = int((uly - roi_n_local) / dheight_mag + 0.5);
		  h2Crop = h1Crop + nycrop;
		  loc_h1 = 0;
	  }
	  else if (roi_n_local > uly) {
		  h1Crop = 0;
		  h2Crop = int((uly - roi_s_local) / dheight_mag + 0.5);
		  loc_h1 = nycrop - (h2Crop -h1Crop);
	  }
	  else {
		  h1Crop = int((uly - roi_n_local) / dheight_mag + 0.5);
		  h2Crop  = nrows_mag;
		  loc_h1 = 0;
	  }
	  hCrop = h2Crop - h1Crop;
	}

	if (wCrop <= 0 || hCrop <= 0) {	// For downsampled, may not have samples even tho areas intersect
		return(1);
	}

	scene = new LTIScene(w1Crop, h1Crop, wCrop, hCrop, mag);

	const lt_uint32 siz = wCrop * hCrop;
	unsigned char *membuft = new lt_uint8[siz*3];
	void* bufs[3] = { membuft+siz*0, membuft+siz*1, membuft+siz*2 };
	LTISceneBuffer bufData(reader->getPixelProps(), wCrop, hCrop, bufs);
   
	// perform the decode
	sts = reader->read(*scene, bufData);
	if (sts != LT_STS_Success) {
	   cout << "image_mrsid_class::read_into_roi: Cant read image " << endl;
	   return(0);
	}

	// ****************************************
	// If only partial intersection, get the partial image and transfer it into the full cropped image
	// ****************************************
	if (partial_flag) {
		unsigned char *datat = new lt_uint8[siz*3];
		sts = bufData.exportDataBIP((void*&)datat);
		if (sts != LT_STS_Success) {
			cout << "image_mrsid_class::read_into_roi: Cant reorder data " << endl;
			return(0);
		}
		for (ih=0, iin=0; ih<hCrop; ih++) {
			iout = (loc_h1 + ih) * nxcrop + loc_w1;
			for (iw=0; iw<wCrop; iw++,iin++,iout++) {
				data[3*iout  ] = datat[3*iin  ];
				data[3*iout+1] = datat[3*iin+1];
				data[3*iout+2] = datat[3*iin+2];
			}
		}
		delete[] datat;
	}

	// ****************************************
	// If ROI enclosed, get the full cropped image into the given data array
	// ****************************************
	else {
		sts = bufData.exportDataBIP((void*&)data);
		if (sts != LT_STS_Success) {
			cout << "image_mrsid_class::read_into_roi: Cant read image " << endl;
			return(0);
		}
	}

	ninter++;
	delete[] membuft;
#endif
	return(1);
}

// ******************************************
/// Close the image.
/// The MrSID reader is also closed, so no further MrSID operations can be done.
// ******************************************
int image_mrsid_class::read_file_close()
{
#if defined(LIBS_MRSID)
   reader->release();
   reader = NULL;
   delete scene;
   scene = NULL;
   if (downsampleFilter != NULL) downsampleFilter->release();
   downsampleFilter = NULL;
   if (cropFilter != NULL) cropFilter->release();
   cropFilter = NULL;
   bsq_flag = 0;
   mag = 1.0;
   downsample_flag = 0;
#endif
   return(1);
}

// ******************************************
/// Write (cropped) image in GeoTiff format.
/// The MrSID reader must be open to do this operation, so it must be done before the read_file_close method is called.
// ******************************************
int image_mrsid_class::write_file(string sfilename)
{
#if defined(LIBS_MRSID)
   LT_STATUS sts = LT_STS_Uninit;
   lt_uint32 w, h;

   if (crop_flag == 1) {
	   wCrop = crop_ncols;
	   hCrop = crop_nrows;
	   double ulxCrop = crop_cen_east  - 0.5 * dwidth  * crop_ncols;
	   double ulyCrop = crop_cen_north + 0.5 * dheight * crop_nrows;
	   double dw = ulxCrop -  ulx;
	   double dh = uly -  ulyCrop;
	   double fw1 = dw / dwidth;
	   double fh1 = dh / dheight;
	   w1Crop = int(fw1);
	   h1Crop = int(fh1);
   }

   else if (crop_flag == 2) {
	   wCrop = iw2 - iw1;
	   hCrop = ih2 - ih1;
	   w1Crop = iw1;
	   h1Crop = ih1;
   }

   // Make the crop filter
   cropFilter = LTICropFilter::create();
   sts = cropFilter->initialize(reader, w1Crop, h1Crop, wCrop, hCrop);
   if (sts != LT_STS_Success) {
	   cout << "image_mrsid_class::write_file: Cant initialize crop filter " << endl;
	   return(0);
   }

   // make the downsample filter
   if (downsample_flag) {
	   downsampleFilter = LTIMultiResFilter::create();
	   //downsampleFilter->initialize(cropFilter, scaleFactor); // Doesnt work -- gives gibberish
       downsampleFilter->initialize(cropFilter);
       downsampleFilter->setDeltaMagXY(mag, mag);
       downsampleFilter->setResampleMethod(LTI_RESAMPLE_NEAREST);
       w = downsampleFilter->getWidth();
       h = downsampleFilter->getHeight();
       downsampleFilter->getDimsAtMag(1.0, w, h);
   }

   // make the TIFF writer
   GeoTIFFImageWriter writer;
   if (downsample_flag) {
       sts = writer.initialize(downsampleFilter);
   }
   else {
       sts = writer.initialize(cropFilter);
   }
   //lt_uint32 wt = writer->getWidth();
   //lt_uint32 ht = writer->getHeight();
   if (sts != LT_STS_Success) {
	   cout << "image_mrsid_class::write_file: Cant initialize TIFF writer " << endl;
	   return(0);
   }

   // set up the output file
   sts = writer.setOutputFileSpec(sfilename.c_str());
   if (sts != LT_STS_Success) {
	   cout << "image_mrsid_class::write_file: Cant open output TIFF file " << endl;
	   return(0);
   }
   
   // Use the whole (cropped) image
   w = int(mag * wCrop + .001);
   h = int(mag * hCrop + .001);
   scene = new LTIScene(0, 0, w, h, 1.0);

   // write the scene to the file   
   sts = writer.write(*scene);
   if (sts != LT_STS_Success) {
	   cout << "image_mrsid_class::write_file: Cant write TIFF image " << endl;
	   return(0);
   }
#endif
    return(1);
}


// ******************************************
/// Get EPSG number directly from a WKT -- Looks for PROJCS[...EPSG val].
/// Should find EPSG whenever the value is explicitly defined in the WKT.
/// @param	wkt		input WKT character string
/// @param	code	output EPSG code
/// @return			1 if can find valid EPSG code, 0 if not

// ******************************************
int image_mrsid_class::get_epsg_by_searching_wkt(const char *wkt, int &code)
{
#if defined(LIBS_GDAL)
	string ssub, swkt(wkt);
	size_t pos = swkt.rfind("PROJCS");
	if (pos == string::npos) return(0);							// 
	pos = swkt.rfind("EPSG");
	if (pos == string::npos) return(0);							// 
	ssub = swkt.substr(pos + 7);
	try {
		code = stoi(ssub);
	}
	catch (const std::invalid_argument& ia) {
		cerr << "image_mrsid cant read EPSG code " << ia.what() << endl;
		return(0);
	}

	// Basic sanity check that GDAL can interpret it, since GDAL used to set up coord system
	OGRSpatialReference osr;
	OGRErr ogrerr;
	ogrerr = osr.importFromEPSG(code);
	if (ogrerr != 0) {
		return(0);
	}
	return(1);
#else
	return(0);
#endif
}

// ******************************************
/// Get EPSG number from a WKT using GDAL.
/// Uses GDAL to convert a description of the coordinate system into an EPSG.
/// @param	wkt		input WKT character string
/// @param	code	output EPSG code
/// @return			1 if can find valid EPSG code, 0 if not
// ******************************************
int image_mrsid_class::get_epsg_using_gdal(const char *wkt, int &code)
{
#if defined(LIBS_GDAL)
	OGRSpatialReference osr;
#if defined(_WIN32) || defined(_WIN64)
	osr.importFromWkt(wkt);			// Only defined in very recent rev -- not yet available in linux
#else
	char *wktt = new char[500];
	strcpy(wktt, wkt);
	osr.importFromWkt(&wktt);		// Older def
	// delete[] wktt;
#endif
	OGRErr err = osr.AutoIdentifyEPSG();					// Looks necessary to define an EPSG if not explicitly given in WKT
	if (err != OGRERR_NONE) {
		cerr << "image_mrsid cant read EPSG code using GDAL" << endl;
		return(0);
	}
	const char *codeChar = osr.GetAuthorityCode("PROJCS");	// Works? when EPSG explicitly listed or AutoIdentifyEPSG(), returns NULL otherwise
	if (codeChar == NULL) {
		cerr << "image_mrsid cant read EPSG code using GDAL" << endl;
		return(0);
	}
	//const char * tt2 = osr.GetAuthorityName("PROJCS");	// Returns "EPSG" so not helpful
	//int tt3 = osr.FindProjParm("PROJCS");					// Returns error (-1)

	code = atoi(codeChar);

	if (code > 0) {
		return(1);
	}
	else {
		return(0);
	}
#else
	return(0);
#endif
}
