set(_source "${CMAKE_CURRENT_SOURCE_DIR}/protobuf")
set(_build "${CMAKE_CURRENT_BINARY_DIR}/protobuf")

set(proj ProtoCall)

ExternalProject_Add(${proj}
  GIT_REPOSITORY ${git_protocol}://github.com/OpenChemistry/protocall.git
  GIT_TAG "origin/master"
  INSTALL_COMMAND ""
  SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
  BINARY_DIR ${proj}-build
  PREFIX ${proj}${ep_suffix}
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS -Dprotobuf_DIR:PATH=${protobuf_DIR}
  DEPENDS protobuf
  )

set(${proj}_DIR ${CMAKE_BINARY_DIR}/${proj}-build)