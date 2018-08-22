cmake_minimum_required( VERSION 3.4 )
project( pollrateverifier )

find_package( X11 REQUIRED )

add_executable( 
  pollrateverifier
  inputspammer.cpp
  main.cpp )

install( TARGETS pollrateverifier
	 DESTINATION bin )
