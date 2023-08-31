include(GenerateExportHeader)

add_library(cxlisp cxlisp.cpp)

add_library(cxlisp::cxlisp ALIAS cxlisp)

target_link_libraries(cxlisp PRIVATE cxlisp_options cxlisp_warnings)

target_include_directories(cxlisp ${WARNING_GUARD} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

target_compile_features(cxlisp PUBLIC cxx_std_17)

set_target_properties(
        cxlisp
        PROPERTIES VERSION ${PROJECT_VERSION}
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES)

generate_export_header(cxlisp EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/cxlisp/cxlisp_export.hpp)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(cxlisp PUBLIC SAMPLE_LIBRARY_STATIC_DEFINE)
endif()