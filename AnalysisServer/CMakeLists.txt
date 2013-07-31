protocallc(_protocall_common_source
  _protocall_service_source
  _protocall_proxy_source
  _protocall_include_dir
  "RemoteAnalysis.proto")

include_directories(
  ${_protocall_include_dir}
  ${CMAKE_SOURCE_DIR}/Base
  ${CMAKE_SOURCE_DIR}/Base/Analysis
  )

set(_server_source
  voAnalysisServer.cpp
  voRemoteAnalysisService.cpp
  )

add_executable(VisomicsAnalysisServer
  ${_protocall_common_source}
  ${_protocall_service_source}
  ${_server_source}
  )

target_link_libraries(VisomicsAnalysisServer
  vtkParallelCore vtkFiltersStatisticsGnuR protobuf)
