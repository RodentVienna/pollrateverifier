cmake_minimum_required( VERSION 3.4 )
project( pollrateverifier )

add_executable( 
  pollrateverifier
  pollrateverifier.cpp
  main.cpp )

install( TARGETS pollrateverifier
	 DESTINATION bin )
