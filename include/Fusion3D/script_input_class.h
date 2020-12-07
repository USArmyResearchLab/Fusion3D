#ifndef _script_input_class_h_
#define _script_input_class_h_
#ifdef __cplusplus

/**
Processes all script inputs and also processes route flyovers.

Processes all script inputs where a sequence of commands is read from the Project file and executes them in order.
Also creates a flyover from a track file and executes the flyover.
The track flyover is very similar to the animation commands from the script.
At this time, scripting requires manually editing the Project file, but route flyover is done from the menus.
In the default stop-step mode, the class executes a command each time the stop/step button is pressed.
In the free-running mode, commands are executed at regular intervals.\n
\n
Button press\n
	buttons_input_class::hitsButton0()\n
		script_input->set_freerun();\n
		sbutton0win->setBitmap(1);\n
		sbutton1win->setBitmap(1);\n
	buttons_input_class::hitsButton1()\n
		script_input->set_stop_step();\n
		sbutton0win->setBitmap(0);\n
		sbutton1win->setBitmap(0);\n
\n
Spaceball button press\n
	spaceball_input_class::spaceball_button_cb\n
		GL_space_button->setValue(which);	# 1 for left, 2 for right\n
\n
	script_input_class::space_nav_cb()		# Monitors GL_space_button\n
		Schedules/unschedules timer\n
		GL_button_mod->setValue(81);		# 81 /82\n
\n
	buttons_input_sw_class::mod_buttons_cb()\n
		Changes button graphics\n


*/
class script_input_class:public base_jfd_class{
   private:
      int script_defined_flag;			///< Script -- 1 iff script defined and entered into command list
      int if_freerun;					///< 0 if stop/step (default), 1 if freerunning
      int if_menus;						///< 1 to display menus/buttons for script
	  int if_loc_defined;				///< 1 iff camera loc (aim point in x,y,z) defined
	  int if_az_defined;				///< 1 iff camera azimuth angle defined
	  int if_el_defined;				///< 1 iff camera elevation angle defined
	  int if_zoom_defined;				///< 1 iff camera zoom defined
	  int fly_route_defined_flag;		///< Route flyover -- 1 iff route-flyover path defined and entered into command list
	  int if_fly_route_use_az;			///< Route flyover -- 1 iff calculated az used (0 to let user control interactively)
	  int i1_flyroute, i2_flyroute;		///< Route flyover -- First and last indices
	  int stride_flyroute;				///< Route flyover -- Stride

      int n_max;						///< Max no of entries allocated for list
      int n_list;						///< No of entries in list
      int i_serve;						///< Entry currently being processed
      float interval;					///< Time interval in s between steps
      float interval_old;
      int loop0;						///< Start of loop
      int direction;					///< 1 for forward, -1 for reverse
      int serve_mode;					///< 0 for stop/step,
      									///< 1 for free run once in forward dir
										///< 2 for loop in forward dir
										///< 3 for loop in both directions
      float daz_max;					///< Max az difference per step for Anim-Flyby
	  float distance_average_interval;	///< Distance (m) for track parameter averaging

	  int snap_enabled_flag;			///< Automatic screen captures -- 1 iff capture enabled
	  int snap_seqno;					///< Automatic screen captures -- Sequence no. for filenames
	  char *snap_dirname;				///< Automatic screen captures -- Dir name for output files

	  double north_current;				///< Current value
	  double east_current;				///< Current value
	  float az_current;					///< Current value
	  float el_current;					///< Current value
	  float zoom_current;				///< Current value
     
      SoTimerSensor		*timer;
      atrlab_manager_class 	**atrlab_manager_a;
	  camera4d_manager_inv_class *camera_manager;
      map3d_index_class* map3d_index;
      int 			n_managers;
	  int i_trackmanager;				// Index of track manager
         
      typedef struct {
         char type[48];
         float float1;
         float float2;
         float float3;
         float float4;
         float float5;
         float delt;
      } TODO_LIST;
      
      TODO_LIST *list;
      TODO_LIST* listt;

      SoSFFloat*	GL_mousem_north;      // OIV Global -- For mouse middle click
      SoSFFloat*	GL_mousem_east;       // OIV Global -- For mouse middle click
      SoSFInt32*	GL_mousem_new;        // OIV Global -- For mouse middle click
      SoSFInt32*	GL_aimpoint_flag;     // OIV Global -- For updates of camera aim point
      SoSFInt32*	GL_tracks_state;	  // OIV Global -- Track
      SoSFInt32*	GL_button_mod;		  // OIV Global -- Modify buttons -- add, gray-out
      SoSFInt32*	GL_fly_track;		  // OIV Global -- For updates of route/track flyover parameters
	  SoSFInt32*	GL_space_button ;	  // OIV Global -- For SpaceNavigator left button

      // Private methods
	  int screen_capture();
      int make_flyby(char *filename, float tstart, float tstop,
                  float zoom_start, float zoom_stop, int nsteps, float delt);
	  int make_route(float delDistAlongTrack, float delTimeWallclock);
      int get_new_interval(float &interval);
	  int realloc_my();
      static void timer_cbx(void *userData, SoSensor *timer);
      void timer_cb();
      static void track_cbx(void *userData, SoSensor *timer);
      void track_cb();
      static void space_nav_cbx(void *userData, SoSensor *timer);
      void space_nav_cb();
   
   public:
      script_input_class();
      ~script_input_class();
   
      int register_managers(atrlab_manager_class** atrlab_manager_a_in, int n_managers_in);
      int register_index(map3d_index_class *index);
      int read_tagged(const char* filename);
      int clear_all() override;
      int get_if_enabled();
      int get_if_freerun();
      int get_if_menus();
      int get_serve_mode();
      int set_freerun();
      int set_stop_step();
      int process_next_event();
	  SoTimerSensor* get_timer();

};

#endif /* __cplusplus */
#endif /* _script_input_class_h_ */
