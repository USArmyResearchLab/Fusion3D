/** @file */
/** @class image_las_class 
Reads and writes point-cloud lidar data in LAS format.
Reads and writes point-cloud lidar data in LAS format.
The class can read up to LAS version 1.4 but only uses information up to LAS version 1.2.  More advanced features will be ignored.
It can read Point Data Record Formats up to type 3 -- more recent Formats will cause the read to abort.
\n
This class has the capability to crop the point cloud to a specified region in x and y (not z).
\n
This class has the capability to cull out points in the point cloud that are close (within a threshold elevation) to a DEM.
\n
The class also has the capability to decimate the point cloud in specified angular regions.
For each 1-deg angular region, 1 out of n points is accepted and the rest are discarded.
\n
The class can write (possibly cropped, culled and decimated) data to a LAS file.
It can also merge multiple files.
The resulting file uses the header of the first file with adjustments to account for the clipping and the reduced number of points.
It copies the contents of the input file up to the beginning of the point data to the output file then overwrites some parameters that have changed.
It assumes that the files are for the same basic area and can be written with a common set of offsets.
It adjusts for different offsets in the files, but assumes the same scaling factors.
*/
#ifndef _image_las_class_h_
#define _image_las_class_h_
#ifdef __cplusplus

// Omit from Doxygen, since essentially private
/// @cond
class LASType0Point
{
public:
   long int x;
   long int y;
   long int z;
   unsigned short intens;
   unsigned char flags;
   unsigned char classification;
   unsigned char sar;
   unsigned char udata;
   unsigned short pointSourceID;
};

class LASheader			// LAS version 1.2 -- 1.3 appends a field and 1.4 appends more
{
public:
  char file_signature[4];
  unsigned short file_source_id;
  unsigned short global_encoding;
  unsigned int project_ID_GUID_data_1;
  unsigned short project_ID_GUID_data_2;
  unsigned short project_ID_GUID_data_3;
  char project_ID_GUID_data_4[8];
  char version_major;
  char version_minor;
  char system_identifier[32];
  char generating_software[32];
  unsigned short file_creation_day;
  unsigned short file_creation_year;
  unsigned short header_size;
  unsigned long offset_to_point_data;
  unsigned long number_of_variable_length_records;
  unsigned char point_data_format;
  unsigned short point_data_record_length;
  unsigned long number_of_point_records;
  unsigned long number_of_points_by_return[5];
  double x_scale_factor;
  double y_scale_factor;
  double z_scale_factor;
  double x_offset;
  double y_offset;
  double z_offset;
  double max_x;
  double min_x;
  double max_y;
  double min_y;
  double max_z;
  double min_z;

  LASheader()
  {
    for (int i = 0; i < sizeof(LASheader); i++) ((char*)this)[i] = 0;
    file_signature[0] = 'L'; file_signature[1] = 'A'; file_signature[2] = 'S'; file_signature[3] = 'F';
    version_major = 1;
    version_minor = 1;
    header_size = 227;
    offset_to_point_data = 227;
    point_data_record_length = 20;
    x_scale_factor = 0.01;
    y_scale_factor = 0.01;
    z_scale_factor = 0.01;
  };
};

struct sGeoKeysHeader
{
   unsigned short wKeyDirectoryVersion;
   unsigned short wKeyRevision;
   unsigned short wMinorRevision;
   unsigned short wNumberOfKeys;
};

struct sGeoKeysEntry
{
   unsigned short wKeyID;
   unsigned short wTIFFTagLocation;
   unsigned short wCount;
   unsigned short wValue_Offset;
};
/// @endcond


class image_las_class:public image_ptcloud_class{
 protected:
   FILE *input_fd;
   LASheader header;						///< Version 1.2
   unsigned long long startWaveformPacket;	///< Appended to version 1.2 header in version 1.3
   unsigned long long startExtendedVLR;		///< Appended to version 1.3 header in version 1.4
   unsigned long nRecExtendedVLR;			///< Appended to version 1.3 header in version 1.4
   unsigned long long nPointRecords;		///< Appended to version 1.3 header in version 1.4
   unsigned long long PtsByReturn[15];		///< Appended to version 1.3 header in version 1.4
   char *headerString;						///< Storage for header string (up to offset to start of data) for later write
   LASType0Point type0Point;				///< Minimal data for a point augmented for other types
   sGeoKeysHeader sGKHeader;
   sGeoKeysEntry sGKEntry;
   
   long int *xa, *ya, *za;					///< LAS store -- scaled location
   unsigned short *ia;						///< LAS store -- intensity
   unsigned short *reda, *grna, *blua;		///< LAS store -- rgb
   unsigned char *rota;						///< LAS store -- rotation angles
   double *timea;							///< LAS store -- time

   int blockSize;							///< Block temp storage -- size of block for read/write
   int nptsb;								///< Block temp storage
   long int *xab, *yab, *zab;				///< Block temp storage
   unsigned short *iab;						///< Block temp storage
   unsigned short *redab, *grnab, *bluab;	///< Block temp storage
   unsigned char *rotab;					///< Block temp storage
   double *timeab;							///< Block temp storage
   unsigned char *flagsab;					///< Block temp storage
   unsigned char *classab;					///< Block temp storage
   unsigned char *udataab;					///< Block temp storage
   unsigned short *pointSourceIDab;			///< Block temp storage

   int point_data_type;						///< Point Data Record Format -- see spec

