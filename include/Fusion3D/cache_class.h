#ifndef _cache_class_h_
#define _cache_class_h_
#ifdef __cplusplus

/**
Caches data arrays on a tile-by-tile basis that may be reused.

Intended to cache map arrays like MrSID texture for tiles that may be reused as user moves around the map and reaccesses the same tiles.
A grid of tiles (typically about 128 pixels in size) is formed using map extents and starting from the upper-left corner of the map.
Data can be stored for each of the tiles.
It is assumed that there is sufficient cache space to at least cover the number of tiles of each type that are displayed at one time.
Typically, there are memory constraints and when they are reached,
the new tile is always stored and an older tile currently not in the display area is deleted to make room.\n
\n
The class is thread safe.
Status variables are protected by a mutex so that only a single user can read/write them.
The storage area is protected while writing by setting the stat array for that tile to 2 before starting to write and setting it to 1 on completion.
That stat array is then checked that it is 1 before reading.\n
It is assumed that a tile will only be written once -- that the class will be rebooted whenever data for tile changes.
The storage area is protected for later reading since it will never be deleted until it out of the display area.
\n
The class is automatically initialized to cache more tiles than are in the currently displayed area.
This means that any tile that requests to be cached will be and that space can always be found;
that a tile not in the display area can always be deleted to make room if the class runs out of space.
It also means that the user can count on tiles within the display area not being deleted before they are used
*/
class cache_class:public base_jfd_class{
 private:
	int nx;					// No of tiles and stored arrays along x
	int ny;					// No of tiles and stored arrays along y
	int na;					// No of stored arrays
	int nchar;				// Size of each storage array in char
	int nstore_max;			// Max no. of tiles to be stored
	int nstore_cur;			// No of tiles currently stored
	unsigned char* stat;	// Array of status flags per tile
							// 0=not currently stored
							// 1=stored good data and not being accessed
							// 2=currently being written
							// 3=currently being read
	unsigned char** cstore;	// Array of na=nx*ny storage arrays per tile, each of size nchar
	int* full_index;		// Indices of tiles filled in order of being filled
	int res_type;			// Resolution of stored data -- 1=hi, 2=med, 3=low
	char diag_name[100];	// Short name for diagnostics only
	
	double map_w, map_n;	// Origin of map in UTM coords
	float dx, dy;			// Size of tile in m in x and y
	int display_dist;

	SbMutex mutex_stat;				// Thread safe arrays stat and full_index and int nstore_cur

	map3d_index_class *map3d_index;	// Helper class -- gets parms
	tiles_rtv_class *tiles_rtv;		// Helper class -- gets parms

	// Private methods
	int delete_buffer_if_necessary();

 public:
	cache_class();
	~cache_class();
   
	int init(map3d_index_class *map3d_index_in, int array_size, int res_type_in, int ntiles_cached_pad, const char *diag_name_in);
	int query(double x, double y);
	unsigned char* get_ptr_for_write(double x, double y);
	int mark_write_complete(double x, double y);
	unsigned char* get_ptr_for_read(double x, double y);
	int mark_stored(double x, double y, int val);
	int copy_to_cache(double x, double y, unsigned char *data);
	int copy_from_cache(double x, double y, unsigned char *data);
	int force_new_store();
	int reset_all();
   
};

#endif /* __cplusplus */
#endif /* _cache_class_h_ */
