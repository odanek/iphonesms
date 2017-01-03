cmake_minimum_required(VERSION 2.6)

############################################################################
#
#  iPhone SMS Export
#
#  Copyright (C) 2010   Ondrej Danek <ondrej.danek@gmail.com>
# 
#############################################################################

# project name
project(iPhoneSmsExport)

#########################################################################
# compilation details
#########################################################################

# turn off some warnings under msc
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -DUNICODE)
endif(MSVC)

# MinGW
if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows -static-libgcc -static-libstdc++")
endif(MINGW)


#########################################################################
#
# iPhone SMS Export
#  
#########################################################################

########################
#  Project source files
########################

# set the list of source files
set(ISE_SOURCES

    source/iPhoneSms.cpp
    source/StringUtf8.h
    source/Message.h
    source/sqlite3.c
    source/sqlite3.h
)

########################
#  Add application
########################

set(ISE_APP_NAME "iPhoneSmsExport" CACHE STRING "Filename of the application.")
set(ISE_APP_DEBUG_NAME "iPhoneSmsExportd" CACHE STRING "Filename of the debug version of the application.")
add_executable(${ISE_APP_NAME} ${ISE_SOURCES})
set_target_properties(${ISE_APP_NAME} PROPERTIES VERSION 1.0.0 DEBUG_OUTPUT_NAME ${ISE_APP_DEBUG_NAME})

########################
#  Install application
########################

# install binary files
install(TARGETS ${ISE_APP_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
