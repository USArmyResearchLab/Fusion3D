#ifndef _internals_h_
#define _internals_h_

/** \mainpage 3-D Fusion Viewer Map3dview

\section intro_sec Introduction 

\verbinclude map3dviewDocIntro.txt

\section Index Index 

An index of general topics is
<A HREF="../../AdditionalDocFiles/Map3dviewDocIndex.html">here</A>.


*/

/**
Contains all include files for program.

*/
#ifdef __cplusplus

#if defined(LIBS_QT) 
#else 
	#include "stdafx.h"		// Has to be done early or conflicts
#endif

#if defined(LIBS_COIN) 
	#if defined(LIBS_SOQT) 
		#include <Inventor/Qt/SoQt.h>
		#include <Inventor/Qt/SoQtRenderArea.h>
		#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
	#elif defined(LIBS_SOWIN)
		#include <Inventor/Win/SoWin.h>
		#include <Inventor/Win/viewers/SoWinExaminerViewer.h>
		#include <Inventor/Win/viewers/SoWinBitmapButton.h>
   #endif
   #include <Inventor/threads/SbThread.h>
   #include <Inventor/threads/SbMutex.h>
   #include <Inventor/events/SoMotion3Event.h>
   #include <Inventor/events/SoSpaceballButtonEvent.h>
#endif


#if defined(LIBS_COIN) 
   #include <Inventor/manips/SoTrackballManip.h>
   #include <Inventor/nodes/SoComplexity.h>
   #include <Inventor/nodes/SoCoordinate3.h>
   #include <Inventor/nodes/SoCube.h>
   #include <Inventor/nodes/SoDirectionalLight.h>
   #include <Inventor/nodes/SoDrawStyle.h>
   #include <Inventor/nodes/SoEventCallback.h>
   #include <Inventor/nodes/SoFont.h>
   #include <Inventor/nodes/SoQuadMesh.h>
   #include <Inventor/nodes/SoFaceSet.h>
   #include <Inventor/nodes/SoIndexedFaceSet.h>
   #include <Inventor/nodes/SoIndexedLineSet.h>
   #include <Inventor/nodes/SoPointSet.h>

   #include <Inventor/nodes/SoLightModel.h>
   #include <Inventor/nodes/SoLineSet.h>
   #include <Inventor/nodes/SoBaseColor.h>
   #include <Inventor/nodes/SoMaterial.h>
   #include <Inventor/nodes/SoMaterialBinding.h>
   #include <Inventor/nodes/SoOrthographicCamera.h>
   #include <Inventor/nodes/SoPerspectiveCamera.h>
   #include <Inventor/nodes/SoSeparator.h>
   #include <Inventor/nodes/SoSelection.h>
   #include <Inventor/nodes/SoSwitch.h>
   #include <Inventor/nodes/SoLOD.h>
   #include <Inventor/nodes/SoLevelOfDetail.h>
   #include <Inventor/nodes/SoTranslation.h>
   #include <Inventor/nodes/SoTexture2.h>
   #include <Inventor/nodes/SoTextureCoordinate2.h>
   #include <Inventor/nodes/SoText2.h>
   #include <Inventor/nodes/SoText3.h>
   #include <Inventor/nodes/SoColorIndex.h>
   #include <Inventor/nodes/SoAnnotation.h>
   #include <Inventor/nodes/SoRotationXYZ.h>
   #include <Inventor/nodes/SoInfo.h>
   #include <Inventor/nodes/SoShapeHints.h>
   #include <Inventor/nodes/SoNormal.h>
   #include <Inventor/nodes/SoImage.h>

   #include <Inventor/events/SoKeyboardEvent.h>
   #include <Inventor/events/SoMouseButtonEvent.h>
   #include <Inventor/events/SoLocation2Event.h>
   #include <Inventor/sensors/SoTimerSensor.h>
   #include <Inventor/sensors/SoIdleSensor.h>

   #include <Inventor/actions/SoWriteAction.h>
   #include <Inventor/actions/SoGetMatrixAction.h>
   #include <Inventor/SoInput.h>
   #include <Inventor/SoDB.h>
   #include <Inventor/fields/SoSFLong.h>
   #include <Inventor/SoPickedPoint.h>
   #include <Inventor/SoOffscreenRenderer.h>
