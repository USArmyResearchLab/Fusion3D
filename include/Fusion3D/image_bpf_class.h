/** @file */
/** @class image_bpf_class 
Reads point-cloud lidar data in .bpf format.
Reads point-cloud lidar data in .bpf format.
\n
This class has the capability to clip the point cloud to a specified region in x and y (not z).
\n
Most of the work is done by helper class image_bpf_class that does the actual reads.
This is the most straightforward use of image_bpf_class and may not be most efficient.
*/
#ifndef _image_bpf_class_h_
#define _image_bpf_class_h_
#ifdef __cplusplus


class image_bpf_class:public image_ptcloud_class{
 protected:
   unsigned short *ia;                  ///< Store -- intensity
   unsigned char *taua;                 ///< Store -- Quality metric TAU -- binned 0-nTauBins

   int nTauBins;							///< TAU -- No. of TAU bins
   float tcrop_min, tcrop_max, deltau_crop;	///< TAU -- Convert float TAU to bins:  ibin = (tau - tcrop_min) / deltau_crop
   int intensFlag;							///< Intensity -- 1 iff intensity values in data

   float  amin, amax;			// Min and max of data array -- intensity
   float  taumin, taumax;       // Min and max of TAU
   
   BPFFile* pbpf;				///< BPFFile -- Helper class that does all the work
   char filename_cur[300];		///< BPFFile -- Current filename
   int ifield_intens;			///< BPFFile -- No of the extra field carrying intensity
   int ifield_tau;				///< BPFFile -- No of the extra field carrying TAU

   FILE *input_fd;

   // Private methods
   int bin_tau_prop();
   int bin_tau_linear();
   int amp_rescale(int nIn);
   int print_record(int irec);
   int alloc_read(int nAlloc);
   int free_read();
   long int kth_smallest(long int *a, int n, int k);
   
 public:
   image_bpf_class();
   ~image_bpf_class();
   
   int read_file(string sfilename) override;
   int read_file_open(string sfilename) override;
   int read_file_header() override;
   int read_file_data() override;
   int read_file_close() override;
   
   int get_rgb_flag();
   double get_x(int i);
   double get_y(int i);
   double get_z(int i);
   unsigned char* get_tau();
   float get_raw_tau(int ibin);
   unsigned short get_intens(int i);
   unsigned short* get_intensa();
   int get_z_at_percentiles(float percentile1, float percentile2, float percentile3, float &z1, float &z2, float &z3, int diag_flag);
};

#endif /* __cplusplus */
#endif /* _image_bpf_class_h_ */
