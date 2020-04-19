cmake_minimum_required(VERSION 3.5)

set( PRO_PATCH $ENV{TRAVIS_BUILD_NUMBER})
if (NOT "${PRO_PATCH}")
  set(PRO_PATCH "1")
endif (NOT "${PRO_PATCH}")

project(bulkmt VERSION 0.0.${PRO_PATCH})

find_package(Threads REQUIRED)

add_executable(bulkmt main.cpp)

add_library(obs STATIC observer.cpp output_observer.cpp record_observer.cpp hard.cpp)

set_target_properties(
  bulkmt obs PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra;"
)

target_link_libraries(bulkmt
        obs
        Threads::Threads
        )

install(TARGETS bulkmt RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_NAME bulkmt)
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_CONTACT mr.alligator@yandex.ru)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
include(CPack)
