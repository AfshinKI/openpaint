set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_qml_module(openpaint_ui STATIC
    URI OpenPaint
    VERSION 1.0
    DEPENDENCIES QtQuick
    OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml/OpenPaint"
    SOURCES ../src/app/Editor.h ../src/app/Editor.cpp
        ../src/render/CanvasItem.h ../src/render/CanvasItem.cpp
        ../src/color/ColorSelection.h ../src/color/ColorSelection.cpp
        ../src/color/ColorPalette.h ../src/color/ColorPalette.cpp
        ../src/color/ColorWheel.h ../src/color/ColorWheel.cpp
    QML_FILES ColorEditor.qml ColorSwatch.qml Main.qml Theme.qml PaintButton.qml IconButton.qml PaintMenu.qml PaintMenuItem.qml PaintDialog.qml PaintSpinBox.qml PaintCheckBox.qml
    RESOURCES icons/save.svg icons/palette.svg icons/redo.svg icons/fit.svg icons/undo.svg icons/swap.svg icons/pencil.svg icons/zoom-in.svg icons/zoom-out.svg icons/eraser.svg
)
target_link_libraries(openpaint_ui PUBLIC openpaint_document openpaint_io Qt6::Concurrent Qt6::Quick Qt6::QuickControls2)
target_include_directories(openpaint_ui PRIVATE ../src/app ../src/render ../src/color)
