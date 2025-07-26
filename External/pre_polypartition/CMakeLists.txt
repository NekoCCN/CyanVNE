set(CMAKE_CXX_STANDARD 20)

set (POLYPARTITION_SRC
        "polypartition/src/polypartition.cpp"
        "polypartition/src/polypartition.h"
)
add_library(polypartition STATIC ${POLYPARTITION_SRC})

target_include_directories(polypartition PUBLIC polypartition)