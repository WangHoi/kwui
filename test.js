﻿"use strict";
"use math";

class __ComponentState__ extends Object {
	renderFn;
	props;
	children;

	constructor(builder, props, children) {
		super();
		this.renderFn = builder;
		this.props = props;
		this.children = children;
	}
	update() {
		// this.scene.updateComponent(this);
	}
	render() {
		return this.renderFn(this.props, this.children);
	}
};


function JSX(tag, atts, kids) {
	if (typeof tag == 'function') {
		return new __ComponentState__(tag, atts, kids);
	} else {
		return {
			tag,
			atts,
			kids,
		};
	}
}

function Div(props, kids) {
	return <div>
		<p>{props.a}</p>
		{kids}
		<p>next</p>
	</div>;
}

var simple = <p style={{ left: 30, top: 100 }}>第一栏<p style={{ left: 20, top: 30 }}>测试2</p></p>;
var complex = <Div a="f1"><p>aa</p><p>bb</p></Div>;
var edit = <line-edit style={{ left: 20, top: 20, width: 200, height: 40 }}></line-edit>
// console.log(JSON.stringify(complex.render()));
app.showDialog(edit);