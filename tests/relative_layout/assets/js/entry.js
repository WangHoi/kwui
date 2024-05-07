import { Theme } from "./Theme.js";

let props = {
    displayName: "测试产品",
    version: "1.0.1",
};

app.showDialog({
    title: props.displayName + "安装向导",
    width: 552,
    height: 408,
    flags: 1,
    modulePath: "./InstallDialog.js",
    moduleParams: props,
});
