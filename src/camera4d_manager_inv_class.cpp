#include "internals.h"

// **********************************************
/// Constructor.
// **********************************************

camera4d_manager_inv_class::camera4d_manager_inv_class(int n_data_max_in)
	:camera4d_manager_class(n_data_max_in)
{
   elev_warning_flag = 0;	// No of warnings issued
   myViewer = NULL;
}

// **********************************************
/// Destructor.
// **********************************************
camera4d_manager_inv_class::~camera4d_manager_inv_class()
{
}

// ********************************************************************************
/// Get a pointer to the perspective camera.
// ********************************************************************************
SoPerspectiveCamera* camera4d_manager_inv_class::get_pers_camera()
{
   return camera_pers;
}

// ********************************************************************************
/// Get a pointer to the viewer.
// ********************************************************************************
fusion3d_viewer_class* camera4d_manager_inv_class::get_viewer()
{
   return myViewer;
}

// **********************************************
/// Register viewpoint transformation.
// **********************************************
int camera4d_manager_inv_class::register_xform(SoTransform *sb_xform_in)
{
   sb_xform = sb_xform_in;
   return(1);
}

// **********************************************
/// Register root and make camera.
// **********************************************
int camera4d_manager_inv_class::register_inv(SoSeparator *root_in)
{
	root = root_in;

	cg_world_y = 0.;
	cg_world_x = 0.;

	// *****************************************
	// Global variables
	// *****************************************
	GL_aimpoint_flag = (SoSFInt32*)SoDB::getGlobalField("New-Aimpoint");
	GL_camera_az = (SoSFFloat*)SoDB::getGlobalField("Camera-Az");
	GL_camera_el = (SoSFFloat*)SoDB::getGlobalField("Camera-El");
	GL_camera_zoom = (SoSFFloat*)SoDB::getGlobalField("Camera-Zoom");
	GL_nav_elmax = (SoSFInt32*)SoDB::getGlobalField("Nav-El-Max");
	GL_stereo_on = (SoSFInt32*)SoDB::getGlobalField("Stereo-On");
	GL_stereo_sep = (SoSFFloat*)SoDB::getGlobalField("Stereo-Sep");

	aimpointSensor = new SoFieldSensor(aim_cbx, this);
	aimpointSensor->attach(GL_aimpoint_flag);
	azSensor = new SoFieldSensor(az_cbx, this);
	azSensor->attach(GL_camera_az);
	elSensor = new SoFieldSensor(el_cbx, this);
	elSensor->attach(GL_camera_el);
	zoomSensor = new SoFieldSensor(zoom_cbx, this);
	zoomSensor->attach(GL_camera_zoom);
	stereoSepSensor = new SoFieldSensor(stereo_sep_cbx, this);
	stereoSepSensor->attach(GL_stereo_sep);

	aimpointSensor->setPriority(50);   // When move with left mouse, make sure camera is updated first

	// *****************************************
	// Define camera and camera transformations
	// *****************************************
	camera_pers = new SoPerspectiveCamera;
	camera_pers->ref();
	root->addChild(camera_pers);

	// Swapped to make Az then El 10 June 03 for RTV
	// Makes script Set-El then Anim-Az work

	camera_elr = new SoRotationXYZ;
	root->addChild(camera_elr);
	camera_elr->axis = SoRotationXYZ::X;
	camera_elr->angle = -3.1415927f * cg_world_el / 180.f;

	camera_azr = new SoRotationXYZ;
	root->addChild(camera_azr);
	camera_azr->axis = SoRotationXYZ::Z;
	camera_azr->angle = 3.1415927f * cg_world_az / 180.f;

	camera_roll = new SoRotationXYZ;
	root->addChild(camera_roll);
	camera_roll->axis = SoRotationXYZ::Y;
	camera_roll->angle = -3.1415927f * cg_world_roll / 180.f;

	flyTranslation = new SoTranslation;
	root->addChild(flyTranslation);
	flyTranslation->translation.setValue(0., 0., 0.);

	// *****************************************
   // Init camera location
   // *****************************************
	init_camera();
   return(1);
}

