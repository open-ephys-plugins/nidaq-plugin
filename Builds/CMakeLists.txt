cmake_minimum_required(VERSION 3.5.0)
project(oe_plugin)
set(CMAKE_SHARED_LIBRARY_PREFIX "")
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(LINUX 1)
endif()

#change this line to modify the plugin name
set(PLUGIN_NAME OEPlugin)
#change this if the base GUI code is located somewhere else
set(GUI_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../plugin-GUI)

include_directories(${GUI_BASE_DIR}/JuceLibraryCode ${GUI_BASE_DIR}/JuceLibraryCode/modules ${GUI_BASE_DIR}/Source/Plugins/Headers)

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
	OEPLUGIN
	"$<$<PLATFORM_ID:Windows>:JUCE_API=__declspec(dllimport)>"
	$<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS>
	$<$<PLATFORM_ID:Linux>:JUCE_DISABLE_NATIVE_FILECHOOSERS=1>
	$<$<CONFIG:Debug>:DEBUG=1>
	$<$<CONFIG:Debug>:_DEBUG=1>
	$<$<CONFIG:Release>:NDEBUG=1>
	)


set(SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../Source)
file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false "${SOURCE_PATH}/*.cpp" "${SOURCE_PATH}/*.h")

if (APPLE)
	add_library(${PLUGIN_NAME} MODULE ${SRC_FILES})
else()
	add_library(${PLUGIN_NAME} SHARED ${SRC_FILES})
endif()

target_compile_features(${PLUGIN_NAME} PUBLIC cxx_auto_type cxx_generalized_initializers)

#Libraries and compiler options
if(MSVC)
	set(GUI_BIN_DIR ${GUI_BASE_DIR}/Builds/VisualStudio2013)
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(GUI_BIN_DIR ${GUI_BIN_DIR}/x64)
	endif()

	target_link_directories(${PLUGIN_NAME} PUBLIC $<$<CONFIG:Debug>:${GUI_BIN_DIR}/Debug/bin>)
	target_link_directories(${PLUGIN_NAME} PUBLIC $<$<CONFIG:Release>:${GUI_BIN_DIR}/Release/bin>)
	target_link_libraries(${PLUGIN_NAME} open-ephys.lib)
	target_compile_options(${PLUGIN_NAME} PRIVATE /sdl-)
	
	install(TARGETS ${PLUGIN_NAME} RUNTIME DESTINATION ${GUI_BIN_DIR}/Debug/bin/plugins CONFIGURATIONS Debug)
	install(TARGETS ${PLUGIN_NAME} RUNTIME DESTINATION ${GUI_BIN_DIR}/Release/bin/plugins CONFIGURATIONS Release)
elseif(LINUX)
	set(GUI_BIN_DIR ${GUI_BASE_DIR}/Builds/Linux/build)

	target_link_libraries(${PLUGIN_NAME} GL X11 Xext Xinerama asound dl freetype pthread rt)
	set_property(TARGET ${PLUGIN_NAME} APPEND_STRING PROPERTY LINK_FLAGS
		"-fvisibility=hidden -fPIC -rdynamic -Wl,-rpath,'$$ORIGIN/../shared'")
	target_compile_options(${PLUGIN_NAME} PRIVATE -fPIC -rdynamic)
	target_compile_options(${PLUGIN_NAME} PRIVATE -O3) #enable optimization for linux debug
	
	install(TARGETS ${PLUGIN_NAME} RUNTIME DESTINATION ${GUI_BIN_DIR}/plugins)
elseif(APPLE)
	set(GUI_BIN_DIR ${GUI_BASE_DIR}/Builds/MacOSX/build/)
	set_target_properties(${PLUGIN_NAME} PROPERTIES BUNDLE TRUE)
	set_property(TARGET ${PLUGIN_NAME} APPEND_STRING PROPERTY LINK_FLAGS
	"-undefined dynamic_lookup")
	
	install(TARGETS ${PLUGIN_NAME} RUNTIME DESTINATION ~/Library/Application\ Support/open-ephys/PlugIns)
endif()

#create filters for vs and xcode

foreach( src_file IN ITEMS ${SRC_FILES})
	get_filename_component(src_path "${src_file}" PATH)
	file(RELATIVE_PATH src_path_rel "${SOURCE_PATH}" "${src_path}")
	string(REPLACE "/" "\\" group_name "${src_path_rel}")
	source_group("${group_name}" FILES "${src_file}")
endforeach()
