#include "internals.h"
#include <stdio.h>
#include <stdlib.h>
// ********************************************************************************
/// Constructor.
/// Constructor -- no storage allocated.
// ********************************************************************************
image_pccsv_class::image_pccsv_class()
        :image_ptcloud_class()
{
	colx = 1;
	coly = 2;
	colz = 3;
	coli = -99;					// Default to not present
	colr = -99;					// Default to not present
	colg = -99;					// Default to not present
	colb = -99;					// Default to not present
	delim = ',';
	nheader = 1;
	nbands = 0;
	epsgTypeCode = 0;

	data_read_flag = 0;
	diag_flag = 1;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_pccsv_class::~image_pccsv_class()
{
}

// ******************************************
/// Set the format for reading -- which columns contain what information, no. of headers and column delimiter.
/// This data format is requires this additional information to adequately specify how to read it.
// ******************************************
int image_pccsv_class::set_format(int icolx, int icoly, int icolz, int icoli, int icolr, int icolg, int icolb, int nHeader, char dDelim)
{
	colx = icolx;
	coly = icoly;
	colz = icolz;
	coli = icoli;
	colr = icolr;
	colg = icolg;
	colb = icolb;
	nheader = nHeader;
	delim = dDelim;
	return(1);
}

// ******************************************
/// Return the array of red values.
// ******************************************
unsigned short* image_pccsv_class::get_reda()
{
	unsigned short *p = rv.data();
	return p;
}

// ******************************************
/// Return the array of green values.
// ******************************************
unsigned short* image_pccsv_class::get_grna()
{
	unsigned short *p = gv.data();
	return p;
}

// ******************************************
/// Return the array of blue values.
// ******************************************
unsigned short* image_pccsv_class::get_blua()
{
	unsigned short *p = bv.data();
	return p;
}

// ******************************************
/// Return the array of intensity values.
// ******************************************
unsigned short* image_pccsv_class::get_intensa()
{
	unsigned short *p = iav.data();
	return p;
}

// ******************************************
/// Return the x-coordinate value for index i.
// ******************************************
double image_pccsv_class::get_x(int i)
{
	return xv[i];
}

// ******************************************
/// Return the y-coordinate value for index i.
// ******************************************
double image_pccsv_class::get_y(int i)
{
	return yv[i];
}

// ******************************************
/// Return the z-coordinate value for index i.
// ******************************************
double image_pccsv_class::get_z(int i)
{
	return zv[i];
}

// ******************************************
/// Return the z-values for the 3 specified percentiles.
/// Only use points that have high TAU values so noise wont dominate results.
// ******************************************
int image_pccsv_class::get_z_at_percentiles(float percentile1, float percentile2, float percentile3, float &z1, float &z2, float &z3, int diag_flag)
{
	/*
	clock_t start_time;	// Timing
	long int *zt = new long int[npts_read];
	int nfilt;

	start_time = clock();
	if (tauFlag) {
		int tauth = nTauBins - 3;
		nfilt = 0;
		for (int i=0; i<npts_read; i++) {		// Following scrambles array, so need to copy
			if (taua[i] >= tauth) zt[nfilt++] = ppccsv->getZValue(i);
		}
	}
	else {
		nfilt = npts_read;
		for (int i=0; i<npts_read; i++) {		// Following scrambles array, so need to copy
			zt[i] = ppccsv->getZValue(i);
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
	*/
	return(1);
}

// ******************************************
/// Open image.
/// Opens the ifstream.
// ******************************************
int image_pccsv_class::read_file_open(string sfilename_in)
{
	// ************************************************************
	isp = new ifstream(sfilename_in);
	if (isp == NULL) {
		cerr << "image_pccsv_class::read_file_open:  Cant open input file" << sfilename << endl;
		return(0);
	}
	sfilename = sfilename_in;
	return(1);
}

// ******************************************
/// Read the BPF header.
/// Header is free form and cant be reliably parsed, so actually reads the data to get header info.
// ******************************************
int image_pccsv_class::read_file_header()
{
	read_file_data();		
	return(1);
}

// ******************************************
/// Close the image.
/// Closes the ifstream.
// ******************************************
int image_pccsv_class::read_file_close()
{
	isp->close();
	return(1);
}

// ******************************************
/// Read image -- open, read header, read data and close.
// ******************************************
int image_pccsv_class::read_file(string sfilename)
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
int image_pccsv_class::read_file_data()
{
	int i, iw;
	unsigned short intens=255, red, grn, blu;
	float x, y, z;
	size_t bufsize = 200;
	char *buf = new char[bufsize];
	char word[100];
	if (data_read_flag == 1) return(1);		// This method may get called multiple times so only execute first time

	emin = nmin = zmin = 999999.;
	emax = nmax = zmax = -999999.;
	int nwords = colx;
	if (nwords < coly) nwords = coly;
	if (nwords < colz) nwords = colz;
	if (nwords < coli) nwords = coli;
	if (nwords < colr) nwords = colr;
	if (nwords < colg) nwords = colg;
	if (nwords < colb) nwords = colb;

	if (colr > 0) {
		nbands = 3;
	}
	else if (coli > 0) {
		nbands = 1;
	}
	else {
		nbands = 1;
	}

	// Skip header lines -- do nothing
	isp->seekg(0);						// Make sure you are at beginning
	for (i = 0; i < nheader; i++) {
		isp->getline(buf, bufsize);
	}

	// Read data lines
	while (isp->getline(buf, bufsize)) {
		stringstream iss(buf);
		for (iw = 0; iw < nwords; iw++) {
			iss.getline(word, 100, delim);
			if (iw == colx - 1) x = atof(word);
			if (iw == coly - 1) y = atof(word);
			if (iw == colz - 1) z = atof(word);
			if (iw == coli - 1) intens = atof(word);
			if (iw == colr - 1) red = atof(word);
			if (iw == colg - 1) grn = atof(word);
			if (iw == colb - 1) blu = atof(word);
		}
		xv.push_back(x);
		yv.push_back(y);
		zv.push_back(z);
		if (nbands == 3) {
			rv.push_back(red);
			gv.push_back(grn);
			bv.push_back(blu);
		}
		else {
			iav.push_back(intens);
		}

		if (emin > x) emin = x;
		if (emax < x) emax = x;
		if (nmin > y) nmin = y;
		if (nmax < y) nmax = y;
		if (zmin > z) zmin = z;
		if (zmax < z) zmax = z;
		if (amin > intens) amin = intens;
		if (amax < intens) amax = intens;
	}
	npts_read = xv.size();
	data_read_flag = 1;
	return(1);
}

// ********************************************************************************
// Find kth smallest -- Private
// ********************************************************************************
long int image_pccsv_class::kth_smallest(long int *a, int n, int k)
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


