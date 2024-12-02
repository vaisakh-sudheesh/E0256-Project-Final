
function(set_version VERSION)
  # Get Major and Minor from Version.txt
  file(STRINGS "Version.txt" VERSION_STR)
  string(REGEX MATCH "PRJ_MAJOR_VERSION=([0-9]+)" _ ${VERSION_STR})
  set(MAJOR_VERSION ${CMAKE_MATCH_1})
  string(REGEX MATCH "PRJ_MINOR_VERSION=([0-9]+)" _ ${VERSION_STR})
  set(MINOR_VERSION ${CMAKE_MATCH_1})
  # Set the version
  set(${VERSION} "${MAJOR_VERSION}.${MINOR_VERSION}")
endfunction()


include(ProcessorCount)
ProcessorCount(CPU_COUNT)

if(NOT CPU_COUNT EQUAL 0)
    message(STATUS "Number of processors: ${CPU_COUNT}")
    if (CPU_COUNT GREATER 16)
        math(EXPR CPU_COUNT "${CPU_COUNT}")
    elseif (CPU_COUNT GREATER 8)
        math(EXPR CPU_COUNT "${CPU_COUNT}/2")
    endif()
else()
    message(WARNING "Failed to determine number of processors")
    set(CPU_COUNT 8)
endif()