cmake_minimum_required(VERSION 3.13)
project(libslic3r)

include(PrecompiledHeader)

string(TIMESTAMP COMPILE_TIME %Y%m%d-%H%M%S)
set(SLIC3R_BUILD_TIME ${COMPILE_TIME})
message("aaaa SLIC3R_APP_NAME=${SLIC3R_APP_NAME}")
if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/libslic3r_version.h")
    if(GENERATE_ORCA_HEADER)
        message("libslic3r_version is exists")
        file(READ "${CMAKE_CURRENT_SOURCE_DIR}/libslic3r_version.h.in" SLIC3R_VERSION_FILE OFFSET 2 LIMIT 1)
        if("${SLIC3R_VERSION_FILE}" MATCHES  "1")
            message(STATUS "need to update libslic3r_version.h")
            configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libslic3r_version.h.in ${CMAKE_CURRENT_BINARY_DIR}/libslic3r_version.h @ONLY)
        endif()
    endif()
else()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libslic3r_version.h.in ${CMAKE_CURRENT_BINARY_DIR}/libslic3r_version.h @ONLY)
endif()
message("xxxx SLIC3R_APP_NAME=${SLIC3R_APP_NAME}")

if (MINGW)
    add_compile_options(-Wa,-mbig-obj)
endif ()

set(OpenVDBUtils_SOURCES "")

set(AUTOMATION_TOOL OFF)
#if(${PROJECT_VERSION_EXTRA} STREQUAL "Alpha")
#    set(AUTOMATION_TOOL ON)
#endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libslic3r_version.h.in ${CMAKE_CURRENT_BINARY_DIR}/libslic3r_version.h)

if (TARGET openvdb_libs)
    set(OpenVDBUtils_SOURCES OpenVDBUtils.cpp OpenVDBUtils.hpp)
endif()

