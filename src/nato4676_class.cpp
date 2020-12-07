#include "internals.h"

// *******************************************
/// Constructor that includes Boost for Multicast.
// *******************************************
#if defined(LIBS_BOOST)
nato4676_class::nato4676_class(boost::asio::io_service& io_servicet)
	:sensor_read_class(), msocket(io_servicet)
{
	strcpy(class_type, "nato4676");
	om_target_flag = 1;					// There should always be targets present
	om_sensor_loc_flag = 1;				// There should always be sensor locs present, but only store most current
	om_footprint_flag = 1;				// There should always be footprint present, but only store most current
	// om_sensor_loc_flag = 2;				// There should always be sensor locs present, store all
	// om_footprint_flag = 2;				// There should always be footprint present, store all

	pio_service  = &io_servicet;
}

// *******************************************
/// Constructor that does not include Boost for multicast.
// *******************************************
#else
nato4676_class::nato4676_class()
	:sensor_read_class()
{
   strcpy(class_type, "nato4676");
}
#endif

// *************************************************************
/// Destructor.
// *************************************************************

nato4676_class::~nato4676_class()
{
}

// *******************************************
/// Join a multicast.
/// Does not throw an error if nothing is being multicasted on the address/port given -- just blocks until something shows up there.
/// @param addr URL for multicast (like "239.0.5.5")
/// @param port	port number (like 1234)
// *******************************************
int nato4676_class::join_multicast(string addr, int port)
{
	mcast_addr = addr;
	mcast_port = port;
	mcast_thread = SbThread::create(thread_methodx, (void*)this);
	return(1);
}

// **********************************************
// 
// **********************************************
void *nato4676_class::thread_methodx(void *userData)
{
	nato4676_class* nato4676t = (nato4676_class*)userData;
	nato4676t->thread_method();
	return NULL;
}

