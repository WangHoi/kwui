"use strict";
"use math";

var port = new __EventPort();

function event_handler(a) {
	console.log("timeout_handler arg ", a);
	if (a === 9) {
		port.removeListener(event_handler);
	}
}

console.log(add(2, 3));
port.addListener(event_handler);
onNativeEvent(5, port);
