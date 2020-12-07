# Fusion3D 3D Terrain Viewer -- library
## [src]
message(STATUS "*********")
message(STATUS "To CMake src:  AUTOUIC ${CMAKE_AUTOUIC} path=${CMAKE_AUTOUIC_SEARCH_PATHS}")

# ********************************************
# CMAKE_ variables set here *****************
# ********************************************
set(CMAKE_VERBOSE_MAKEFILE ON)

# ********************************************
# Add in files
# ********************************************
file(GLOB SOURCES CONFIGURE_DEPENDS "*.cpp")
file(GLOB HEADERS CONFIGURE_DEPENDS "../include/Fusion3D/*.h")
file(GLOB FORMS CONFIGURE_DEPENDS "../ui/*.ui")
add_library(lfusion3d STATIC ${SOURCES} ${HEADERS} ${FORMS} )
## [src]

# *******************************************
# Add include directories -- Fusion3d and Qt
# *******************************************
target_include_directories(lfusion3d PRIVATE "../include/Fusion3D")			# This adds to properties -> additional include files
target_include_directories(lfusion3d PRIVATE "GeneratedFiles")

target_include_directories(lfusion3d PRIVATE ${Qt5Core_INCLUDE_DIRS})	
target_include_directories(lfusion3d PRIVATE ${Qt5Gui_INCLUDE_DIRS})		
target_include_directories(lfusion3d PRIVATE ${Qt5OpenGL_INCLUDE_DIRS})		
target_include_directories(lfusion3d PRIVATE ${Qt5Widgets_INCLUDE_DIRS})		

message(STATUS "*********")
message(STATUS "lfusion3d adds include dir  ../include/Fusion3D")
message(STATUS "lfusion3d adds include dir  GeneratedFiles")
message(STATUS "lfusion3d adds Qt5Core    include dirs:  ${Qt5Core_INCLUDE_DIRS}")
message(STATUS "lfusion3d adds Qt5Gui     include dirs:  ${Qt5Gui_INCLUDE_DIRS}")
message(STATUS "lfusion3d adds Qt5OpenGL  include dirs:  ${Qt5OpenGL_INCLUDE_DIRS}")
message(STATUS "lfusion3d adds Qt5Widgets include dirs:  ${Qt5Widgets_INCLUDE_DIRS}")

# *******************************************
# Add include directories -- Coin
# *******************************************
if (ADD_DIRS_COIN)
	target_include_directories(lfusion3d PRIVATE "${PATH_COIN_INCLUDE}")	
	message(STATUS "lfusion3d adds include dir  ${PATH_COIN_INCLUDE}")
else()
	message(STATUS "lfusion3d looks for Coin includes in default dirs")
endif()

# *******************************************
# Add include directories -- SoQt
# *******************************************
if (ADD_DIRS_SOQT)
	target_include_directories(lfusion3d PRIVATE "${PATH_SOQT_INCLUDE}")	
	message(STATUS "lfusion3d adds include dir  ${PATH_SOQT_INCLUDE}")
else()
	message(STATUS "lfusion3d looks for SoQt includes in default dirs")
endif()

# *******************************************
# Add include directories -- MrSID
# *******************************************
if (ADD_DIRS_MRSID)
	target_include_directories(lfusion3d PRIVATE "${PATH_MRSID_INCLUDE}")	
	message(STATUS "lfusion3d adds include dir  ${PATH_MRSID_INCLUDE}")
else()
	message(STATUS "lfusion3d looks for MrSID includes in default dirs")
endif()

# *******************************************
# Add include directories -- GDAL
# *******************************************
if (ADD_DIRS_GDAL)
	target_include_directories(lfusion3d PRIVATE "${PATH_GDAL_INCLUDE}")	
	message(STATUS "lfusion3d adds include dir  ${PATH_GDAL_INCLUDE}")
else()
	message(STATUS "lfusion3d looks for GDAL includes in default dirs")
endif()

# *******************************************
# Add include directories -- Assimp
# *******************************************
if (ADD_DIRS_ASSIMP)
	target_include_directories(lfusion3d PRIVATE "${PATH_ASSIMP_INCLUDE}")	
	message(STATUS "lfusion3d adds include dir  ${PATH_ASSIMP_INCLUDE}")
elseif()
	message(STATUS "lfusion3d looks for Assimp includes in default dirs")
endif()

