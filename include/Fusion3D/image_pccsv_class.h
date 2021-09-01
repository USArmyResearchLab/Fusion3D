/** @file */
/** @class image_pccsv_class 
Reads point-cloud lidar data in cvs format.
Reads point-cloud lidar data in cvs format.
\n
This class is currently experimental -- it has only been tested on a few files.
\n
There is not method to reset the class, so it can only read a single file.
*/
#ifndef _image_pccsv_class_h_
#define _image_pccsv_class_h_
#ifdef __cplusplus


class image_pccsv_class :public image_ptcloud_class {
protected:
	int colx;					///< Format -- Col no. for x values
	int coly;					///< Format -- Col no. for y values
	int colz;					///< Format -- Col no. for z values
	int coli;					///< Format -- Col no. for intensity values
	int colr;					///< Format -- Col no. for red values
	int colg;					///< Format -- Col no. for green values
	int colb;					///< Format -- Col no. for blue values
	char delim;					///< Format -- Delimiter character between cols of data, eg ',' or ' '
	int nheader;				///< Format -- No of header lines

	vector<float> xv, yv, zv;			///< Values -- Array storage for locations in local coords
	vector<unsigned short> rv, gv, bv;	///< Values -- Array storage for colors
	vector<unsigned short> iav;			///< Values -- Array storage for intensities

	ifstream *isp;
	string sfilename;
	float  amin, amax;			// Min and max of data array -- intensity
	int data_read_flag;			///< 0 if data not yet read, 1 if data read

	// Private methods
	long int kth_smallest(long int *a, int n, int k);

public:
	image_pccsv_class();
	~image_pccsv_class();

	int set_format(int icolx, int icoly, int icolz, int icoli, int icolr, int icolg, int icolb, int nheader, char delim);

	int read_file(string sfilename) override;
	int read_file_open(string sfilename) override;
	int read_file_header() override;
	int read_file_data() override;
	int read_file_close() override;

	double get_x(int i) override;
	double get_y(int i) override;
	double get_z(int i) override;
	unsigned short* get_intensa() override;
	unsigned short* get_reda() override;
	unsigned short* get_grna() override;
	unsigned short* get_blua() override;

	int get_z_at_percentiles(float percentile1, float percentile2, float percentile3, float &z1, float &z2, float &z3, int diag_flag) override;
};

#endif /* __cplusplus */
#endif /* _image_pccsv_class_h_ */
