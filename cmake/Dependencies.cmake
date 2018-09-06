find_package(Threads)
if(THREADS_FOUND)
  if(${CMAKE_USE_WIN32_THREADS_INIT})
    message(STATUS "Found Threads : " "win32 thread")
  elseif(${CMAKE_USE_WIN32_THREADS_INIT})
    message(STATUS "Found Threads : " "pthread")
  endif()
  list(APPEND mlfe_library_dependencies ${CMAKE_THREAD_LIBS_INIT})
endif()

find_package(Eigen3 QUIET)

if(EIGEN3_FOUND)
  message(STATUS "Found Eigen3 : " ${EIGEN3_INCLUDE_DIRS})
  list(APPEND mlfe_include_dirs ${EIGEN3_INCLUDE_DIRS})
else()
  message(STATUS "[Can not find Eigen3. Using third party dir.]")
  list(APPEND mlfe_include_dirs ${PROJECT_SOURCE_DIR}/third_party/eigen)
endif()

if(BUILD_TEST)
  set(TEMP_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
  set(BUILD_SHARED_LIBS OFF)
  set(BUILD_GTEST ON)
  set(INSTALL_GTEST OFF)
  set(BUILD_GMOCK OFF)
  add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/googletest)
  list(APPEND mlfe_include_dirs ${PROJECT_SOURCE_DIR}/third_party/googletest/googletest/include)
  set(BUILD_SHARED_LIBS ${TEMP_BUILD_SHARED_LIBS})
endif()

find_package(flatbuffers QUIET)

if(FLATBUFFERS_FOUND)
  message(STATUS "Found flatbuffers : " ${FLATBUFFERS_INCLUDE_DIRS})
  list(APPEND mlfe_include_dirs ${FLATBUFFERS_INCLUDE_DIRS})
  list(APPEND mlfe_library_dependencies flatbuffers)
else()
  message(STATUS "[Can not find flatbuffers. Using third party dir.]")
  set(FLATBUFFERS_CODE_COVERAGE OFF)
  set(FLATBUFFERS_BUILD_TESTS OFF)
  set(FLATBUFFERS_INSTALL OFF)
  set(FLATBUFFERS_BUILD_FLATLIB ON)
  set(FLATBUFFERS_BUILD_FLATC ON)
  set(FLATBUFFERS_BUILD_FLATHASH OFF)
  set(FLATBUFFERS_BUILD_GRPCTEST OFF)
  set(FLATBUFFERS_BUILD_SHAREDLIB OFF)
  add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/flatbuffers)
  list(APPEND mlfe_include_dirs ${PROJECT_SOURCE_DIR}/third_party/flatbuffers/include)
  list(APPEND mlfe_library_dependencies flatbuffers)
endif()

if(BUILD_APPS)
  find_package(OpenCV REQUIRED)
endif()

include(cmake/cudnn.cmake)

if(USE_CUDA)
    find_package(CUDA REQUIRED)
    if(CUDA_FOUND)
        FindCudnn(${CUDA_INCLUDE_DIRS} ${CUDA_TOOLKIT_ROOT_DIR}/lib/x64)
        list(APPEND mlfe_library_dependencies ${CUDA_CUBLAS_LIBRARIES} ${CUDA_curand_LIBRARY} ${CUDNN_LIBRARIES})
        list(APPEND mlfe_include_dirs ${CUDA_INCLUDE_DIRS})
        list(APPEND mlfe_include_dirs ${PROJECT_SOURCE_DIR}/third_party/cub)
        set(ARCH_LIST 30 35 37 50 52 53 60 61 62 70)
        foreach(var ${ARCH_LIST})
          set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -gencode arch=compute_${var},code=sm_${var}")
        endforeach(var)
    endif()
endif()
