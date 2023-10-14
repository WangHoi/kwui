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
	"button": {
		"display": "inline"
	},
	".title-bar": {
		"position": "absolute",
		"top": 10,
		"right": 10,
		"margin-left": 0,
		"margin-top": 0,
		"margin-right": 0,
		"margin-bottom": 0
	}
};
var hello_world_full = (<body>
    <div style={{"background-color": "#0f0"}}>橙讯安装程序</div>
    <div class={"title-bar"} style={{ "background-color": "#0ff"}}>
        <button>最小化</button>
        <button style={{"background-color": "#f0f"}}>最大化</button>
        <button>关闭</button>
    </div>
    <div style={{"margin-top": 40, "text-align": "center"}}>
        <img src={"cx_logo_2.svg"} />
    </div>
    <div style={{"margin-top": 40, "text-align": "center"}}>
        <span style={{position:"relative", "font-size": 20}}>
            {"橙讯"}
            <span style={{position:"absolute", left:"100%", top:-10, "font-size":10}}>2.7.0</span>
        </span>
    </div>
    <div style={{"text-align":"center"}}>
        <button style={{"margin-top": 40}}>安装</button>
    </div>
</body>);

var hello_world = (
    <div style={{"margin-top": 40, "text-align": "center"}}>
        <span style={{position:"relative", "font-size": 20}}>
            {"橙讯"}
            <span style={{position:"absolute", left:"100%", top:-10, "font-size":10}}>2.7.0</span>
        </span>
    </div>
);

console.log(JSON.stringify(hello_world));

app.showDialog(hello_world_full, simple_stylesheet);