set(lisbslic3r_sources
    ArcFitter.cpp
    ArcFitter.hpp
    pchheader.cpp
    pchheader.hpp
    AABBTreeIndirect.hpp
    AABBTreeLines.hpp
    AABBMesh.hpp
    AABBMesh.cpp
    Algorithm/PathSorting.hpp
    Algorithm/RegionExpansion.hpp
    Algorithm/RegionExpansion.cpp
    AnyPtr.hpp
    BoundingBox.cpp
    BoundingBox.hpp
    BridgeDetector.cpp
    BridgeDetector.hpp
    FaceDetector.cpp
    FaceDetector.hpp
    Brim.cpp
    Brim.hpp
    BuildVolume.cpp
    BuildVolume.hpp
    Circle.cpp
    Circle.hpp
    clipper.cpp
    clipper.hpp
    ClipperUtils.cpp
    ClipperUtils.hpp
    Clipper2Utils.cpp
    Clipper2Utils.hpp
    ClipperZUtils.hpp
    Color.cpp
    Color.hpp
    Config.cpp
    Config.hpp
    CurveAnalyzer.cpp
    CurveAnalyzer.hpp
    EdgeGrid.cpp
    EdgeGrid.hpp
    ElephantFootCompensation.cpp
    ElephantFootCompensation.hpp
    Emboss.cpp
    Emboss.hpp
    EmbossShape.hpp
    enum_bitmask.hpp
    ExPolygon.cpp
    ExPolygon.hpp
    ExPolygonSerialize.hpp
    ExPolygonsIndex.cpp
    ExPolygonsIndex.hpp
    Extruder.cpp
    Extruder.hpp
    ExtrusionEntity.cpp
    ExtrusionEntity.hpp
    ExtrusionEntityCollection.cpp
    ExtrusionEntityCollection.hpp
    ExtrusionSimulator.cpp
    ExtrusionSimulator.hpp
    FileParserError.hpp
    Fill/Fill.cpp
    Fill/Fill.hpp
    Fill/Fill3DHoneycomb.cpp
    Fill/Fill3DHoneycomb.hpp
    Fill/FillAdaptive.cpp
    Fill/FillAdaptive.hpp
    Fill/FillBase.cpp
    Fill/FillBase.hpp
    Fill/FillConcentric.cpp
    Fill/FillConcentric.hpp
    Fill/FillConcentricInternal.cpp
    Fill/FillConcentricInternal.hpp
    Fill/FillCrossHatch.cpp
    Fill/FillCrossHatch.hpp
    Fill/FillHoneycomb.cpp
    Fill/FillHoneycomb.hpp
	Fill/FillCross.cpp
    Fill/FillCross.hpp
    Fill/FillGyroid.cpp
    Fill/FillGyroid.hpp
    Fill/FillPlanePath.cpp
    Fill/FillPlanePath.hpp
    Fill/FillLine.cpp
    Fill/FillLine.hpp
    Fill/FillLightning.cpp
    Fill/FillLightning.hpp
    Fill/Lightning/DistanceField.cpp
    Fill/Lightning/DistanceField.hpp
    Fill/Lightning/Generator.cpp
    Fill/Lightning/Generator.hpp
    Fill/Lightning/Layer.cpp
    Fill/Lightning/Layer.hpp
    Fill/Lightning/TreeNode.cpp
    Fill/Lightning/TreeNode.hpp
	Fill/Cross/SierpinskiFillProvider.h
    Fill/Cross/SierpinskiFillProvider.cpp
    Fill/Cross/SierpinskiFill.h
    Fill/Cross/SierpinskiFill.cpp
    Fill/Cross/ImageBasedDensityProvider.h
    Fill/Cross/ImageBasedDensityProvider.cpp
    Fill/FillRectilinear.cpp
    Fill/FillRectilinear.hpp
	Fill/FillQuarter.cpp
    Fill/FillQuarter.hpp
	Fill/FillTetrahedral.cpp
    Fill/FillTetrahedral.hpp
	Fill/Quarter/NoZigZagConnectorProcessor.cpp
    Fill/Quarter/NoZigZagConnectorProcessor.h
	Fill/Quarter/ZigzagConnectorProcessor.cpp
    Fill/Quarter/ZigzagConnectorProcessor.h
    Fill/FillTpmsD.cpp
    Fill/FillTpmsD.hpp
    Flow.cpp
    Flow.hpp
    FlushVolCalc.cpp
    FlushVolCalc.hpp
    format.hpp
    Format/3mf.cpp
    Format/3mf.hpp
    Format/bbs_3mf.cpp
    Format/bbs_3mf.hpp
    Format/AMF.cpp
    Format/AMF.hpp
    Format/OBJ.cpp
    Format/OBJ.hpp
    Format/objparser.cpp
    Format/objparser.hpp
    Format/STEP.cpp
    Format/STEP.hpp
    Format/STL.cpp
    Format/STL.hpp
    Format/SL1.hpp
    Format/SL1.cpp
	Format/svg.hpp
    Format/svg.cpp
    Format/ZipperArchiveImport.hpp
    Format/ZipperArchiveImport.cpp
	Format/AssimpModel.cpp
    Format/AssimpModel.hpp
    GCode/ThumbnailData.cpp
    GCode/ThumbnailData.hpp
    GCode/CoolingBuffer.cpp
    GCode/CoolingBuffer.hpp
	GCode/FanMover.cpp
    GCode/FanMover.hpp
    GCode/PostProcessor.cpp
    GCode/PostProcessor.hpp
    GCode/PressureEqualizer.cpp
    GCode/PressureEqualizer.hpp
    GCode/PrintExtents.cpp
    GCode/PrintExtents.hpp
    GCode/RetractWhenCrossingPerimeters.cpp
    GCode/RetractWhenCrossingPerimeters.hpp
    GCode/SmallAreaInfillFlowCompensator.cpp
    GCode/SmallAreaInfillFlowCompensator.hpp
    GCode/SpiralVase.cpp
    GCode/SpiralVase.hpp
    GCode/SeamPlacer.cpp
    GCode/SeamPlacer.hpp
    GCode/ToolOrdering.cpp
    GCode/ToolOrdering.hpp
    GCode/WipeTower.cpp
    GCode/WipeTower.hpp
    GCode/WipeTower2.cpp
    GCode/WipeTower2.hpp
    GCode/GCodeProcessor.cpp
    GCode/GCodeProcessor.hpp
    GCode/AvoidCrossingPerimeters.cpp
    GCode/AvoidCrossingPerimeters.hpp
    GCode/ExtrusionProcessor.hpp
    GCode/ConflictChecker.cpp
    GCode/ConflictChecker.hpp
	GCode/SmoothSpeedAndAcc.cpp
    GCode/SmoothSpeedAndAcc.hpp
    GCode.cpp
    GCode.hpp
    GCodeReader.cpp
    GCodeReader.hpp
    # GCodeSender.cpp
    # GCodeSender.hpp
    GCodeWriter.cpp
    GCodeWriter.hpp
    Geometry.cpp
    Geometry.hpp
    Geometry/Bicubic.hpp
    Geometry/Circle.cpp
    Geometry/Circle.hpp
    Geometry/ConvexHull.cpp
    Geometry/ConvexHull.hpp
    Geometry/Curves.hpp
    Geometry/MedialAxis.cpp
    Geometry/MedialAxis.hpp
	Geometry/Voronoi.cpp
    Geometry/Voronoi.hpp
    Geometry/VoronoiOffset.cpp
    Geometry/VoronoiOffset.hpp
	Geometry/VoronoiUtils.hpp
    Geometry/VoronoiUtils.cpp
    Geometry/VoronoiUtilsCgal.cpp
    Geometry/VoronoiUtilsCgal.hpp
    Geometry/VoronoiVisualUtils.hpp
    Int128.hpp
    KDTreeIndirect.hpp
    Layer.cpp
    Layer.hpp
    LayerRegion.cpp
    libslic3r.h
    libslic3r.cpp
    Line.cpp
    Line.hpp
    BlacklistedLibraryCheck.cpp
    BlacklistedLibraryCheck.hpp
    LocalesUtils.cpp
    LocalesUtils.hpp
    CutUtils.cpp
    CutUtils.hpp
    Model.cpp
    Model.hpp
    ModelArrange.hpp
    ModelArrange.cpp
    MultiMaterialSegmentation.cpp
    MultiMaterialSegmentation.hpp
    Measure.hpp
    Measure.cpp
    MeasureUtils.hpp
    CustomGCode.cpp
    CustomGCode.hpp
    Arrange.hpp
    Arrange.cpp
    NormalUtils.cpp
    NormalUtils.hpp
    ObjColorUtils.hpp
    Orient.hpp
    Orient.cpp
    MultiPoint.cpp
    MultiPoint.hpp
    MutablePriorityQueue.hpp
    NSVGUtils.cpp
    NSVGUtils.hpp
    ObjectID.cpp
    ObjectID.hpp
    OverhangOptimization.cpp
    OverhangOptimization.hpp
	ParameterUtils.cpp
    ParameterUtils.hpp
    PerimeterGenerator.cpp
    PerimeterGenerator.hpp
    PlaceholderParser.cpp
    PlaceholderParser.hpp
    Platform.cpp
    Platform.hpp
    Point.cpp
    Point.hpp
    Polygon.cpp
    Polygon.hpp
    MutablePolygon.cpp
    MutablePolygon.hpp
    PolygonTrimmer.cpp
    PolygonTrimmer.hpp
    Polyline.cpp
    Polyline.hpp
    Preset.cpp
    Preset.hpp
    PresetBundle.cpp
    PresetBundle.hpp
    ProjectTask.cpp
    ProjectTask.hpp
    PrincipalComponents2D.hpp
    PrincipalComponents2D.cpp
    AppConfig.cpp
    AppConfig.hpp
    Print.cpp
    Print.hpp
    PrintApply.cpp
    PrintBase.cpp
    PrintBase.hpp
    PrintConfig.cpp
    PrintConfig.hpp
    PrintObject.cpp
    PrintObjectSlice.cpp
    PrintRegion.cpp
    PNGReadWrite.hpp
    PNGReadWrite.cpp
    QuadricEdgeCollapse.cpp
    QuadricEdgeCollapse.hpp
    Semver.cpp
    ShortEdgeCollapse.cpp
    ShortEdgeCollapse.hpp
    ShortestPath.cpp
    ShortestPath.hpp
    SLAPrint.cpp
    SLAPrintSteps.cpp
    SLAPrintSteps.hpp
    SLAPrint.hpp
    Slicing.cpp
    Slicing.hpp
    SlicesToTriangleMesh.hpp
    SlicesToTriangleMesh.cpp
    SlicingAdaptive.cpp
    SlicingAdaptive.hpp
    #Support/SupportCommon.cpp
    #Support/SupportCommon.hpp
    #Support/SupportDebug.cpp
    #Support/SupportDebug.hpp
    #Support/SupportLayer.hpp
    # Support/SupportMaterial.cpp
    # Support/SupportMaterial.hpp
    #Support/SupportParameters.cpp
    #Support/SupportParameters.hpp
    #Support/OrganicSupport.cpp
    #Support/OrganicSupport.hpp
    #Support/TreeSupport.cpp
    #Support/TreeSupport.hpp
    #Support/TreeSupportCommon.cpp
    #Support/TreeSupportCommon.hpp
    #Support/TreeModelVolumes.cpp
    #Support/TreeModelVolumes.hpp
    #SupportMaterial.cpp
    #SupportMaterial.hpp
	support_new/SupportCommon.cpp
	support_new/SupportCommon.hpp
	support_new/SupportLayer.hpp
	support_new/SupportMaterial.cpp
	support_new/SupportMaterial.hpp
	support_new/SupportParameters.hpp
	support_new/TreeModelVolumes.cpp
	support_new/TreeModelVolumes.hpp
	support_new/TreeSupport.cpp
	support_new/TreeSupport.hpp
	support_new/TreeSupport3D.cpp
	support_new/TreeSupport3D.hpp
	support_new/TreeSupportCommon.hpp
	
    PrincipalComponents2D.cpp
    PrincipalComponents2D.hpp
    #SupportSpotsGenerator.cpp
    #SupportSpotsGenerator.hpp
    #TreeSupport.hpp 
    #TreeSupport.cpp
    MinimumSpanningTree.hpp
    MinimumSpanningTree.cpp
    Surface.cpp
    Surface.hpp
    SurfaceCollection.cpp
    SurfaceCollection.hpp
    SurfaceMesh.hpp
    SVG.cpp
    SVG.hpp
    Technologies.hpp
    Tesselate.cpp
    Tesselate.hpp
    TextConfiguration.hpp
    TriangleMesh.cpp
    TriangleMesh.hpp
    TriangleMeshSlicer.cpp
    TriangleMeshSlicer.hpp
    MeshSplitImpl.hpp
    TriangulateWall.hpp
    TriangulateWall.cpp
    utils.cpp
    Utils.hpp
    Time.cpp
    Time.hpp
    Timer.cpp
    Timer.hpp
    Thread.cpp
    Thread.hpp
    TriangleSelector.cpp
    TriangleSelector.hpp
    TriangleSetSampling.cpp
    TriangleSetSampling.hpp
    MTUtils.hpp
    VariableWidth.cpp
    VariableWidth.hpp
    Zipper.hpp
    Zipper.cpp
    MinAreaBoundingBox.hpp
    MinAreaBoundingBox.cpp
    miniz_extension.hpp
    miniz_extension.cpp
    MarchingSquares.hpp
    Execution/Execution.hpp
    Execution/ExecutionSeq.hpp
    Execution/ExecutionTBB.hpp
    Optimize/Optimizer.hpp
    Optimize/NLoptOptimizer.hpp
    Optimize/BruteforceOptimizer.hpp
    SLA/Pad.hpp
    SLA/Pad.cpp
    SLA/SupportTreeBuilder.hpp
    SLA/SupportTreeMesher.hpp
    SLA/SupportTreeMesher.cpp
    SLA/SupportTreeBuildsteps.hpp
    SLA/SupportTreeBuildsteps.cpp
    SLA/SupportTreeBuilder.cpp
    SLA/Concurrency.hpp
    SLA/SupportTree.hpp
    SLA/SupportTree.cpp
#    SLA/SupportTreeIGL.cpp
    SLA/Rotfinder.hpp
    SLA/Rotfinder.cpp
    SLA/BoostAdapter.hpp
    SLA/SpatIndex.hpp
    SLA/SpatIndex.cpp
    SLA/RasterBase.hpp
    SLA/RasterBase.cpp
    SLA/AGGRaster.hpp
    SLA/RasterToPolygons.hpp
    SLA/RasterToPolygons.cpp
    SLA/ConcaveHull.hpp
    SLA/ConcaveHull.cpp
    SLA/Hollowing.hpp
    SLA/Hollowing.cpp
    SLA/JobController.hpp
    SLA/SupportPoint.hpp
    SLA/SupportPointGenerator.hpp
    SLA/SupportPointGenerator.cpp
    SLA/IndexedMesh.hpp
    SLA/IndexedMesh.cpp
    SLA/Clustering.hpp
    SLA/Clustering.cpp
    SLA/ReprojectPointsOnMesh.hpp
    Arachne/BeadingStrategy/BeadingStrategy.hpp
    Arachne/BeadingStrategy/BeadingStrategy.cpp
    Arachne/BeadingStrategy/BeadingStrategyFactory.hpp
    Arachne/BeadingStrategy/BeadingStrategyFactory.cpp
    Arachne/BeadingStrategy/DistributedBeadingStrategy.hpp
    Arachne/BeadingStrategy/DistributedBeadingStrategy.cpp
    Arachne/BeadingStrategy/LimitedBeadingStrategy.hpp
    Arachne/BeadingStrategy/LimitedBeadingStrategy.cpp
    Arachne/BeadingStrategy/OuterWallInsetBeadingStrategy.hpp
    Arachne/BeadingStrategy/OuterWallInsetBeadingStrategy.cpp
    Arachne/BeadingStrategy/RedistributeBeadingStrategy.hpp
    Arachne/BeadingStrategy/RedistributeBeadingStrategy.cpp
    Arachne/BeadingStrategy/WideningBeadingStrategy.hpp
    Arachne/BeadingStrategy/WideningBeadingStrategy.cpp
    Arachne/utils/ExtrusionJunction.hpp
    Arachne/utils/ExtrusionJunction.cpp
    Arachne/utils/ExtrusionLine.hpp
    Arachne/utils/ExtrusionLine.cpp
    Arachne/utils/HalfEdge.hpp
    Arachne/utils/HalfEdgeGraph.hpp
    Arachne/utils/HalfEdgeNode.hpp
    Arachne/utils/SparseGrid.hpp
    Arachne/utils/SparsePointGrid.hpp
    Arachne/utils/SparseLineGrid.hpp
    Arachne/utils/SquareGrid.hpp
    Arachne/utils/SquareGrid.cpp
    Arachne/utils/PolygonsPointIndex.hpp
    Arachne/utils/PolygonsSegmentIndex.hpp
    Arachne/utils/PolylineStitcher.hpp
    Arachne/utils/PolylineStitcher.cpp
    Arachne/SkeletalTrapezoidation.hpp
    Arachne/SkeletalTrapezoidation.cpp
    Arachne/SkeletalTrapezoidationEdge.hpp
    Arachne/SkeletalTrapezoidationGraph.hpp
    Arachne/SkeletalTrapezoidationGraph.cpp
    Arachne/SkeletalTrapezoidationJoint.hpp
    Arachne/WallToolPaths.hpp
    Arachne/WallToolPaths.cpp
    Shape/TextShape.hpp
    Shape/TextShape.cpp
    calib.hpp
    calib.cpp
	GCode/Thumbnails.cpp
	GCode/Thumbnails.hpp
	Interlocking/InterlockingGenerator.hpp
    Interlocking/InterlockingGenerator.cpp
	Interlocking/VoxelUtils.hpp
    Interlocking/VoxelUtils.cpp
    GlobalConfig.cpp
    GlobalConfig.hpp
)
list(APPEND lisbslic3r_sources
    common_header/common_header.h
    common_header/common_header.cpp
)
if(WIN32)
    list(APPEND lisbslic3r_sources
        baseline.hpp
        baseline.cpp
        baselineorcinput.hpp
        baselineorcinput.cpp
        UnittestFlow.cpp
        UnittestFlow.hpp
        AutomationMgr.hpp
        AutomationMgr.cpp
    )
