add_subdirectory(graphics)
add_subdirectory(control)

target_sources(kwui PUBLIC
    Dialog.h
    PopupShadow.h
    EncodingManager.h)
target_sources(kwui PRIVATE
    Dialog.cpp
    PopupShadow.cpp
    EncodingManager.cpp
    theme.h
    theme.cpp)
