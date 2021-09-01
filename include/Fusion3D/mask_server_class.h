#ifndef _mask_server_class_h_
#define _mask_server_class_h_
#ifdef __cplusplus

/**
Implements masking operations including LOS shadowing masks.

The only case currently implemented is the LOS mask (for ground-to-ground, standoff sensor and sun shadowing).
There are 2 ways the mask is implemented.
The first method is that 1) the mask is defined in this class including its location, size and the definition and allocation 
of the array holding the mask data;
2) a pointer to the mask array is passed out of this class to the LOS class which populates the class;
3) this class then applies the mask wherever textures are calculated.\n
\n
The second method is that 1) the mask is read in from a Geotiff file that defines its location and size
and the mask array is allocated and populated from that file;
2) this class then applies the mask wherever textures are calculated.\n
\n
This class can write the mask to a Geotiff file.
It writes the data as [0,255] (as opposed to [0,1] when the mask is calculated) where 255 indicates that the pixel is masked.
It scales the data so that the data is easier to see with any viewer.\n
\n
It recognizes both mask values of 1 and 255 as indicating a shadowed pixel.
The mask value of 2 has been used previously in change detection experiments and is used to color a pixel blue.\n

*/
class mask_server_class:public base_jfd_class{
 private:
	 int mask_tex_flag;			///< Current texture mask -- 0 for no elevations defined, 1 for DEM elevations 
	 unsigned char *mask_tex;   ///< Current texture mask -- pointer to elevation array
	 double west_tex;			///< Current texture mask -- file map bounds
	 double north_tex;			///< Current texture mask -- file map bounds
	 int nx_tex;				///< Current texture mask -- no. of pixels in mosaic image
	 int ny_tex;				///< Current texture mask -- no. of pixels in mosaic image
	 float dx_tex;				///< Current texture mask -- pixel size
	 float dy_tex;				///< Current texture mask -- pixel size

	// Private methods

public:
	mask_server_class();
	~mask_server_class();
   
	int read_file(string sfilename) override;
	int write_file(string sfilename) override;
	int read_tagged(const char* filename);
	int write_parms(FILE *out_fd) override;
	int clear_all() override;

	int register_mask_tex(double north, double west, float dx, float dy, int nx, int ny);
	int get_mask_parms_tex(double &north, double &west, float &dx, float &dy, int &nx, int &ny);
	unsigned char *get_mask_tex();
	int apply_mask_tex(unsigned char* data, double north, double west, float dx, float dy, int nx, int ny);
};

#endif /* __cplusplus */
#endif /* _mask_server_class_h_ */
