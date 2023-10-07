target_sources(kwui PUBLIC
  scene2d.h
)
target_sources(kwui PRIVATE 
  geom_types.h
  Node.h
  Node.cpp
  Scene.h
  Scene.cpp
  Event.h
  Attribute.h
  Control.h
  Control.cpp
  Layout.h
  Layout.cpp
  BoxConstraintSolver.h
  BoxConstraintSolver.cpp
)
