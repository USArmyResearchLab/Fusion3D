#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
script_input_class::script_input_class()
	:base_jfd_class()
{
   GL_mousem_north  = (SoSFFloat*)  SoDB::getGlobalField("Mouse-Mid-North");
   GL_mousem_east   = (SoSFFloat*)  SoDB::getGlobalField("Mouse-Mid-East");
   GL_mousem_new    = (SoSFInt32*)  SoDB::getGlobalField("Mouse-Mid-New");
   GL_aimpoint_flag = (SoSFInt32*)  SoDB::getGlobalField("New-Aimpoint");
   GL_tracks_state  = (SoSFInt32*)  SoDB::getGlobalField("Tracks-State");
   GL_button_mod    = (SoSFInt32*)  SoDB::getGlobalField("Button-Mod");
   GL_fly_track     = (SoSFInt32*)  SoDB::getGlobalField("Fly-Track");
   GL_space_button  = (SoSFInt32*)  SoDB::getGlobalField("Space-Button");

   SoFieldSensor *trackSensor = new SoFieldSensor(track_cbx, this);
   trackSensor->attach(GL_tracks_state);

   SoFieldSensor *spaceNavSensor = new SoFieldSensor(space_nav_cbx, this);
   spaceNavSensor->attach(GL_space_button);

   n_max = 4000;
   distance_average_interval = 200.;
   snap_dirname = new char[400];
   strcpy(snap_dirname, "");
   clear_all();
   i_trackmanager = -99;		// Dont clear this -- just set once at program init
   atrlab_manager_a = NULL;
   camera_manager = NULL;
   map3d_index 		= NULL;
   list = new TODO_LIST[n_max];
   timer = new SoTimerSensor(timer_cbx, this);
   if (timer->isScheduled()) timer->unschedule();
   timer->setInterval(0.2); // Interval between events in s

}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
script_input_class::~script_input_class()
{
   delete timer;
   delete[] snap_dirname;
   delete[] list;
}

// ********************************************************************************
/// Register all the managers.
// ********************************************************************************
int script_input_class::register_managers(atrlab_manager_class** atrlab_manager_a_in, int n_managers_in)
{
   n_managers = n_managers_in;
   atrlab_manager_a = atrlab_manager_a_in;
   camera_manager = (camera4d_manager_inv_class*) atrlab_manager_a[0];
   for (int i=0; i<n_managers; i++) {
	   if (strcmp(atrlab_manager_a[i]->get_type(), "track") == 0) i_trackmanager = i;
   }
   return(1);
}
   
// **********************************************
/// Register a map index.
/// Pass a pointer to a map3d_index_class so this class can refer to the underlying 3D terrain map.
// **********************************************
int script_input_class::register_index(map3d_index_class *index)
{
   map3d_index = index;
   return(1);
}

// ********************************************************************************
/// Clear all.
// ********************************************************************************
int script_input_class::clear_all()
{
   script_defined_flag = 0;		// Default to no script
   if_freerun = 0;		// Default to stop/step
   if_menus   = 1;		// Default to display menus/buttons
   if_loc_defined = 0;	// Default to angle undefined
   if_az_defined = 0;	// Default to angle undefined
   if_el_defined = 0;	// Default to angle undefined
   if_zoom_defined = 0;	// Default to zoom undefined

   fly_route_defined_flag = 0;	// Default to route flythrough undefined
   if_fly_route_use_az = 0;
   stride_flyroute = 1;
   
   n_list = 0;
   i_serve = 0;
   serve_mode = 1;
   loop0 = 0;
   interval = -99.;
   interval_old = -99.;
   direction = 1;
   daz_max = 3000.0;
   north_current = -99.;
   east_current = -99.;
   az_current = -99.;
   el_current = -99.;
   zoom_current = -99.;

   snap_enabled_flag = 0;
   snap_seqno = 0;

   GL_fly_track->setValue(20 * 32);	// Default to averaging interval = 20 * scalingFactor=10 starting in bit 7
   return(1);
}
   
// ********************************************************************************
/// Return 1 if scripting enabled .
// ********************************************************************************
SoTimerSensor* script_input_class::get_timer()
{
   return timer;
}
   
// ********************************************************************************
/// Return 1 if scripting enabled .
// ********************************************************************************
int script_input_class::get_if_enabled()
{
   return script_defined_flag;
}
   
// ********************************************************************************
/// Return 1 if scripting is currently free-running.
// ********************************************************************************
int script_input_class::get_if_freerun()
{
   return if_freerun;
}
   
// ********************************************************************************
/// Return 1 if menus are turned on.
// ********************************************************************************
int script_input_class::get_if_menus()
{
   return if_menus;
}
   
// ********************************************************************************
/// Set to free-running mode. 
// ********************************************************************************
int script_input_class::set_freerun()
{
   if (!script_defined_flag && !fly_route_defined_flag) {
	   warning(1, "Neither a script nor a route/track flyover is defined -- do nothing");
	   return(0);
   }
   if (!if_freerun) timer->schedule();
   if (diag_flag > 0) cout << "script_input_class::set_freerun() interval " << timer->getInterval().getValue() << endl;
   if_freerun = 1;
   return(1);
}
   
// ********************************************************************************
/// Set to stop-step mode. 
// ********************************************************************************
int script_input_class::set_stop_step()
{
   if (!script_defined_flag && !fly_route_defined_flag) {
	   warning(1, "Neither a script nor a route/track flyover is defined -- do nothing");
	   return(0);
   }
   if (if_freerun) timer->unschedule();
   if_freerun = 0;
   return(1);
}
   
