set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_qml_module(openpaint_ui STATIC
    URI OpenPaint
    VERSION 1.0
    OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml/OpenPaint"
    QML_FILES Main.qml Theme.qml
)
target_link_libraries(openpaint_ui PUBLIC Qt6::Quick Qt6::QuickControls2)