// **********************************************
/// Initialize the 3-d map -- does the portion that is map-specific and may be called multiple times.
/// If no map is currently defined, this method returns immediately, doing nothing.
// **********************************************
int camera4d_manager_inv_class::make_scene_3d()
{
	init_camera();
	gps_calc->set_aim_point_rel(0., 0.);							// Set aim point to reference point
	if (if_elmax_user) GL_nav_elmax->setValue((int)camera_el_max);	// User has overridden max el angle -- transfer to OIV Global
	return(1);
}

// ********************************************************************************
/// Set the scene size -- camera will be set up to view a scene of this size.
/// Size should be set a little larger than the object to be viewed.
/// @param size -- size of scene.
// ********************************************************************************
int camera4d_manager_inv_class::init_camera()
{
	// *****************************************
	// Init coord transformations to put aim point at origin
	// *****************************************
	flyTranslation->translation.setValue(0., 0., 0.);

	// *****************************************
	// Init camera location
	// *****************************************
	x_cam = 0.0;
	y_cam = 0.0;
	z_cam_pers = init_scene_size / height_angle;
	SoSFVec3f camera_position;
	camera_position.setValue(x_cam, y_cam, z_cam_pers);
	camera_pers->position = camera_position;

	// *****************************************
	// Init camera frustrum
	// *****************************************
	SbVec3f camera_points_at = SbVec3f(0., 0., 0.);	// Aim point translated to origin, so point at origin
	camera_pers->pointAt(camera_points_at);

	camera_pers->focalDistance = z_cam_pers;			// Set focal length -- not included in pointAt method
	camera_pers->heightAngle = height_angle;

	// *****************************************
	// Init camera clipping planes
	// *****************************************
	camera_pers->nearDistance = clip_near_pers;
	camera_pers->farDistance = clip_far_pers;
	return(1);
}

// **********************************************
/// Clear all when new ref point defined.
// **********************************************
int camera4d_manager_inv_class::clear_all()
{
	// Reset translations to camera position so it points at the ref point again
	x_aim = 0.;
	y_aim = 0.;
	z_aim = 0.;
	cg_world_x = 0.;
	cg_world_y = 0.;
	cg_world_z = 0.;
	init_camera();
	return(1);
}
// **********************************************
/// Adjust camera zoom.
/// Zoom is adjusted by changing the angular field of view -- like a camera zoom rather than dolly (moving camera closer or farther).
/// The alternate method of camera zoom is like camera dolly in that it adjusts camera distance from the aim point.
/// This method does not need to adjust stereo separation like the other method.
/// This method shows less perspective (more like an orthographic camera) as you zoom in.
/// @param	camera_height	Half-height of the field of view on the ground.
// **********************************************
int camera4d_manager_inv_class::set_zoom_factor(float camera_height)
{
	if (camera_height_old == camera_height) return(1);

	SoSFVec3f postt = camera_pers->position;
	SbVec3f posttt = postt.getValue();
	float xt, yt, zt, ha;
	posttt.getValue(xt, yt, zt);
	ha = atan2(camera_height, zt);
	// had = 180. * ha / 3.14159;
	camera_pers->heightAngle = ha;
	camera_height_old = camera_height;
	return(1);
}

// **********************************************
// 
// **********************************************
int camera4d_manager_inv_class::get_zoom_factor(float &camera_height)
{
	float camera_angle = camera_pers->heightAngle.getValue();
	float camera_focald = camera_pers->focalDistance.getValue();
	camera_height = camera_angle * camera_focald;
	return(1);
}

// **********************************************
// 
// **********************************************
int camera4d_manager_inv_class::set_focal_length(float focal_length)
{
	SoSFFloat focald;
	focald = focal_length;
	camera_pers->focalDistance = focald;
	return(1);
}

// **********************************************
/// Get the camera focal length. 
// **********************************************
int camera4d_manager_inv_class::get_focal_length(float &focal_length)
{
	SoSFFloat focald;
	focald = camera_pers->focalDistance;
	focal_length = focald.getValue();
	return(1);
}

