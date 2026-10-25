cmake_minimum_required(VERSION 3.28)
project(OpenPaint VERSION 2.0.0 LANGUAGES CXX)
if(APPLE)
    enable_language(OBJCXX)
    set(CMAKE_OBJCXX_STANDARD 20)
    set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
endif()

set(OPENPAINT_VERSION_SUFFIX "" CACHE STRING "SemVer prerelease suffix; empty for stable")
set(OPENPAINT_INSTALL_KIND "source" CACHE STRING "source, portable, appimage, flatpak, system, or macos")
set_property(CACHE OPENPAINT_INSTALL_KIND PROPERTY STRINGS source portable appimage flatpak system macos)
if(NOT OPENPAINT_INSTALL_KIND MATCHES "^(source|portable|appimage|flatpak|system|macos)$")
    message(FATAL_ERROR "Invalid OPENPAINT_INSTALL_KIND")
endif()
set(OPENPAINT_VERSION "${PROJECT_VERSION}")
set(OPENPAINT_CHANNEL "stable")
if(NOT OPENPAINT_VERSION_SUFFIX STREQUAL "")
    if(NOT OPENPAINT_VERSION_SUFFIX MATCHES "^[0-9A-Za-z]+([.-][0-9A-Za-z]+)*$")
        message(FATAL_ERROR "Invalid OPENPAINT_VERSION_SUFFIX")
    endif()
    string(APPEND OPENPAINT_VERSION "-${OPENPAINT_VERSION_SUFFIX}")
    set(OPENPAINT_CHANNEL "prerelease")
    if(OPENPAINT_VERSION_SUFFIX STREQUAL "dev")
        set(OPENPAINT_CHANNEL "development")
    endif()
endif()
configure_file(src/platform/BuildConfig.h.in generated/BuildConfig.h @ONLY)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options("-ffile-prefix-map=${CMAKE_SOURCE_DIR}=."
        "-fdebug-prefix-map=${CMAKE_BINARY_DIR}=./build")
endif()

option(OPENPAINT_SANITIZERS "Enable address and undefined-behavior sanitizers" OFF)
if(OPENPAINT_SANITIZERS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "Sanitizer preset requires GCC or Clang")
    endif()
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Concurrent Network)
qt_standard_project_setup(REQUIRES 6.8)
include(CTest)
add_library(openpaint_document STATIC src/document/Document.cpp src/tools/Fill.cpp src/tools/ColorSelect.cpp src/tools/Brush.cpp src/tools/Shape.cpp src/tools/Text.cpp src/tools/Transform.cpp)
target_include_directories(openpaint_document PUBLIC src)
target_link_libraries(openpaint_document PUBLIC Qt6::Gui)
target_compile_options(openpaint_document PRIVATE -Wall -Wextra -Wpedantic)
find_package(ZLIB REQUIRED)
find_package(LibArchive REQUIRED)
add_library(openpaint_io STATIC src/io/PngIo.cpp src/io/ImageIo.cpp src/io/OraIo.cpp)
target_link_libraries(openpaint_io PUBLIC openpaint_document PRIVATE ZLIB::ZLIB LibArchive::LibArchive)
target_compile_options(openpaint_io PRIVATE -Wall -Wextra -Wpedantic)
add_subdirectory(qml)

