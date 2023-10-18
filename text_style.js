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
	"div": {
		"margin-top": "10",
		"width": "40%",
		"background-color": "#80a080",
	},
	"line-edit": {
		"width": 100,
		"height": 30,
	},
	".italic": {
		"font-style": "italic"
	},	
	".bold": {
		"font-weight": "bold",
	},
	".fs-20": {
		"font-size": "20px",
	},
};


var hello_world = (<div>
	<p class="bold">bold</p>
	<p class="italic">italic</p>
	<p class="bold italic">bold italic</p>
	<p class="fs-20">20 pixel size</p>
</div>);

app.showDialog(hello_world, simple_stylesheet);
