file(GLOB node_srcs "*.cc")
file(GLOB node_hdrs "*.h")
list(APPEND mlfe_source_files ${node_srcs})
list(APPEND mlfe_header_files ${node_hdrs})
if(NOT USE_CUDA)
    foreach(FILE ${mlfe_source_files})
        if ("${FILE}" MATCHES ".*\\.*(cuda|cudnn).*")
            list(REMOVE_ITEM mlfe_source_files ${FILE})
        endif()
    endforeach()

    foreach(FILE ${mlfe_header_files})
        if ("${FILE}" MATCHES ".*\\.*(cuda|cudnn).*")
            list(REMOVE_ITEM mlfe_header_files ${FILE})
        endif()
    endforeach()
endif()
if(USE_CUDA)
  file(GLOB op_cuda_srcs "*.cu")
  list(APPEND mlfe_cuda_source_files ${op_cuda_srcs})
  set(mlfe_cuda_source_files ${mlfe_cuda_source_files} PARENT_SCOPE)
endif()
set(mlfe_source_files ${mlfe_source_files} PARENT_SCOPE)
set(mlfe_header_files ${mlfe_header_files} PARENT_SCOPE)
