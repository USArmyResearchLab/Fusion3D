#ifndef _write_manager_inv_class_h_
#define _write_manager_inv_class_h_	
#ifdef __cplusplus

/**
Manages writing.

The primary function at this time is to dump the screen to a file.\n
\n
The user has 2 options.  He can supply a filename and dump the screen to that file.
He can also dump the screen to a predefined filename containing a sequence number that is automatically incremented.
This option is designed for making short movies by dumping an animated sequence of screen captures to a sequence of files.

*/
class buttons_input_class;

class write_manager_inv_class:public atrlab_manager_class{
   private:
      SoSeparator* 		classBase;			// Base of tree for class
      SoSeparator* 		root;				// Root of tree to be rendered to file
      SoOffscreenRenderer*	osr;			// Does off-screen rendering to file
      
      SoSFInt32*        GL_open_flag;		// OIV Global -- For opening file for read/write
      SoSFString*		GL_string_val;		// OIV Global -- Dump screen to sequence of files -- Base name
      SoSFInt32*		GL_write_nx;		// OIV Global -- Dump screen to sequence of files -- Size
      SoSFInt32*		GL_write_ny;		// OIV Global -- Dump screen to sequence of files -- Size
      SoSFString*       GL_filename;        // OIV Global -- Open new file
      SoSFInt32*		GL_new_frames_flag;	// OIV Global -- For movies/realtime indicating new frame
      SoFieldSensor*	goSensor;			// Initiate write
      SoFieldSensor*	newFrameSensor;		// New frame

      int write_type_flag;					// type of write
											//	1 for usr-specified screen dump
											//	2 for sequenced screen dump
											//	3 for current user parms
	  char *out_name;						// Prefix for output file for sequenced screen dumps

      int osr_width, osr_height;			// Size of rendered file
      int osr_width_prev, osr_height_prev;	// Size of rendered file -- previous
      int iseq;								// Sequence no. of rendered file
	  int dump_new_frames_flag;				// 1 iff dump each new frame
	  int gdal_init_flag;					// 1 iff GDAL has been initialized

	  int n_managers;
      atrlab_manager_class 	**atrlab_manager_a;
	  globals_inv_class *globals_inv;
	  buttons_input_class *buttons_input;
      script_input_class*		script_input;
        
      // Private methods
      int dump_to_seq(int iseq);
	  int write_jpeg(char *filename, unsigned char *buf);
	  int write_all_classes_parms();
	  static void go_cbx(void *userData, SoSensor *timer);
      void go_cb();
      static void new_frame_cbx(void *userData, SoSensor *timer);
      void new_frame_cb();

   public:
      write_manager_inv_class(int n_data_max_in);
      ~write_manager_inv_class();

      int register_inv(SoSeparator* classBase_in);
      int register_root(SoSeparator* root_in);
      int register_image_3d(image_3d_class* image_3d_in);
	  int register_managers(atrlab_manager_class** atrlab_manager_a_in, int n_managers_in);
	  int register_globals_inv_class(globals_inv_class *globals_inv_in);
	  int register_buttons_input_class(buttons_input_class *buttons_input_sw_in);
	  int register_script(script_input_class *script_input_in);
      int write_parms(FILE *out_fd) override;

      int get_type(char* type);
      int read_tagged(const char* filename);
	  int read_all_classes_parms(const char* filename);
	  int refresh();
};

#endif /* __cplusplus */
#endif /* _write_manager_inv_class_h_ */
