# ======================================================================
#  ubevt main build file
#
#  cd .../path/to/build/directory
#  source .../path/to/lardataobj/ups/setup_for_development <-d|-p>
#  buildtool [-I /install/path] -bt[i] -jN
# ======================================================================

cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

find_package(cetmodules REQUIRED)
project(ubevt VERSION 10.10.03 LANGUAGES CXX)

include(CetCMakeEnv)
cet_cmake_env()

cet_set_compiler_flags(DIAGS CAUTIOUS
  WERROR
  NO_UNDEFINED
  EXTRA_FLAGS -pedantic -Wno-unused-local-typedefs
)

cet_report_compiler_flags(REPORT_THRESHOLD VERBOSE)

find_package( ubobj REQUIRED EXPORT )
find_package( larevt REQUIRED EXPORT )
find_package( nurandom REQUIRED EXPORT )
find_package( ifdh_art REQUIRED EXPORT )
find_package( gallery REQUIRED EXPORT )
find_package( GSL REQUIRED EXPORT )

cet_cmake_module_directories(Modules BINARY)

# macros for dictionary and simple_plugin
include(ArtDictionary)
include(ArtMake)
include(BuildPlugins)

# ADD SOURCE CODE SUBDIRECTORIES HERE
add_subdirectory(ubevt)

# Unit tests.
add_subdirectory(test)

# ups - table and config files

# packaging utility
cet_cmake_config()