// **********************************************
/// Refresh the camera.
// **********************************************
int camera4d_manager_inv_class::refresh()
{
   SoSFFloat sheight, focald;
   float dist, camera_angle, xtt, ytt, ztt, camera_focald;
   SbVec3f camera_posv;
   double pick_north, pick_east;
   int if_find_model=0;
   
   
   // **********************************************
   // RESET
   // **********************************************
   if (check_count(49)) {
      // xxx myViewer->resetToHomePosition();
      cg_world_az = 0.;
      cg_world_el = 0.;
      camera_azr->angle = 0.;
      camera_elr->angle = 0.;
      camera_roll->angle = 0.;
      x_aim = 0.;
      y_aim = 0.;
      z_aim = 0.;
	  cg_world_x = 0.;
	  cg_world_y = 0.;
	  cg_world_z = 0.;
	  flyTranslation->translation.setValue(x_aim, y_aim, z_aim);
      return(1);
   }
   
   // **********************************************
   // North Up
   // **********************************************
   if (check_count(48)) {
      cg_world_az = 0.;
      camera_azr->angle = 0.;
      return(1);
   }
   
   // **********************************************
   // 
   // **********************************************
   // If in target mode, want to manipulate target so do nothing
   check_mode_change();
   if (mode_current == 2) return(1);

   refresh_pending = 0;
   
   // Decrease elevation
   if (check_count(50)) {
      if (cg_world_el >= 70.) {
         if (!elev_warning_flag) {
	    cout << "WARNING -- Maps are best viewed and navigated at higher elevations" << endl;
	    elev_warning_flag++;
	 }
	 cg_world_el = cg_world_el + 2.5f;		// Smaller steps near ground level
      }
      else {
         cg_world_el = cg_world_el + 10.f;
      }
      // if (cg_world_el >= 90.) cg_world_el = 90.;	// Dont allow camera below ground level
      if (cg_world_el >= 85.f) cg_world_el = 85.f;	// Dont allow camera TOO CLOSE TO ground level
      cout << "To set el " << cg_world_el << endl;
      camera_elr->angle = - 3.1415927f * cg_world_el / 180.0f;
      refresh_pending = 1;
   }
   
   // Increase elevation
   if (check_count(51)) {
      if (cg_world_el > 80.) {
         cg_world_el = cg_world_el - 2.5f;		// Smaller steps near ground level
      }
      else {
         cg_world_el = cg_world_el - 10.f;
      }
      if (cg_world_el <= 0.) cg_world_el = 0.f;	// Dont allow camera to look behind you
      cout << "To set el " << cg_world_el << endl;
      camera_elr->angle = - 3.1415927f * cg_world_el / 180.f;
      refresh_pending = 1;
   }
   
   // Reset elevation
   if (check_count(2) || check_count(12) ) {
      cout << "To reset elevation" << endl;
      cg_world_el = 0.;
      camera_elr->angle = - 3.1415927f * cg_world_el / 180.f;
      refresh_pending = 1;
   }
   
   // Increase azimuth
   if (check_count(52)) {
      cg_world_az = cg_world_az + 2.5f;
      cout << "To set az " << cg_world_az << endl;
      camera_azr->angle = 3.1415927f * cg_world_az / 180.f;
      refresh_pending = 1;
   }
   
   // Decrease azimuth
   if (check_count(53)) {
      cg_world_az = cg_world_az - 2.5f;
      cout << "To set az " << cg_world_az << endl;
      camera_azr->angle = 3.1415927f * cg_world_az / 180.f;
      refresh_pending = 1;
   }
   
   // Zoom in
   if (check_count(5) || check_count(15)) {
	   camera_angle = camera_pers->heightAngle.getValue();
	   // camera_angle = 0.95 * camera_angle;
	   camera_angle = camera_angle - 4.f * 3.14159f / 180.f;
	   camera_pers->heightAngle = camera_angle;
	   refresh_pending = 1;
   }

   // Zoom out
   if (check_count(19)) {
	   camera_angle = camera_pers->heightAngle.getValue();
	   // camera_angle = camera_angle / 0.95;
	   camera_angle = camera_angle + 4.f * 3.14159f / 180.f;
	   camera_pers->heightAngle = camera_angle;
	   cout << "Camera angle " << 180.f * camera_angle / 3.14159f << endl;
	   refresh_pending = 1;
   }
   
   // Decrease focal length
   if (check_count(7) || check_count(17) || check_count(37)) {
	   dist = camera_pers->focalDistance.getValue();
	   dist = 0.9f * dist;
	   camera_pers->focalDistance = dist;
	   cout << "To decrease focal length to  " << dist << endl;
	   refresh_pending = 1;
   }

   // Increase focal length
   if (check_count(6) || check_count(16) || check_count(36)) {
	   dist = camera_pers->focalDistance.getValue();
	   dist = dist / 0.9f;
	   camera_pers->focalDistance = dist;
	   cout << "To increase focal length to  " << dist << endl;
	   refresh_pending = 1;
   }

   // Translation and azimuth rotation
   if (check_count(8)) {

      // Rotation
      SbVec3f rotation_axis_new;
      float rotation_rad_new;
      sb_xform->rotation.getValue(rotation_axis_new, rotation_rad_new);
      // cout << "Rot axis 0 " << rotation_axis_new[0] << " 1 " << rotation_axis_new[1] << " 2 " << rotation_axis_new[2] << endl;
      float rot_dir = 0.;
      
      // If rotation is mostly azimuth, do azimuth only
      if (fabs(rotation_axis_new[1]) > fabs(rotation_axis_new[0])) {
         if (rotation_axis_new[1] < -0.8f && rotation_rad_new > 0.5f) rot_dir =   1.f;
         if (rotation_axis_new[1] >  0.8f && rotation_rad_new > 0.5f) rot_dir = - 1.f;
         if (rot_dir != 0.) {
            cg_world_az = cg_world_az - 1.0f * rot_dir * rotation_rad_new;
	    cout << "Rot angle " << rotation_rad_new << " camera az " << cg_world_az << " rot_dir " << rot_dir << endl;
            camera_azr->angle = 3.1415927f * cg_world_az / 180.0f;
	 }
      }
      
      // If rotation is mostly elevation, do elevation only
      else {
         if (rotation_axis_new[0] < -0.8f && rotation_rad_new > 0.5f) rot_dir =   1.f;
         if (rotation_axis_new[0] >  0.8f && rotation_rad_new > 0.5f) rot_dir = - 1.f;
         if (rot_dir != 0.) {
            cg_world_el = cg_world_el + 1.0f * rot_dir * rotation_rad_new;
            if (cg_world_el >= 90.f) cg_world_el = 90.f;	// Dont allow camera below ground level
            if (cg_world_el <= 0.f) cg_world_el = 0.f;	// Dont allow camera to look behind you
            camera_elr->angle = - 3.1415927f * cg_world_el / 180.0f;
	 }
      }
      
      // Translation -- only if no rotation
         if (rot_dir == 0.) {
         SbVec3f translate_current;
         translate_current = sb_xform->translation.getValue();
         float x_trans, y_trans, z_trans;
         translate_current.getValue(x_trans, y_trans, z_trans);
         // cout << "Camera Translate x " << x_trans << " y " << y_trans << " z " << z_trans << endl;
      
		 // Zoom using push down / pull up on spaceball
		 if (fabs(y_trans) > fabs(x_trans) && fabs(y_trans) > fabs(z_trans)) {
			 camera_focald = camera_pers->focalDistance.getValue();
			 camera_focald = camera_focald + .005f * camera_focald * y_trans;
			 camera_pers->focalDistance = camera_focald;
			 camera_posv = camera_pers->position.getValue();
			 camera_posv.getValue(xtt, ytt, ztt);
			 ztt = ztt + .005f * ztt * y_trans;
			 camera_pers->position.setValue(xtt, ytt, ztt);
			 refresh_pending = 1;
		 }

		 // Translate horizontally only in search mode
		 else if (mode_current == 0) {
			 // Must rotate translations into East-North coordinates using azimuth angle
			 float sinaz = sin(3.1415927f * cg_world_az / 180.f);
			 float cosaz = cos(3.1415927f * cg_world_az / 180.f);
			 float a1 = x_trans * cosaz - z_trans * sinaz;
			 float a2 = x_trans * sinaz + z_trans * cosaz;

			 float camera_angle = camera_pers->heightAngle.getValue();
			 float camera_focald = camera_pers->focalDistance.getValue();
			 float fov_height = camera_angle * camera_focald;

			 x_aim = x_aim + 0.01f * fov_height * a1;
			 y_aim = y_aim - 0.01f * fov_height * a2;
			 flyTranslation->translation.setValue(-x_aim, -y_aim, -z_aim);
			 // cout << "    fly east " << x_aim << " fly north " << y_aim << endl;
			 refresh_pending = 1;
		 }
	  }
   }

   // Translate camera
   if (if_translated) {
      pick_north = gps_calc->get_ref_utm_north() + cg_world_y;
      pick_east  = gps_calc->get_ref_utm_east()  + cg_world_x;
	  set_loc(pick_north, pick_east);
   }
   
   // Rotate camera if 
   if (if_rotated) {
      // cout << "Camera rotated to az " << cg_world_az << " el " << cg_world_el << endl;
      camera_azr->angle  =   3.1415927f * cg_world_az / 180.f;
      camera_elr->angle  = - 3.1415927f * cg_world_el / 180.f;
      camera_roll->angle =   3.1415927f * cg_world_roll / 180.f;
      refresh_pending = 1;
      if_rotated = 0;
   }
   
   // Transfer info from icon, if a new one has been input
   process_icon();
   
   if (!refresh_pending) {
      return(1);
   }
   
   
   return(1);
}
// **********************************************
/// Process icon -- Private.
// **********************************************
int camera4d_manager_inv_class::process_icon()
{
   if (!if_new_icon) return(1);		// No new icon, do nothing
   
   if (icon == NULL) return(1);		// Dont change camera if click off any icons
   
   // *******************************
   // Translate camera
   // *******************************
   cg_world_x = icon->east;
   cg_world_y = icon->north;
   cg_world_z = icon->height;
   
   if (icon->camera_adjust) {
      cg_world_x = cg_world_x + icon->camera_deast;
      cg_world_y = cg_world_y + icon->camera_dnorth;
      cg_world_z = cg_world_z + icon->camera_dheight;
   }
   
   cout << "Camera moved to icon east " << cg_world_x << " north " << cg_world_y << " ht " << cg_world_z << endl;
   flyTranslation->translation.setValue(-cg_world_x, -cg_world_y, -cg_world_z);
   x_aim = cg_world_x;
   y_aim = cg_world_y;
   z_aim = cg_world_z;
   cout << "Camera moved to icon east " << cg_world_x << " north " << cg_world_y << " ht " << cg_world_z << endl;
   flyTranslation->translation.setValue(-cg_world_x, -cg_world_y, -cg_world_z);
   
   // *******************************
   // Rotate camera
   // *******************************
   if (icon->camera_adjust) {
      cg_world_az = icon->camera_az;
      cg_world_el =icon->camera_el;
      cout << "Camera rotated to az " << cg_world_az << " el " << cg_world_el << endl;
      camera_azr->angle  = 3.1415927f * cg_world_az / 180.f;
      camera_elr->angle  = - 3.1415927f * cg_world_el / 180.f;
      camera_roll->angle = 3.1415927f * cg_world_roll / 180.f;
   }
   refresh_pending = 1;
   if_new_icon = 0;
   return(1);
}

