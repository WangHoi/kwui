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
	"div.abs": {
		"position": "absolute",
		"top": "10px",
		"left": "10px",
		//"width": "200px",
		//"line-height": "20px",
		//"margin-top": "10",
		// "width": "20px",
		//"text-align": "left",
	},
	".bg-green": {
		"background-color": "#b0e0a0",
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
		"font-size": "30px",
	},
	"button": {
		"width": "auto",
		"border-color": "#000",
		"border-left-width": 10,
		"border-top-width": 10,
		"border-right-width": 10,
		"border-bottom-width": 10,
		"text-align": "center"
	}
};

var hello_world2 = (
	<div class="abs">
		<button style={{"margin-top":20}}>First</button>
		<button>Second</button>
		<button>Third<button>Fourth</button></button>
	</div>
);

var hello_world3 = (
	<div>
		<button>Third<button style={{"margin-left":0}}>Fourth</button></button>
	</div>
);

app.showDialog(hello_world3, simple_stylesheet);
