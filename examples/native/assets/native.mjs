"use strict";
"use math";

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
