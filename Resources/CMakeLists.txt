file(GLOB_RECURSE RESOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.qrc"
)

qt_add_resources(RESOURCE_BUNDLE "${RESOURCES}")

add_library(ResourceLibrary STATIC ${RESOURCE_BUNDLE})
