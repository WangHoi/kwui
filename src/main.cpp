#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "scene2d/scene2d.h"
#include "script/script.h"
#include "windows/graphics/GraphicDevice.h"
#include "windows/Dialog.h"

int main()
{
    base::initialize_log();
    /*
    script::Context ctx;
    ctx.loadFile("d:/projects/kwui/test.js");
    LOG(INFO) << "started";
    scene2d::Node actor(scene2d::NodeType::NODE_ELEMENT);
    LOG(INFO) << "end";
    */
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    windows::graphics::GraphicDevice::get()->Init();

    int exit_code = 0;
    MSG msg;
    PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE);

    windows::Dialog diag(640, 480,L"d", NULL, 0, absl::nullopt, absl::nullopt);
    diag.Show();

    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CoUninitialize();
    return exit_code;
}