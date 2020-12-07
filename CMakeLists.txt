# Fusion3D 3D Terrain Viewer -- Executable
## [main]
cmake_minimum_required(VERSION 3.15)
# cmake_minimum_required(VERSION 3.1...3.15)
project(Fusion3D VERSION 6.0 LANGUAGES CXX)
include(CMakeDependentOption)				# Reads in a module that implements the cmake_dependent_option macro

# *******************************************
# CMAKE basic build configuration 
# *******************************************
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)	# Constrain builds to either Debug or Release
# Uncommenting next statement (or specifying on the command line) will restrict build to single configuration 
# If left undefined, both Release and Debug will be built for IDEs like Visual Studio 
# set(CMAKE_BUILD_TYPE Release)		# Debug / Release

set(CMAKE_GENERATOR_PLATFORM x64)	# For Visual Studio, specifies target architecture x86 / x64
set(CMAKE_INSTALL_PREFIX "C:/Fusion3D")	# 

# For Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "../ui")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# *******************************************
# Platform dependent -- Only Windows and Linux have been tested
# *******************************************
if     (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows") 
elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
	add_compile_options(-fPIC -std=c++11 -pthread)
	set(CMAKE_XXX_FLAGS "-fPIC -std=c++11 -pthread")
	add_link_options(-pthread)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)	# Required for linux, not for Windows (same as -fPIC?)
else()
	message(FATAL_ERROR "Building on untested host architecture ${CMAKE_HOST_SYSTEM_NAME}")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	message(STATUS "Debug only on host ${CMAKE_HOST_SYSTEM_NAME}, cmpl defaults ${CMAKE_CXX_FLAGS_DEBUG} plus ${CMAKE_XXX_FLAGS}")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
	message(STATUS "Release only on host ${CMAKE_HOST_SYSTEM_NAME}, cmpl defaults ${CMAKE_CXX_FLAGS_RELEASE} plus ${CMAKE_XXX_FLAGS}")
else()
	message(STATUS "Both Release on host ${CMAKE_HOST_SYSTEM_NAME}, cmpl defaults ${CMAKE_CXX_FLAGS_RELEASE} plus ${CMAKE_XXX_FLAGS}")
	message(STATUS " and Debug with compiler defaults ${CMAKE_CXX_FLAGS_DEBUG} plus ${CMAKE_XXX_FLAGS}")
endif()

# *******************************************
# CMake Options
# *******************************************
option(ASSIMP_FLAG "Include Assimp lib for overlaying CAD models" ON)	

# Set the following if compiler must look for libs and includes in dirs other than defaults like /usr/lib or /usr/include
option(ADD_DIRS_QT    "Libs and includes not in default dirs -- Qt"   OFF)	
option(ADD_DIRS_COIN  "Libs and includes not in default dirs -- Coin"  ON)	
option(ADD_DIRS_SOQT  "Libs and includes not in default dirs -- SoQt"  ON)	
option(ADD_DIRS_MRSID "Libs and includes not in default dirs -- MrSID" ON)	
option(ADD_DIRS_GDAL  "Libs and includes not in default dirs -- GDAL"  ON)	
option(ADD_DIRS_KML   "Libs and includes not in default dirs -- KML"   ON)	
cmake_dependent_option(ADD_DIRS_ASSIMP "Libs and includes not in default dirs -- Assimp" ON "ASSIMP_FLAG" OFF) # Only if ASSIMP_FLAG ON
	
set(SCENE_TEST ON)			# Add test exec 'TestScene' into the install bin dir -- opens sample test map automatically

# *******************************************
# Set paths to external libraries if needed -- Qt
# *******************************************
message(STATUS "*********")
if (ADD_DIRS_QT)
	set(PATH_QT     "D:/Qt/Qt5.12.6/5.12.6/msvc2017/lib/cmake/Qt5")		# Path for find_package
	message(STATUS "find_package looks for Qt in dir  ${PATH_QT}")
else()
	message(STATUS "find_package looks for Qt in default dirs")
endif()

