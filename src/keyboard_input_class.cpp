#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
keyboard_input_class::keyboard_input_class()
{
   int i;
   
   // For Function key numbers
   fun_count = new int[14];
   for (i=0; i<14; i++) {
      fun_count[i] = -1;
   }
   
   // For Pad numbers
   pad_count = new int[12];
   for (i=0; i<12; i++) {
      pad_count[i] = -1;
   }
   
   // For keyboard numbers
   number_count = new int[10];
   for (i=0; i<10; i++) {
      number_count[i] = -1;
   }
   
   // For Pad 
   mult_count = -1;
   div_count = -1;
   add_count = -1;
   sub_count = -1;

   // For cursor control
   curs_left_count  = -1;
   curs_right_count = -1;
   curs_up_count    = -1;
   curs_down_count  = -1;
   curs_home_count  = -1;
   curs_pup_count   = -1;
   curs_pdn_count   = -1;
   curs_prior_count = -1;
   curs_next_count  = -1;
   curs_end_count   = -1;
   
   clock_input = NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
keyboard_input_class::~keyboard_input_class()
{
	delete[] fun_count;
	delete[] pad_count;
	delete[] number_count;
}

// ********************************************************************************
/// Initialize OpenInventor stuff.
// ********************************************************************************
int keyboard_input_class::register_2d(SoSeparator* root)
{
   SoEventCallback *keyEventCB = new SoEventCallback;
   keyEventCB->addEventCallback(SoKeyboardEvent::getClassTypeId(), key_cbx, this);
   root->addChild(keyEventCB);
   return(1);
}
   
// ********************************************************************************
/// Register managers.
// ********************************************************************************
int keyboard_input_class::register_managers(atrlab_manager_class** atrlab_manager_a_in, int n_managers_in)
{
   n_managers = n_managers_in;
   atrlab_manager_a = atrlab_manager_a_in;
   return(1);
}
   
// ********************************************************************************
/// Register the clock.
// ********************************************************************************
int keyboard_input_class::register_clock(clock_input_class *clock_input_in)
{
   clock_input = clock_input_in;
   return(1);
}
   
// ********************************************************************************
/// Process a user input event.
// ********************************************************************************
int keyboard_input_class::process_event(const SoEvent *event)
{
   int i;
   if      (SO_KEY_PRESS_EVENT(event, PAD_0)){ 
      std::cout << "To PAD_0 " << std::endl;
      pad_count[0]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, PAD_1)){ 
      pad_count[1]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, PAD_2)){ 
      pad_count[2]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, PAD_3)){ 
      pad_count[3]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, PAD_4)){ 
      pad_count[4]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, PAD_5)){ 
      pad_count[5]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, PAD_6)){ 
      pad_count[6]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, PAD_7)){ 
      pad_count[7]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, PAD_8)){ 
      pad_count[8]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, PAD_9)){ 
      pad_count[9]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, PAD_PERIOD)){ 
      pad_count[10]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, UP_ARROW)) {
      curs_up_count++;
   }   
   else if (SO_KEY_PRESS_EVENT(event, DOWN_ARROW)) {
      curs_down_count++;
   }
   else if (SO_KEY_PRESS_EVENT(event, LEFT_ARROW)) {
      curs_left_count++;
   }
   else if (SO_KEY_PRESS_EVENT(event, RIGHT_ARROW)) {
      curs_right_count++;
   }
   else if (SO_KEY_PRESS_EVENT(event, PAGE_DOWN)) {
      curs_pdn_count++;
   }
   else if (SO_KEY_PRESS_EVENT(event, PAGE_UP)) {
      curs_pup_count++;
   }
   else if (SO_KEY_PRESS_EVENT(event, NUMBER_0)){ 
      number_count[0]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, NUMBER_1)){ 
      number_count[1]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, NUMBER_2)){ 
      number_count[2]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, NUMBER_3)){ 
      number_count[3]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, NUMBER_4)){ 
      number_count[4]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, NUMBER_5)){ 
      number_count[5]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, NUMBER_6)){ 
      number_count[6]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, NUMBER_7)){ 
      number_count[7]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, NUMBER_8)){ 
      number_count[8]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, NUMBER_9)){ 
      number_count[9]++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, PAD_MULTIPLY)){ 
      mult_count++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, PAD_DIVIDE)){ 
      div_count++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, PAD_ADD)){ 
      add_count++;
   }  
   else if (SO_KEY_PRESS_EVENT(event, PAD_SUBTRACT)){ 
      sub_count++;
   }
   else if (SO_KEY_PRESS_EVENT(event, F1)){ 
      fun_count[1]++;
   }
   else if (SO_KEY_PRESS_EVENT(event, F2)){ 
      fun_count[2]++;
   }
   else if (SO_KEY_PRESS_EVENT(event, F3)){ 
      fun_count[3]++;
   }
   else if (SO_KEY_PRESS_EVENT(event, F4)){ 
      fun_count[4]++;
   }
   else if (SO_KEY_PRESS_EVENT(event, F5)){ 
      fun_count[5]++;
   }
   else if (SO_KEY_PRESS_EVENT(event, F6)){ 
      fun_count[6]++;
   }
   else if (SO_KEY_PRESS_EVENT(event, F7)){ 
      fun_count[7]++;
   }
   else if (SO_KEY_PRESS_EVENT(event, F8)){ 
      fun_count[8]++;
   }
   else if (SO_KEY_PRESS_EVENT(event, F9)){ 
      fun_count[9]++;
   }
   else if (SO_KEY_PRESS_EVENT(event, F10)){ 
      fun_count[10]++;
   }
   else if (SO_KEY_PRESS_EVENT(event, F11)){ 
      fun_count[11]++;
      if (clock_input != NULL) clock_input->start_clock();
   }
   else if (SO_KEY_PRESS_EVENT(event, F12)){ 
      fun_count[12]++;
      if (clock_input != NULL) clock_input->stop_clock();
   }
   else if (SO_KEY_PRESS_EVENT(event, ESCAPE)){ // Only works on PC
      std::cout << "To escape -- gracefully kill program" << std::endl;
      for (i=0; i<n_managers; i++) {
         delete atrlab_manager_a[i];
      }
      exit(1);
   }  
   else if (SO_KEY_PRESS_EVENT(event, BACKSPACE)){ 
      std::cout << "To backspace -- gracefully kill program" << std::endl;
      for (i=0; i<n_managers; i++) {
         delete atrlab_manager_a[i];
      }
      exit(1);
   }  
   // If you cant service, return 0
   else {
      return(0);
   }  

   // **************************************
   // Refresh managers, if you have registered them
   // **************************************
   for (i=0; i<n_managers; i++) {
      atrlab_manager_a[i]->refresh();
   }
   return(1);
}
   
// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void keyboard_input_class::key_cbx(void *userData, SoEventCallback *eventCB)
{
   const SoEvent *event = eventCB->getEvent();
   keyboard_input_class* keyt = (keyboard_input_class*)  userData;
   if (keyt->process_event(event)) {
      eventCB->setHandled();
   }
}

