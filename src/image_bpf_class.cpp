#include "internals.h"

// ********************************************************************************
/// Constructor.
/// Constructor -- no storage allocated.
// ********************************************************************************
image_bpf_class::image_bpf_class()
        :image_ptcloud_class()
{
   ifield_tau = -99;
   ifield_intens = -99;
   tauFlag = 0;
   intensFlag = 0;
   nTauBins = 128;

   ia = NULL;
   taua = NULL;
   pbpf = NULL;
   diag_flag = 1;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_bpf_class::~image_bpf_class()
{
	free_read();
	if (pbpf != NULL) delete pbpf;
	pbpf = NULL;
}

// ******************************************
/// Return the TAU (point metric) value for index i.
// ******************************************
unsigned char* image_bpf_class::get_tau()
{
	return taua;
}

// ******************************************
/// Given a TAU bin, find the TAU value corresponding to the lower edge of that bin.
// ******************************************
float image_bpf_class::get_raw_tau(int ibin)
{
	if (ibin < 0 || ibin >= nTauBins) return 0.;
	float tau = tcrop_min + ibin * deltau_crop; 
	return tau;
}

// ******************************************
/// Return the intensity value for index i.
// ******************************************
unsigned short image_bpf_class::get_intens(int i)
{
	return ia[i];
}

// ******************************************
/// Return the array of intensity values.
// ******************************************
unsigned short* image_bpf_class::get_intensa()
{
	return ia;
}

// ******************************************
/// Return the x-coordinate value for index i.
// ******************************************
double image_bpf_class::get_x(int i)
{
	return pbpf->getXValue(i);
}

// ******************************************
/// Return the y-coordinate value for index i.
// ******************************************
double image_bpf_class::get_y(int i)
{
	return pbpf->getYValue(i);
}

// ******************************************
/// Return the z-coordinate value for index i.
// ******************************************
double image_bpf_class::get_z(int i)
{
	return pbpf->getZValue(i);
}

// ******************************************
/// Return 1 if the data includes rgb, 0 if not.
// ******************************************
int image_bpf_class::get_rgb_flag()
{
	return(0);
}

// ******************************************
/// Return the z-values for the 3 specified percentiles.
/// Only use points that have high TAU values so noise wont dominate results.
// ******************************************
int image_bpf_class::get_z_at_percentiles(float percentile1, float percentile2, float percentile3, float &z1, float &z2, float &z3, int diag_flag)
{
	clock_t start_time;	// Timing
	long int *zt = new long int[npts_read];
	int nfilt;

	start_time = clock();
	if (tauFlag) {
		int tauth = nTauBins - 3;
		nfilt = 0;
		for (int i=0; i<npts_read; i++) {		// Following scrambles array, so need to copy
			if (taua[i] >= tauth) zt[nfilt++] = pbpf->getZValue(i);
		}
	}
	else {
		nfilt = npts_read;
		for (int i=0; i<npts_read; i++) {		// Following scrambles array, so need to copy
			zt[i] = pbpf->getZValue(i);
		}
	}
	int p1 = percentile1 * nfilt;
	int p2 = percentile2 * nfilt;
	int p3 = percentile3 * nfilt;
	long int zi1 = kth_smallest(zt, nfilt, p1);
	long int zi2 = kth_smallest(zt, nfilt, p2);
	long int zi3 = kth_smallest(zt, nfilt, p3);
	
	delete[] zt;
	z1 = zi1;
	z2 = zi2;
	z3 = zi3;
	double elapsed_time = (double)(clock() - start_time) / double(CLOCKS_PER_SEC);
	if (diag_flag) cout << "  Elapsed time for percentile ops " << elapsed_time << endl;
	return(1);
}

// ******************************************
/// Open image.
/// All work is done by helper class BPFFile which does its own open, so this method does nothing but copy the filename to internal storage.
// ******************************************
int image_bpf_class::read_file_open(string sfilename)
{
	strcpy(filename_cur, sfilename.c_str());
	return(1);
}

// ******************************************
/// Read the BPF header.
/// BPFFile opens file, reads header and closes file
// ******************************************
int image_bpf_class::read_file_header()
{
	if (pbpf == NULL) pbpf = new BPFFile();
	if (!pbpf->readBPFFile(filename_cur, true)) {
		cout << "image_bpf_class::read_file_header:  Cant open/read file " << filename_cur << endl;
		return(0);
	}

	npts_file = pbpf->getNumberOfPoints();

	int iSys = pbpf->getCoordinateSystem();
	int zone = pbpf->getZone();
	if (iSys == 1) {
		epsgTypeCode = 32600 + zone;
	}
	else {
		warning(1, "image_bpf_class: Unknown coordinate system");
		epsgTypeCode = 32600 + 42;

	}
	nbands = 1;	// Intensity only

	int nf = pbpf->getNumberOfExtraFields();
	string s1;
	for (int i=0; i<nf; i++) {
		s1 = pbpf->getExtraFieldLabel(i);
		if (s1.compare("Chi") == 0 || s1.compare("SNR") == 0) {
			ifield_tau = i;
			tauFlag = 1;
		}
		if (s1.compare("psfFltScanWghtdPhtnCnt") == 0 || s1.compare("Relative Intensity") == 0 || s1.compare("Intensity") == 0 || s1.compare("Relative Reflectivity") == 0) {
			ifield_intens = i;
			intensFlag = 1;
		}
		cout << "Add fields " << i << " with name " << s1 << endl;
	}
	bytes_per_point = 3 * sizeof(float);
	if (tauFlag)    bytes_per_point = bytes_per_point + sizeof(float);
	if (intensFlag) bytes_per_point = bytes_per_point + sizeof(float);


	// Suppress reading of extra fields if we cant use them -- saves memory and time
	for (int i=0; i<nf; i++) {
		if (i != ifield_tau && i != ifield_intens) pbpf->suppressExtraField(i);
	}

	int isCompressed = pbpf->getIsCompressed();
	if (isCompressed == 1) {
		warning(1, "image_bpf_class::read_file_header():  Quick lz compression not implemented");
		return(0);
	}

	// Find min/max from header
	emin = pbpf->getXMin();
	emax = pbpf->getXMax();
	nmin = pbpf->getYMin();
	nmax = pbpf->getYMax();
	zmin = pbpf->getZMin();
	zmax = pbpf->getZMax();
	cout << emin << " <x< " << emax << ", " << nmin << " <y< " << nmax << ", " << zmin << " <z< " << zmax << endl;
	if (tauFlag) {
		taumin = pbpf->getFieldMin(ifield_tau);
		taumax = pbpf->getFieldMax(ifield_tau);
		cout << taumin << " <tau< " << taumax << endl;
	}
	else {
		cout << "Cant find tau data in file" << endl;
	}
	if (intensFlag) {
		amin = pbpf->getFieldMin(ifield_intens);
		amax = pbpf->getFieldMax(ifield_intens);
		cout << amin << " <intens< " << amax << endl;
	}
	else {
		cout << "Cant find intensity data in file" << endl;
	}

	utm_cen_north = 0.5 * (nmin + nmax);	
	utm_cen_east  = 0.5 * (emin + emax);
	return(1);
}

// ******************************************
/// Close the image.
/// All work is done by helper class BPFFile which does its own close, so this method does nothing.
// ******************************************
int image_bpf_class::read_file_close()
{
	return(1);
}

// ******************************************
/// Read image -- open, read header, read data and close.
// ******************************************
int image_bpf_class::read_file(string sfilename)
{
   read_file_open(sfilename);
   read_file_header();
   read_file_data();
   read_file_close();
   return(1);
}

// ******************************************
/// Read image data.
// ******************************************
int image_bpf_class::read_file_data()
{
	int i;
	clock_t start_time;	// Timing
	

	pbpf->setNStride(decimation_n);
	npts_read = npts_file / decimation_n;

	// Read
	start_time = clock();
	if (!pbpf->readBPFFile(filename_cur, false)){
		warning_s("image_bpf_class::read_file_data():  Cant read data from file ", filename_cur);
		return(0);
	}

	// Clip extents -- this must be done after reading
	if (clip_extent_flag) {
		pbpf->clipPoints(clip_extent_w, clip_extent_e, clip_extent_s, clip_extent_n, zmin, zmax);	// Limits are in UTM coordinates
		npts_read = pbpf->getNumberOfPoints();
		if (emin < clip_extent_w) emin = clip_extent_w;
		if (emax > clip_extent_e) emax = clip_extent_e;
		if (nmin < clip_extent_s) nmin = clip_extent_s;
		if (nmax > clip_extent_n) nmax = clip_extent_n;
		utm_cen_north = 0.5 * (nmin + nmax);	
		utm_cen_east  = 0.5 * (emin + emax);
	}

	double elapsed_time = (double)(clock() - start_time) / double(CLOCKS_PER_SEC);
	cout << "  Elapsed time in s for read " << elapsed_time << endl;
	
	alloc_read(npts_read);

	// Convert TAU
	if (tauFlag) {start_time = clock();
		bin_tau_linear();
		elapsed_time = (double)(clock() - start_time) / double(CLOCKS_PER_SEC);
		if (diag_flag) cout << "  Elapsed time in s for TAU conversion " << elapsed_time << endl;
	}

	// Convert amp
	if (intensFlag) {
		float arange = amax - amin;
		for (i=0; i<npts_read; i++) {
			float ft = (pbpf->getExtraFieldValueByNumber(ifield_intens, i) - amin) / arange;
			ia[i] = int(255. * ft);
		}
	}
	else {
		for (i=0; i<npts_read; i++)  ia[i] = 255;
		ia[0] = 0;
	}

	// Elevation limits dominated by noise, so redo using only high-TAU values and percentiles
	//float z1, z2, z3;
	//get_z_at_percentiles(0.1, 0.5, 0.9, z1, z2, z3, 1);
	//zmin = z1;
	//zmax = z3;
	//cout << "Elevation bounds filtered by top 3 TAU bins:  " << zmin << " <z< " << zmax << endl;
	return(1);
}

// ******************************************
// Rescale amp to user-limits -- Private
// ******************************************
int image_bpf_class::amp_rescale(int nIn)
{
	int nin = nIn;
	for (int i=0; i<nin; i++) {
		float ft = 255. * (ia[i] - amp_min) / (amp_max  - amp_min);
		if (ft < 0.) ft = 0.;
		if (ft > 255.) ft = 255.;
		ia[i] = ft;
	}
	return(1);
}

// ******************************************
// Print record for single hit of type 1 -- Private
// ******************************************
int image_bpf_class::print_record(int i)
{
   return(1);
}

// ******************************************
/// Allocate.
// ******************************************
int image_bpf_class::alloc_read(int nAlloc)
{
   ia = new unsigned short[nAlloc];
   taua = new unsigned char[nAlloc];
   for (int i=0; i<nAlloc; i++) {
	   taua[i] = nTauBins - 1;
 	   ia[i] = 127;
   }
   return(1);
}

// ********************************************************************************
/// Free memory local to the class -- Private.
// ********************************************************************************
int image_bpf_class::free_read()
{
   if (ia != NULL) delete[] ia;
   if (taua != NULL) delete[] taua;

   ia = NULL;
   taua = NULL;
   return(1);
}

// ********************************************************************************
// Find kth smallest -- Private
// ********************************************************************************
long int image_bpf_class::kth_smallest(long int *a, int n, int k)
{
   // Algorithm by N. Wirth thru N. Devillard
   // Input a is array with n elements.  Returns the kth smallest (rank k) from that array
   
   int i, j, l, m;
   long int x, flt;
   
   l = 0;
   m = n - 1;
   while (l < m) {
      x = a[k];
      i = l;
      j = m;
      do {
         while (a[i] < x) i++;
	 while (x < a[j]) j--;
	 if (i <= j) {
	    flt  = a[j];
	    a[j] = a[i];
	    a[i] = flt;
	    i++;
	    j--;
	 }
      } while (i <= j);
      if (j < k) l=i;
      if (k < i) m=j;
   }
   return a[k];
}

// ********************************************************************************
/// Convert TAU from float to bin numbers -- proportional.
/// Put roughly the same number of points in each bin.
// ********************************************************************************
int image_bpf_class::bin_tau_prop()
{
	int i, ibin, histi[1000];
	float *taulim = new float[nTauBins];
	//for (i=0; i<1000; i++) histi[i] = 0;
	memset(histi, 0, 1000*sizeof(int));

	// Min will be close to zero so assume 0.  Round max
	int it = int(10.*taumax) + 1;
	float taumaxr = 0.1 * it;
	float deltau = taumaxr / 1000.;

	// Make histogram
	for (i=0; i<npts_read; i++) {
		ibin = pbpf->getExtraFieldValueByNumber(ifield_tau, i)/ deltau;
		histi[ibin]++;
	}

	// Integrate histogram
	for (i=1; i<1000; i++){
		histi[i] = histi[i-1] + histi[i];
	}

	// Find lower limits of each bin such that no. of pts in each bin is approx the same
	i = 0;
	for (ibin=0; ibin<nTauBins; ibin++) {
		float tmin = float(ibin) * float(npts_read) / float(nTauBins);
		while (histi[i] < tmin) {
			i++;
		}
		taulim[ibin] = i * deltau;
		cout << ibin << " " << tmin << " " << i << " " << histi[i] << " " << taulim[ibin] << endl;
	}

	// Put bin no. into taua array
	for (i=0; i<npts_read; i++) {
		float tau = pbpf->getExtraFieldValueByNumber(ifield_tau, i);
		for (ibin=nTauBins-1; ibin>=0; ibin--) {
			if (tau >= taulim[ibin]) {
				taua[i] = ibin;
				break;
			}
		}
	}
	return(1);
}

// ********************************************************************************
/// Convert TAU from float to bin numbers -- linear.
/// TAU space is first truncated then split up linearly (same TAU span in each bin) into bins.
// ********************************************************************************
int image_bpf_class::bin_tau_linear()
{
	int i, ibin, histi[1000];
	memset(histi, 0, 1000*sizeof(int));
	float hcrop_min = 0.1 * npts_read;		// Crop off lower 10% 
	float hcrop_max = 0.995 * npts_read;		// Crop off upper 0.5%

	// Min will be close to zero so assume 0.  Round max
	int it = int(10.*taumax) + 1;
	float taumaxr = 0.1 * it;
	float deltau = taumaxr / 1000.;

	// Make histogram
	for (i=0; i<npts_read; i++) {
		ibin = pbpf->getExtraFieldValueByNumber(ifield_tau, i)/ deltau;
		histi[ibin]++;
	}

	// Integrate histogram
	for (i=1; i<1000; i++){
		histi[i] = histi[i-1] + histi[i];
	}

	// Find crop limit TAU values
	for (i=0; i<1000; i++){
		if (histi[i] > hcrop_min) {
			tcrop_min = i * deltau;
			break;
		}
	}
	for (i=1000-1; i>=0; i--){
		if (histi[i] < hcrop_max) {
			tcrop_max = i * deltau;
			break;
		}
	}
	deltau_crop = (tcrop_max - tcrop_min) /nTauBins;
	cout << "TAU crop min=" << tcrop_min << ", max=" << tcrop_max << ", delTAU=" << deltau_crop << endl;

	// Put bin no. into taua array
	for (i=0; i<npts_read; i++) {
		float tau = pbpf->getExtraFieldValueByNumber(ifield_tau, i);
		ibin = (tau - tcrop_min) / deltau_crop;
		if (ibin < 0) ibin = 0;
		if (ibin > nTauBins-1) ibin = nTauBins-1;
		taua[i] = ibin;
	}
	return(1);
}




