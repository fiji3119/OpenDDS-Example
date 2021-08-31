project (my_example CXX)
cmake_minimum_required(VERSION 3.8.2)

find_package(OpenDDS REQUIRED)
find_package(OpenCV REQUIRED)

set(CMAKE_CXX_COMPILER ${OPENDDS_COMPILER})

set(opendds_libs
  OpenDDS::Dcps # Core OpenDDS Library
  OpenDDS::InfoRepoDiscovery OpenDDS::Tcp # For run_test.pl
  OpenDDS::Rtps OpenDDS::Rtps_Udp # For run_test.pl --rtps
  OpenDDS::Multicast
  OpenDDS::Model
  OpenDDS::Udp OpenDDS::Tcp
  TAO::Svc_Utils
)
set(PUB_EXEC "${PROJECT_NAME}_pub")
set(SUB_EXEC "${PROJECT_NAME}_sub")
set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INC "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(IDL "${CMAKE_CURRENT_SOURCE_DIR}/idl")
set(DST "${CMAKE_CURRENT_BINARY_DIR}")

# Copy CPP files
foreach (file
         ${SRC}/main_sub.cpp
         ${SRC}/main_pub.cpp
        )
   configure_file(${file} ${DST} COPYONLY)
endforeach()

# Copy IDL files
foreach (file
         ${IDL}/Common.idl
         ${IDL}/Image.idl
        )
   configure_file(${file} ${DST} COPYONLY)
endforeach()



######## Application PUB ###########
add_executable(${PUB_EXEC} main_pub.cpp)

#Include headers
target_include_directories(${PUB_EXEC}
                           PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

OPENDDS_TARGET_SOURCES(${PUB_EXEC} ${DST}/Image.idl ${DST}/Common.idl ${DST}/main_pub.cpp)

target_link_libraries(${PUB_EXEC}
                      ${opendds_libs}
                     )


######## Application SUB ###########
add_executable(${SUB_EXEC} main_sub.cpp)

#Include headers
target_include_directories(${SUB_EXEC}
                           PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

OPENDDS_TARGET_SOURCES(${SUB_EXEC} ${DST}/Image.idl ${DST}/Common.idl ${DST}/main_sub.cpp)

target_link_libraries(${SUB_EXEC} 
                      ${OpenCV_LIBRARIES}
                      ${opendds_libs}
                     )