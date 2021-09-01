#ifndef _camera4d_manager_class_h_
#define _camera4d_manager_class_h_	
#ifdef __cplusplus

class status_overlay_manager_inv_class;

/**
Manages camera movement.

The camera manages the user view direction and field of view.\n
\n
The camera angles determine the user view direction.
The camera elevation angle is constrained.
The minimum angle is nadir, looking straight down, and the camera is not allowed to look backwards
(this 'upside-down' view is unnatural).
The maximum angle is controlled by a tag, but is defaulted to 85 deg.
This puts the user close to ground level but not quite.
Although a ground-level view is tempting, it causes severe stereo depth problems and does not
look good with typical lidar map data that is taken from above and doesnt capture vertical surfaces.

*/

class camera4d_manager_class:public atrlab_manager_class{
   protected:
      float x_aim, y_aim, z_aim;		///< Camera fields -- Aim point (origin of camera system)
      float x_cam, y_cam, z_cam_pers;	///< Camera fields -- Location of camera
      float height_angle;				///< Camera fields -- Pers -- vertical angle of camera vol
	  float init_scene_size;			///< Camera fields -- Initial scene size
      float clip_near_pers;				///< Camera fields -- 
      float clip_far_pers;				///< Camera fields -- 
      
	  int if_elmax_user;									///< Lims on camera elevation -- 1 iff user override of max el angle
	  float camera_el_min, camera_el_max;					///< Lims on camera elevation -- (default 0-85 deg for terrain)

	  status_overlay_manager_inv_class *status_overlay;		///< Update compass when camera changes

   public:
      camera4d_manager_class(int n_camera_max);
      ~camera4d_manager_class();
      
	  int register_status_overlay_manager(status_overlay_manager_inv_class *status_overlay_in);
	  int set_camera_el_lims(float elmin, float elmax);
	  int set_scene_size(float size);
      
      int get_aim_point(float &x_aim_out, float &y_aim_out, float &z_aim_out);
      
      int read_tagged(const char* filename);
      int write_parms(FILE *out_fd) override;
};

#endif /* __cplusplus */
#endif /* _camera4d_manager_class_h_ */
