target_sources(kwui PRIVATE
  FILE_SET scene2d
  TYPE CXX_MODULES FILES
  "scene2d.ixx"
  "scene2d-Actor.ixx"
  "scene2d-Stage.ixx"
)
target_sources(kwui PRIVATE 
  Actor.cpp
  Stage.cpp
)
