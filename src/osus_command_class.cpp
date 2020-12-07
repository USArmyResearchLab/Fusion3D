#include "internals.h"

// *******************************************
/// Constructor.
// *******************************************
osus_command_class::osus_command_class()
	:base_jfd_class()
{
	lat = 0;
	lon = 0;
	elev = 0.;
	id = "N/A";
	name = "unknown";
	osus_addr = "unknown";
	int port = 0;
}

// *************************************************************
/// Destructor.
// *************************************************************

osus_command_class::~osus_command_class()
{
}


// *******************************************
/// Set sensor name required by all commands.
// *******************************************
int osus_command_class::set_name(string name_in)
{
	name = name_in;
	return(1);
}

// *******************************************
/// Set location for command that requires one.
// *******************************************
int osus_command_class::set_loc(double lat_in, double lon_in, float elev_in)
{
	lat = lat_in;
	lon = lon_in;
	elev = elev_in;
	return(1);
}

// *******************************************
/// Set ID for command that requires one.
// *******************************************
int osus_command_class::set_id(string id_in)
{
	id = id_in;
	return(1);
}

// *******************************************
/// Set ID for command that requires one.
// *******************************************
int osus_command_class::set_addr(string addr_in)
{
	osus_addr = addr_in;
	return(1);
}

// *******************************************
/// Set ID for command that requires one.
// *******************************************
int osus_command_class::set_port(int port_in)
{
	osus_port = port_in;
	return(1);
}

// *******************************************
/// Issue command to OSUS to capture an image.
/// @parm type 1 for CapturImageCommand only, 2 to include pointing command.
// *******************************************
int osus_command_class::command_osus(int type)
{
	if (id.compare("unknown") == 0 || name.compare("unknown") == 0 || osus_addr.compare("unknown") == 0) {
		warning(0, "osus_command_class::command_osus:  Not enough parms defined to issue OSUS command");
		return(0);
	}

	if (type == 1) {
		osusThread = SbThread::create(command_nonex, (void*)this);
	}
	else if (type == 2) {
		osusThread = SbThread::create(command_loc_idx, (void*)this);
	}
	return(1);
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void *osus_command_class::command_nonex(void *userData)
{
	osus_command_class* kmlt = (osus_command_class*)userData;
	kmlt->command_none();
	return NULL;
}

// ********************************************************************************
// Callback wrapper so that can be called from within class
// If callback in main program, you can put anything into clientData, not just 'this'
// ********************************************************************************
void *osus_command_class::command_loc_idx(void *userData)
{
	osus_command_class* kmlt = (osus_command_class*)userData;
	kmlt->command_loc_id();
	return NULL;
}

// *******************************************
/// Issue CaptureImageCommand without the setPointingLocationCommand -- Private.
/// A 'sensorId' field is included as required by DragonFly -- other sensors should ignore this field.
/// First composes request -- uses proper OSUS standard systax, so should be extensible.
/// Opens socket to communicate with OSUS server,  sends request, receives a verification.
/// If verification string indicates no errors, close socket and exit.
// *******************************************
void osus_command_class::command_none()
{
	int port = 0, pid = 0;
	size_t inlen, outlen;
	char temp[300];

	// *************************************************************
	// Compose command CaptureImageCommand
	// *************************************************************
	string commandc = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
	commandc.append("<CaptureImageCommand xmlns:ns2=\"commands.asset.core.th.dod.mil\" xmlns:ns3=\"http://th.dod.mil/core/types/image\" xmlns:ns4=\"http://th.dod.mil/core/types/spatial\" xmlns:ns5=\"http://th.dod.mil/core/types\"");
	commandc.append(" sensorId=\"");
	commandc.append(id);
	commandc.append("\">\n");
	commandc.append("    <ns2:reserved>\n");
	commandc.append("        <ns5:key>AssetName</ns5:key>\n");
	commandc.append("        <ns5:value xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" xsi:type=\"xs:string\">");
	commandc.append(name);
	commandc.append("</ns5:value>\n");
	commandc.append("    </ns2:reserved>\n");
	commandc.append("</CaptureImageCommand>\n");
	if (diag_flag > 0) fprintf(stdout, "%s\n\n\n", commandc.c_str());

	// *************************************************************
	// Open socket to talk to OSUS
	// *************************************************************
#if defined(LIBS_BOOST)
	boost::array<char, 1> bufin;
	bufin[0] = '\0';
	boost::array<char, 500> bufout;
	boost::system::error_code error;
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::socket socket(io_service);
	boost::asio::ip::address address = boost::asio::ip::address::from_string(osus_addr);
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(osus_addr), osus_port); // Only looks for 1 place to connect -- may need to look for several (see below)
	socket.connect(endpoint, error);
	if (error) {
		sprintf(temp, "Cant connect to OSUS server at %s, port %d\n", osus_addr.c_str(), osus_port);
		warning(1, temp);
		SbThread::destroy(osusThread);
		return;
	}
	if (diag_flag > 0) cout << "Get MV at clicked loc -- Connected to OSUS server at addr " << osus_addr << ":" << osus_port << endl;

	// *************************************************************
	// Send command to capture image
	// *************************************************************
	try {
		inlen = socket.write_some(boost::asio::buffer(commandc), error);
		if (error) {
			throw boost::system::system_error(error); // Actual error
		}
		inlen = socket.write_some(boost::asio::buffer(bufin), error);
		if (error) {
			throw boost::system::system_error(error); // Actual error
		}
		if (diag_flag > 0) cout << "Get MV at clicked loc -- sent CaptureImageCommand to OSUS server" << endl;

		outlen = socket.read_some(boost::asio::buffer(bufout), error);		// Should send official OSUS response in CaptureImageResponse.xml
		if (error == boost::asio::error::eof) {
			// OK, connection closed cleanly by server
		}
		else if (error) {
			throw boost::system::system_error(error); // Actual error
		}
		string verifyImg(bufout.begin(), bufout.end());
		if (diag_flag > 0) fprintf(stdout, "%s\n\n\n", verifyImg.c_str());
		if (verifyImg.find("ERROR:") != string::npos) {
			warning(1, "OSUS server returns error for set-position command");
		}
		else {
			if (diag_flag > 0) cout << "Get MV at clicked loc -- OSUS server verified received" << endl;
		}
	}

	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		sprintf(temp, "Cant read/write capture-image command to OSUS server at %s, port %d %s ", osus_addr.c_str(), osus_port, e.what());
		warning(1, temp);
		socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		socket.close();
		SbThread::destroy(osusThread);
		return;
	}

	// *************************************************************
	// Cleanup -- dont need either socket or this thread any more
	// *************************************************************
	try {
		socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		socket.close();
		SbThread::destroy(osusThread);
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		sprintf(temp, "Cant shutdown socket to OSUS server -- error %s ", e.what());
		warning(1, temp);
		// socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		socket.close();
		SbThread::destroy(osusThread);
		return;
	}
