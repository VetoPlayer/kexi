ecm_add_tests(
    GlobalSearchTest.cpp
    LINK_LIBRARIES
        Qt5::Test
        keximain
        kexiextendedwidgets
)
target_compile_definitions(GlobalSearchTest
    PRIVATE
        CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        FILES_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}"
)
