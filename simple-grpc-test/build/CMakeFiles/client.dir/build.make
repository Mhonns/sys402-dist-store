# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/st1322017/sys402-dist-store/simple-grpc-test

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/st1322017/sys402-dist-store/simple-grpc-test/build

# Include any dependencies generated for this target.
include CMakeFiles/client.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/client.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/client.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/client.dir/flags.make

CMakeFiles/client.dir/src/client.cc.o: CMakeFiles/client.dir/flags.make
CMakeFiles/client.dir/src/client.cc.o: ../src/client.cc
CMakeFiles/client.dir/src/client.cc.o: CMakeFiles/client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/st1322017/sys402-dist-store/simple-grpc-test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/client.dir/src/client.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/client.dir/src/client.cc.o -MF CMakeFiles/client.dir/src/client.cc.o.d -o CMakeFiles/client.dir/src/client.cc.o -c /home/st1322017/sys402-dist-store/simple-grpc-test/src/client.cc

CMakeFiles/client.dir/src/client.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/client.dir/src/client.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/st1322017/sys402-dist-store/simple-grpc-test/src/client.cc > CMakeFiles/client.dir/src/client.cc.i

CMakeFiles/client.dir/src/client.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/client.dir/src/client.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/st1322017/sys402-dist-store/simple-grpc-test/src/client.cc -o CMakeFiles/client.dir/src/client.cc.s

# Object files for target client
client_OBJECTS = \
"CMakeFiles/client.dir/src/client.cc.o"

# External object files for target client
client_EXTERNAL_OBJECTS =