   int cullDSMFlag;							///< Cull -- 1 iff cull all returns close to DSM
   float cullDSMThreshold;					///< Cull -- Elevation threshold (m) -- points culled if within this distance of DSM
   int cullOnlyFlag;						///< Cull -- 1 iff cull out all only returns (used in demify to avoid duplication of a1/a2)
   int cullRegionFlag;						///< Cull -- 1 iff cull out all pulses outside region
   double ulx_cull, uly_cull;				///< Cull -- UTM region
   double lrx_cull, lry_cull;				///< Cull -- UTM region
   int cullTimeFlag;						///< Cull -- 1 iff cull outside time interval
   double startTime, stopTime;				///< Cull -- Crop times

   int amp_min_raw, amp_max_raw;			///< Min,max amplitudes of raw data
   
   float xyunits_to_m;								///< Convert -- x- and y-units to m on output
   float zunits_to_m;								///< Convert -- z-units to m on output
   int xyunits_key;									///< Convert -- Work 9001/9002/9003 from either tag 3076 (tag 2052 shouldnt be used? but check it anyway)
   int zunits_key;									///< Convert -- Work 9001/9002/9003 from either tag 4099
   double xmult_meters, ymult_meters, zmult_meters;	///< Convert -- output = mult_meters * int + offset_meters
   double xoff_meters, yoff_meters, zoff_meters;	///< Convert -- output = mult_meters * int + offset_meters	

   int clean_izmin;								///< Clean bad z values -- hard limit values lower than this value
   int clean_izmax;								///< Clean bad z values -- hard limit values lower than this value
   int nclean;									///< Clean bad z values -- No. of bad values modified

   int clip_extent_enorm;						///< Clip extent -- east boundary normalized to input coords
   int clip_extent_wnorm;						///< Clip extent -- west boundary normalized to input coords
   int clip_extent_nnorm;						///< Clip extent -- north boundary normalized to input coords
   int clip_extent_snorm;						///< Clip extent -- south boundary normalized to input coords

   int will_write_flag;							///< Write -- 0=no write, 1=write internal after each read block, 2=write later (save some extra stuff only used to match input on write)
   int nfiles_in;								///< Write -- Do some things only on the first file -- incremented on close-input-file
   int nptsWrite;								///< Write -- No of points written -- may extend over multiple files
   unsigned char *flagsa;						///< Write -- additional storage only used if file written
   unsigned char *classa;						///< Write -- additional storage only used if file written
   unsigned char *udataa;						///< Write -- additional storage only used if file written
   unsigned short *pointSourceIDa;				///< Write -- additional storage only used if file written
   double xoff_orig, yoff_orig, zoff_orig;		///< Write -- Offsets from first file
   int xoff_cur, yoff_cur, zoff_cur;			///< Write -- add to current vals to adjust for different header offsets
   unsigned int number_of_points_by_return[5];	///< Write -- No. of points per return number 
   FILE *output_fd;								///< Write -- file designator for write

   int udata_flag;								///< Use udata -- Store udata and make available for output

   int decimate_flag;							///< Decimate -- 1 iff decimation turned on
   int decimate_n[361];							///< Decimate -- per 1-deg angular regions, keep only 1 out of decimate_n[ang]
   int decimate_i[361];							///< Decimate -- per 1-deg angular regions, current count 0 <= decimate_i < decimage_n

   int rotang_limits_flag;						///< 1 iff there are limits on rotation angle
   float rotang_min, rotang_max;				///< Mobile mapping -- limits to rotation angle (sar)

   map3d_index_class *map3d_index;				///< Helper class used to cull points near DEM

   // Private methods
   int read_block(int nInput, int nSkipInput, int &nOutput);
   int write_block(int nmax);
   int transfer_persistent(int nIn, int iOutStart, int &nOut, int nSkip);
   int write_header_adjustments(int ifile_in);
   int cull_near_dsm(int nIn, int &nOut);
   int clip_extent(int nIn, int &nOut);
   int amp_rescale(int nIn);
   int print_record(int irec);
   int read_var_record();
   int read_geokeys();
   int alloc_read(int nAlloc);
   int free_read();
   int alloc_block(int nAlloc);
   int free_block();
   long int kth_smallest(long int *a, int n, int k);
   
 public:
   image_las_class();
   ~image_las_class();
   
   int register_map3d_index_class(map3d_index_class *map3d_index_in);
   int set_udata_flag(int flag);
   int set_rotang_limits(float angmin, float angmax);
   int set_time_crop(double startTimeIn, double stopTimeIn);
   int set_cull_near_dsm(float thresh);
   int set_cull_only_returns();
   int set_decimate_angular_region(int angMin, int angMax, int nDecimateMin, int nDecimateMax);
   int set_x(int i, double xin);
   int set_y(int i, double yin);
   int set_z(int i, float  zin);

   int read_file(string sfilename) override;
   int read_file_open(string sfilename) override;
   int read_file_header() override;
   int read_file_data() override;
   int read_file_close() override;

   int set_write_parms_before_read(int write_parm);
   int begin_write_image(string filename);
   int end_write_image();

   int add_block_init(int npts_block, double xmin, double xmax, double ymin, double ymax, double zmini, double zmaxi, int rgb_flag);
   int add_block_type0(double *xaf, double *yaf, double *zaf, unsigned char *iac, unsigned char *mask, float maxRange);
   
   int get_rgb_flag();
   double get_x(int i);
   double get_y(int i);
   double get_z(int i);
   int get_sar(int i);
   unsigned short get_intens(int i);
   unsigned short get_red(int i);
   unsigned short get_grn(int i);
   unsigned short get_blu(int i);
   unsigned short* get_intensa();
   unsigned short* get_reda();
   unsigned short* get_grna();
   unsigned short* get_blua();
   double get_time(int i);
   unsigned char get_udata(int i);
   int get_z_at_percentiles(float percentile1, float percentile2, float percentile3, float &z1, float &z2, float &z3, int diag_flag);
};

#endif /* __cplusplus */
#endif /* _image_las_class_h_ */