#endif

#if defined(LIBS_QT) 
	#include <QtWidgets/QMainWindow>
	#include <QtWidgets/QApplication>
	#include <QtWidgets/QMenuBar>
	#include <QtWidgets/QFileDialog>
	#include <QtCore/QUrl>
	#include <QtCore/QDirIterator> 
	#include <QtCore/QString>
	#include <QtCore/QDateTime> 
	#include <QtWidgets/qinputdialog.h>
	#include <QtWidgets/QMessageBox>
#else
	#include "resource.h"
	#include <intrin.h>				// For _cpuid
	#include <atlbase.h>			/* For MFC dialogs, cursor */
	#include <windows.h>
#endif

#define TRUE 1
#define FALSE 0



typedef float Matrix[4][4];

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
/* #include<fstream.h>  Not recognized by Windows, not needed by SGI */
#include <math.h>
//#include <mutex>				/* For std::mutex  -- not supported in older C++*/

/* #include <scsl_fft.h> */	/* For scfft */

#if defined(_WIN32) || defined(_WIN64)
	#include <io.h>
#else                   // For unix _unix_
	#include <sys/io.h>
	#include <thread>
	#include <pthread.h>
#endif
#include <iostream>
#include <fstream>
#include <sstream>			// Needed for vs2010 but not for vs2017
#define nsp_UsesTransform	/* For Intel Signal Processing Lib (FFT) */
#define nsp_UsesVector	/* For Intel array processing */
							// #import "progid:TDxInput.Device" embedded_idl no_namespace /* For spaceNavigator */

using namespace std;


// Omit from Doxygen, since essentially private
/// @cond

#if defined(LIBS_MRSID) 
   #include "lt_fileSpec.h"
   #include "lti_scene.h"
   #include "lti_navigator.h"
   #include "lti_sceneBuffer.h"
   #include "MrSIDImageReader.h"
   #include "GeoTIFFImageWriter.h"
   #include "lti_multiresFilter.h"
   #include "lti_cropFilter.h"

   LT_USE_NAMESPACE(LizardTech);
#endif

#if defined(LIBS_ASSIMP) 
   #include <assimp/Importer.hpp>
   #include <assimp/scene.h>
   #include <assimp/postprocess.h>
#endif

#if defined(LIBS_BOOST) 
	#include "boost/asio.hpp"
	#include "boost/bind.hpp"
	#include "boost/array.hpp"
	#include <boost/filesystem.hpp>
#endif

#if defined(LIBS_GDAL)
	#if defined(_WIN32) || defined(_WIN64)
		#include "gdal_priv.h"
		#include "cpl_conv.h"
		#include "ogrsf_frmts.h"
	#else                   // For unix _unix_
		#include "gdal/gdal_priv.h"
		#include "gdal/cpl_conv.h"
		#include "gdal/ogrsf_frmts.h"
	#endif
#endif

#if defined(LIBS_KML)
   #include "kml/dom.h"
   #include "kml/engine.h"
   #include "kml/base/file.h"

   using kmldom::CoordinatesPtr;
   using kmldom::KmlPtr;
   using kmldom::KmlFactory;
   using kmldom::PlacemarkPtr;
   using kmldom::PointPtr;
   using kmldom::FolderPtr;
   using kmldom::DocumentPtr;
   using kmldom::GroundOverlayPtr;
   using kmldom::StylePtr;
   using kmldom::IconStylePtr;
   using kmldom::IconStyleIconPtr;
   using kmldom::LineStringPtr;

   using kmlengine::KmlFile;
   using kmlengine::KmlFilePtr;
