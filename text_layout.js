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
		// "line-height": "20px",
		"margin-top": "10",
		"width": "220px",
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
		"font-size": "20px",
	},
	".tall": {
		"line-height": "16px",
	},
};

// <span class="bold">bold </span>
// <span class="italic">italic </span>
var hello_world = (<div>
	<span>“焕新”“焕新”“焕新”“焕新”“焕新”“焕新”“焕新”“焕新”“焕新”“焕新”“焕新”</span>
	<span class="tall">The term real-time clock is used to avoid confusion with ordinary hardware clocks which are only signals that govern digital electronics, and do not count time in human units. RTC should not be confused with real-time computing, which shares its three-letter acronym but does not directly relate to time of day.</span>
	<span class="bold italic">bold italic </span>
	<span class="fs-20">20 pixel size</span>
</div>);

var hello_world_1 = (<div>
	<span class="tall">RTC should not be confused with real-time computing, which shares its three-letter acronym but does not directly relate to time of day.</span>
</div>);

app.showDialog(hello_world, simple_stylesheet);