client: CMakeFiles/client.dir/src/client.cc.o
client: CMakeFiles/client.dir/build.make
client: libprotolib.a
client: /usr/local/lib/libgrpc++.a
client: /usr/local/lib/libgrpc.a
client: /usr/local/lib/libupb_collections_lib.a
client: /usr/local/lib/libupb_json_lib.a
client: /usr/local/lib/libupb_textformat_lib.a
client: /usr/local/lib/libutf8_range_lib.a
client: /usr/local/lib/libupb.a
client: /usr/local/lib/libre2.a
client: /usr/local/lib/libz.a
client: /usr/local/lib/libcares.a
client: /usr/local/lib/libgpr.a
client: /usr/local/lib/libabsl_random_distributions.a
client: /usr/local/lib/libabsl_random_seed_sequences.a
client: /usr/local/lib/libabsl_random_internal_pool_urbg.a
client: /usr/local/lib/libabsl_random_internal_randen.a
client: /usr/local/lib/libabsl_random_internal_randen_hwaes.a
client: /usr/local/lib/libabsl_random_internal_randen_hwaes_impl.a
client: /usr/local/lib/libabsl_random_internal_randen_slow.a
client: /usr/local/lib/libabsl_random_internal_platform.a
client: /usr/local/lib/libabsl_random_internal_seed_material.a
client: /usr/local/lib/libabsl_random_seed_gen_exception.a
client: /usr/local/lib/libssl.a
client: /usr/local/lib/libcrypto.a
client: /usr/local/lib/libaddress_sorting.a
client: /usr/local/lib/libprotobuf.a
client: /usr/local/lib/libabsl_statusor.a
client: /usr/local/lib/libabsl_log_internal_check_op.a
client: /usr/local/lib/libabsl_leak_check.a
client: /usr/local/lib/libabsl_die_if_null.a
client: /usr/local/lib/libabsl_log_internal_conditions.a
client: /usr/local/lib/libabsl_log_internal_message.a
client: /usr/local/lib/libabsl_log_internal_nullguard.a
client: /usr/local/lib/libabsl_examine_stack.a
client: /usr/local/lib/libabsl_log_internal_format.a
client: /usr/local/lib/libabsl_log_internal_proto.a
client: /usr/local/lib/libabsl_log_internal_log_sink_set.a
client: /usr/local/lib/libabsl_log_sink.a
client: /usr/local/lib/libabsl_log_entry.a
client: /usr/local/lib/libabsl_flags.a
client: /usr/local/lib/libabsl_flags_internal.a
client: /usr/local/lib/libabsl_flags_marshalling.a
client: /usr/local/lib/libabsl_flags_reflection.a
client: /usr/local/lib/libabsl_flags_config.a
client: /usr/local/lib/libabsl_flags_program_name.a
client: /usr/local/lib/libabsl_flags_private_handle_accessor.a
client: /usr/local/lib/libabsl_flags_commandlineflag.a
client: /usr/local/lib/libabsl_flags_commandlineflag_internal.a
client: /usr/local/lib/libabsl_log_initialize.a
client: /usr/local/lib/libabsl_log_globals.a
client: /usr/local/lib/libabsl_log_internal_globals.a
client: /usr/local/lib/libabsl_raw_hash_set.a
client: /usr/local/lib/libabsl_hash.a
client: /usr/local/lib/libabsl_city.a
client: /usr/local/lib/libabsl_low_level_hash.a
client: /usr/local/lib/libabsl_hashtablez_sampler.a
client: /usr/local/lib/libabsl_status.a
client: /usr/local/lib/libabsl_cord.a
client: /usr/local/lib/libabsl_cordz_info.a
client: /usr/local/lib/libabsl_cord_internal.a
client: /usr/local/lib/libabsl_cordz_functions.a
client: /usr/local/lib/libabsl_exponential_biased.a
client: /usr/local/lib/libabsl_cordz_handle.a
client: /usr/local/lib/libabsl_crc_cord_state.a
client: /usr/local/lib/libabsl_crc32c.a
client: /usr/local/lib/libabsl_crc_internal.a
client: /usr/local/lib/libabsl_crc_cpu_detect.a
client: /usr/local/lib/libabsl_bad_optional_access.a
client: /usr/local/lib/libabsl_str_format_internal.a
client: /usr/local/lib/libabsl_strerror.a
client: /usr/local/lib/libabsl_synchronization.a
client: /usr/local/lib/libabsl_stacktrace.a
client: /usr/local/lib/libabsl_symbolize.a
client: /usr/local/lib/libabsl_debugging_internal.a
client: /usr/local/lib/libabsl_demangle_internal.a
client: /usr/local/lib/libabsl_graphcycles_internal.a
client: /usr/local/lib/libabsl_kernel_timeout_internal.a
client: /usr/local/lib/libabsl_malloc_internal.a
client: /usr/local/lib/libabsl_time.a
client: /usr/local/lib/libabsl_civil_time.a
client: /usr/local/lib/libabsl_time_zone.a
client: /usr/local/lib/libabsl_bad_variant_access.a
client: /usr/local/lib/libutf8_validity.a
client: /usr/local/lib/libabsl_strings.a
client: /usr/local/lib/libabsl_string_view.a
client: /usr/local/lib/libabsl_strings_internal.a
client: /usr/local/lib/libabsl_base.a
client: /usr/local/lib/libabsl_spinlock_wait.a
client: /usr/local/lib/libabsl_int128.a
client: /usr/local/lib/libabsl_throw_delegate.a
client: /usr/local/lib/libabsl_raw_logging_internal.a
client: /usr/local/lib/libabsl_log_severity.a
client: CMakeFiles/client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/st1322017/sys402-dist-store/simple-grpc-test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable client"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/client.dir/build: client
.PHONY : CMakeFiles/client.dir/build

CMakeFiles/client.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/client.dir/cmake_clean.cmake
.PHONY : CMakeFiles/client.dir/clean

CMakeFiles/client.dir/depend:
	cd /home/st1322017/sys402-dist-store/simple-grpc-test/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/st1322017/sys402-dist-store/simple-grpc-test /home/st1322017/sys402-dist-store/simple-grpc-test /home/st1322017/sys402-dist-store/simple-grpc-test/build /home/st1322017/sys402-dist-store/simple-grpc-test/build /home/st1322017/sys402-dist-store/simple-grpc-test/build/CMakeFiles/client.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/client.dir/depend

