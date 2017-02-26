Trying to go for a superbuild where each dependency may or may not be installed on the system. 

Managed to build a library (cobs_lib), now I want to use that lib in cobs_cli. 

https://cmake.org/cmake/help/v3.4/module/ExternalProject.html
https://github.com/Kitware/CMake/blob/master/Modules/ExternalProject.cmake
http://stackoverflow.com/questions/29533159/what-is-install-dir-useful-for-in-externalproject-add-command
https://www.mail-archive.com/cmake@cmake.org/msg51663.html
https://github.com/mfreiholz/cmake-example-external-project/blob/master/CMakeLists-ExternalProjects.txt