// *******************************************
///  Thread method to join multicast -- Private
// *******************************************
void nato4676_class::thread_method()
{
#if defined(LIBS_BOOST)
	mcast_data = new char[mcast_max_length];
	boost::asio::ip::udp::endpoint listen_endpoint(boost::asio::ip::address::from_string("0.0.0.0"), mcast_port);
	msocket.open(listen_endpoint.protocol());
	msocket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
	msocket.bind(listen_endpoint);

	// Join multicast group
	boost::asio::ip::address baddr = boost::asio::ip::address::from_string(mcast_addr);
	msocket.set_option(boost::asio::ip::multicast::join_group(boost::asio::ip::address::from_string(mcast_addr)));
	msocket.async_receive_from(boost::asio::buffer(mcast_data, mcast_max_length), sender_endpoint,
		boost::bind(&nato4676_class::get_multicast_datagram, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

	pio_service->run();
#endif
}

#if defined(LIBS_BOOST)
// *******************************************
///  Read a upd datagram.
// *******************************************
void nato4676_class::get_multicast_datagram(const boost::system::error_code& error, size_t bytes_recvd)
{
	if (!error) {
		//std::cout.write(mcast_data, bytes_recvd);
		//std::cout << std::endl;
		if (diag_flag > 2) cout << "Read 4676 observation" << n_om << endl;
		string mcast_string = mcast_data;
		parse_string(mcast_string);
	}
	else {
		cout << error.message() <<  " -- ";
		warning(0, "nato4676_class cant read multicast datagram");
	}

	msocket.async_receive_from(boost::asio::buffer(mcast_data, mcast_max_length), sender_endpoint,
		boost::bind(&nato4676_class::get_multicast_datagram, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	return;
}
#endif

// *******************************************
/// Read from file.
// *******************************************
int nato4676_class::read_file(string sfilename)

{
	// ************************************************************
	// Open file and read entire file into a string
	// ************************************************************
	std::ifstream is (sfilename);
	if(!is) {
		warning_s("nato4676_class::read_file:  Cant open input file", sfilename);
      return(0);
	}
	is.seekg(0, is.end);
	int length = is.tellg();
	is.seekg(0, is.beg);
	char buf[4096];
	if (length < 4096) {
		is.read(buf, length);
		level0.append(buf, length);
	}
	else {
		while (is.read(buf, sizeof(buf)))
			level0.append(buf, sizeof(buf));
		level0.append(buf, is.gcount());
	}
	if (diag_flag > 2) std::cout << level0 << endl;
	if (diag_flag > 0) std::cout << "nato4676_class::read_file:  read " << sfilename.c_str() << ", length " << length << endl;
	parse_string(level0);
	return(1);
}

// *******************************************
/// Read from file.
// *******************************************
int nato4676_class::parse_string(std::string inString)
{
	double lat, lon, north, east;
	float elev, target_dn, target_de, target_del, sensor_dn, sensor_de, sensor_delev;
	int ntemp, target_present_flag=0, sensor_present_flag=0, foot_present_flag=0, priority_flag;
	float footprint_verts[8];				// 4 points defining footprint (x1, y1, x2, y2, x3, y3, x4, y4)
	string time_in, pos_in, itemString, stemp;

	// ************************************************************
	// Loop over dwells 
	// ************************************************************
	int dnoff, noff = 0;
	while (find_substring_within(inString.substr(noff), itemString, "<items xsi:type=\"TrackPoint\">",  "</items>",  dnoff)) {	// Should contain all info for a single dwell
		noff = noff + dnoff;

		// ************************************************************
		// Get time
		// ************************************************************
		if (find_substring_within(itemString, time_in, "<trackItemTime>",  "</trackItemTime>",  ntemp)){	// 
		}
		else {
			time_in = "unknown";
		}

		// ************************************************************
		// Get target location
		// ************************************************************
		if (find_substring_within(itemString, pos_in, "<trackPointPosition xsi:type=\"GeodeticPosition\">",  "</trackPointPosition>",  ntemp)) {	// 
			std::string::size_type sz;
			find_substring_within(pos_in, stemp, "<latitude>",  "</latitude>",  ntemp);
			lat = std::stod(stemp, &sz) + sensor_offset_lat;
			find_substring_within(pos_in, stemp, "<longitude>", "</longitude>", ntemp);
			lon  = std::stod(stemp, &sz) + sensor_offset_lon;
			find_substring_within(pos_in, stemp, "<elevation>", "</elevation>", ntemp);
			elev = std::stof(stemp, &sz)+ sensor_offset_elev;
			gps_calc->ll_to_proj(lat, lon, north, east); 
			target_dn  = float(north - gps_calc->get_ref_utm_north());
			target_de  = float(east  - gps_calc->get_ref_utm_east());
			target_del =       elev  - gps_calc->get_ref_elevation();
		}
		else {
			pos_in = "unknown";
		}

		// ************************************************************
		// Get sensor location and sensor footprint -- assumes comma-separated values, all there and in right order
		// ************************************************************
		if (om_sensor_loc_flag > 0 || om_footprint_flag > 0) {
			if (find_substring_within(itemString, pos_in, "<trackIdentityInformation>", "</trackIdentityInformation>", ntemp)) {	// 
				std::string::size_type sz, sz2, sz3;
				if (find_substring_within(pos_in, stemp, "<trackComment>", "</trackComment>", ntemp)) {
					// Sensor loc
					lat = std::stod(stemp, &sz) + sensor_offset_lat;
					lon = std::stod(stemp.substr(sz+1), &sz2) + sensor_offset_lon;				// +1 accounts for comma
					elev = std::stof(stemp.substr(sz+sz2+2), &sz3) + sensor_offset_elev;
					gps_calc->ll_to_proj(lat, lon, north, east);
					sensor_dn = float(north - gps_calc->get_ref_utm_north());
					sensor_de = float(east - gps_calc->get_ref_utm_east());
					sensor_delev = elev - gps_calc->get_ref_elevation();
					sensor_present_flag = 1;

					// Sensor footprint
					sz = sz + sz2 + sz3 + 3;
					lat = std::stod(stemp.substr(sz), &sz2) + sensor_offset_lat;
					lon = std::stod(stemp.substr(sz + sz2 + 1), &sz3) + sensor_offset_lon;
					gps_calc->ll_to_proj(lat, lon, north, east);
					footprint_verts[0] = float(east - gps_calc->get_ref_utm_east());
					footprint_verts[1] = float(north - gps_calc->get_ref_utm_north());

					sz = sz + sz2 + sz3 + 2;
					lat = std::stod(stemp.substr(sz), &sz2) + sensor_offset_lat;
					lon = std::stod(stemp.substr(sz + sz2 + 1), &sz3) + sensor_offset_lon;
					gps_calc->ll_to_proj(lat, lon, north, east);
					footprint_verts[2] = float(east - gps_calc->get_ref_utm_east());
					footprint_verts[3] = float(north - gps_calc->get_ref_utm_north());

					sz = sz + sz2 + sz3 + 2;
					lat = std::stod(stemp.substr(sz), &sz2) + sensor_offset_lat;
					lon = std::stod(stemp.substr(sz + sz2 + 1), &sz3) + sensor_offset_lon;
					gps_calc->ll_to_proj(lat, lon, north, east);
					footprint_verts[4] = float(east - gps_calc->get_ref_utm_east());
					footprint_verts[5] = float(north - gps_calc->get_ref_utm_north());

					sz = sz + sz2 + sz3 + 2;
					lat = std::stod(stemp.substr(sz), &sz2) + sensor_offset_lat;
					lon = std::stod(stemp.substr(sz + sz2 + 1), &sz3) + sensor_offset_lon;
					gps_calc->ll_to_proj(lat, lon, north, east);
					footprint_verts[6] = float(east - gps_calc->get_ref_utm_east());
					footprint_verts[7] = float(north - gps_calc->get_ref_utm_north());
					foot_present_flag = 1;

				}
				else {		// This should be covered in reasonability checks below ??
				}
			}
			else {			// This should be covered in reasonability checks below ??
			}
		}

		// ************************************************************
		// Set priority flag
		// ************************************************************
		if (itemString.find("SUSPECT") != string::npos) {
			priority_flag = 1;
		}
		else {
			priority_flag = 0;
		}

		// ************************************************************
		// Do some reasonability tests.  If not, return 0
		// ************************************************************
		if (om_sensor_loc_flag && !sensor_present_flag) {					// Should be for all dwells or none
			warning(1, "nato4676_class::parse_string:  Sensor loc missing");
			return(0);
		}
		if (om_footprint_flag && !foot_present_flag) {						// Should be for all dwells or none
			warning(0, "nato4676_class::parse_string:  footprints missing");
			return(0);
		}
	
	
		// ************************************************************
		// All inputs present -- process
		// ************************************************************
		char *omtime = new char[time_in.length()+1];
		strcpy(omtime, time_in.c_str());
		time_conversion->set_char(omtime);
		mutex_lock();
		float om_timet = time_conversion->get_float();
		float dtime_day = 24 * 60 * 60;		// Seconds per day
		if (om_time.size() > 0 && om_timet < om_time[0] - 120.) om_timet = om_timet + dtime_day;	// If you cross day boundary gmt, add a day to time 
		om_time.push_back(time_conversion->get_float());

		om_target_deast.push_back(target_de);
		om_target_dnorth.push_back(target_dn);

		if (om_sensor_loc_flag == 0) {
		}
		else if (om_sensor_loc_flag == 1 && om_sensor_dnorth.size() > 0) {
			om_sensor_dnorth[0] = sensor_dn;
			om_sensor_deast[0] = sensor_de;
			om_sensor_delev[0] = sensor_delev;
		}
		else  {
			om_sensor_dnorth.push_back(sensor_dn);
			om_sensor_deast.push_back(sensor_de);
			om_sensor_delev.push_back(sensor_delev);
		}

		if (om_footprint_flag == 0) {
		}
		else if (om_footprint_flag == 1 && om_foot_deast.size() > 0) {
			for (int i = 0; i < 4; i++) {
				om_foot_deast[i]  = footprint_verts[2 * i];
				om_foot_dnorth[i] = footprint_verts[2 * i + 1];
			}
		}
		else {
			for (int i = 0; i < 4; i++) {
				om_foot_deast.push_back(footprint_verts[2 * i]);
				om_foot_dnorth.push_back(footprint_verts[2 * i + 1]);
			}
		}

		om_flags.push_back(priority_flag);
		n_om++;
		mutex_unlock();
	}

	return(1);   
  
}

// *************************************************************
// Find the substring within the two specified strings, NOT including the specified strings.
///@param stringIn		Input string
///@param stringOut		Output string (not including search strings)
///@param stringBeg		Beginning string for search
///@param stringEnd		End string for search
///@param n_offset		Output offset from beginning of input string to end of end search string
///@return				1 if string found, 0 if not  
// *************************************************************
int nato4676_class::find_substring_within(std::string stringIn, std::string &stringOut, std::string stringBeg, std::string stringEnd, int &n_offset)
{
	size_t istart = stringIn.find(stringBeg);
	if (istart == string::npos) return(0);
	size_t beglen = stringBeg.length();

	size_t istop = stringIn.find(stringEnd, istart + stringBeg.length() + 1);
	if (istop == string::npos) return(0);
	size_t endlen = stringEnd.length();

	stringOut = stringIn.substr(istart+beglen, istop-istart-beglen);		// Including both beginning and ending strings/tags
	n_offset = istop + endlen;
	return(1);   
}