// ********************************************************************************
/// Private.
// ********************************************************************************
int script_input_class::get_new_interval(float &interval_out)
{
   if (interval <=0. || interval == interval_old) {
      return(0);
   }
   interval_old = interval;
   interval_out = interval;
   return(1);
}
   
// ********************************************************************************
/// Get the current operating mode -- 0 for stop/step, 1 for free run once in forward dir,2 for loop in forward dir, 3 for loop in both directions
// ********************************************************************************
int script_input_class::get_serve_mode()
{
   return serve_mode;
}
   
// **********************************************
/// Read commands from a file in tagged ascii format.
// **********************************************
int script_input_class::read_tagged(const char* filename)
{
     char tiff_tag[240], tiff_junk[240];
     FILE *tiff_fd;
     float a0, a1, dela, delt, az, el, roll, zoom, mouse_x, mouse_y, first_range, last_range, ax0, ax1, val, tauf;
     float focal_length, t0, t1, zoom0, zoom1, delzoom;
     int ntiff, n_tags_read = 1, vis_flag, i_ang, n_ang, n_skip, n0, n1, nsteps, end_flag=0, tau, tau0, tau1, n_tau, i_tau, flag;
     listt = list;

	 // ******************************
	 // Read-tagged from file
	 // ******************************
     if (!(tiff_fd= fopen(filename,"r"))) {
        cerr << "script_input_class::read_tagged:  unable to open input setup file " << filename << endl;
        return (0);
     }

     do {
       /* Read tag */
       ntiff = fscanf(tiff_fd,"%s",tiff_tag);
       n_tags_read += ntiff;


       /* If cant read any more (EOF), do nothing */
       if (ntiff != 1) { 
       }
       else if (strcmp(tiff_tag,"Script-Enabled") == 0) {
          script_defined_flag = 1;
       }
       else if (strcmp(tiff_tag,"Script-End") == 0) {
          end_flag = 1;
       }
	   else if (!script_defined_flag) {										// Ignore script entries before tag  Script-Enabled
	   }
	   else if (end_flag) {											// Ignore script entries after  tag  Script-End
	   }
       else if (strcmp(tiff_tag,"Script-Init-Freerun") == 0) {
          if_freerun = 1;
       }
       else if (strcmp(tiff_tag,"Script-Suppress-Menus") == 0) {
          if_menus = 0;
       }
       else if (strcmp(tiff_tag,"Anim-Flyby") == 0) {
		  if (!if_el_defined) exit_safe(1, "El angle must be defined in script before tag 'Anim-Flyby'");
          fscanf(tiff_fd,"%s %f %f %f %f %d %f", tiff_junk, &t0, &t1, &zoom0, &zoom1, &nsteps, &delt);
          make_flyby(tiff_junk, t0, t1, zoom0, zoom1, nsteps, delt);
		  if_loc_defined = 1;
		  if_az_defined = 1;
		  if_zoom_defined = 1;
       }
       else if (strcmp(tiff_tag,"Anim-Flyby-Daz-Max") == 0) {
          fscanf(tiff_fd,"%f", &daz_max);
       }
       else if (strcmp(tiff_tag,"Anim-Az-Zoom") == 0) {
		  if (!if_loc_defined) exit_safe(1, "Camera loc must be defined in script before tag 'Anim-Az-Zoom'");
		  if (!if_el_defined)  exit_safe(1, "El angle must be defined in script before tag 'Anim-Az-Zoom'");
          fscanf(tiff_fd,"%f %f %f %f %d %f", &a0, &a1, &zoom0, &zoom1, &n_ang, &delt);
          dela    = (a1-a0)       / float(n_ang-1);
          delzoom = (zoom1-zoom0) / float(n_ang-1);
          for (i_ang=0; i_ang<n_ang; i_ang++) {
			 if (n_list >= n_max) realloc_my();
             strcpy(listt->type, "Anim-Az-Zoom");
             listt->float1 = a0    + i_ang * dela;
             listt->float2 = zoom0 + i_ang * delzoom;
             listt->delt   = delt;
             listt++;
             n_list++;
          }
		  if_az_defined = 1;
		  if_zoom_defined = 1;
       }
       else if (strcmp(tiff_tag,"Anim-Az") == 0) {
		  if (!if_loc_defined)  exit_safe(1, "Camera loc must be defined in script before tag 'Anim-Az'");
		  if (!if_el_defined)   exit_safe(1, "El angle must be defined in script before tag 'Anim-Az'");
		  if (!if_zoom_defined) exit_safe(1, "Zoom must be defined in script before tag 'Anim-Az'");
          fscanf(tiff_fd,"%f %f %d %f", &a0, &a1, &n_ang, &delt);
          dela = (a1-a0) / float(n_ang-1);
          for (i_ang=0; i_ang<n_ang; i_ang++) {
			 if (n_list >= n_max) realloc_my();
             strcpy(listt->type, "Anim-Az");
             listt->float1 = a0 + i_ang * dela;
	         listt->float2 = -99.;
             listt->delt   = delt;
             listt++;
             n_list++;
          }
		  if_az_defined = 1;
       }
       else if (strcmp(tiff_tag,"Anim-El") == 0) {
		  if (!if_loc_defined) exit_safe(1, "Camera loc must be defined in script before tag 'Anim-El'");
		  if (!if_az_defined)   exit_safe(1, "Az angle must be defined in script before tag 'Anim-El'");
		  if (!if_zoom_defined) exit_safe(1, "Zoom must be defined in script before tag 'Anim-El'");
          fscanf(tiff_fd,"%f %f %d %f", &a0, &a1, &n_ang, &delt);
          dela = (a1-a0) / float(n_ang-1);
          for (i_ang=0; i_ang<n_ang; i_ang++) {
			 if (n_list >= n_max) realloc_my();
             strcpy(listt->type, "Anim-El");
             listt->float1 = a0 + i_ang * dela;
             listt->float2 = -99.;
             listt->delt   = delt;
             listt++;
             n_list++;
          }
		  if_el_defined = 1;
       }
       else if (strcmp(tiff_tag,"Anim-Roll") == 0) {
          fscanf(tiff_fd,"%f %f %d %f", &a0, &a1, &n_ang, &delt);
          dela = (a1-a0) / float(n_ang-1);
          for (i_ang=0; i_ang<n_ang; i_ang++) {
			 if (n_list >= n_max) realloc_my();
             strcpy(listt->type, "Set-Roll");
             listt->float1 = a0 + i_ang * dela;
             listt->float2 = -99.;
			 listt->delt   = delt;
	         listt++;
	         n_list++;
	      }
       }
       else if (strcmp(tiff_tag,"Anim-Range-Near-In") == 0) {
          fscanf(tiff_fd,"%d %d %d %f", &n0, &n1, &n_skip, &delt);
          for (i_ang=n0; i_ang<=n1; i_ang=i_ang+n_skip) {
			 if (n_list >= n_max) realloc_my();
             strcpy(listt->type, "Set-Range-Span");
             listt->float1 = i_ang;
             listt->float2 = -99.;
             listt->delt   = delt;
             listt++;
             n_list++;
          }
       }
       else if (strcmp(tiff_tag,"Anim-Range-Near-Out") == 0) {
          fscanf(tiff_fd,"%d %d %d %f", &n0, &n1, &n_skip, &delt);
          for (i_ang=n0; i_ang>=n1; i_ang=i_ang-abs(n_skip)) {
			 if (n_list >= n_max) realloc_my();
             strcpy(listt->type, "Set-Range-Span");
             listt->float1 = i_ang;
             listt->float2 = -99.;
             listt->delt   = delt;
             listt++;
             n_list++;
          }
       }
       else if (strcmp(tiff_tag,"Anim-Range-Far-In") == 0) {
          fscanf(tiff_fd,"%d %d %d %f", &n0, &n1, &n_skip, &delt);
          for (i_ang=n0; i_ang<=n1; i_ang=i_ang+n_skip) {
			 if (n_list >= n_max) realloc_my();
             strcpy(listt->type, "Set-Range-Span");
             listt->float1 = -99.;
             listt->float2 = i_ang;
             listt->delt   = delt;
             listt++;
             n_list++;
          }
       }
       else if (strcmp(tiff_tag,"Anim-Range-Far-Out") == 0) {
          fscanf(tiff_fd,"%d %d %d %f", &n0, &n1, &n_skip, &delt);
          for (i_ang=n0; i_ang>=n1; i_ang=i_ang-abs(n_skip)) {
			 if (n_list >= n_max) realloc_my();
             strcpy(listt->type, "Set-Range-Span");
             listt->float1 = -99.;
             listt->float2 = i_ang;
             listt->delt   = delt;
             listt++;
             n_list++;
          }
       }
       else if (strcmp(tiff_tag,"Anim-Tau") == 0) {
          fscanf(tiff_fd,"%d %d %d %f", &tau0, &tau1, &n_tau, &delt);
          for (i_tau=0; i_tau<n_tau; i_tau++) {
			 if (n_list >= n_max) realloc_my();
			 tauf = tau0 + i_tau * (tau1 - tau0) / (n_tau-1);
             strcpy(listt->type, "Set-Tau");
             listt->float1 = tauf;
             listt->delt   = delt;
             listt++;
             n_list++;
          }
       }
       else if (strcmp(tiff_tag,"Anim-Zoom") == 0) {
		  if (!if_loc_defined) exit_safe(1, "Camera loc must be defined in script before tag 'Anim-Zoom'");
		  if (!if_az_defined)  exit_safe(1, "Az angle must be defined in script before tag 'Anim-Zoom'");
		  if (!if_el_defined)  exit_safe(1, "El angle must be defined in script before tag 'Anim-Zoom'");
          fscanf(tiff_fd,"%f %f %d %f", &a0, &a1, &n_ang, &delt);
          ax0 = 20. * log10(a0);
          ax1 = 20. * log10(a1);
          dela = (ax1-ax0) / float(n_ang-1);
          if (diag_flag > 0) cout << "log start " << ax0 << "stop " << ax1 << " del " << dela << endl;
          for (i_ang=0; i_ang<n_ang; i_ang++) {
			 if (n_list >= n_max) realloc_my();
             strcpy(listt->type, "Anim-Zoom");
             val           = ax0 + i_ang * dela;
             listt->float1 = pow(10.,val /20.);;
             listt->float2 = -99.;
             listt->delt   = delt;
             listt++;
             n_list++;
          }
		  if_zoom_defined = 1;
       }
       else if (strcmp(tiff_tag,"Anim-Camera-Z") == 0) {
          fscanf(tiff_fd,"%f %f %d %f", &a0, &a1, &n_ang, &delt);
          dela = (a1-a0) / float(n_ang-1);
          for (i_ang=0; i_ang<n_ang; i_ang++) {
			 if (n_list >= n_max) realloc_my();
             strcpy(listt->type, "Set-Camera-Z");
             listt->float1 = a0 + i_ang * dela;
             listt->float2 = -99.;
             listt->delt   = delt;
             listt++;
             n_list++;
          }
       }
       else if (strcmp(tiff_tag,"Script-Mouse-Left") == 0) {
          fscanf(tiff_fd,"%f %f %f", &mouse_x, &mouse_y, &delt);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Mouse-Left");
          listt->float1 = mouse_x;
          listt->float2 = mouse_y;
          listt->delt   = delt;
          listt++;
          n_list++;
		  if_loc_defined = 1;
       }
       else if (strcmp(tiff_tag,"Script-Mouse-Middle") == 0) {
          fscanf(tiff_fd,"%f %f %f", &mouse_x, &mouse_y, &delt);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Mouse-Middle");
          listt->float1 = mouse_x;
          listt->float2 = mouse_y;
          listt->delt   = delt;
          listt++;
          n_list++;
       }
       else if (strcmp(tiff_tag,"Mode-Loop-Forward") == 0) {
          fgets(tiff_junk,240,tiff_fd);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Set-Mode");
          listt->float1 = 2;
          listt->float2 = -99.;
          listt->delt   = 0.;
          listt++;
          n_list++;
          loop0 = n_list;
       }
       else if (strcmp(tiff_tag,"Mode-Loop-Twoway") == 0) {
          fgets(tiff_junk,240,tiff_fd);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Set-Mode");
          listt->float1 = 3;
          listt->float2 = -99.;
          listt->delt   = 0.;
          listt++;
          n_list++;
          loop0 = n_list;
       }
       else if (strcmp(tiff_tag,"Mode-Pause") == 0) {
          fscanf(tiff_fd,"%f", &delt);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Pause");
          listt->float1 = -99.;
          listt->float2 = -99.;
          listt->delt   = delt;
          listt++;
           n_list++;
       }
       else if (strcmp(tiff_tag,"Mode-Restart") == 0) {
          fscanf(tiff_fd,"%f", &delt);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Restart");
          listt->float1 = -99.;
          listt->float2 = -99.;
          listt->delt   = delt;
          listt++;
          n_list++;
       }
       else if (strcmp(tiff_tag,"Mode-Run-Once") == 0) {
          fgets(tiff_junk,240,tiff_fd);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Set-Mode");
          listt->float1 = 1;
          listt->float2 = -99.;
          listt->delt   = 0.;
          listt++;
          n_list++;
       }
       else if (strcmp(tiff_tag,"Mode-Stop-Step") == 0) {
	      fgets(tiff_junk,240,tiff_fd);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Set-Mode");
          listt->float1 = 0;
          listt->float2 = -99.;
          listt->delt   = 0.;
          listt++;
          n_list++;
       }
       else if (strcmp(tiff_tag,"Script-Set-Az") == 0) {
          fscanf(tiff_fd,"%f", &az);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Set-Az");
          listt->float1 = az;
          listt->float2 = -99.;
          listt->delt   = 0.;
          listt++;
          n_list++;
		  if_az_defined = 1;
       }
       else if (strcmp(tiff_tag,"Script-Set-El") == 0) {
          fscanf(tiff_fd,"%f", &el);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Set-El");
          listt->float1 = el;
          listt->float2 = -99.;
          listt->delt   = 0.;
          listt++;
          n_list++;
		  if_el_defined = 1;
       }
       else if (strcmp(tiff_tag,"Script-Set-Roll") == 0) {
          fscanf(tiff_fd,"%f", &roll);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Set-Roll");
          listt->float1 = roll;
          listt->float2 = -99.;
          listt->delt   = 0.;
          listt++;
          n_list++;
       }
       else if (strcmp(tiff_tag,"Script-Set-Tau") == 0) {
          fscanf(tiff_fd,"%d", &tau);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Set-Tau");
          listt->float1 = tau;
          listt->float2 = -99.;
          listt->delt   = 0.;
          listt++;
          n_list++;
       }
       else if (strcmp(tiff_tag,"Script-Set-Zoom") == 0) {
          fscanf(tiff_fd,"%f", &zoom);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Set-Zoom");
          listt->float1 = zoom;
          listt->float2 = -99.;
          listt->delt   = 0.;
          listt++;
          n_list++;
		  if_zoom_defined = 1;
       }
       else if (strcmp(tiff_tag,"Script-Set-Focal-Length") == 0) {
          fscanf(tiff_fd,"%f", &focal_length);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Set-Focal-Length");
          listt->float1 = focal_length;
          listt->float2 = -99.;
          listt->delt   = 0.;
          listt++;
          n_list++;
       }
       else if (strcmp(tiff_tag,"Script-Set-Range-Span") == 0) {
          fscanf(tiff_fd,"%f %f", &first_range, &last_range);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Set-Range-Span");
          listt->float1 = first_range;
          listt->float2 = last_range;
          listt->delt   = 0.;
          listt++;
          n_list++;
       }
       else if (strcmp(tiff_tag,"Script-Set-Camera-Z") == 0) {
          fscanf(tiff_fd,"%f", &az);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Set-Camera-Z");
          listt->float1 = az;
          listt->float2 = -99.;
          listt->delt   = 0.;
          listt++;
          n_list++;
       }
       else if (strcmp(tiff_tag,"Script-Snap") == 0) {
          fscanf(tiff_fd,"%d", &flag);
		  if (strcmp(snap_dirname, "") == 0) fscanf(tiff_fd,"%s", snap_dirname);	// Only read dirname first time thru
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Script-Snap");
          listt->float1 = float(flag);
          listt->float2 = -99.;
          listt->delt   = 0.;
          listt++;
          n_list++;
       }
       else if (strcmp(tiff_tag,"Script-Vis-Map") == 0) {
          fscanf(tiff_fd,"%d", &vis_flag);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Vis-Map");
          listt->float1 = vis_flag;
          listt->float2 = -99;
          listt->delt   = 0.;
          listt++;
          n_list++;
       }
       else if (strcmp(tiff_tag,"Script-Vis-Truth") == 0) {
          fscanf(tiff_fd,"%d", &vis_flag);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Vis-Truth");
          listt->float1 = vis_flag;
          listt->float2 = -99;
          listt->delt   = 0.;
          listt++;
          n_list++;
       }
       else if (strcmp(tiff_tag,"Script-Vis-Vector") == 0) {
          fscanf(tiff_fd,"%d", &vis_flag);
		  if (n_list >= n_max) realloc_my();
          strcpy(listt->type, "Vis-Vector");
          listt->float1 = vis_flag;
          listt->float2 = -99;
          listt->delt   = 0.;
          listt++;
          n_list++;
       }
       else if (strcmp(tiff_tag,"Script-Diag") == 0) {
          fscanf(tiff_fd,"%d", &diag_flag);
       }
       else {
          fgets(tiff_junk,240,tiff_fd);
       }
     } while (ntiff == 1);
   fclose(tiff_fd);
   
   if (if_freerun) timer->schedule();
   listt = list;
   return(1);
}

