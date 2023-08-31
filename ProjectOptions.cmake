include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)



macro(cxlisp_supports_sanitizers)
    if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
        set(SUPPORTS_UBSAN ON)
    else()
        set(SUPPORTS_UBSAN OFF)
    endif()

    if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
        set(SUPPORTS_ASAN OFF)
    else()
        set(SUPPORTS_ASAN ON)
    endif()
endmacro()

macro(cxlisp_setup_options)
    option(cxlisp_ENABLE_HARDENING "Enable hardening" ON)
    option(cxlisp_ENABLE_COVERAGE "Enable coverage reporting" OFF)
    cmake_dependent_option(
            cxlisp_ENABLE_GLOBAL_HARDENING
            "Attempt to push hardening options to built dependencies"
            ON
            cxlisp_ENABLE_HARDENING
            OFF)

    cxlisp_supports_sanitizers()

    if(NOT PROJECT_IS_TOP_LEVEL OR cxlisp_PACKAGING_MAINTAINER_MODE)
        option(cxlisp_ENABLE_IPO "Enable IPO/LTO" OFF)
        option(cxlisp_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
        option(cxlisp_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
        option(cxlisp_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
        option(cxlisp_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
        option(cxlisp_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
        option(cxlisp_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
        option(cxlisp_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
        option(cxlisp_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
        option(cxlisp_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
        option(cxlisp_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
        option(cxlisp_ENABLE_PCH "Enable precompiled headers" OFF)
        option(cxlisp_ENABLE_CACHE "Enable ccache" OFF)
    else()
        option(cxlisp_ENABLE_IPO "Enable IPO/LTO" ON)
        option(cxlisp_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
        option(cxlisp_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
        option(cxlisp_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
        option(cxlisp_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
        option(cxlisp_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
        option(cxlisp_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
        option(cxlisp_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
        option(cxlisp_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
        option(cxlisp_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
        option(cxlisp_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
        option(cxlisp_ENABLE_PCH "Enable precompiled headers" OFF)
        option(cxlisp_ENABLE_CACHE "Enable ccache" ON)
    endif()

    if(NOT PROJECT_IS_TOP_LEVEL)
        mark_as_advanced(
                cxlisp_ENABLE_IPO
                cxlisp_WARNINGS_AS_ERRORS
                cxlisp_ENABLE_USER_LINKER
                cxlisp_ENABLE_SANITIZER_ADDRESS
                cxlisp_ENABLE_SANITIZER_LEAK
                cxlisp_ENABLE_SANITIZER_UNDEFINED
                cxlisp_ENABLE_SANITIZER_THREAD
                cxlisp_ENABLE_SANITIZER_MEMORY
                cxlisp_ENABLE_UNITY_BUILD
                cxlisp_ENABLE_CLANG_TIDY
                cxlisp_ENABLE_CPPCHECK
                cxlisp_ENABLE_COVERAGE
                cxlisp_ENABLE_PCH
                cxlisp_ENABLE_CACHE)
    endif()

    cxlisp_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
    if(LIBFUZZER_SUPPORTED AND (cxlisp_ENABLE_SANITIZER_ADDRESS OR cxlisp_ENABLE_SANITIZER_THREAD OR cxlisp_ENABLE_SANITIZER_UNDEFINED))
        set(DEFAULT_FUZZER ON)
    else()
        set(DEFAULT_FUZZER OFF)
    endif()

    option(cxlisp_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(cxlisp_global_options)
    if(cxlisp_ENABLE_IPO)
        include(cmake/InterproceduralOptimization.cmake)
        cxlisp_enable_ipo()
    endif()

    cxlisp_supports_sanitizers()

    if(cxlisp_ENABLE_HARDENING AND cxlisp_ENABLE_GLOBAL_HARDENING)
        include(cmake/Hardening.cmake)
        if(NOT SUPPORTS_UBSAN
                OR cxlisp_ENABLE_SANITIZER_UNDEFINED
                OR cxlisp_ENABLE_SANITIZER_ADDRESS
                OR cxlisp_ENABLE_SANITIZER_THREAD
                OR cxlisp_ENABLE_SANITIZER_LEAK)
            set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
        else()
            set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
        endif()
        message("${cxlisp_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${cxlisp_ENABLE_SANITIZER_UNDEFINED}")
        cxlisp_enable_hardening(cxlisp_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
    endif()
endmacro()

macro(cxlisp_local_options)
    if(PROJECT_IS_TOP_LEVEL)
        include(cmake/StandardProjectSettings.cmake)
    endif()

    add_library(cxlisp_warnings INTERFACE)
    add_library(cxlisp_options INTERFACE)

    include(cmake/CompilerWarnings.cmake)
    cxlisp_set_project_warnings(
            cxlisp_warnings
            ${cxlisp_WARNINGS_AS_ERRORS}
            ""
            ""
            ""
            "")

    if(cxlisp_ENABLE_USER_LINKER)
        include(cmake/Linker.cmake)
        configure_linker(cxlisp_options)
    endif()

    include(cmake/Sanitizers.cmake)
    cxlisp_enable_sanitizers(
            cxlisp_options
            ${cxlisp_ENABLE_SANITIZER_ADDRESS}
            ${cxlisp_ENABLE_SANITIZER_LEAK}
            ${cxlisp_ENABLE_SANITIZER_UNDEFINED}
            ${cxlisp_ENABLE_SANITIZER_THREAD}
            ${cxlisp_ENABLE_SANITIZER_MEMORY})

    set_target_properties(cxlisp_options PROPERTIES UNITY_BUILD ${cxlisp_ENABLE_UNITY_BUILD})

    if(cxlisp_ENABLE_PCH)
        target_precompile_headers(
                cxlisp_options
                INTERFACE
                <vector>
                <string>
                <utility>)
    endif()

    if(cxlisp_ENABLE_CACHE)
        include(cmake/Cache.cmake)
        cxlisp_enable_cache()
    endif()

    include(cmake/StaticAnalyzers.cmake)
    if(cxlisp_ENABLE_CLANG_TIDY)
        cxlisp_enable_clang_tidy(cxlisp_options ${cxlisp_WARNINGS_AS_ERRORS})
    endif()

    if(cxlisp_ENABLE_CPPCHECK)
        cxlisp_enable_cppcheck(${cxlisp_WARNINGS_AS_ERRORS} "" # override cppcheck options
        )
    endif()

    if(cxlisp_ENABLE_COVERAGE)
        include(cmake/Tests.cmake)
        cxlisp_enable_coverage(cxlisp_options)
    endif()

    if(cxlisp_WARNINGS_AS_ERRORS)
        check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
        if(LINKER_FATAL_WARNINGS)
            # This is not working consistently, so disabling for now
            # target_link_options(cxlisp_options INTERFACE -Wl,--fatal-warnings)
        endif()
    endif()

    if(cxlisp_ENABLE_HARDENING AND NOT cxlisp_ENABLE_GLOBAL_HARDENING)
        include(cmake/Hardening.cmake)
        if(NOT SUPPORTS_UBSAN
                OR cxlisp_ENABLE_SANITIZER_UNDEFINED
                OR cxlisp_ENABLE_SANITIZER_ADDRESS
                OR cxlisp_ENABLE_SANITIZER_THREAD
                OR cxlisp_ENABLE_SANITIZER_LEAK)
            set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
        else()
            set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
        endif()
        cxlisp_enable_hardening(cxlisp_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
    endif()

endmacro()