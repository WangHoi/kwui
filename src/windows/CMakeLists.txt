add_subdirectory(graphics)

target_sources(kwui PUBLIC
    # Dialog.h
    EncodingManager.h)
target_sources(kwui PRIVATE
    # Dialog.cpp
    EncodingManager.cpp)
