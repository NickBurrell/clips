include(cmake/CPM.cmake)

function(cxlisp_setup_dependencies)

    if(NOT TARGET fmtlib::fmtlib)
        cpmaddpackage("gh:fmtlib/fmt#9.1.0")
    endif()

    if(NOT TARGET spdlog::spdlog)
        cpmaddpackage(
                NAME
                spdlog
                VERSION
                1.11.0
                GITHUB_REPOSITORY
                "gabime/spdlog"
                OPTIONS
                "SPDLOG_FMT_EXTERNAL ON")
    endif()

    if(NOT TARGET Catch2::Catch2WithMain)
        cpmaddpackage("gh:catchorg/Catch2@3.3.2")
    endif()

    if(NOT TARGET tools::tools)
        cpmaddpackage("gh:lefticus/tools#update_build_system")
    endif()

endfunction()