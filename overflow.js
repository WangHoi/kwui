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
		"overflow-y": "auto",
		"width": "200px",
		"height": "100px",
		"margin-left": "8px",
		"margin-top": "8px",
		"padding-top": "50px",
		"padding-bottom": "50px",
		"padding-left": "10px",
		"padding-right": "10px",
		"background-color": "#eee",
		"border-top-width": "2px",
		"border-color": "#000",
	},
	".inner": {
		"width": "80%",
		"height": "50%",
		"background-color": "#00ffff",
		"font-size": "16px",
		"border-top-width": "2px",
		"border-color": "#080",
		"margin-left": "10px",
		"margin-top": "10px",
	},
	"span": {
		"font-size": 16,
	}
};

var span1 = <span>This property is a shorthand for the following CSS properties: border-top-left-radius border-top-right-radius border-bottom-right-radius border-bottom-left-radius Syntax</span>;
var span2 = <span>This property is a shorthand for the following</span>;
var hello_world = (<div class="outer">
    <div class="inner"></div>
	<p>{span2}</p>
</div>);

app.showDialog(hello_world, simple_stylesheet);
