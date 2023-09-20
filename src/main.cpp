import scene2d;

#include "quickjs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main() {
    JSRuntime *rt = JS_NewRuntime();
    auto ctx = JS_NewContext(rt);
    const char *source = R"foo(
function JSX(tag,atts,kids) {
  return [tag,atts,kids];
}
        <div>Hello world!</div>
        )foo";
    auto val = JS_Eval2(ctx, source, strlen(source), "", 0, 0);
    uint32_t n = 0;
    JSValue *sub = nullptr;
    JS_GetFastArray(ctx, val, &sub, &n);
    printf("ret=%x\n", n);
    printf("tag=%s\n", JS_ToCString(ctx, val));
    scene2d::Actor actor(scene2d::ActorType::ACTOR_ELEMENT);
    return 0;
}