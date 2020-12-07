# Fusion3D 3D Terrain Viewer
## [test]
message(STATUS "*********")
message(STATUS "To start test subdirectory")

# CMAKE_ variables set here *****************
set(CMAKE_VERBOSE_MAKEFILE ON)

# *******************************************
# Add in files
# *******************************************
file(GLOB Tests "*.cpp")
add_executable(testSampleScene ${Tests})

# ********************************************
# Add in library directories -- Coin
# ********************************************
message(STATUS "*********")
if (ADD_DIRS_COIN)
	target_link_directories(testSampleScene PRIVATE ${PATH_COIN_LIB})
	message(STATUS "testSampleScene adds link dir ${PATH_COIN_LIB}")
else()
	message(STATUS "testSampleScene looks for Coin libs in default dirs")
endif()

# ********************************************
# Add in library directories -- SoQt
# ********************************************
if (ADD_DIRS_SOQT)
	target_link_directories(testSampleScene PRIVATE ${PATH_SOQT_LIB})
	message(STATUS "testSampleScene adds link dir ${PATH_SOQT_LIB}")
else()
	message(STATUS "TestAdelatestSampleScene ide looks for SoQt libs in default dirs")
endif()

# ********************************************
# Add in library directories -- MrSid
# ********************************************
if (ADD_DIRS_MRSID)
	target_link_directories(testSampleScene PRIVATE ${PATH_MRSID_LIB})
	message(STATUS "testSampleScene adds link dir ${PATH_MRSID_LIB}")
else()
	message(STATUS "testSampleScene looks for MrSID libs in default dirs")
endif()

# ********************************************
# Add in library directories -- KML
# ********************************************
if (ADD_DIRS_KML)
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")								# Single config -- Debug
		target_link_directories(testSampleScene PRIVATE ${PATH_KML_LIB})
		message(STATUS "testSampleScene adds link dir ${PATH_KML_LIB}")
	elseif (CMAKE_BUILD_TYPE STREQUAL "Release")							# Single config -- Release
	else()														# Multiple configs for IDEs like Visual Studio
		target_link_directories(testSampleScene PRIVATE $<$<CONFIG:DEBUG>:${PATH_KML_LIB}>)
		message(STATUS "testSampleScene adds link dir for Debug only   ${PATH_KML_LIB}")
	endif()
else()
	message(STATUS "testSampleScene looks for KML libs in default dirs")
endif()

# ********************************************
# Add in library directories -- GDAL
# ********************************************
if (ADD_DIRS_GDAL)
	target_link_directories(testSampleScene PRIVATE ${PATH_GDAL_LIB})
	message(STATUS "testSampleScene adds link dir ${PATH_GDAL_LIB}")
else()
	message(STATUS "testSampleScene looks for GDAL libs in default dirs")
endif()

# ********************************************
# Add in library directories -- Assimp
# ********************************************
if (ADD_DIRS_ASSIMP)
	target_link_directories(testSampleScene PRIVATE ${PATH_ASSIMP_LIB})
	message(STATUS "testSampleScene adds link dir ${PATH_ASSIMP_LIB}")
else()
	message(STATUS "testSampleScene looks for Assimp libs in default dirs")
endif()

# *******************************************
# Add library files without paths -- from dirs entered above or default dirs
# *******************************************
target_link_libraries(testSampleScene lfusion3d)
target_link_libraries(testSampleScene ${COIN_LIB_FILE})
target_link_libraries(testSampleScene ${SOQT_LIB_FILE})
target_link_libraries(testSampleScene ${MRSID_LIB_FILE})
target_link_libraries(testSampleScene ${KML_LIB_FILE})
target_link_libraries(testSampleScene ${GDAL_LIB_FILE})
if (ASSIMP_FLAG)
	target_link_libraries(testSampleScene ${ASSIMP_LIB_FILE})
endif()

message(STATUS "*********")
message(STATUS "testSampleScene adds lib lfusion3d")
message(STATUS "testSampleScene adds lib ${COIN_LIB_FILE}")
message(STATUS "testSampleScene adds lib ${SOQT_LIB_FILE}")
message(STATUS "testSampleScene adds lib ${MRSID_LIB_FILE}")
message(STATUS "testSampleScene adds lib ${KML_LIB_FILE}")
message(STATUS "testSampleScene adds lib ${GDAL_LIB_FILE}")
if (ASSIMP_FLAG)
	message(STATUS "testSampleScene adds lib ${ASSIMP_LIB_FILE}")
endif()

# *******************************************
# Add library files with complete paths (easier for Qt)
# *******************************************
target_link_libraries(testSampleScene Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
message(STATUS "testSampleScene adds Qt libs")

