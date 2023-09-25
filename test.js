"use strict";
"use math";

function JSX(tag,atts,kids) {
	return [tag,atts,kids];
}

class Div extends Object {
	render(props, kids) {
		return <div>
			<p>{props.a}</p>
			{kids}
			<p>next kids</p>
		</div>
	}
};

function main() {
	var a = <Div a="attr1"><p>aa<Div>jjj</Div></p></Div>;
	console.log(`${JSON.stringify(a)}`);
}

main();