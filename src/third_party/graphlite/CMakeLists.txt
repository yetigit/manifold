project (graphlite)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.cpp *.h)
file(GLOB_RECURSE SRC_HDS CONFIGURE_DEPENDS *.h)
add_library(${PROJECT_NAME} ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES
                                         PUBLIC_HEADER "${SRC_HDS}"
)



target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_compile_options(${PROJECT_NAME} PRIVATE ${MANIFOLD_FLAGS})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
install(TARGETS ${PROJECT_NAME}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

