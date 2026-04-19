# CMake generated Testfile for 
# Source directory: D:/Projects/Game_Project
# Build directory: D:/Projects/Game_Project/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(BunkerSmokeChecks "D:/Projects/Game_Project/build/Debug/BunkerSmokeChecks.exe")
  set_tests_properties(BunkerSmokeChecks PROPERTIES  _BACKTRACE_TRIPLES "D:/Projects/Game_Project/CMakeLists.txt;97;add_test;D:/Projects/Game_Project/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(BunkerSmokeChecks "D:/Projects/Game_Project/build/Release/BunkerSmokeChecks.exe")
  set_tests_properties(BunkerSmokeChecks PROPERTIES  _BACKTRACE_TRIPLES "D:/Projects/Game_Project/CMakeLists.txt;97;add_test;D:/Projects/Game_Project/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(BunkerSmokeChecks "D:/Projects/Game_Project/build/MinSizeRel/BunkerSmokeChecks.exe")
  set_tests_properties(BunkerSmokeChecks PROPERTIES  _BACKTRACE_TRIPLES "D:/Projects/Game_Project/CMakeLists.txt;97;add_test;D:/Projects/Game_Project/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(BunkerSmokeChecks "D:/Projects/Game_Project/build/RelWithDebInfo/BunkerSmokeChecks.exe")
  set_tests_properties(BunkerSmokeChecks PROPERTIES  _BACKTRACE_TRIPLES "D:/Projects/Game_Project/CMakeLists.txt;97;add_test;D:/Projects/Game_Project/CMakeLists.txt;0;")
else()
  add_test(BunkerSmokeChecks NOT_AVAILABLE)
endif()
subdirs("external/glfw")
