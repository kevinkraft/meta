project(meta-utf)

add_subdirectory(tools)

message("META_STATIC_UTF: " ${META_STATIC_UTF}) #none
if (META_STATIC_UTF)
  add_library(meta-utf STATIC segmenter.cpp transformer.cpp utf.cpp)
else()
  add_library(meta-utf SHARED segmenter.cpp transformer.cpp utf.cpp)
endif()
message("ICU_LIBRARIES: " ${ICU_LIBRARIES})
message("ICU_INCLUDE_DIRS: " ${ICU_INCLUDE_DIRS})
target_link_libraries(meta-utf PUBLIC meta-definitions)
target_link_libraries(meta-utf PRIVATE ${ICU_LIBRARIES})
target_include_directories(meta-utf PRIVATE SYSTEM ${ICU_INCLUDE_DIRS})

install(TARGETS meta-utf
        EXPORT meta-exports
        DESTINATION lib)
