cmake_minimum_required (VERSION 3.2)
project(nori)

add_subdirectory(ext ext_build)

include_directories(
  # Nori include files
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  # tinyformat string formatting library
  ${TFM_INCLUDE_DIR}
  # command line parser library
  ${CMDLINE_INCLUDE_DIR}
  # Eigen linear algebra library
  SYSTEM ${EIGEN_INCLUDE_DIR}
  # OpenEXR high dynamic range bitmap library
  SYSTEM ${OPENEXR_INCLUDE_DIRS}
  # Intel Thread Building Blocks
  SYSTEM ${TBB_INCLUDE_DIR}
  # Pseudorandom number generator
  ${PCG32_INCLUDE_DIR}
  # PugiXML parser
  ${PUGIXML_INCLUDE_DIR}
  # Helper functions for statistical hypothesis tests
  ${HYPOTHESIS_INCLUDE_DIR}
  # GLFW library for OpenGL context creation
  SYSTEM ${GLFW_INCLUDE_DIR}
  # GLEW library for accessing OpenGL functions
  SYSTEM ${GLEW_INCLUDE_DIR}
  # NanoVG drawing library
  SYSTEM ${NANOVG_INCLUDE_DIR}
  # NanoGUI user interface library
  SYSTEM ${NANOGUI_INCLUDE_DIR}
  SYSTEM ${NANOGUI_EXTRA_INCS}
  # Portable filesystem API
  SYSTEM ${FILESYSTEM_INCLUDE_DIR}
  # STB Image Write
  SYSTEM ${STB_IMAGE_WRITE_INCLUDE_DIR}
)

# The following lines build the main executable. If you add a source
# code file to Nori, be sure to include it in this list.
add_executable(nori

# Header files
  include/tools/bbox.h
  include/tools/timer.h
  include/tools/dpdf.h
  include/tools/frame.h
  
  include/objects/bsdf.h
  include/objects/camera.h
  include/objects/integrator.h
  include/objects/emitter.h
  include/objects/mesh.h
  include/objects/rfilter.h
  include/objects/sampler.h
  include/objects/scene.h
  include/objects/object.h
  include/objects/texture.h
  include/objects/tfilter.h
  
  include/core/block.h
  include/core/bitmap.h
  include/core/accel.h
  include/core/color.h
  include/core/common.h
  include/core/parser.h
  include/core/proplist.h
  include/core/ray.h
  include/core/transform.h
  include/core/vector.h
  include/core/warp.h

  # core
  src/core/bitmap.cpp
  src/core/block.cpp
  src/core/accel.cpp
  src/core/chi2test.cpp
  src/core/common.cpp
  src/core/gui.cpp
  src/core/independent.cpp
  src/core/main.cpp
  src/core/mesh.cpp
  src/core/obj.cpp
  src/core/object.cpp
  src/core/parser.cpp
  src/core/proplist.cpp
  src/core/scene.cpp
  src/core/ttest.cpp
  src/core/warp.cpp
  src/core/bsdf.cpp
  src/core/emitter.cpp
  src/core/texture.cpp

  # intergrators
  src/integrators/normals.cpp
  src/integrators/simple.cpp
  src/integrators/whitted.cpp
  src/integrators/path_mats.cpp
  src/integrators/path_ems.cpp
  src/integrators/path_mis.cpp
  src/integrators/ao.cpp

  # emitters
  src/emitters/area.cpp

  # materials
  src/materials/dielectric.cpp
  src/materials/diffuse.cpp
  src/materials/microfacet.cpp
  src/materials/mirror.cpp

  # cameras
  src/cameras/perspective.cpp
  src/cameras/rfilter.cpp

  # textures
  src/textures/tfilter.cpp
  src/textures/image.cpp
  src/textures/solid.cpp
  src/textures/procedural.cpp
)

add_definitions(${NANOGUI_EXTRA_DEFS})

# The following lines build the warping test application
add_executable(warptest
  include/core/warp.h
  src/core/warp.cpp
  src/core/warptest.cpp
  src/core/object.cpp
  src/core/proplist.cpp
  src/core/common.cpp

  src/materials/microfacet.cpp
)

if (WIN32)
  target_link_libraries(nori tbb_static pugixml IlmImf nanogui ${NANOGUI_EXTRA_LIBS} zlibstatic)
else()
  target_link_libraries(nori tbb_static pugixml IlmImf nanogui ${NANOGUI_EXTRA_LIBS})
endif()

target_link_libraries(warptest tbb_static nanogui ${NANOGUI_EXTRA_LIBS})

# Force colored output for the ninja generator
if (CMAKE_GENERATOR STREQUAL "Ninja")
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcolor-diagnostics")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
  elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
  endif()
endif()

target_compile_features(warptest PRIVATE cxx_std_17)
target_compile_features(nori PRIVATE cxx_std_17)

# vim: set et ts=2 sw=2 ft=cmake nospell:
