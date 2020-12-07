#ifndef _elev_estimate_class_h_
#define _elev_estimate_class_h_
#ifdef __cplusplus

/**
This class calculates low-resolution elevations.

This class estimates the low-res elevation at a point by finding the kth-smallest elevation value
for a block of elevation data centered at the point.
The size of the tile is set to be nx-by-ny pixels and the class is passed a block of data of that size
The kth-smallest value has been hard-wired to be 20%.

*/
class elev_estimate_class {
 private:
   int nx, ny;				// Data dimensions
   float *data_a2;			// Input data array [nx_data, ny_data]

   float *work;				// Work array

   // Private methods
float kth_smallest(float *a, int n, int k);
   
 public:
   elev_estimate_class();
   ~elev_estimate_class();
   
   int set_tile_dim(int nx_data, int ny_data);
   int register_data(float *data);
   float get_elev_kth_smallest(float kth_frac);
};

#endif /* __cplusplus */
#endif /* _elev_estimate_class_h_ */
