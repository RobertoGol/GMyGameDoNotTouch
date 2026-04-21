# CMake generated Testfile for 
# Source directory: D:/Projects/Game_Project
# Build directory: D:/Projects/Game_Project/build_verify_ninja_fresh
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(BunkerSmokeChecks "D:/Projects/Game_Project/build_verify_ninja_fresh/BunkerSmokeChecks.exe")
set_tests_properties(BunkerSmokeChecks PROPERTIES  _BACKTRACE_TRIPLES "D:/Projects/Game_Project/CMakeLists.txt;104;add_test;D:/Projects/Game_Project/CMakeLists.txt;0;")
subdirs("external/glfw")
