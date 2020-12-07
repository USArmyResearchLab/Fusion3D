#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
base_jfd_class::base_jfd_class()
{
#if defined(WIN32) 
   opsys_flag = 1;
#else
   opsys_flag = 0;
#endif
   
   diag_flag = 0;
   class_type = new char[30];
   strcpy(class_type, "base_jfd");
   localCoordFlag = -1;	// Unknown local coords
   gps_calc = NULL;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
base_jfd_class::~base_jfd_class()
{
}


// **********************************************
//// Clear all memory and reset parameters to initial values -- Virtual.
// **********************************************
int base_jfd_class::clear_all()
{
	return(1);
}

// ********************************************************************************
/// Read file: all functions including open, read header and data and close -- Virtual.
// ********************************************************************************
int base_jfd_class::read_file(string sfilename)
{
	cout << "WARNING -- virtual method base_jfd_class::read_file -- do nothing" << endl;
	return(1);
}

// ********************************************************************************
/// Read file: only the open part of the entire read-file -- Virtual.
// ********************************************************************************
int base_jfd_class::read_file_open(string sfilename)
{
	cout << "WARNING -- virtual method base_jfd_class::read_file_open -- do nothing" << endl;
	return(1);
}

// ********************************************************************************
/// Read file: only the read-header part of the entire read-file -- Virtual.
// ********************************************************************************
int base_jfd_class::read_file_header()
{
	cout << "WARNING -- virtual method base_jfd_class::read_file_header -- do nothing" << endl;
	return(1);
}

// ********************************************************************************
/// Read file: only the read-data part of the entire read-file -- Virtual.
// ********************************************************************************
int base_jfd_class::read_file_data()
{
	cout << "WARNING -- virtual method base_jfd_class::read_file_data -- do nothing" << endl;
	return(1);
}

// ********************************************************************************
/// Read file: only the close part of the entire read-file -- Virtual.
// ********************************************************************************
int base_jfd_class::read_file_close()
{
	cout << "WARNING -- virtual method base_jfd_class::read_file_close -- do nothing" << endl;
	return(1);
}

// ********************************************************************************
/// Write file: entire process of open, write header and data and close -- Virtual.
// ********************************************************************************
int base_jfd_class::write_file(string sfilename)
{
	cout << "WARNING -- virtual method base_jfd_class::write_file -- do nothing" << endl;
	return(1);
}


// ******************************************
/// Set level of diagnostics.
// ******************************************
int base_jfd_class::set_diag_flag(int flag)
{
   diag_flag = flag;
   return(1);
}

// ******************************************
/// Register the common gps_calc_class for defining the current coordinate systems and transformations between them.
/// A single gps_calc_class class should be held in common over all classes that need coordinate systems or transforms and should do all transforms.
// ******************************************
int base_jfd_class::register_coord_system(gps_calc_class *gps_calc_in)
{
   gps_calc = gps_calc_in;
   return(1);
}

// ******************************************
/// Open dialog window with warning.
/// Program pauses until dialog window is closed before proceeding.
/// @param oiv_window_flag 1 to open dialog window, 0 to only write message to text window.
/// @param message Message appearing in dialog window
// ******************************************
int base_jfd_class::warning(int oiv_window_flag, const char* message)
{
	std::cout << message << std::endl;
	if (oiv_window_flag) {
#if defined(LIBS_QT)
		QString tt = message;
		QMessageBox::warning(NULL, "Warning", tt, QMessageBox::Ok, QMessageBox::Ok);	// Warning -- no user action
#elif defined(LIBS_SOWIN)
		Widget topWidget = SoWin::getTopLevelWidget();
		SoWin::createSimpleErrorDialog(topWidget, "WARNING", message);
		// CString cMessage(message);
		// MessageBox(topWidget,cMessage,L"WARNING",0);
#else 
		cout << "WARNING " << message << endl;
#endif
	}
	return(1);
}

// ******************************************
/// Open dialog window with 2-part warning message.
/// Program pauses until dialog window is closed before proceeding.
/// @param cmessage		First part of message appearing in dialog window -- eg char string like "cant open file"
/// @param smessage		Second part of message appearing in dialog window -- eg a filename
// ******************************************
int base_jfd_class::warning_s(string cmessage, string smessage)
{
	std::cout << "WARNING " << cmessage<< " " << smessage << std::endl;
#if defined(LIBS_QT)
	QString tt = QString::fromStdString(cmessage);
	tt.append(" ");
	tt.append(QString::fromStdString(smessage));
	QMessageBox::warning(NULL, "Warning", tt, QMessageBox::Ok, QMessageBox::Ok);	// Warning -- no user action
#elif defined(LIBS_SOWIN)
	Widget topWidget = SoWin::getTopLevelWidget();
	string message = cmessage;
	message.append(" ");
	message.append(smessage);
	SoWin::createSimpleErrorDialog(topWidget, "WARNING", message.c_str());
#endif
	return(1);
}

// ******************************************
/// Open dialog window with fatal error message.
/// Program pauses until dialog window is closed before proceeding.
/// @param oiv_window_flag 1 to open dialog window, 0 to only write message to text window (use before OpenInventor initialized).
/// @param message Message appearing in dialog window
// ******************************************
int base_jfd_class::exit_safe(int oiv_window_flag, const char* message)
{
	std::cout << "Unrecoverable error " << message << " -- exit" << std::endl;

	// If OIV libraries, open a window to display error
	if (oiv_window_flag) {
#if defined(LIBS_QT)
		QString tt = message;
		QMessageBox::warning(NULL, "FATAL ERROR", tt, QMessageBox::Ok, QMessageBox::Ok);	// Warning -- no user action
#elif defined(LIBS_SOWIN)
		Widget topWidget = SoWin::getTopLevelWidget();
		SoWin::createSimpleErrorDialog(topWidget, "FATAL ERROR", message);
		// CString cMessage(message);
		// MessageBox(topWidget,cMessage,L"FATAL ERROR",0);
#else 
		cout << "FATAL ERROR " << message << endl;
#endif
	}

	// If Windows and no dialog box, ensure that text window stays open while you read message
	if (opsys_flag == 1 && oiv_window_flag == 0) {
		char ctemp[10];
		std::cout << "Enter any character + enter to close window" << std::endl;
		cin >> ctemp;
	}
	exit(96);
	return(1);
}

// ******************************************
/// Open dialog window with fatal error message.
/// Program pauses until dialog window is closed before proceeding.
/// @param oiv_window_flag 1 to open dialog window, 0 to only write message to text window (use before OpenInventor initialized).
/// @param message Message appearing in dialog window
// ******************************************
int base_jfd_class::exit_safe_s(string cmessage, string smessage)
{
	std::cout << "Unrecoverable error " << cmessage << " " << smessage << " -- exit" << std::endl;

	// If OIV libraries, open a window to display error
#if defined(LIBS_QT)
	QString tt = QString::fromStdString(cmessage);
	tt.append(" ");
	tt.append(QString::fromStdString(smessage));
	QMessageBox::warning(NULL, "FATAL ERROR", tt, QMessageBox::Ok, QMessageBox::Ok);	// Warning -- no user action
#elif defined(LIBS_SOWIN)
	Widget topWidget = SoWin::getTopLevelWidget();
	string message = cmessage;
	message.append(" ");
	message.append(smessage);
	SoWin::createSimpleErrorDialog(topWidget, "FATAL ERROR", message.c_str());
#else 
	cout << "FATAL ERROR " << cmessage << " " << smessage << endl;
#endif
	exit(96);
	return(1);
}

// *******************************************
/// Get the name of the class.
/// Usually the type of the data it manages e.q. "KML".
// *******************************************
const char* base_jfd_class::get_type()
{
   return class_type;
}

// *******************************************
/// Write parameters to the currently opened ASCII file -- for saving all relevent parameters to an output Parameter file.
/// Assumes that the file has already been opened and will be closed outside this class.
// *******************************************
int base_jfd_class::write_parms(FILE *tiff_fd)
{
	std::cout << "base_jfd_class::write_parms: Dummy write for " << class_type << std::endl;
   return(0);
}


