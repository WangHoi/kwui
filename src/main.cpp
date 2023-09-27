#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "scene2d/scene2d.h"
#include "script/script.h"

int main()
{
    base::initialize_log();
    script::Context ctx;
    ctx.loadFile("d:/projects/kwui/test.js");
    LOG(INFO) << "started";
    scene2d::Node actor(scene2d::NodeType::NODE_ELEMENT);
    LOG(INFO) << "end";
    return 0;
}