#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
cache_class::cache_class()
	:base_jfd_class()
{
	nx = 0;
	ny = 0;
	na = 0;
	nchar = 0;
	nstore_cur = 0;

	stat = NULL;
	cstore = NULL;
	full_index = NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
cache_class::~cache_class()
{
	reset_all();
}

// ********************************************************************************
/// Reset class.
// ********************************************************************************
int cache_class::reset_all()
{
	for (int i=0; i<na; i++) {
		if (cstore[i] != NULL) delete[] cstore[i];
	}
	delete[] stat;
	delete[] cstore;
	delete[] full_index;
	stat = NULL;
	cstore = NULL;
	full_index = NULL;

	nstore_cur = 0;
	return(1);
}

// ********************************************************************************
/// Force a new store for all tiles -- required if texture moded as with LOS.
/// Thread safe.
/// WARNING -- will cause memory leaks if you move before you finish the new store for the current location. 
// ********************************************************************************
int cache_class::force_new_store()
{
	mutex_stat.lock();							// Blocking operation
	memset(stat, 0, na*sizeof(unsigned char));
	nstore_cur = 0;
	mutex_stat.unlock();
	return(1);
}

// ********************************************************************************
/// Initialize storage.
/// Cache storage is allocated for each tile.  The method finds the size of the tile and the size of the map to find the no. of tiles that must be allocated.
/// It finds the size of the displayed area in tiles and adds the ntiles_cached_pad parameter to get the total no. of tiles to be cached.
/// @param map3d_index_in		Helper map3d_index_class to get size of tiles, size of map, size of display area
/// @param array_size			Size of data arrays to be cached		
/// @param res_type_in			1 = hi-res, 2=medium res, low-res not implemented		
/// @param ntiles_cached_pad	No. of tiles to be cached over and above the minimum no. required for the displayed area	
/// @param diag_name_in			Name or ID used for diagnostic print (eg. 'MrSID hi-res') when several caches active
// ********************************************************************************
int cache_class::init(map3d_index_class *map3d_index_in, int array_size, int res_type_in, int ntiles_cached_pad, const char *diag_name_in)
{
	int ntilesx;

	// ****************************************************
	// Find size of map and size of storage arrays and alloc local cache
	// *****************************************************
	map3d_index = map3d_index_in;
	tiles_rtv = map3d_index->get_tiles_rtv_class();
	
	// Init storage arrays and tiling
	nx = map3d_index->get_internal_tiles_nx();
	ny = map3d_index->get_internal_tiles_ny(); 
	na = nx * ny;
	nchar = array_size;

	dx = map3d_index->get_internal_tiles_dx();
	dy = map3d_index->get_internal_tiles_dy();
	map_w = map3d_index->map_w;
	map_n = map3d_index->map_n;

	stat	= new unsigned char[na];
	cstore	= new unsigned char*[na];
	for (int i=0; i<na; i++) {
		cstore[i] = NULL;
		stat[i] = 0;
	}

	// Calculate max no. of tiles you can store so you can delete older ones if necessary
	res_type = res_type_in;
	if (res_type == 1) {
		ntilesx = tiles_rtv->get_ntilesl_hi();
	}
	else {
		ntilesx = tiles_rtv->get_ntilesl_med();
	}
	display_dist = ntilesx / 2;
	nstore_max = ntilesx * ntilesx + ntiles_cached_pad;
	full_index = new int[nstore_max];

	strcpy(diag_name, diag_name_in);
	if (diag_flag > 0) fprintf(stdout, "%s:  Cache Ntiles=%d, Bytes per tile= %d\n", diag_name, na, nchar);
	return(1);
}

// ********************************************************************************
/// Query the status of storage for the tile centered at (x,y) -- Return -1=off map, 0=empty, 1=full, good data, 3=full, bad data (maybe no data for this tile).
/// Thread safe -- waits until writing the array is finished if necesssary.
/// @param x					Easting in m in UTM coordinates
/// @param y					Northing in m in UTM coordinates
/// @param check_remote_flag	1 if check remote cache to retrieve a tile, 0 not to check
// ********************************************************************************
int cache_class::query(double x, double y)
{
	int retval;
	float fx = (x - map_w) /  dx;
	float fy = (map_n - y) /  dy;
	if (fx < 0. || fy < 0. || fx >= float(nx) || fy >= float(ny)) {
		return -1;
	}
	int ix = int(fx);
	int iy = int(fy);
	int ip = iy * nx + ix;
	mutex_stat.lock();							// Blocking operation
	retval = stat[ip];
	mutex_stat.unlock();

	// Wont return until writes to this buffer are complete
	while (retval == 2) {
		cross_sleep(2);
		//cout << "cache_class read waiting for write to finish -- sleep " << endl;
		mutex_stat.lock();							// Blocking operation
		retval = stat[ip];
		mutex_stat.unlock();
	}
	return retval;
}

// ********************************************************************************
/// Get pointer to internal storage array to store data.
/// The storage array is indexed by the (Northing, Easting) which determines a (iy, ix) tile index.
/// Thread safe.
/// @param x	Easting in m in UTM coordinates
/// @param y	Northing in m in UTM coordinates
// ********************************************************************************
unsigned char* cache_class::get_ptr_for_write(double x, double y)
{
	mutex_stat.lock();				// Blocking operation
	delete_buffer_if_necessary();	// If max memory has will be exceeded, you must first del one of the old buffers -- not thread safe
	int ix = (x - map_w) / dx;
	int iy = (map_n - y) / dy;
	int ip = iy * nx + ix;
	if (cstore[ip] == NULL) {
		cstore[ip] = new unsigned char[nchar];
	}
	full_index[nstore_cur] = ip;
	nstore_cur++;
	if (diag_flag > 0) {
		int memmb = nstore_cur * nchar / 1000000;
		fprintf(stdout, "%s:  Cache Ntiles=%d mem in MB=%d\n", diag_name, nstore_cur, memmb);
	}
	stat[ip] = 2;
	mutex_stat.unlock();
	return cstore[ip];
}

// ********************************************************************************
/// Get pointer to internal storage array to read data.
/// The storage array is indexed by the (Northing, Easting) which determines a (iy, ix) tile index.
/// @param x	Easting in m in UTM coordinates
/// @param y	Northing in m in UTM coordinates
// ********************************************************************************
unsigned char* cache_class::get_ptr_for_read(double x, double y)
{
	int ix = (x - map_w) / dx;
	int iy = (map_n - y) / dy;
	int ip = iy * nx + ix;
	return cstore[ip];
}

// ********************************************************************************
/// Mark the tile at the given location as having stored data. 
/// Thread safe.
/// @param x	Easting in m in UTM coordinates
/// @param y	Northing in m in UTM coordinates
/// @param val	1 to mark stored as good data, 3 to mark stored as bad data (usually because data not avilable for this tile)
// ********************************************************************************
int cache_class::mark_stored(double x, double y, int val)
{
	int ix = (x - map_w) / dx;
	int iy = (map_n - y) / dy;
	int ip = iy * nx + ix;
	mutex_stat.lock();							// Blocking operation
	stat[ip] = val;
	mutex_stat.unlock();
	return(1);
}

// ********************************************************************************
/// Mark that writing is complete for the tile at the given location. 
/// Thread safe.
/// @param x	Easting in m in UTM coordinates
/// @param y	Northing in m in UTM coordinates
/// @param val	1 to mark stored as good data, 3 to mark stored as bad data (usually because data not avilable for this tile)
// ********************************************************************************
int cache_class::mark_write_complete(double x, double y)
{
	int ix = (x - map_w) / dx;
	int iy = (map_n - y) / dy;
	int ip = iy * nx + ix;
	mutex_stat.lock();							// Blocking operation
	stat[ip] = 1;
	mutex_stat.unlock();
	return(1);
}


// ********************************************************************************
/// Copy data array to cache memory. 
/// Thread safe.
/// @param x	Easting in m in UTM coordinates
/// @param y	Northing in m in UTM coordinates
/// @param data	Array of data to be copied -- No. of character defined in method init will be copied
// ********************************************************************************
int cache_class::copy_to_cache(double x, double y, unsigned char *data)
{
	unsigned char *cptr = get_ptr_for_write(x,y);
	memcpy(cptr, data, nchar);
	mark_stored(x, y, 1);
	return(1);
}

// ********************************************************************************
/// Copy data array from cache memory. 
/// Thread safe.
/// @param x	Easting in m in UTM coordinates
/// @param y	Northing in m in UTM coordinates
/// @param data	Array of data to be copied -- No. of character defined in method init will be copied
// ********************************************************************************
int cache_class::copy_from_cache(double x, double y, unsigned char *data)
{
	unsigned char *cptr = get_ptr_for_write(x, y);
	memcpy(data, cptr, nchar);
	return(1);
}

// ********************************************************************************
/// If max number of buffers is exceeded, delete the first filled that is outside the display region -- private. 
/// Not thread safe.
// ********************************************************************************
int cache_class::delete_buffer_if_necessary()
{
	int iindex, ip, ix, iy, idel;
	if (nstore_max - nstore_cur > 0) return(1);

	int ix_cam = tiles_rtv->get_camera_ix();
	int iy_cam = tiles_rtv->get_camera_iy();
	for (iindex=0; iindex<nstore_max; iindex++) {	// FIFO
		ip = full_index[iindex];
		iy = ip / nx;
		ix = ip % nx;
		if (abs(iy-iy_cam) > display_dist || abs(ix-ix_cam) > display_dist) {
			delete[] cstore[ip];
			cstore[ip] = NULL;
			stat[ip] = 0;
			nstore_cur--;

			// Move the rest of full_index array down to erase the deleted tile
			for (idel=iindex+1; idel<nstore_max; idel++) {
				full_index[idel-1] = full_index[idel];
			}
			break;
		}
	}
	return(1);
}

