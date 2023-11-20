"use strict";
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

var simple_stylesheet = {
	".outer": {
		"display": "block",
		"overflow-y": "scroll",
		"overflow-x": "scroll",
		"width": "200px",
		"height": "100px",
		"margin-left": "8px",
		"margin-top": "8px",
		"padding-top": "50px",
		"padding-bottom": "50px",
		"padding-left": "0px",
		"padding-right": "0px",
		"background-color": "#fff",
		"border-top-width": "2px",
		"border-right-width": "2px",
		"border-bottom-width": "2px",
		"border-left-width": "2px",
		"border-color": "#000",
	},
	".inner": {
		//"width": "90%",
		"width": "200px",
		"height": "50%",
		"background-color": "#00ffff",
		"font-size": "16px",
		"border-top-width": "2px",
		"border-right-width": "2px",
		"border-bottom-width": "2px",
		"border-left-width": "2px",
		"border-color": "#080",
		"margin-left": "50px",
		//"margin-top": "10px",
	},
	"span": {
		"font-size": 16,
	}
};

var span1 = <span>This property is a shorthand for the following CSS properties: border-top-left-radius border-top-right-radius border-bottom-right-radius border-bottom-left-radius Syntax</span>;
var span2 = <span>a b c d e f g h i j k l m o p q r s t u v w x y z</span>;
//	<p>{span2}</p>
var hello_world = (<div class="outer">
	<qqq style={{"background-color": "#0aa", "height":120, "overflow-y": "scroll"}}>{span1}</qqq>
    <div class="inner"></div>
</div>);

app.showDialog(hello_world, simple_stylesheet);