endif()

if (APPLE)
    list(APPEND lisbslic3r_sources
			MacUtils.mm
            Format/ModelIO.hpp
            Format/ModelIO.mm
        )
endif ()

include(creality.cmake)

add_library(libslic3r STATIC ${lisbslic3r_sources} 
    "${CMAKE_CURRENT_BINARY_DIR}/libslic3r_version.h"
    "${CMAKE_CURRENT_BINARY_DIR}/buildinfo.h"
    ${OpenVDBUtils_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${lisbslic3r_sources})

if (SLIC3R_STATIC)
    set(CGAL_Boost_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
endif ()
set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE ON CACHE BOOL "" FORCE)

cmake_policy(PUSH)
cmake_policy(SET CMP0011 NEW)
find_package(CGAL REQUIRED)
find_package(OpenCV REQUIRED core)
cmake_policy(POP)

set(lisbslic3r_cgal_sources    CutSurface.hpp CutSurface.cpp
            IntersectionPoints.hpp IntersectionPoints.cpp
            TryCatchSignal.hpp TryCatchSignal.cpp
            Triangulation.hpp Triangulation.cpp)
if(BUILD_CLOUD_SLICER)
    add_definitions(-DCLOUD_SKIP_MESHBOOLEAN)
else()
    list(APPEND lisbslic3r_cgal_sources MeshBoolean.hpp MeshBoolean.cpp)
endif()    

add_library(libslic3r_cgal STATIC ${lisbslic3r_cgal_sources})

target_include_directories(libslic3r_cgal PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Reset compile options of libslic3r_cgal. Despite it being linked privately, CGAL options
# (-frounding-math) still propagate to dependent libs which is not desired.
get_target_property(_cgal_tgt CGAL::CGAL ALIASED_TARGET)
if (NOT TARGET ${_cgal_tgt})
    set (_cgal_tgt CGAL::CGAL)
endif ()
get_target_property(_opts ${_cgal_tgt} INTERFACE_COMPILE_OPTIONS)
if (_opts)
    set(_opts_bad "${_opts}")
    set(_opts_good "${_opts}")
    list(FILTER _opts_bad INCLUDE REGEX frounding-math)
    list(FILTER _opts_good EXCLUDE REGEX frounding-math)
    set_target_properties(${_cgal_tgt} PROPERTIES INTERFACE_COMPILE_OPTIONS "${_opts_good}")
    target_compile_options(libslic3r_cgal PRIVATE "${_opts_bad}")
endif()

target_link_libraries(libslic3r_cgal PRIVATE ${_cgal_tgt} libigl mcut)
if(TARGET cereal::cereal)
    target_link_libraries(libslic3r_cgal PRIVATE cereal::cereal)
endif()

target_link_libraries(libslic3r_cgal PRIVATE tbb_libs)

if (MSVC AND "${CMAKE_SIZEOF_VOID_P}" STREQUAL "4") # 32 bit MSVC workaround
    target_compile_definitions(libslic3r_cgal PRIVATE CGAL_DO_NOT_USE_MPZF)
endif ()

encoding_check(libslic3r)

target_compile_definitions(libslic3r PUBLIC -DUSE_TBB -DTBB_USE_CAPTURED_EXCEPTION=0)
target_include_directories(libslic3r PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(libslic3r PUBLIC ${EXPAT_INCLUDE_DIRS})

# Find the OCCT and related libraries
set(OpenCASCADE_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/occt")
find_package(OpenCASCADE REQUIRED)
target_include_directories(libslic3r PUBLIC ${OpenCASCADE_INCLUDE_DIR})

find_package(JPEG REQUIRED)

set(OCCT_LIBS
    TKXDESTEP
    TKSTEP
    TKSTEP209
    TKSTEPAttr
    TKSTEPBase
    TKXCAF
    TKXSBase
    TKVCAF
    TKCAF
    TKLCAF
    TKCDF
    TKV3d
    TKService
    TKMesh
    TKBO
    TKPrim
    TKHLR
    TKShHealing
    TKTopAlgo
    TKGeomAlgo
    TKBRep
    TKGeomBase
    TKG3d
    TKG2d
    TKMath
    TKernel
)

target_link_libraries(libslic3r
    libnest2d
    admesh
    cereal::cereal
    libigl
    miniz
    boost_libs
    clipper
    ${EXPAT_LIBRARIES}
    glu-libtess
    qhull
    semver
    tbb_libs
    libslic3r_cgal
    ${CMAKE_DL_LIBS}
    PNG::PNG
    ZLIB::ZLIB
    ${OCCT_LIBS}
    Clipper2
    mcut
    JPEG::JPEG
    qoi
    )

if(TARGET opencv_world)
    target_link_libraries(libslic3r opencv_world)
endif()

if(TARGET opencv::opencv)
    target_link_libraries(libslic3r opencv::opencv)
endif()

if(TARGET nowide)
    target_link_libraries(libslic3r nowide)
endif()

target_link_libraries(libslic3r ${_cgal_tgt})

if(TARGET openssl::openssl)
    target_link_libraries(libslic3r_cgal PRIVATE openssl::openssl)
    target_link_libraries(libslic3r openssl::openssl)
endif()

if(NOT BUILD_CLOUD_SLICER)
    if(NOT WIN32)
        target_link_libraries(libslic3r freetype)
        if (NOT APPLE)
            target_link_libraries(libslic3r fontconfig)
        endif()
    endif()
endif()

if (APPLE)
    find_library(FOUNDATION Foundation REQUIRED)
    find_library(MODELIO ModelIO REQUIRED)
    target_link_libraries(libslic3r ${FOUNDATION} ${MODELIO})
endif ()

if (TARGET openvdb_libs)
    target_link_libraries(libslic3r openvdb_libs)
endif()

if(WIN32)
    target_link_libraries(libslic3r Psapi.lib)
endif()

if(SLIC3R_PROFILE)
    target_link_libraries(libslic3r Shiny)
endif()

if (SLIC3R_PCH AND NOT SLIC3R_SYNTAXONLY)
    add_precompiled_header(libslic3r pchheader.hpp FORCEINCLUDE)
endif ()
