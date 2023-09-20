target_sources(quickui PRIVATE
  FILE_SET modules
  TYPE CXX_MODULES FILES
  "scene2d.ixx"
  "scene2d-Actor.ixx"
  "scene2d-Stage.ixx"
)
target_sources(quickui PRIVATE 
  Actor.cpp
  Stage.cpp
)