# *******************************************
# Set paths to external libraries if needed -- Coin
# *******************************************
if (ADD_DIRS_COIN)
	set(PATH_COIN   "C:/coin-4.0.0-msvc15-x64/Coin3D")
	set(PATH_COIN_INCLUDE  "${PATH_COIN}/include")
	set(PATH_COIN_LIB  "${PATH_COIN}/lib")
	message(STATUS "Coin include dir:  ${PATH_COIN_INCLUDE}")
	message(STATUS "Coin lib     dir:  ${PATH_COIN_LIB}")
else()
	message(STATUS "Coin in default dirs")
endif()

# *******************************************
# Set paths to external libraries if needed -- SoQt
# *******************************************
if (ADD_DIRS_SOQT)
	set(PATH_SOQT "C:/soqt-1.6.0-msvc15-x64/Coin3D")
	set(PATH_SOQT_INCLUDE  "${PATH_SOQT}/include")
	set(PATH_SOQT_LIB  "${PATH_SOQT}/lib")
	message(STATUS "SoQt include dir:  ${PATH_SOQT_INCLUDE}")
	message(STATUS "SoQt lib     dir:  ${PATH_SOQT_LIB}")
else()
	message(STATUS "SoQt in default dirs")
endif()

# *******************************************
# Set paths to external libraries if needed -- MrSID
# *******************************************
if (ADD_DIRS_MRSID)
	set(PATH_MRSID "C:/MrSID_DSDK-9.5.4.4709-win64-vc15/Raster_DSDK")
	set(PATH_MRSID_INCLUDE  "${PATH_MRSID}/include")
	set(PATH_MRSID_LIB  "${PATH_MRSID}/lib")
	message(STATUS "MrSID include dir:  ${PATH_MRSID_INCLUDE}")
	message(STATUS "MrSID lib     dir:  ${PATH_MRSID_LIB}")
else()
	message(STATUS "MrSID in default dirs")
endif()

# *******************************************
# Set paths to external libraries if needed -- KML
# *******************************************
if (ADD_DIRS_KML)
	set(PATH_KML "C:/libkml-x64/msvc/x64")
	set(PATH_KML_INCLUDE  "${PATH_KML}/include")
	set(PATH_KML_LIB  "${PATH_KML}/Debug")
	message(STATUS "KML include dir:  ${PATH_KML_INCLUDE}")
	message(STATUS "KML lib     dir:  ${PATH_KML_LIB}")
else()
	message(STATUS "KML in default dirs")
endif()

# *******************************************
# Set paths to external libraries if needed -- GDAL
# *******************************************
if (ADD_DIRS_GDAL)
	set(PATH_GDAL "C:/gdal-1911-x64-3-1-2")
	set(PATH_GDAL_INCLUDE  "${PATH_GDAL}/include")
	set(PATH_GDAL_LIB  "${PATH_GDAL}/lib")
	message(STATUS "GDAL include dir:  ${PATH_GDAL_INCLUDE}")
	message(STATUS "GDAL lib     dir:  ${PATH_GDAL_LIB}")
else()
	message(STATUS "GDAL in default dirs")
endif()

# *******************************************
# Set paths to external libraries if needed -- Assimp
# *******************************************
if (ADD_DIRS_ASSIMP)
	set(PATH_ASSIMP "C:/assimp-5.0.1-msvc15-x64")
	set(PATH_ASSIMP_INCLUDE  "${PATH_ASSIMP}/include")
	set(PATH_ASSIMP_LIB  "${PATH_ASSIMP}/lib")
	message(STATUS "Assimp include dir:  ${PATH_ASSIMP_INCLUDE}")
	message(STATUS "Assimp lib     dir:  ${PATH_ASSIMP_LIB}")
else()
	message(STATUS "Assimp in default dirs")
endif()