// **********************************************
/// Set the camera view including the aim point, azimuth and elevation angles, and the zoom factor.
/// @param	north	Northing of the desired aim point in m.
/// @param	east	Easting of the desired aim point in m.
/// @param	az		Azimuth angle in deg,<-1000000 to not set, leave at current value.
/// @param	el		Elevation angle in deg. 0 is straight down, <0 to not set, leave at current value.
/// @param	zoom	Zoom factor (see method set_zoom_factor).  Set <0 to not set, leave at current value.
// **********************************************
int camera4d_manager_inv_class::set_view(double north, double east, float az, float el, float zoom)
{
	int zoomFlag = 0;
	set_loc(north, east);
	if (az > -1000000.) {
		set_cg_az(az);
	}
	if (el >= 0.) {
		set_cg_el(el);
	}
	if (zoom >= 0.) {
		set_zoom_factor(zoom);
		zoomFlag = 1;
	}
	status_overlay->update_compass(zoomFlag);
	return(1);
}

// ********************************************************************************
/// Set the camera aim point.
/// If a map exists, the elevation of the aim point is calculated from the low-res map elevation data.
/// Otherwise, it is assumed to be 0 (the elevation of the data origin).
/// @param	north	Northing of the desired aim point in m.
/// @param	east	Easting of the desired aim point in m.
// ********************************************************************************
int camera4d_manager_inv_class::set_loc(double north, double east)
{
	cg_world_y = float(north - gps_calc->get_ref_utm_north());
	cg_world_x = float(east  - gps_calc->get_ref_utm_east());
	cg_world_z = map3d_lowres->get_lowres_elev_camera(north, east) - gps_calc->get_ref_elevation();

	  if (diag_flag > 0) {
	     cout << "Camera: To rel east " << cg_world_x << " north " << cg_world_y << " ht " << cg_world_z << endl;
         cout.precision(8);
         cout << "   UTM       E=" << east << " N=" << north << endl;
      
         double latt, lont;
         gps_calc->proj_to_ll(north, east, latt, lont);
         cout << "   verify Lat=" << latt << " long=" << lont << endl;

        // Convert to MGRS and print
         char GridLetters[10];
         gps_calc->UTM_to_GridLetters(north, east, GridLetters);
		 int utm_lon_zone_ref = gps_calc->get_utm_lon_zone_ref();
		 char utm_lat_zone_char_ref = gps_calc->UTMLetterDesignator(latt);
         int northi = int( north + 0.5f);
         int easti  = int( east + 0.5f);
         int north_mgrs = northi % 100000;
         int east_mgrs =  easti  % 100000;
         cout.width(5);
         cout.fill('0');
         cout << "   MGRS " << utm_lon_zone_ref << " " << utm_lat_zone_char_ref << " " << GridLetters << " ";
         cout.width(5);		// Applies only to next field ?
         cout.fill('0');
         cout << east_mgrs << "       ";
         cout.width(5);		// Applies only to next field ?
         cout.fill('0');
         cout << north_mgrs << endl;
	  }
      
	flyTranslation->translation.setValue(-cg_world_x, -cg_world_y, -cg_world_z);
	x_aim = cg_world_x;
	y_aim = cg_world_y;
	z_aim = cg_world_z;
	return(1);
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void camera4d_manager_inv_class::aim_cbx(void *userData, SoSensor *timer)
{
   camera4d_manager_inv_class* camt = (camera4d_manager_inv_class*)  userData;
   camt->aim_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void camera4d_manager_inv_class::aim_cb()
{
   cg_world_y = float(gps_calc->get_aim_rel_north());
   cg_world_x = float(gps_calc->get_aim_rel_east());
   int ipick = GL_aimpoint_flag->getValue();
   if_translated = 1;
   refresh();
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void camera4d_manager_inv_class::az_cbx(void *userData, SoSensor *timer)
{
   camera4d_manager_inv_class* camt = (camera4d_manager_inv_class*)  userData;
   camt->az_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void camera4d_manager_inv_class::az_cb()
{
	float daz = GL_camera_az->getValue();	// Value is delta
	cg_world_az = cg_world_az + daz;
	if_rotated = 1;
	refresh();
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void camera4d_manager_inv_class::el_cbx(void *userData, SoSensor *timer)
{
   camera4d_manager_inv_class* camt = (camera4d_manager_inv_class*)  userData;
   camt->el_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void camera4d_manager_inv_class::el_cb()
{
   float del = GL_camera_el->getValue();	// Value is delta
   cg_world_el = cg_world_el + del;
   float camera_el_max_loc = float(GL_nav_elmax->getValue());
   if (cg_world_el < camera_el_min    ) cg_world_el = camera_el_min;		// Dont allow camera to look behind you
   if (cg_world_el > camera_el_max_loc) cg_world_el = camera_el_max_loc;	// Dont allow camera TOO CLOSE TO ground level
   if_rotated = 1;
   refresh();
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void camera4d_manager_inv_class::zoom_cbx(void *userData, SoSensor *timer)
{
	camera4d_manager_inv_class* camt = (camera4d_manager_inv_class*)userData;
	camt->zoom_cb();
}

// ********************************************************************************
/// Adjust camera zoom by a percentage of the current camera distance (so the apparent rate is the same independent of scale) -- Actual callback.
/// Zoom is adjusted like camera dolly in that it adjusts camera distance from the aim point.
/// The alternate method of camera zoom is by changing the angular field of view -- like a camera zoom rather than dolly (moving camera closer or farther).
/// This method needs to adjust stereo separation since it moves the camera.
/// This method shows the same amount of perspective as you zoom in and out (unlike the alternate method).
// 
// ********************************************************************************
void camera4d_manager_inv_class::zoom_cb()
{
	float zoom, delZoom;
	zoom = GL_camera_zoom->getValue();
	delZoom = 0.1f * zoom;
	// cout << "To camera4d_manager_inv_class::zoom_cb" << delZoom << endl;

	// Code lifted from SoXtExaminerViewer
	// shorter/grow the focal distance given the wheel rotation
	float focalDistance = camera_pers->focalDistance.getValue();;
	float newFocalDist = focalDistance;
	newFocalDist *= powf(2.0, delZoom);

	// finally reposition the camera
	SbMatrix mx;
	mx = camera_pers->orientation.getValue();
	SbVec3f forward(-mx[2][0], -mx[2][1], -mx[2][2]);
	camera_pers->position = camera_pers->position.getValue() + (focalDistance - newFocalDist) * forward;
	camera_pers->focalDistance = newFocalDist;

	// *****************************************
	// Both Coin3D and vsg/fei/thermo use distance to set the stereo separation.
	//	This is 'real' like a human but results in virtually no stereo zoomed out and extreme stereo zoomed far in
	//	We would like to have comfortable stereo at all zooms so 
	//	You need to scale the stereo-offset with zoom -- converting from distance to angle and keeping a constant anngle
	// *****************************************
	if (GL_stereo_on->getValue() != 0) {
		float stereo_sep_angle = GL_stereo_sep->getValue();
		float stereo_sep_tan = tan(3.1415927f * stereo_sep_angle / 180.0f);
		float stereo_dist = newFocalDist * stereo_sep_tan;
		myViewer->setStereoOffset(stereo_dist);
		camera_height_old = -9999.;		// Other zoom method must be notified that you have changed zoom with this method
	}
	refresh();
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void camera4d_manager_inv_class::stereo_sep_cbx(void *userData, SoSensor *timer)
{
	camera4d_manager_inv_class* camt = (camera4d_manager_inv_class*)userData;
	camt->stereo_sep_cb();
}

// ********************************************************************************
/// Both Coin3D and vsg/fei/thermo use distance to set the stereo separation.
///	This is 'real' like a human but results in virtually no stereo zoomed out and extreme stereo zoomed far in
///	We would like to have comfortable stereo at all zooms so 
///	You need to scale the stereo-offset with zoom -- converting from distance to angle and keeping a constant anngle
// ********************************************************************************
void camera4d_manager_inv_class::stereo_sep_cb()
{
	float focalDistance;

	// Code lifted from SoXtExaminerViewer
	focalDistance = camera_pers->focalDistance.getValue();
	// *****************************************
	// *****************************************
	if (GL_stereo_on->getValue() != 0) {
		float stereo_sep_angle = GL_stereo_sep->getValue();
		float stereo_sep_tan = tan(3.1415927f * stereo_sep_angle / 180.0f);
		float stereo_dist = focalDistance * stereo_sep_tan;
		myViewer->setStereoOffset(stereo_dist);
		cout << "Set camera offset distance to " << stereo_dist << endl;
	}
}

