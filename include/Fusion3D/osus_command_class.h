#ifndef _osus_command_class_h_
#define _osus_command_class_h_	
#ifdef __cplusplus

/**
Issues commands in the OSUS Standard format to task sensors to take imagery.

A new thread is initialized, the command issued, a verification that the command was received by OSUS
(not necessarily executed) and the thread is destroyed.

*/
class osus_command_class:public base_jfd_class{
   
   private:
	   double lat;				///< Command parms -- lat
	   double lon;				///< Command parms -- lon
	   float elev;				///< Command parms -- elevation
	   string name;				///< Command parms -- name
	   string id;				///< Command parms -- id
	   string osus_addr;		///< Command parms -- OSUS addr
	   int osus_port;			///< Command parms -- OSUS port
	   SbThread *osusThread;	///< All commands issued within this thread

	  // Private methods
	   static void *command_nonex(void *userData);
	   static void *command_loc_idx(void *userData);
	   void command_none();
	   void command_loc_id();

   public:
      osus_command_class();
      ~osus_command_class();

	  int set_name(string name_in);
	  int set_addr(string addr_in);
	  int set_port(int port_in);
	  int set_loc(double lat_in, double lon_in, float elev_in);
	  int set_id(string id_in);

	  int command_osus(int type);

};

#endif /* __cplusplus */
#endif /* _osus_command_class_h_ */
