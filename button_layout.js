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
		"position": "absolute",
		"top": "10px",
		"left": "10px",
		"width": "200px",
		//"line-height": "20px",
		//"margin-top": "10",
		// "width": "20px",
		"background-color": "#b0e0a0",
		//"text-align": "left",
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
		"width": 100,
	}
};

var hello_world = (<div>
	<p class="bold">Test button(inline layout)</p>
	<div>
		<button>First button</button>
		<button>Second button</button>
		<button>Third button</button>
	</div>
</div>);

app.showDialog(hello_world, simple_stylesheet);
