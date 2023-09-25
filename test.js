"use strict";
"use math";


function JSX(tag,atts,kids) {
	return [tag,atts,kids];
}

function main() {
	var a = <div id="aa">Hello world!</div>;
	console.log(JSON.stringify(a));
}

main();
