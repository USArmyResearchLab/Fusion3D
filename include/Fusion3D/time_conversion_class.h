#ifndef _time_conversion_class_h_
#define _time_conversion_class_h_
#ifdef __cplusplus

/**
Does format conversions for time and date data.

Converts time data between character, integer and floating point formats.
There are many different character formats -- this includes several that have been encountered,
but probably not all of them.

*/
class time_conversion_class:public base_jfd_class{
 private:
   int time_i;			// time in integer -- inputs converted to this value
   float time_f;		// time in float -- inputs converted to this value
   char* time_c;		// time in char (hh.mm.ss)
   int year, month, day;	// date
   
   
 public:
   time_conversion_class();
   ~time_conversion_class();
   
   int set_char(const char *tchar);
   int set_int(int time);
   int set_float(float time);
   int set_date_mmddyyyy_slashes(const char *tchar);
   int set_las_standard(double time);
   int set_unix_time(double time);
   
   int get_int();
   float get_float();
   char* get_char();
   char* get_char2();
   char* get_char_kml();
   char* get_date_mmddyyyy_slashes();
};

#endif /* __cplusplus */
#endif /* _time_conversion_class_h_ */
