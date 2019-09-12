#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

#if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  #set (CMAKE_C_FLAGS "--std=c99 -g ${CMAKE_C_FLAGS}")
#endif()

include_directories("/usr/local/include/azureiot"
                    "/usr/local/include/azureiot/inc/")

# Look for required libraries
#SET(requiredlibs)

FIND_PACKAGE(CURL)
IF(CURL_FOUND)
  INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
  SET(requiredlibs ${requiredlibs} ${CURL_LIBRARIES} )
ELSE(CURL_FOUND)
  MESSAGE(FATAL_ERROR "Could not find the CURL library and development files.")
ENDIF(CURL_FOUND)

find_package( OpenCV REQUIRED )

# set(SOURCE main.c)
add_executable(FaceDetection main.c)
target_link_libraries(FaceDetection -lm 
																	 curl
                              		 wiringPi
                              		 serializer
                              		 iothub_client
                              		 iothub_client_mqtt_transport
                              		 umqtt
                              		 aziotsharedutil
                              		 ssl
                              		 crypto
                              		 pthread
                              		 parson
                              		 m
                              		 ${OpenCV_LIBS})


