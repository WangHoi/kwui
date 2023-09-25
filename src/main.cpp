import scene2d;
import script;

#include "quickjs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main() {
    script::Context ctx;
    ctx.loadFile("d:/projects/kwui/test.js");
    scene2d::Actor actor(scene2d::ActorType::ACTOR_ELEMENT);
    return 0;
}