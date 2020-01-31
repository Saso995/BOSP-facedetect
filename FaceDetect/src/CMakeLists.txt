
#----- Check for the required RTLib library
find_package(BbqRTLib REQUIRED)
find_package( OpenCV REQUIRED )

#----- Add compilation dependencies
include_directories(${BBQUE_RTLIB_INCLUDE_DIR}
					${OpenCV_INCLUDE_DIRS} )

#----- Add "facedetect" target application
set(FACEDETECT_SRC version FaceDetect_exc FaceDetect_main)
add_executable(facedetect ${FACEDETECT_SRC})

#----- Linking dependencies
target_link_libraries(
	facedetect
	${Boost_LIBRARIES}
	${BBQUE_RTLIB_LIBRARY}
	${CMAKE_THREAD_LIBS_INIT}
	${OpenCV_LIBS} 
)

# Use link path ad RPATH
set_property(TARGET facedetect PROPERTY
	INSTALL_RPATH_USE_LINK_PATH TRUE)

#----- Install the FaceDetect files
install (TARGETS facedetect RUNTIME
	DESTINATION ${FACEDETECT_PATH_BINS})

#----- Generate and Install FaceDetect configuration file
configure_file (
	"${PROJECT_SOURCE_DIR}/FaceDetect.conf.in"
	"${PROJECT_BINARY_DIR}/FaceDetect.conf"
)
install (FILES "${PROJECT_BINARY_DIR}/FaceDetect.conf"
	DESTINATION ${FACEDETECT_PATH_CONFIG})