#endif

#ifndef S_IRUSR
	#define S_IRUSR 0
#endif
#ifndef S_IWUSR
	#define S_IWUSR 0
#endif
#ifndef S_IRGRP
	#define S_IRGRP 0
#endif
#ifndef S_IWGRP
	#define S_IWGRP 0
#endif
#ifndef S_IROTH
	#define S_IROTH 0
#endif

#if defined(_WIN32) || defined(_WIN64)
   #ifndef Widget
      #define Widget  HWND
   #endif
#endif

#ifndef O_BINARY		// Needed for linux
#define O_BINARY 0
#endif
#ifndef _int64			// Needed for linux
#define _int64 long long
#endif
   /// @endcond

#include "gps_calc_class.h"
#include "base_jfd_class.h"
#include "osus_command_class.h"
#include "sun_pos_class.h"
#if defined(LIBS_SOQT) 
	#include "fusion3d_viewer_class.h"
#elif defined(LIBS_SOWIN)
	#include "fusion3d_viewer_class_sowin.h"
#endif

#include "BPFFile.h"
#include "globals_inv_class.h"
#include "dir_class.h"
#include "dir_monitor_class.h"
#include "elev_estimate_class.h"
#include "interval_calc_class.h"
#include "time_conversion_class.h"
#include "map3d_lowres_class.h"
#include "draw_data_class.h"
#include "color_balance_class.h"
#include "clock_input_class.h"
#include "vector_index_class.h"
#include "vector_layer_class.h"
#include "shapefile_class.h"
#include "track_analysis_class.h"
#include "sensor_read_class.h"
#include "sensor_read_osus_class.h"
#include "roi_meta_class.h"
#include "nato4607_class.h"
#include "nato4676_class.h"
#include "cad_class.h"
#include "csv_class.h"
#include "sbet_class.h"
#include "xml_class.h"
#include "kml_class.h"
#include "ogr_class.h"
#include "osus_image_store_class.h"	// Uses xml_class
#include "tiles_rtv_class.h"
#include "image_resample_class.h"
#include "image_2d_class.h"
#include "image_2d_inv_class.h"
#include "image_gdal_class.h"
#include "image_pnm_class.h"
#include "image_geo_class.h"
#include "image_tif_class.h"
#include "image_mrsid_class.h"
#include "texture_server_class.h"
#include "map3d_index_class.h"
#include "draw_data_inv_class.h"
#include "cache_class.h"
#include "image_3d_class.h"
#include "image_ptcloud_class.h"
#include "image_las_class.h"
#include "image_las_demify_class.h"
#include "image_bpf_class.h"
#include "image_bpf_demify_class.h"
#include "time_track_class.h"
#include "icon_class.h"
#include "atrlab_manager_class.h"
#include "image_transformation_class.h"
#include "image_yaff_class.h"
#include "image_rtv_class.h"
#include "transform.h"
#include "camera4d_manager_class.h"
#include "camera4d_manager_inv_class.h"
#include "status_overlay_manager_class.h"
#include "status_overlay_manager_inv_class.h"
#include "image_seq_display_class.h"
#include "sensor_kml_manager_class.h"
#include "sensor_kml_manager_inv_class.h"
#include "sensor_osus_manager_class.h"
#include "sensor_osus_manager_inv_class.h"
#include "spaceball_input_class.h"
#include "script_input_class.h"
#if defined(LIBS_SOQT) 
	#include "buttons_input_class.h"
	#include "menu_input_class.h"
#elif defined(LIBS_SOWIN)
	#include "buttons_input_class_sowin.h"
	#include "menu_input_class_sowin.h"
