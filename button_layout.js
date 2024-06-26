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

var simple_css = `
	div.abs {
		position: absolute;
		top: 10px;
		left: 10px;
	}
	.bg-green {
		background-color: #b0e0a0;
	}
	line-edit {
		width: 100;
		height: 30;
	}
	button {
		width: auto;
		border: 2px solid #888;
		text-align: left;
		background-color: #0b0;
		border-radius: 4px;
	}
	button:hover {
		background-color: #080;
	}
	button:active {
		border-color: lightblue;
		background-color: #0e0;
	}
`;

var hello_world2 = (
	<div class="abs">
		<button style="margin-top:20;">First</button>
		<button>Second</button>
		<button>Third<button>Fourth</button></button>
	</div>
);

var hello_world3 = (
	<div>
		<button>Third<button>Fourth</button></button>
	</div>
);

app.showDialog(hello_world2, simple_css);
