#ifndef _nato4676_class_h_
#define _nato4676_class_h_	
#ifdef __cplusplus

/*
** GetUxx_BE() - Get unsigned xx-bit integer big-endian
*/


/**
   Imports target track and sensor footprint metadata in NATO 4676 format.

*/
class nato4676_class:public sensor_read_class{
   
	private:
	  std::string level0;					///< Working -- Stores entire file with comments erased (parsers may get bogus info from comments)
	  float footprint_verts[8];				///< Working -- 4 points defining footprint (x1, y1, x2, y2, x3, y3, x4, y4)
      
	  string mcast_addr;								///< Multicast -- multicast address (like 239.0.5.5)
	  int mcast_port;									///< Multicast -- multicast port (like 1234)
	  SbThread *mcast_thread;							///< Multicast -- all multicast done in this thread so it doesnt hang main thread
	  char *mcast_data;									///< Multicast -- data buffer
	  enum { mcast_max_length = 3000 };					///< Multicast -- data buffer max length DIFFERENT FROM AUSTRALIA 
#if defined(LIBS_BOOST)
	  boost::asio::ip::udp::endpoint sender_endpoint;	///< Multicast -- 
	  boost::asio::ip::udp::socket msocket;				///< Multicast --
	  boost::asio::io_service *pio_service;				///< Multicast --
#endif

	  // Private methods
	  int parse_string(std::string inString);
	  int find_substring_within(std::string stringIn, std::string &stringOut, std::string stringBeg, std::string stringEnd, int &n_pos);
	  static void *thread_methodx(void *userData);
	  void thread_method();

    public:
#if defined(LIBS_BOOST)
		nato4676_class(boost::asio::io_service& io_service);
#else
		nato4676_class();
#endif
		~nato4676_class();


		int join_multicast(string addr, int port);
#if defined(LIBS_BOOST)
		void get_multicast_datagram(const boost::system::error_code& error, size_t bytes_recvd);
#endif

	  int read_file(string sfilename) override;
};

#endif /* __cplusplus */
#endif /* _nato4676_class_h_ */