#endif
#include "keyboard_input_class.h"
#include "atr_class.h"
#include "atr_los_class.h"
#include "ladar_mm_manager_class.h"
#include "ladar_mm_manager_inv_class.h"
#include "los_manager_class.h"
#include "los_manager_inv_class.h"
#include "cad_manager_class.h"
#include "cad_manager_inv_class.h"
#include "kml_manager_class.h"
#include "kml_manager_inv_class.h"
#include "bookmark_manager_class.h"
#include "bookmark_manager_inv_class.h"
#include "map3d_manager_class.h"
#include "map3d_manager_inv_class.h"
#include "draw_manager_class.h"
#include "draw_manager_inv_class.h"
#include "track_manager_class.h"
#include "track_manager_inv_class.h"
#include "mouse_input_class.h"
#include "write_manager_inv_class.h"
#include "mensuration_manager_class.h"
#include "mensuration_manager_inv_class.h"

#if defined(LIBS_QT) 
	#include "dialogFileDemOther.h"
	#include "dialogFileDemBuck.h"
	#include "dialogFileScreen.h"
	#include "dialogVecDisplay.h"
	#include "dialogVecFly.h"
	#include "dialogVecToggle.h"
	#include "dialogPcParms.h"
	#include "dialogPcFilter.h"
	#include "dialogPcThresh.h"
	#include "dialogLocJump.h"
	#include "dialogLosParms.h"
	#include "dialogDrawAlt.h"
	#include "dialogDrawNew.h"
	#include "dialogMeasureRoute.h"
	#include "dialogMeasureElev.h"
	#include "dialogStereoSettings.h"
	#include "dialogClockTime.h"
	#include "dialogOptionsParms.h"
	#include "dialogCadOpen.h"
#else
    #include "DirDialog.h"
	#include "dialog_load_dir.h"
	#include "dialog_load_dem_other.h"
	#include "dialog_dump.h"
	#include "dialog_toggle_vector_file_visibility.h"
    #include "dialog_sensors.h"
    #include "dialog_make_dem.h"
	#include "dialog_terr_lidar.h"
	#include "dialog_resources.h"
	#include "dialog_clear_track.h"
	#include "dialog_clock.h"
	#include "dialog_jump_to_loc_read_loc.h"
	#include "dialog_view_change_LOS_parameters.h"
	#include "dialog_hilite_thesholds.h"
	#include "dialog_stereo_preferences.h"
	#include "dialog_about.h"
	#include "dialog_fine.h"
	#include "dialog_finesl.h"
	#include "dialog_route_class.h"
	#include "dialog_fly_route.h"
	#include "dialog_bookmark_name.h"
	#include "dialog_draw_pt_class.h"
    #include "dialog_draw_altitude_class.h"
    #include "dialog_epsg_class.h"
	#include "dialog_vector_class.h"
	#include "dialog_cad_open.h"
#endif

#if defined(LIBS_QT) 
	int lfusion3d(const char* projFile);
#else 
	LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	int lfusion3d(int argc, TCHAR **argv);
#endif

int find_file_in_exec_tree(string name, string &path);
int find_all_files_matching_pattern(string dirname, string pattern, int recursive_flag, vector<string> &listNames, vector<time_t> &listTimes);
int match_with_asterisks(char *file, char* pat);
int check_file_exists(string filename);
int check_dir_exists(string dirname);
int parse_filepath(string name_in, string &filename, string &dirname);
int cross_sleep(int msec);
int read_defaults(int &nlowx, int &nlowy, int &nmed, int &nhi, int &stereoType, int &PCMaxD, int &elMax);

void byteswap(int *, int, int);
void byteswap_ulong(unsigned long *, int, int);
void byteswap_i2(short int *ia, int n);
void byteswap_ui2(unsigned short int *ia, int n);
void byteswap_double(double *iarray, int alen);
int comp_nums(const void *num1, const void *num2);
int set_stereo_mode(fusion3d_viewer_class *myViewer);
int read_string_with_spaces(FILE *tiff_fd, string &name);
int convert_name_to_absolute(string reading_from_name, string &name, string &dirname);

#endif /* __cplusplus */
#endif /* _internals_h_ */
