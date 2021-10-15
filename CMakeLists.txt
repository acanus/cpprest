cmake_minimum_required(VERSION 3.9)
project(cpprest VERSION 0.1.0)

include(CTest)
enable_testing()
set (CMAKE_PREFIX_PATH /usr/lib/aarch64-linux-gnu/cmake/)
find_package(Threads REQUIRED)
find_package(CUDA REQUIRED)
include_directories(/usr/local/cuda-10.2/targets/aarch64-linux/include)
link_directories(/usr/local/cuda-10.2/targets/aarch64-linux/lib)

message("-- CUDA version: ${CUDA_VERSION}")
set(
	CUDA_NVCC_FLAGS
	${CUDA_NVCC_FLAGS}; 
    -O3 
	-gencode arch=compute_53,code=sm_53
	-gencode arch=compute_62,code=sm_62
)
if(CUDA_VERSION_MAJOR GREATER 9)
	message("-- CUDA ${CUDA_VERSION_MAJOR} detected, enabling SM_72")

	set(
		CUDA_NVCC_FLAGS
		${CUDA_NVCC_FLAGS}; 
		-gencode arch=compute_72,code=sm_72
	)
endif()
find_package(cpprestsdk REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED system)
add_executable(cpprest src/main.cpp)
target_link_libraries(cpprest PRIVATE cpprestsdk::cpprest )
target_link_libraries(cpprest PRIVATE cpprestsdk::cpprest ${Boost_SYSTEM_LIBRARY} )
target_link_libraries(cpprest PRIVATE ${OpenCV_LIBS} )
target_link_libraries(cpprest PRIVATE ${NVINFER_LIBRARY})
target_link_libraries(cpprest PRIVATE ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(cpprest PRIVATE nvinfer nvinfer_plugin nvcaffe_parser cudart)
target_link_libraries(cpprest PRIVATE ${CUDA_LIBRARIES})
if(CUDA_VERSION_MAJOR GREATER 9)
	target_link_libraries(cpprest PRIVATE nvonnxparser)

    if(HAS_OPENCV) 
        message("-- Linking jetson-inference with OpenCV " ${OpenCV_VERSION})
        target_link_libraries(cpprest PRIVATE opencv_core opencv_calib3d)
    endif()
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