# *******************************************
# Set filenames for external libs 
# *******************************************
if     (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows") 
	set(MRSID_LIB_FILE "lti_dsdk.lib")								# Same for Debug, Release
	set(GDAL_LIB_FILE "gdal_i.lib;expat.lib") 							# Same for Debug, Release
	set(KML_LIB_FILE  "libkmlbase.lib;libkmlconvenience.lib;libkmldom.lib") 	# Same for Debug, Release
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")							# Single config -- Debug
		set(COIN_LIB_FILE   "Coin4d.lib")
		set(SOQT_LIB_FILE   "SoQt1d.lib")
		set(ASSIMP_LIB_FILE "assimp-vc141-mtd.lib")
	elseif (CMAKE_BUILD_TYPE STREQUAL "Release")						# Single config -- Release
		set(COIN_LIB_FILE   "Coin4.lib")	
		set(SOQT_LIB_FILE   "SoQt1.lib")
		set(ASSIMP_LIB_FILE "assimp-vc141-mt.lib")
	else()													# Multiple configs for Visual Studio
		set(COIN_LIB_FILE   "Coin4$<$<CONFIG:DEBUG>:d>.lib")
		set(SOQT_LIB_FILE   "SoQt1$<$<CONFIG:DEBUG>:d>.lib")
		set(ASSIMP_LIB_FILE "assimp-vc141-mt$<$<CONFIG:DEBUG>:d>.lib")
	endif()
elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
	set(COIN_LIB_FILE   "libCoin.so")
	set(SOQT_LIB_FILE   "libSoQt.so")
	set(MRSID_LIB_FILE "libltidsdk.so")
	set(GDAL_LIB_FILE "libgdal.so;libexpat.so")
	set(KML_LIB_FILE  "libkmlbase.so;libkmlconvenience.so;libkmldom.so")
	set(ASSIMP_LIB_FILE "libassimp.so")
endif()

message(STATUS "*********")
message(STATUS "Coin   lib file:  ${COIN_LIB_FILE}")
message(STATUS "SoQt   lib file:  ${SOQT_LIB_FILE}")
message(STATUS "MrSid  lib file:  ${MRSID_LIB_FILE}")
message(STATUS "GDAL   lib file:  ${GDAL_LIB_FILE}")
message(STATUS "KML    lib file:  ${KML_LIB_FILE}")
message(STATUS "ASSIMP lib file:  ${ASSIMP_LIB_FILE}")

# *******************************************
# Find packages 
# *******************************************
message(STATUS "*********")
if (ADD_DIRS_QT)
	message(STATUS "find_package looks for Qt5 in ${PATH_QT}.")
	find_package(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets PATHS ${PATH_QT})
else()
	message(STATUS "find_package looks for Qt5 in default path")
	find_package(Qt5 REQUIRED COMPONENTS Core Gui OpenGL Widgets)
endif()
message(STATUS "find_package finds Qt5: Core in dir ${Qt5Core_DIR}")

# *******************************************
# Add preprocessor flags
# *******************************************
add_definitions(-DSOQT_DLL -DQT_CORE_LIB -DQT_GUI_LIB -DQT_OPENGL_LIB -DQT_WIDGETS_LIB)	# Qt flags -- required
add_definitions(-DLIBS_QT -DLIBS_COIN -DLIBS_SOQT)					# My flags -- basic interface/graphics
add_definitions(-DLIBS_FASTLZ)									# My flags -- BPF decode lib included in basic code
add_definitions(-DLIBS_MRSID -DLIBS_GDAL -DLIBS_KML)					# My flags -- required for processing
if (ASSIMP_FLAG) 
	add_definitions(-DLIBS_ASSIMP)								# My flags -- optional for overlay CAD
endif(ASSIMP_FLAG)

# *******************************************
# Add app, test and src directories
# *******************************************
add_subdirectory("apps")
add_subdirectory("src")

if (SCENE_TEST) 
	add_subdirectory("test")
	# enable_testing()		# This generates a RUN_TESTS project that isnt useful
	# add_test(NAME Test1 COMMAND Test1.exe)
endif()

# *******************************************
# Install
# *******************************************
# executable is build/apps/Release/Fusion3D.exe
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	# If building only Debug, then dont install anything
else()
	# If building Only Release or multiple configs then include install project
	install(TARGETS Fusion3D RUNTIME DESTINATION bin)
	install(DIRECTORY data/GDAL data/milSymbols DESTINATION bin)
	install(DIRECTORY data/Scene DESTINATION data)
	install(FILES data/CustomParms.txt DESTINATION bin)
	install(DIRECTORY doc DESTINATION .)
	if (SCENE_TEST) 
		install(TARGETS testSampleScene RUNTIME DESTINATION bin)
	endif()
endif()
## [main]


