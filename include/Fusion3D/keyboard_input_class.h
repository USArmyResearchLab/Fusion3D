#ifndef _keyboard_input_class_h_
#define _keyboard_input_class_h_
#ifdef __cplusplus

/**
Processes keyboard inputs.

This class modifies standard locations in memory.
It expects that these locations will be wired to managers.
When it processes an event, it modifies a location and then updates all managers.
In turn, these managers monitor specific locations and modify themselves accordingly.
*/
class keyboard_input_class{
   private:
      atrlab_manager_class** 	atrlab_manager_a;
      int 			n_managers;
   
      static void key_cbx(void *userData, SoEventCallback *eventCB);
      int process_event(const SoEvent *event);

 
   public:
      int* pad_count;		// Count incremented every time pad number is pressed
      int* number_count;	// Count incremented every time keyboard number is pressed
      int* fun_count;		// Count incremented every time keyboard function key pressed
      int mult_count;		// Count incremented every time pad multiply pressed
      int div_count;		// Count incremented every time pad divide pressed
      int add_count;		// Count incremented every time pad add pressed
      int sub_count;		// Count incremented every time pad subtract pressed

      // For cursor control
      int curs_left_count;	// Count incremented every time cursor left arrow is pressed
      int curs_right_count;	// Count incremented every time cursor right arrow is pressed
      int curs_up_count;	// Count incremented every time cursor up arrow is pressed
      int curs_down_count;	// Count incremented every time cursor down arrow is pressed
      int curs_home_count;	// Count incremented every time cursor home is pressed
      int curs_pup_count;	// Count incremented every time cursor page up is pressed
      int curs_pdn_count;	// Count incremented every time cursor page down is pressed
      int curs_prior_count;	// Count incremented every time cursor page down is pressed
      int curs_next_count;	// Count incremented every time cursor page down is pressed
      int curs_end_count;	// Count incremented every time cursor page down is pressed
      
      clock_input_class* 		clock_input;

   keyboard_input_class();
   ~keyboard_input_class();
   
   int register_2d(SoSeparator* root);
   int register_managers(atrlab_manager_class** atrlab_manager_a_in, int n_managers_in);
   int register_clock(clock_input_class *clock_input_in);
};

#endif /* __cplusplus */
#endif /* _keyboard_input_class_h_ */
