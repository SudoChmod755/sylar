# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Produce verbose output by default.
VERBOSE = 1

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/sz123/workspace/sylar

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sz123/workspace/sylar

# Include any dependencies generated for this target.
include CMakeFiles/test_iomanager.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/test_iomanager.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test_iomanager.dir/flags.make

CMakeFiles/test_iomanager.dir/tests/test_iomanager.cc.o: CMakeFiles/test_iomanager.dir/flags.make
CMakeFiles/test_iomanager.dir/tests/test_iomanager.cc.o: tests/test_iomanager.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sz123/workspace/sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test_iomanager.dir/tests/test_iomanager.cc.o"
	/usr/bin/c++  $(CXX_DEFINES) -D__FILE__=\"tests/test_iomanager.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test_iomanager.dir/tests/test_iomanager.cc.o -c /home/sz123/workspace/sylar/tests/test_iomanager.cc

CMakeFiles/test_iomanager.dir/tests/test_iomanager.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_iomanager.dir/tests/test_iomanager.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"tests/test_iomanager.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sz123/workspace/sylar/tests/test_iomanager.cc > CMakeFiles/test_iomanager.dir/tests/test_iomanager.cc.i

CMakeFiles/test_iomanager.dir/tests/test_iomanager.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_iomanager.dir/tests/test_iomanager.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) -D__FILE__=\"tests/test_iomanager.cc\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sz123/workspace/sylar/tests/test_iomanager.cc -o CMakeFiles/test_iomanager.dir/tests/test_iomanager.cc.s

# Object files for target test_iomanager
test_iomanager_OBJECTS = \
"CMakeFiles/test_iomanager.dir/tests/test_iomanager.cc.o"

# External object files for target test_iomanager
test_iomanager_EXTERNAL_OBJECTS =

bin/test_iomanager: CMakeFiles/test_iomanager.dir/tests/test_iomanager.cc.o
bin/test_iomanager: CMakeFiles/test_iomanager.dir/build.make
bin/test_iomanager: lib/libsylar.so
bin/test_iomanager: CMakeFiles/test_iomanager.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/sz123/workspace/sylar/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable bin/test_iomanager"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_iomanager.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test_iomanager.dir/build: bin/test_iomanager

.PHONY : CMakeFiles/test_iomanager.dir/build

CMakeFiles/test_iomanager.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test_iomanager.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test_iomanager.dir/clean

CMakeFiles/test_iomanager.dir/depend:
	cd /home/sz123/workspace/sylar && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sz123/workspace/sylar /home/sz123/workspace/sylar /home/sz123/workspace/sylar /home/sz123/workspace/sylar /home/sz123/workspace/sylar/CMakeFiles/test_iomanager.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test_iomanager.dir/depend

