cmake_minimum_required(VERSION 3.3...3.12 FATAL_ERROR)


project (QDicom3DViewer)
find_package(VTK REQUIRED)
vtk_module_config(VTK
  vtkCommonCore
  vtkCommonDataModel
  vtkCommonColor
  vtkIOImage
  vtkIOGeometry
  vtkIOXML
  vtkInteractionWidgets
  vtkFiltersSources
  vtkFiltersGeometry
  vtkGUISupportQt
  vtkInteractionImage
  vtkInteractionStyle
  vtkRenderingCore
  vtkRenderingFreeType
  vtkRenderingVolumeOpenGL2
  vtkRenderingAnnotation
  vtkImagingStatistics
  vtkImagingColor
  vtkImagingCore
  vtkImagingGeneral
  vtkImagingMath
  vtkImagingMorphological
  vtkImagingStencil
)
include(${VTK_USE_FILE})

if("${VTK_QT_VERSION}" STREQUAL "")
  message(FATAL_ERROR "VTK was not built with Qt")
endif()

set( Srcs 
    main.cpp 
    mainwindow.cpp 
    volumeRenderProcess.cpp 
    colorTransferFunction.cpp 
    opacityTransferFunction.cpp 
    RangeSlider.cpp 
    boundVisulizer.cpp
    dicomVisualizer.cpp
    roiVisualizer.cpp
)

set( Hdrs 
    mainwindow.h 
    volumeRenderProcess.h 
    colorTransferFunction.h 
    opacityTransferFunction.h 
    transferFunction.h 
    breakPoints.h 
    myQColor.h 
    mySeriesInteractorStyle.h 
    RangeSlider.h 
    seriesVisualizer.h
    boundVisulizer.h 
    dicomVisualizer.h 
    roiVisualizer.h 
)

set( MOC_Hdrs mainwindow.h )

set( UIs mainwindow.ui )

# Use the include path and library for Qt that is used by VTK.
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Widgets Core REQUIRED QUIET)
qt5_wrap_ui(UI_Srcs ${UIs})

add_executable(QDicom3DViewer
  MACOSX_BUNDLE  ${Srcs} ${Hdrs} ${UI_Srcs} ${MOC_Hdrs})
target_link_libraries(QDicom3DViewer Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(QDicom3DViewer ${VTK_LIBRARIES})