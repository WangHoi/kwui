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
		"height": "200px",
		"background-color": "#aaa",
	},
	".inner": {
		"margin-left": "auto",
		"margin-right": "auto",
		"width": "80%",
		"height": "100%",
		"background-color": "#0ff",
	},
	".italic": {
		"font-style": "italic"
	},	
	".bold": {
		"font-weight": "bold",
	},
	".fs-20": {
		"font-size": "30px",
	},
};

var hello_world = (<div class="outer">
    <div class="inner"></div>
	<p>
	<span>This property is a shorthand for the following CSS properties: border-top-left-radius border-top-right-radius border-bottom-right-radius border-bottom-left-radius Syntax</span>
	</p>
</div>);

app.showDialog(hello_world, simple_stylesheet);
