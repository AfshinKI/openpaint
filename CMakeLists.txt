cmake_minimum_required(VERSION 3.28)
project(OpenPaint VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(OPENPAINT_SANITIZERS "Enable address and undefined-behavior sanitizers" OFF)
if(OPENPAINT_SANITIZERS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "Sanitizer preset requires GCC or Clang")
    endif()
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Concurrent)
qt_standard_project_setup(REQUIRES 6.8)
include(CTest)
add_library(openpaint_document STATIC src/document/Document.cpp src/tools/Fill.cpp src/tools/Brush.cpp src/tools/Shape.cpp src/tools/Text.cpp src/tools/Transform.cpp)
target_include_directories(openpaint_document PUBLIC src)
target_link_libraries(openpaint_document PUBLIC Qt6::Gui)
target_compile_options(openpaint_document PRIVATE -Wall -Wextra -Wpedantic)
find_package(ZLIB REQUIRED)
add_library(openpaint_io STATIC src/io/PngIo.cpp)
target_link_libraries(openpaint_io PUBLIC openpaint_document PRIVATE ZLIB::ZLIB)
target_compile_options(openpaint_io PRIVATE -Wall -Wextra -Wpedantic)
add_subdirectory(qml)

qt_add_executable(openpaint src/app/main.cpp)
target_link_libraries(openpaint PRIVATE openpaint_ui openpaint_uiplugin Qt6::QuickControls2)
target_compile_options(openpaint PRIVATE -Wall -Wextra -Wpedantic)

if(BUILD_TESTING)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Test)
    qt_add_executable(openpaint_transform_tests tests/unit/tst_transform.cpp)
    target_link_libraries(openpaint_transform_tests PRIVATE openpaint_ui Qt6::Test)
    add_test(NAME transform COMMAND openpaint_transform_tests)
    qt_add_executable(openpaint_clipboard_tests tests/unit/tst_clipboard.cpp)
    target_link_libraries(openpaint_clipboard_tests PRIVATE openpaint_ui Qt6::Test)
    add_test(NAME clipboard COMMAND openpaint_clipboard_tests)
    qt_add_executable(openpaint_text_tests tests/unit/tst_text.cpp)
    target_link_libraries(openpaint_text_tests PRIVATE openpaint_ui Qt6::Test)
    add_test(NAME text COMMAND openpaint_text_tests)
    qt_add_executable(openpaint_canvas_tests tests/unit/tst_canvas.cpp)
    target_link_libraries(openpaint_canvas_tests PRIVATE openpaint_ui Qt6::Test)
    add_test(NAME canvas COMMAND openpaint_canvas_tests)
    qt_add_executable(openpaint_colors_tests tests/unit/tst_colors.cpp)
    target_link_libraries(openpaint_colors_tests PRIVATE openpaint_ui Qt6::Test)
    add_test(NAME colors COMMAND openpaint_colors_tests)
    qt_add_executable(openpaint_editor_tests tests/unit/tst_editor.cpp)
    target_link_libraries(openpaint_editor_tests PRIVATE openpaint_ui Qt6::Test)
    add_test(NAME editor COMMAND openpaint_editor_tests)
    qt_add_executable(openpaint_io_tests tests/unit/tst_pngio.cpp)
    target_link_libraries(openpaint_io_tests PRIVATE openpaint_io Qt6::Test ZLIB::ZLIB)
    add_test(NAME png-io COMMAND openpaint_io_tests)
    qt_add_executable(openpaint_document_tests tests/unit/tst_document.cpp)
    target_link_libraries(openpaint_document_tests PRIVATE openpaint_document Qt6::Test)
    add_test(NAME document COMMAND openpaint_document_tests)
    qt_add_executable(openpaint_shell_tests tests/ui/tst_shell.cpp)
    target_link_libraries(openpaint_shell_tests PRIVATE
        openpaint_ui openpaint_uiplugin Qt6::QuickControls2 Qt6::Test)
    target_compile_options(openpaint_shell_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME shell COMMAND openpaint_shell_tests)
    set_tests_properties(shell PROPERTIES TIMEOUT 30)
    add_test(NAME cli-version COMMAND openpaint --version)
    set_tests_properties(cli-version PROPERTIES
        PASS_REGULAR_EXPRESSION "OpenPaint 0.1.0-dev" TIMEOUT 10)
endif()

find_package(Python3 3.11 REQUIRED COMPONENTS Interpreter)
find_program(CLANG_FORMAT NAMES clang-format-18 clang-format REQUIRED)
add_custom_target(format-check
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/check_format.py"
        --clang-format "${CLANG_FORMAT}" --qmlformat "$<TARGET_FILE:Qt6::qmlformat>"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    VERBATIM)

file(GLOB OPENPAINT_QML_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/qml/*.qml")

# Unlike the generated all_qmllint target, this explicitly fails on warnings.
add_custom_target(qml-lint
    COMMAND Qt6::qmllint --max-warnings 0 -I "${CMAKE_BINARY_DIR}/qml"
        ${OPENPAINT_QML_FILES}
    DEPENDS openpaint_ui
    VERBATIM)
