#ifndef _map3d_lowres_class_h_
#define _map3d_lowres_class_h_
#ifdef __cplusplus

class map3d_index_class;

/**
Implements lowres map calculations.

Estimates the map elevations for camera movement and approximate location of points on the ground 
using a lowres elevation image derived from DEMs as a first option
and a lowres elevation image derived from point clouds as a second option.\n

Lowres elevations from DEMs:  Making lowres image done in map3d_index_class (needs lots of methods from this class)
and then passed to this class. \n

Lowres elevations from PCs:  Making lowres image done in this class using values passed from ladar_mm_manager_inv class
as it is rendering the point cloud.\n

Lowres texture from map tiles:  This is the default method and is done outside this class.
It uses the same procedures that are used to make higher-res textures.\n

Lowres texture from mosaic:  The mosaic is made offline using utitity package 'gridder' and saved to a file.
If the file is available, then it read within this class and low-res tiles are made within this class.
Implements faster low-res tile rendering when there is a precomputed downsampled texture mosaic available.
The class reads the texture mosaic on initialization then uses the texture data for all tiles as they are constructed.
This bypasses the default procedure of reading texture data separately for each tile, then clipping it, then downsampling it.
This method is a lot faster even for smaller maps and absolutely necessary for very large maps where making low-res tiles
the default way can take upwards of an hour.


*/
class map3d_lowres_class:public base_jfd_class{
 private:
	 double east_tex;			///< Texture -- lowres mosaic file map bounds
	 double west_tex;			///< Texture -- lowres mosaic file map bounds
	 double north_tex;			///< Texture -- lowres mosaic file map bounds
	 double south_tex;			///< Texture -- lowres mosaic file map bounds
	 double cenn_tex;			///< Texture -- lowres mosaic file map center
	 double cene_tex;			///< Texture -- lowres mosaic file map center
	 int nx_tex;				///< Texture -- no. of pixels in mosaic image
	 int ny_tex;				///< Texture -- no. of pixels in mosaic image
	 float dx_tex;				///< Texture -- pixel size
	 float dy_tex;				///< Texture -- pixel size
	 int data_type_tex;			///< Texture -- Data type -- 5=gray-scale (1 byte per pixel), 6=color (3 bytes per pixel)
	 float brt0_tex;			///< Texture -- Min display brightness
	 float lowres_texture_complexity;///< Texture --  Lowres texture complexity for texture mosaic (0.1 for pixelated to 1.0 for default)
	 int ibrt_offmap_tex;		///< Texture -- Brightness level for off-map pixels

	 int elev_dem_flag;				///< lowres DEM elev -- 0 for no elevations defined, 1 for DEM elevations 
	 float *elev_dem;               ///< lowres DEM elev -- pointer to elevation array
	 double west_elev_dem;			///< Lowres DEM elev -- file map bounds
	 double north_elev_dem;			///< Lowres DEM elev -- file map bounds
	 int nx_elev_dem;				///< Lowres DEM elev -- no. of pixels in mosaic image
	 int ny_elev_dem;				///< Lowres DEM elev -- no. of pixels in mosaic image
	 float dx_elev_dem;				///< Lowres DEM elev -- pixel size
	 float dy_elev_dem;				///< Lowres DEM elev -- pixel size

	 int elev_pc_flag;				///< lowres PC elev -- 0 for no elevations defined, 1 for DEM elevations 
	 float *elev_pc;				///< lowres PC elev -- elevation array
	 int *elev_pc_n;				///< Lowres PC elev -- working -- no. of pts adding into each member of elev array			
	 double west_elev_pc;			///< Lowres PC elev -- file map bounds
	 double north_elev_pc;			///< Lowres PC elev -- file map bounds
	 int nx_elev_pc;				///< Lowres PC elev -- no. of pixels in mosaic image
	 int ny_elev_pc;				///< Lowres PC elev -- no. of pixels in mosaic image
	 float dx_elev_pc;				///< Lowres PC elev -- pixel size
	 float dy_elev_pc;				///< Lowres PC elev -- pixel size
	 string filename_elev_pc;		///< Lowres PC elev -- filename to store and retrived lowres image

	 unsigned char *data_tex;	///< Work -- Array of entire texture image from file
	 unsigned char *data_tile;	///< Work -- Subarray of texture data for an individual tile
	 float(*coords_lod)[3];		///< Work -- Vertex Coords for individual tile
	 float(*coords_lod_tex)[2];	///< Work -- Texture Coords for individual tile

	dir_class *dir;						///< Helper class -- for map filenames
	mask_server_class*	mask_server;	///< Helper class -- to overlay masks like LOS

	// Private methods
	float get_lowres_elev_at_loc_dem(double north, double east);
	float get_lowres_elev_at_loc_pc(double north, double east);
	float get_lowres_elev_camera_dem(double north, double east);
	float get_lowres_elev_camera_pc(double north, double east);
	int read_elev_dem(string filename);
public:
	map3d_lowres_class();
	~map3d_lowres_class();
   
	int read_tagged(const char* filename);
	int write_parms(FILE *out_fd) override;
	int clear_all() override;

	int register_mask_server(mask_server_class*	mask_server_in);
	int register_dir(dir_class *dirin);
	int register_elev_dem(double north, double west, float dx, float dy, int nx, int ny, float* elev);
	int register_elev_pc (double north, double south, double east, double west);
	int register_tex_dem (double north, double west, float dx, float dy, int nx, int ny, float* tex, int ibrt_ofmap);
	int make_elev_dem();
	int make_elev_pc(float(*coords)[3], int ncoords);
	int finish_elev_pc();
	int make_tex_mosaic();

	int get_nx_tex();
	int get_ny_tex();
	float get_resx_tex();
	float get_resy_tex();
	float get_lowres_elev_at_loc(double north, double east);
	float get_lowres_elev_camera(double north, double east);
	float get_elev_lowres_post(double north, double east);
	int get_texture(double north_cen, double east_cen, float height_tile, float width_tile, int &nx_out, int &ny_out, unsigned char *data);
	int make_newtile_low_from_mosaic(double north_cen, double east_cen, float height_tile, float width_tile, SoSeparator *loBase);
   
};

#endif /* __cplusplus */
#endif /* _map3d_lowres_class_h_ */