// ********************************************************************************
/// Execute the next command on the list.
// ********************************************************************************
int script_input_class::process_next_event()
{
   int i, i_manager, if_pending, first_range, last_range, vis_flag, to_stop_step = 0, to_restart = 0, fly_route_flag=0;
   TODO_LIST* listt;

   if (!script_defined_flag && !fly_route_defined_flag) {
	   warning(1, "Neither a script nor a route/track flyover is defined -- do nothing");
	   return(0);
   }

   if (i_serve >= n_list) {
      return(0);
   }
   
   do {
      // ********************************************************************************
      // Execute next command on list 
      // ********************************************************************************
      listt = list + i_serve;
      if (strcmp(listt->type, "Anim-Route") == 0) {
		 az_current = listt->float4;
		 if (direction == -1) az_current = az_current + 180.;
		 if (!if_fly_route_use_az) az_current = -99000000.;	// If az < -1000000 it wont be modified
         north_current = gps_calc->get_ref_utm_north() + listt->float2;
         east_current  = gps_calc->get_ref_utm_east()  + listt->float1;
		 camera_manager->set_view(north_current, east_current, az_current, -99., -99.);
		 fly_route_flag = 1;
      }
      else if (strcmp(listt->type, "Anim-Flyby") == 0) {
		 az_current = listt->float4;
		 zoom_current = listt->float5;
         north_current = gps_calc->get_ref_utm_north() + listt->float2;
         east_current  = gps_calc->get_ref_utm_east()  + listt->float1;
		 camera_manager->set_view(north_current, east_current, az_current, el_current, zoom_current);
      }
      else if (strcmp(listt->type, "Mouse-Left") == 0) {
         east_current  = listt->float1;
         north_current = listt->float2;
		 gps_calc->set_aim_point_UTM(north_current, east_current);
         GL_aimpoint_flag->setValue(-1);
      }
      else if (strcmp(listt->type, "Mouse-Middle") == 0) {
         GL_mousem_north->setValue(listt->float2);
         GL_mousem_east->setValue(listt->float1);
         GL_mousem_new->setValue(1);
      }
      else if (strcmp(listt->type, "Pause") == 0) {
      }
      else if (strcmp(listt->type, "Restart") == 0) {
         to_restart = 1;
      }
      else if (strcmp(listt->type, "Set-Camera-Z") == 0) {
         camera_manager->set_cg_z(listt->float1);
      }
      else if (strcmp(listt->type, "Anim-Az-Zoom") == 0) {
		 az_current = listt->float1;
		 zoom_current = listt->float2;
		 camera_manager->set_view(north_current, east_current, az_current, el_current, zoom_current);
      }
      else if (strcmp(listt->type, "Anim-Az") == 0) {
		 az_current = listt->float1;
		 camera_manager->set_view(north_current, east_current, az_current, el_current, zoom_current);
      }
      else if (strcmp(listt->type, "Anim-El") == 0) {
		 el_current = listt->float1;
		 camera_manager->set_view(north_current, east_current, az_current, el_current, zoom_current);
      }
      else if (strcmp(listt->type, "Anim-Zoom") == 0) {
		 zoom_current = listt->float1;
		 camera_manager->set_view(north_current, east_current, az_current, el_current, zoom_current);
      }
      else if (strcmp(listt->type, "Set-Az") == 0) {
         camera_manager->set_cg_az(listt->float1);
		 az_current = listt->float1;
      }
      else if (strcmp(listt->type, "Set-El") == 0) {
         camera_manager->set_cg_el(listt->float1);
		 el_current = listt->float1;
      }
      else if (strcmp(listt->type, "Set-Roll") == 0) {
               camera_manager->set_cg_roll(listt->float1);
      }
      else if (strcmp(listt->type, "Set-Tau") == 0) {
		SoSFInt32 *GL_mobmap_fine_cur   = (SoSFInt32*)  SoDB::getGlobalField("Mobmap-Fine-Cur");	// OIV Global --
		SoSFInt32* GL_mobmap_mod     = (SoSFInt32*)  SoDB::getGlobalField("Mobmap-Mod");	// OIV Global -- Modify point cloud flag
		int tau = int(listt->float1 + 0.49);
		GL_mobmap_fine_cur->setValue(tau);
		GL_mobmap_mod->setValue(5);

      }
      else if (strcmp(listt->type, "Set-Zoom") == 0) {
         for (i_manager=0; i_manager<n_managers; i_manager++) {
            atrlab_manager_a[i_manager]->set_zoom_factor(listt->float1);
         }
		 zoom_current = listt->float1;
      }
      else if (strcmp(listt->type, "Set-Focal-Length") == 0) {
         for (i_manager=0; i_manager<n_managers; i_manager++) {
            atrlab_manager_a[i_manager]->set_focal_length(listt->float1);
         }
      }
      else if (strcmp(listt->type, "Set-Range-Span") == 0) {
         first_range = listt->float1;
         last_range = listt->float2;
         for (i_manager=0; i_manager<n_managers; i_manager++) {
            if (strcmp(atrlab_manager_a[i_manager]->get_type(), "ladar") == 0 && atrlab_manager_a[i_manager]->get_if_visible()) {
               if (first_range >= 0) atrlab_manager_a[i_manager]->set_displayed_range_min(first_range);
	           if (last_range  >= 0) atrlab_manager_a[i_manager]->set_displayed_range_max(last_range);
            }
         }
      }
      else if (strcmp(listt->type, "Script-Snap") == 0) {
		 snap_enabled_flag = (int)listt->float1;
      }
      else if (strcmp(listt->type, "Vis-Map") == 0) {
		 if ((listt->float1 == 1 && direction == 1) || (listt->float1 == 0 && direction == -1)) {	// If going backward, want to undo vis change
			  vis_flag = 1;
		 }
		 else {
			  vis_flag = 0;
		 }
         for (i_manager=0; i_manager<n_managers; i_manager++) {
            if (strcmp(atrlab_manager_a[i_manager]->get_type(), "map3d") == 0) {
               atrlab_manager_a[i_manager]->set_if_visible(vis_flag);
            }
         }
      }
      else if (strcmp(listt->type, "Vis-Truth") == 0) {
		 if ((listt->float1 == 1 && direction == 1) || (listt->float1 == 0 && direction == -1)) {	// If going backward, want to undo vis change
			  vis_flag = 1;
		 }
		 else {
			  vis_flag = 0;
		 }
         for (i_manager=0; i_manager<n_managers; i_manager++) {
            if (strcmp(atrlab_manager_a[i_manager]->get_type(), "track") == 0 || strcmp(atrlab_manager_a[i_manager]->get_type(), "bookmark") == 0 ) {
               atrlab_manager_a[i_manager]->set_if_visible(vis_flag);
            }
         }
      }
      else if (strcmp(listt->type, "Vis-Vector") == 0) {
		 if ((listt->float1 == 1 && direction == 1) || (listt->float1 == 0 && direction == -1)) {	// If going backward, want to undo vis change
			  vis_flag = 1;
		 }
		 else {
			  vis_flag = 0;
		 }
         for (i_manager=0; i_manager<n_managers; i_manager++) {
            if (strcmp(atrlab_manager_a[i_manager]->get_type(), "kml") == 0) atrlab_manager_a[i_manager]->set_if_visible(vis_flag);
         }
      }
      else if (strcmp(listt->type, "Set-Mode") == 0) {
         serve_mode = listt->float1;
         if (serve_mode == 0) to_stop_step = 1;
      }
      else {
         cerr << "Illegal event " << listt->type << endl;
         //return(0);
      }
   
      if (to_stop_step) {
         interval = 1.;
         if_pending = 0;
         to_stop_step = 0;
      }
      else if (listt->delt > 0.) {
         interval = listt->delt;
         if_pending = 0;
      }
      else {
         if_pending = 1;
      }
      if (diag_flag > 0) cout << "Script " << listt->type << " " << listt->float1 << " " << listt->float2 << " " << listt->float3 << " " << listt->float4 << " " << listt->delt << endl;
      
      // ********************************************************************************
      // Screen capture, if requested
      // ********************************************************************************
	  if (snap_enabled_flag) {
		  screen_capture();
	  }
      
      // ********************************************************************************
      // Determine next command to be processed 
      // ********************************************************************************
      if (fly_route_flag) {
		  i_serve = i_serve + stride_flyroute * direction;
		  if (i_serve > n_list-1) i_serve = n_list-1;	// At list end and going forward -- stay at end of list
		  if (i_serve < 0       ) i_serve = 0;			// At list begin and going backward -- stay at begin of list
	  }
	  else if (to_restart) {					// Go to beginning of list
         i_serve = 0;
         to_restart = 0;
      }
      else if (i_serve == n_list-1) {			// At list end
         if (serve_mode == 2) {
            i_serve = loop0;
         }
         else if (serve_mode == 3) {
            i_serve--;
            direction = -1;
         }
         else {
            if_pending = 0;
            i_serve++;
         } 
      }
      else if (i_serve == loop0 && direction == -1) {	// At loop beginning
         i_serve++;
         direction = 1;
      }
      else {
         i_serve = i_serve + direction;
      }
   } while (if_pending);
   
   // ********************************************************************************
   // Update managers 
   // ********************************************************************************
   for (i=0; i<n_managers; i++) {
      atrlab_manager_a[i]->refresh();
   }
   return(1);
}

