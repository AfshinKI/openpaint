set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_qml_module(openpaint_ui STATIC
    URI OpenPaint
    VERSION 1.0
    DEPENDENCIES QtQuick
    OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml/OpenPaint"
    SOURCES ../src/render/LayerThumbnail.h ../src/render/LayerThumbnail.cpp ../src/render/ScreenshotPreview.h ../src/render/ScreenshotPreview.cpp ../src/platform/BlurSupport.h ../src/platform/BlurSupport.cpp ../src/platform/Appearance.h ../src/platform/Appearance.cpp ../src/updates/UpdateService.h ../src/updates/UpdateService.cpp ../src/platform/AppInfo.h ../src/platform/AppInfo.cpp ../src/render/ShapePreview.h ../src/render/ShapePreview.cpp ../src/render/BrushPreview.h ../src/render/BrushPreview.cpp ../src/app/Editor.h ../src/app/Editor.cpp ../src/app/EditorSelection.cpp ../src/app/EditorTransform.cpp
        ../src/render/CanvasItem.h ../src/render/CanvasItem.cpp
        ../src/color/ColorSelection.h ../src/color/ColorSelection.cpp
        ../src/color/ColorPalette.h ../src/color/ColorPalette.cpp
        ../src/color/ColorSpectrum.h ../src/color/ColorSpectrum.cpp
    QML_FILES LayerTile.qml LayersPanel.qml WelcomeDialog.qml GlassSetupDialog.qml AppPreview.qml FontPicker.qml GlassSurface.qml AppearanceDialog.qml ResizeHandles.qml ToolSlider.qml ShapeGallery.qml UpdatesDialog.qml ExportDialog.qml ResizeDialog.qml SelectionOptions.qml InlineText.qml TextOptions.qml ShapePalette.qml ShapeOptions.qml BrushMenu.qml ColorEditor.qml ColorSwatch.qml Main.qml Theme.qml PaintButton.qml IconButton.qml PaintMenu.qml PaintMenuItem.qml PaintDialog.qml PaintSpinBox.qml PaintCheckBox.qml
    RESOURCES icons/invert-colors.svg icons/layer-up.svg icons/layer-down.svg icons/layer-merge.svg icons/layer-eye.svg icons/layer-eye-hidden.svg icons/layer-add.svg icons/layers.svg icons/wand.svg previews/glass.png previews/opaque.png icons/eraser-paper.svg icons/shape-outline-off.svg icons/fill-solid.svg icons/shape-outline.svg icons/none.svg icons/settings.svg icons/transparent.svg icons/rotate-right.svg icons/rotate-left.svg icons/rotate-half.svg icons/flip-vertical.svg icons/stroke-size.svg icons/new.svg icons/open.svg icons/export.svg icons/exit.svg icons/copy.svg icons/cut.svg icons/paste.svg icons/delete.svg icons/lasso.svg icons/invert.svg icons/opacity.svg icons/info.svg icons/update.svg icons/menu.svg icons/align-left.svg icons/align-center.svg icons/align-right.svg icons/percent.svg icons/rotate.svg icons/flip.svg icons/app.svg icons/app-dark.svg icons/resize.svg icons/crop.svg icons/selection.svg icons/text.svg icons/line.svg icons/rectangle.svg icons/ellipse.svg icons/brush.svg icons/fill.svg icons/eyedropper.svg icons/save.svg icons/palette.svg icons/redo.svg icons/fit.svg icons/undo.svg icons/swap.svg icons/pencil.svg icons/zoom-in.svg icons/zoom-out.svg icons/eraser.svg
)
target_link_libraries(openpaint_ui PUBLIC openpaint_document openpaint_io Qt6::Network Qt6::Concurrent Qt6::Quick Qt6::QuickControls2)
target_include_directories(openpaint_ui PRIVATE ../src/updates ../src/platform ../src/app ../src/render ../src/color "${PROJECT_BINARY_DIR}/generated")
