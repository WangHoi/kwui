"use strict";
"use math";

import { add } from "./test_mod.mjs";

console.log("module add 2 + 3 =", add(2, 3));

globalThis.port = new __EventPort();

globalThis.scriptAdd = function (a, b) {
	return a + b;
}

function event_handler(a) {
	console.log("timeout_handler arg ", a);
	if (a === 3) {
		port.removeListener(event_handler);
		port = undefined;
	}
}

console.log(add(2, 3));
port.addListener(event_handler);
onNativeEvent(5, port);
