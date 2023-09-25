#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "scene2d/scene2d.h"
#include "script/script.h"

int main() {
    script::Context ctx;
    ctx.loadFile("d:/projects/kwui/test.js");
    scene2d::Node actor(scene2d::ActorType::ACTOR_ELEMENT);
    return 0;
}