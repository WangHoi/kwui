"use strict";
"use math";

globalThis.scriptAdd = function (a, b) {
	return a + b;
}

function event_handler(event, arg) {
	console.log("script recv: event", event, ", arg", arg);
}

console.log("native add 2 + 3 =", add(2, 3));
console.log("post event from script:");
app.post("test-event", "abc")
app.addListener("test-event", event_handler);