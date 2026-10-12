set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_qml_module(openpaint_ui STATIC
    URI OpenPaint
    VERSION 1.0
    DEPENDENCIES QtQuick
    OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml/OpenPaint"
    SOURCES ../src/app/Editor.h ../src/app/Editor.cpp
        ../src/render/CanvasItem.h ../src/render/CanvasItem.cpp
    QML_FILES Main.qml Theme.qml
)
target_link_libraries(openpaint_ui PUBLIC openpaint_document Qt6::Quick Qt6::QuickControls2)
target_include_directories(openpaint_ui PRIVATE ../src/app ../src/render)