#endif
	return;
}

// *******************************************
/// Issue command that requires sensor name, pointing location and sensor id -- Private.
/// A 'sensorId' field is included in the CaptureImageCommand as required by DragonFly -- other sensors should ignore this field.
/// The setPointingLocationCommand is included to request an image at a particular location for remote sensors that can be aimed.
/// First composes request -- uses proper OSUS standard systax, so should be extensible.
/// Opens socket to communicate with OSUS server,  sends request, receives a verification.
/// If verification string indicates no errors, close socket and exit.
// *******************************************
void osus_command_class::command_loc_id()
{
	int port = 0, pid = 0;
	size_t inlen, outlen;
	char temp[300];

	// *************************************************************
	// Compose command setPointingLocationCommand -- AF FMV, AF WAMI and MX-20 should all use this command (SetPostionCommand should be superceded)
	// *************************************************************
	string commands = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
	commands.append("<SetPointingLocationCommand xmlns:ns2=\"commands.asset.core.th.dod.mil\" xmlns:ns3=\"http://th.dod.mil/core/types/spatial\" xmlns:ns4=\"http://th.dod.mil/core/types/image\" xmlns:ns5=\"http://th.dod.mil/core/types\">\n");
	commands.append("    <ns2:reserved>\n");
	commands.append("        <ns5:key>AssetName</ns5:key>\n");
	commands.append("        <ns5:value xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" xsi:type=\"xs:string\">");
	commands.append(name);
	commands.append("</ns5:value>\n");
	commands.append("    </ns2:reserved>\n");
	commands.append("    <ns2:location>\n");
	commands.append("        <ns3:longitude>");
	commands.append(to_string(lon));
	commands.append("</ns3:longitude>\n");
	commands.append("        <ns3:latitude>");
	commands.append(to_string(lat));
	commands.append("</ns3:latitude>\n");
	commands.append("        <ns3:altitude>");
	commands.append(to_string(elev));
	commands.append("</ns3:altitude>\n");
	commands.append("    </ns2:location>\n");
	commands.append("</SetPointingLocationCommand>\n");
	if (diag_flag > 1) fprintf(stdout, "%s\n\n\n", commands.c_str());

	// *************************************************************
	// Compose command CaptureImageCommand
	// *************************************************************
	string commandc = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
	commandc.append("<CaptureImageCommand xmlns:ns2=\"commands.asset.core.th.dod.mil\" xmlns:ns3=\"http://th.dod.mil/core/types/image\" xmlns:ns4=\"http://th.dod.mil/core/types/spatial\" xmlns:ns5=\"http://th.dod.mil/core/types\"");
		commandc.append(" sensorId=\"");
		commandc.append(id);
		commandc.append("\">\n");
	commandc.append("    <ns2:reserved>\n");
	commandc.append("        <ns5:key>AssetName</ns5:key>\n");
	commandc.append("        <ns5:value xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" xsi:type=\"xs:string\">");
	commandc.append(name);
	commandc.append("</ns5:value>\n");
	commandc.append("    </ns2:reserved>\n");
	commandc.append("</CaptureImageCommand>\n");
	if (diag_flag > 1) fprintf(stdout, "%s\n\n\n", commandc.c_str());

	// *************************************************************
	// Open socket to talk to OSUS
	// *************************************************************
#if defined(LIBS_BOOST)
	boost::array<char, 1> bufin;
	bufin[0] = '\0';
	boost::array<char, 500> bufout;
	boost::system::error_code error;
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::socket socket(io_service);
	boost::asio::ip::address address = boost::asio::ip::address::from_string(osus_addr);
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(osus_addr), osus_port); // Only looks for 1 place to connect -- may need to look for several (see below)
	socket.connect(endpoint, error);
	if (error) {
		sprintf(temp, "Cant connect to OSUS server at %s, port %d\n", osus_addr.c_str(), osus_port);
		warning(1, temp);
		SbThread::destroy(osusThread);
		return;
	}
	if (diag_flag > 0) cout << "Get MV at clicked loc -- Connected to OSUS server at addr " << osus_addr << ":" << osus_port << endl;

	// *************************************************************
	// Send command to set position
	// *************************************************************
	try {
		// More general way to establish connection -- tries multiple endpoints -- I dont think I need this
		// tutorial boost::asio::ip::tcp::resolver resolver(io_service);
		// tutorial boost::asio::ip::tcp::resolver::query query(url, portc, boost::asio::ip::resolver_query_base::numeric_host);
		// tutorial boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		// tutorial boost::asio::ip::tcp::socket socket(io_service);
		// tutorial boost::asio::connect(socket, endpoint_iterator);

		inlen = socket.write_some(boost::asio::buffer(commands), error);
		if (error) {
			throw boost::system::system_error(error); // Actual error
		}
		inlen = socket.write_some(boost::asio::buffer(bufin), error);
		if (error) {
			throw boost::system::system_error(error); // Actual error
		}
		if (diag_flag > 0) cout << "Get MV at clicked loc -- sent setPositionCommand to OSUS server" << endl;

		outlen = socket.read_some(boost::asio::buffer(bufout), error);		// Should send official OSUS response in GetPositionResponse.xml
		if (error == boost::asio::error::eof) {
			// OK, connection closed cleanly by server
		}
		else if (error) {
			throw boost::system::system_error(error); // Actual error
		}
		string verifyPos(bufout.begin(), bufout.end());	// Transfer weird boost array to string
		if (diag_flag > 0) fprintf(stdout, "%s\n\n\n", verifyPos.c_str());
		if (verifyPos.find("ERROR:") != string::npos) {
			warning(1, "OSUS server returns error for set-position command");
		}
		else {
			if (diag_flag > 0) cout << "Get MV at clicked loc -- OSUS server verified received" << endl;
		}
	}

	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		sprintf(temp, "Cant read/write set-postion command to OSUS server at %s, port %d %s ", osus_addr.c_str(), osus_port, e.what());
		warning(1, temp);
		socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		socket.close();
		SbThread::destroy(osusThread);
		return;
	}

	// *************************************************************
	// Send command to capture image
	// *************************************************************
	try {
		inlen = socket.write_some(boost::asio::buffer(commandc), error);
		if (error) {
			throw boost::system::system_error(error); // Actual error
		}
		inlen = socket.write_some(boost::asio::buffer(bufin), error);
		if (error) {
			throw boost::system::system_error(error); // Actual error
		}
		if (diag_flag > 0) cout << "Get MV at clicked loc -- sent CaptureImageCommand to OSUS server" << endl;

		outlen = socket.read_some(boost::asio::buffer(bufout), error);		// Should send official OSUS response in CaptureImageResponse.xml
		if (error == boost::asio::error::eof) {
			// OK, connection closed cleanly by server
		}
		else if (error) {
			throw boost::system::system_error(error); // Actual error
		}
		string verifyImg(bufout.begin(), bufout.end());
		if (diag_flag > 0) fprintf(stdout, "%s\n\n\n", verifyImg.c_str());
		if (verifyImg.find("ERROR:") != string::npos) {
			warning(1, "OSUS server returns error for set-position command");
		}
		else {
			if (diag_flag > 0) cout << "Get MV at clicked loc -- OSUS server verified received" << endl;
		}
	}

	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		sprintf(temp, "Cant read/write capture-image command to OSUS server at %s, port %d %s ", osus_addr.c_str(), osus_port, e.what());
		warning(1, temp);
		socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		socket.close();
		SbThread::destroy(osusThread);
		return;
	}

	// *************************************************************
	// Cleanup -- dont need either socket or this thread any more
	// *************************************************************
	try {
		socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		socket.close();
		SbThread::destroy(osusThread);
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		sprintf(temp, "Cant shutdown socket to OSUS server -- error %s ", e.what());
		warning(1, temp);
		// socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		socket.close();
		SbThread::destroy(osusThread);
		return;
	}
#endif
	return;
}