// ********************************************************************************
/// Do screen capture -- Private.
///
// ********************************************************************************
int script_input_class::screen_capture()
{
	char filename[300];
#if defined(LIBS_QT)
	sprintf(filename, "%s/snap_%4.4d.bmp", snap_dirname, snap_seqno);
	QWidget *w = QApplication::activeWindow();
	if (w) {
		QPixmap p = w->grab();
		p.save(QString::fromStdString(filename));
	}
	else {
		warning(1, "script_input_class::screen_capture:  Cant screen capture main window");
	}
#else
	char tname[400], command[400];
	int nbytes = GetModuleFileNameA(NULL, tname, 400);
	char *pdest = strrchr(tname, '\\');
	*pdest = '\0';
	if (strstr(tname, "\\Debug") != NULL) strcpy(tname, "C:\\Fusion3D\\bin");	// Just for debugging

	sprintf(command, "%s\\CmdCapture.exe /f %s\\snap_%4.4d.bmp", tname, snap_dirname, snap_seqno);
	cout << "To issue command to take screen shot:";
	cout << "     " << command << endl;
	system(command);
	snap_seqno++;
#endif
	return(1);
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void script_input_class::timer_cbx(void *userData, SoSensor *timer)
{
   script_input_class* scriptt = (script_input_class*)  userData;
   scriptt->timer_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void script_input_class::timer_cb()
{
   float interval_loc;
   if (process_next_event()) {
      if (get_new_interval(interval_loc)) {
		  timer->setInterval(interval_loc);
		  if (diag_flag > 0) cout << "script_input_class::timer_cb() new interval=" << interval_loc << endl;
	  }
   }
   else {
      timer->unschedule();		// Script finished -- turn off clock
   }
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void script_input_class::space_nav_cbx(void *userData, SoSensor *timer)
{
   script_input_class* scriptt = (script_input_class*)  userData;
   scriptt->space_nav_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void script_input_class::space_nav_cb()
{
	if (fly_route_defined_flag || script_defined_flag) {
		if (if_freerun) {			// Freerunning -- switch to stop-step
			timer->unschedule();
			GL_button_mod->setValue(81);
			if_freerun = 0;
		}
		else {
			timer->schedule();		// Stop-step -- switch to freerunning
			GL_button_mod->setValue(82);
			if_freerun = 1;
		}
	}
	else if (script_defined_flag) {
	}
	else return;
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void script_input_class::track_cbx(void *userData, SoSensor *timer)
{
   script_input_class* scriptt = (script_input_class*)  userData;
   scriptt->track_cb();
}

// ********************************************************************************
// Actual callback
// ********************************************************************************
void script_input_class::track_cb()
{
	int istate = GL_tracks_state->getValue();
	GL_fly_track     = (SoSFInt32*)  SoDB::getGlobalField("Fly-Track");
	int val = GL_fly_track->getValue();
	int dir_flag    =  val % 2;
	int ontop_flag  = (val % 4) / 2;
	int useraz_flag = (val % 8) / 4;
	int speed_flag  = (val % 32) / 8;
	int avg_flag    = (val % 2048) / 32;
	int loc_flag    =  val / 2048;

	if (!gps_calc->is_ref_defined()) return;			// No map so do nothing

	if (speed_flag == 0) {
		stride_flyroute = 1;
	}
	else if (speed_flag == 1) {
		stride_flyroute = 2;
	}
	else if (speed_flag == 2) {
		stride_flyroute = 4;
	}
	else if (speed_flag == 3) {
		stride_flyroute = 6;
	}
	distance_average_interval = 10 * avg_flag;
	float loc_percent = float(loc_flag) / 63.;


	if (istate == 0) {
		n_list = 0;
		fly_route_defined_flag = 0;
		direction = 1;
        GL_fly_track->setValue(20 * 64);	// Default to averaging interval = 20 * scalingFactor=10 starting in bit 7
	}
	else if (istate == 12 && fly_route_defined_flag == 0) {
		if (!make_route(1., 0.05)) {
			warning(1,"No track defined -- do nothing");
			return;
		}
		i_serve = i1_flyroute;
		fly_route_defined_flag = 1;
		process_next_event();
		GL_button_mod->setValue(99);	 // Signal buttons to add 2 buttons for script
	}
	else if (istate == 13) {					// Go forward 
		if (direction == -1) {
			//float percent = float(i_serve - i1_flyroute) / float(i2_flyroute-i1_flyroute);// Original idea was to flip the track rather than just change dir
			//i_serve = i2_flyroute - float(i2_flyroute-i1_flyroute) * percent;
			direction = 1;
		}
		process_next_event();
	}
	else if (istate == 14) {					// Go backward
		if (direction == 1) {
			//float percent = float(i_serve - i1_flyroute) / float(i2_flyroute-i1_flyroute);
			//i_serve = i2_flyroute - float(i2_flyroute-i1_flyroute) * percent;
			direction = -1;
		}
		process_next_event();
	}
	else if (istate == 15) {					// Change averaging interval
		//distance_average_interval = GL_slider_val->getValue();
		make_route(1., 0.05);
	}
	else if (istate == 16) {					// Go to location on track
		//float percent = GL_slider_val->getValue();	// Relative distance along track [0,1]
		if (direction == 1) {
			i_serve = i1_flyroute + float(i2_flyroute-i1_flyroute) * loc_percent;
		}
		else {
			i_serve = i1_flyroute + float(i2_flyroute-i1_flyroute) * (1. - loc_percent);
		}
		process_next_event();
	}
	else if (istate == 17) {					// Change speed -- already done above
	}
	else if (istate == 18) {					// Use calculated az
		if_fly_route_use_az = 1;
	}
	else if (istate == 19) {					// Allow user to control az interactively
		if_fly_route_use_az = 0;
	}
	else {
		return;
	}

}

// ********************************************************************************
/// Construct flyby sequence -- Private.
// ********************************************************************************
int script_input_class::make_flyby(char *filename, float tstart_norm, float tstop_norm,
                                   float zoom_start, float zoom_stop, int nsteps, float delt)
{
   int i, npts;
   float t, east, north, elev, az, az_old, az_track, track_length, dt, tstart, tstop, dzoom, zoom, timeIndex;
   vector_layer_class *vector_layer;
   draw_data_class *draw_data = new draw_data_class();		// All flags are default

   if (strstr(filename, ".kml") != NULL) {
	   vector_layer = new kml_class();
   }
   else if (strstr(filename, ".shp") != NULL || strstr(filename, ".osm") != NULL) {
	   vector_layer = new ogr_class();
   }
   else if (strstr(filename, ".sbet") != NULL) {
	   vector_layer = new sbet_class();
   }
   else if (strstr(filename, ".csv") != NULL) {
	   vector_layer = new csv_class();
   }
   else {
	   warning_s("Script Flyby:  Cant read vector overlay format for file", filename);
	   return(0);
   }
   vector_layer->register_coord_system(gps_calc);
   vector_layer->register_draw_data_class(draw_data);
   vector_layer->set_diag_flag(diag_flag);
   if (!vector_layer->read_file(filename)) {
	   warning_s("Script Flyby:  Cant read vector overlay file", filename);
	   delete vector_layer;
	   return(0);
   }
   delete vector_layer;

	npts = draw_data->plx[0].size();
	if (npts == 0) return(0);
   
	interval_calc_class *interval_calc = new interval_calc_class();
	interval_calc->set_time_equal_distance(draw_data->plx[0], draw_data->ply[0]);
	track_length = interval_calc->get_internal_time_at_index(npts - 1);
	tstart = tstart_norm * track_length;
	tstop  = tstop_norm  * track_length;
   
	dt = (tstop - tstart) / (nsteps - 1);
	dzoom = (zoom_stop - zoom_start) / (nsteps - 1);
	while (n_list+nsteps > n_max) {
		realloc_my();
	}
   
   for (i=0; i<nsteps; i++) {
      t    = tstart     + i * dt;
	  interval_calc->calc_index_internal(t, timeIndex);
	  draw_data->get_loc_along_line_at_index(0, timeIndex, east, north, elev, az_track);	// 
	  zoom = zoom_start + i * dzoom;
	  if (i == 0) az_old = az_track;
      strcpy(listt->type, "Anim-Flyby");
      listt->float1 = east;
      listt->float2 = north;
      listt->float3 = elev;
	  if (az_track - az_old > daz_max) {
		  az = az_old + daz_max;
	  }
	  else if (az_old - az_track > daz_max) {
		  az = az_old - daz_max;
	  }
	  else {
		  az = az_track;
	  }
      listt->float4 = 90.f - az;
      listt->float5 = zoom;
      listt->delt   = delt;
      listt++;
      n_list++;
	  az_old = az;
   }
   return(1);
}

// ********************************************************************************
/// Construct flythrough for a track/route -- Private.
/// For the purposes of the fly-through, speed is 1 m/s, so distance and time are the same.
/// The track/route is the currently defined track stored in the track_manager_class.
/// The method assumes that the route is the first thing in the display list.
/// @param	delDistAlongTrack	The distance along the track between sample points.
/// @param	delTimeWallclock	The wall-clock time between sample points.
// ********************************************************************************
int script_input_class::make_route(float delDistAlongTrack, float delTimeWallclock)
{
	int i,npts;
	float t, east, north, elev, az, az_track, tstart, tstop, timeIndex;
   
	if (i_trackmanager < 0) return(0);
   
	// ***************************
	// Transfer track points from track manager 
	// Functionality of time_track_class being transfered to draw_data_class, but transfer not complete to duplicated for now
	// Getting tstart and tstop not yet transferred for sure
	// ***************************
	track_manager_class *track_manager = (track_manager_class*) atrlab_manager_a[i_trackmanager];
	if (track_manager == NULL) return(0);
	draw_data_class *draw_data = track_manager->get_draw_data_class();
	if (draw_data == NULL) return(0);
	npts = draw_data->plx[0].size();
	if (npts == 0) return(0);
   
	interval_calc_class *interval_calc = new interval_calc_class();
	interval_calc->set_time_equal_distance(draw_data->plx[0], draw_data->ply[0]);

	// ***************************
	// Fill list
	// ***************************
	tstart = 0.;		// Time equals distance
	tstop = interval_calc->get_internal_time_at_index(npts-1);
	int nsteps = int((tstop - tstart) / delDistAlongTrack) + 1;
   
	if (distance_average_interval > 0.) {
		draw_data->set_distance_average_interval(distance_average_interval);
		draw_data->distance_average(0);
	}
	i1_flyroute = 0;
	i2_flyroute = nsteps-1;
	n_list = 0;
	while (nsteps > n_max) {
		realloc_my();
	}
	listt = list;
	for (i=0; i<nsteps; i++) {
		t    = tstart     + i * delDistAlongTrack;
		interval_calc->calc_index_internal(t, timeIndex);
		draw_data->get_loc_along_line_at_index(0, timeIndex, east, north, elev, az_track);	// 
		strcpy(listt->type, "Anim-Route");
		listt->float1 = east;
		listt->float2 = north;
		listt->float3 = elev;
		az = az_track;
		listt->float4 = 90.0f - az;
		listt->delt   = delTimeWallclock;
		// cout << i << " " << east << endl;
		listt++;
		n_list++;
	}

	return(1);
}

// ********************************************************************************
/// Double the size of the memory allocated for the list -- Private.
// ********************************************************************************
int script_input_class::realloc_my()
{
	TODO_LIST *list_loc= new TODO_LIST[2 * n_max];
	memcpy(list_loc, list, n_list * sizeof(TODO_LIST));
	int nn = sizeof(TODO_LIST);
	delete[] list;
	list = list_loc;
	n_max = 2 * n_max;
	listt = list + n_list;
	return(1);

}