qt_add_executable(openpaint src/app/main.cpp)
if(APPLE)
    qt_add_executable(openpaint_icon_tool tools/macos_icon.cpp)
    target_link_libraries(openpaint_icon_tool PRIVATE Qt6::Gui)
    set_target_properties(openpaint PROPERTIES
        MACOSX_BUNDLE TRUE OUTPUT_NAME OpenPaint
        MACOSX_BUNDLE_GUI_IDENTIFIER io.github.AfshinKI.OpenPaint
        MACOSX_BUNDLE_BUNDLE_NAME OpenPaint
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/packaging/macos/Info.plist.in")
endif()
target_link_libraries(openpaint PRIVATE openpaint_ui openpaint_uiplugin Qt6::QuickControls2)
target_compile_options(openpaint PRIVATE -Wall -Wextra -Wpedantic)

if(BUILD_TESTING)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Test)
    if(APPLE)
        qt_add_executable(openpaint_macglass_tests tests/unit/tst_macglass.mm)
        target_link_libraries(openpaint_macglass_tests PRIVATE openpaint_ui Qt6::Test "-framework AppKit")
        set_source_files_properties(tests/unit/tst_macglass.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
        add_test(NAME macos-native-glass COMMAND openpaint_macglass_tests)
        set_tests_properties(macos-native-glass PROPERTIES TIMEOUT 30 ENVIRONMENT "QT_QPA_PLATFORM=cocoa;QT_QUICK_BACKEND=rhi;QSG_RHI_BACKEND=metal")
    endif()
    qt_add_executable(openpaint_layers_tests tests/unit/tst_layers.cpp)
    target_link_libraries(openpaint_layers_tests PRIVATE openpaint_ui Qt6::Test LibArchive::LibArchive)
    add_test(NAME layers COMMAND openpaint_layers_tests)
    qt_add_executable(openpaint_blur_tests tests/unit/tst_blur.cpp)
    target_link_libraries(openpaint_blur_tests PRIVATE openpaint_ui Qt6::Test)
    add_test(NAME blur-support COMMAND openpaint_blur_tests)
    qt_add_executable(openpaint_update_tests tests/unit/tst_updates.cpp)
    target_link_libraries(openpaint_update_tests PRIVATE openpaint_ui Qt6::Test)
    add_test(NAME updates COMMAND openpaint_update_tests)
    qt_add_executable(openpaint_appinfo_tests tests/unit/tst_appinfo.cpp)
    target_link_libraries(openpaint_appinfo_tests PRIVATE openpaint_ui Qt6::Test)
    add_test(NAME app-info COMMAND openpaint_appinfo_tests)
    qt_add_executable(openpaint_imageio_tests tests/unit/tst_imageio.cpp)
    target_link_libraries(openpaint_imageio_tests PRIVATE openpaint_ui Qt6::Test)
    add_test(NAME image-io COMMAND openpaint_imageio_tests)
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
    # Keep each bounded shell shard below 30 seconds as coverage grows.
    # Cocoa's window/animation lifecycle is slower than the offscreen backend.
    # Keep the same 30-second bound for each behavior instead of timing out
    # an entire multi-behavior group before its later cases can execute.
    function(add_shell_tests name)
        if(APPLE)
            foreach(behavior IN LISTS ARGN)
                add_test(NAME "${name}-${behavior}" COMMAND openpaint_shell_tests "${behavior}" -v2)
                set_tests_properties("${name}-${behavior}" PROPERTIES TIMEOUT 30
                    ENVIRONMENT "QT_QPA_PLATFORM=cocoa;QT_QUICK_BACKEND=rhi;QSG_RHI_BACKEND=metal")
            endforeach()
        else()
            add_test(NAME "${name}" COMMAND openpaint_shell_tests ${ARGN})
            set_tests_properties("${name}" PROPERTIES TIMEOUT 30)
        endif()
    endfunction()
    add_shell_tests(shell
        launchAndResize
        toolCursorsAndIndicators
        eraserModeMenu
        themedApplicationIcon
        splineKeyboardWorkflow
        selectAllKeyboard
        floatingToolControls
        quickActionPlacement
        themeActions
        keyboardAboutAndEscape
        drawingAndTransforms
        interruptedStrokeAndRightPan
        welcomeWorkflow
        glassSetupWorkflow
        updatesWorkflow)
    add_shell_tests(shell-editing
        layersWorkflow
        invertColorsWorkflow
        coloredRibbonWorkflow
        rulersWorkflow
        zoomSliderWorkflow
        exportDialogWorkflow
        resizeDialogWorkflow
        ribbonActionMemory
        imageTransformWorkflow
        freeFormSelectionWorkflow
        colorSelectionWorkflow
        clipboardMoveWorkflow
        rectangularSelectionWorkflow)
    add_shell_tests(shell-text
        fontPickerWorkflow
        inlineTextWorkflow
        textRangeFormattingWorkflow
        textInsertionFormatsWorkflow
        transparentBackgroundChoice
        canvasCornerResize
        cornerResizeHandles)
    add_shell_tests(shell-more
        textToolbarOutsideClickCommit
        chordAndPanLifecycle
        unsavedCancelAndSaveClose
        failedSavePreventsClose
        fileDialogCancel
        nativePortalWorkflow
        keyboardFileWorkflow
        topRightControls
        roundedClientCorners
        frostedToolSelection
        glassAppearanceSettings
        glassBackdropRendering
        actualFloatingPanelBlur
        fileMenuBackdropBlur)
    add_shell_tests(shell-controls
        brushMainActivation
        ribbonShapeKeyboard
        shapeStyleMenus
        shapeOptionsKeyboard
        brushMenuKeyboardAndLayout)
    add_shell_tests(shell-colors
        fillOptionsLayout
        keyboardToolsAndAccessibility
        keyboardControlsAndAccessibility
        customColorEditor
        toolbarSlotAndRoleWorkflow
        renderTiming
        quitClosesWindow)
    add_test(NAME cli-version COMMAND openpaint --version)
    set_tests_properties(cli-version PROPERTIES
        PASS_REGULAR_EXPRESSION "OpenPaint ${OPENPAINT_VERSION}" TIMEOUT 10)
endif()

if(BUILD_TESTING)
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
            --resource "${CMAKE_BINARY_DIR}/qml/.qt/rcc/qmake_OpenPaint.qrc"
            --resource "${CMAKE_BINARY_DIR}/qml/.qt/rcc/openpaint_ui_raw_qml_0.qrc"
            ${OPENPAINT_QML_FILES}
        DEPENDS openpaint_ui
        VERBATIM)

endif()

include(GNUInstallDirs)
install(TARGETS openpaint BUNDLE DESTINATION . RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
if(NOT APPLE)
install(FILES packaging/io.github.AfshinKI.OpenPaint.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES packaging/io.github.AfshinKI.OpenPaint.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
install(FILES qml/icons/app.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps RENAME io.github.AfshinKI.OpenPaint.svg)
install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/io.github.AfshinKI.OpenPaint)
install(FILES README.md CHANGELOG.md DESTINATION ${CMAKE_INSTALL_DOCDIR})

endif()
