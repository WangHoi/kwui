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

class __ComponentState__ extends Object {
	scene;
	renderFn;
	props;
	children;

	constructor(scene, builder, props, children) {
		this.scene = scene;
		this.renderFn = builder;
		this.props = props;
		this.children = children;
	}
	update() {
		this.scene.updateComponent(this);
	}
	render() {
		return this.renderFn(this.props, this.children);
	}
}

function main() {
	var a = <Div a="attr1"><p>aa<Div>jjj</Div></p></Div>;
	//console.log(`${JSON.stringify(a)}`);
}
console.log("first");
var bb = app.showDialog(<Div>aaa</Div>);
console.log(`showDialog: ${bb}`);

main();