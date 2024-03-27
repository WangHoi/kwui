add_executable(skia_raster main.cpp)
target_link_libraries(skia_raster skia tgaimage
d3d12.lib d3dcompiler.lib DXGI.lib OpenGL32.lib)