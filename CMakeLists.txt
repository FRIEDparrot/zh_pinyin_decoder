cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set (CMAKE_CXX_STANDARD 11)   # c++11 build standard
set (CMAKE_CXX_STANDARD_REQUIRED True)

# set variables
set(HEADER_DIRS
	zh_pinyin_decoder 
	CJSON
	codeconv
	)

set(SOURCES
	GB2312search.cpp
	zh_pinyin_decoder/zh_pinyin_decoder.c
	zh_pinyin_decoder/zh_code_table.c
	zh_pinyin_decoder/zh_hash_boost.c
	CJSON/cJSON.c
	codeconv/codeconv.cpp
	)

project(GB2312_pinyin_decoder)

add_executable(GB2312_pinyin_decoder ${SOURCES})

# set include directories 
target_include_directories(GB2312_pinyin_decoder PRIVATE
	${HEADER_DIRS}
)

# Copy the entire bin directory to the output directory
add_custom_command(TARGET GB2312_pinyin_decoder POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/zh_pinyin_decoder/bin"
    $<TARGET_FILE_DIR:GB2312_pinyin_decoder>/zh_pinyin_decoder/bin)
