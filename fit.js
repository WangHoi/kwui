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
	"p": {
		"margin-left": 10,
		"margin-top": 10,
		"margin-right": 10,
		"margin-bottom": 10,
		"border-left-width": 10,
		"border-top-width": 10,
		"border-right-width": 10,
		"border-bottom-width": 10,
		"padding-left": 10,
		"padding-top": 10,
		"padding-right": 10,
		"padding-bottom": 10,
		"border-color": "#0f0",
		"background-color": "#0ff",
	}
};

var simple = <p style={{ left: 30, top: 100 }}>第一栏<p style={{ left: 20, top: 30 }}>测试2</p></p>;
var complex = <Div a="f1"><p>aa</p><p>bb</p></Div>;
var edit = <line-edit style={{ left: 20, top: 20, width: 200, height: 40 }}></line-edit>
// console.log(JSON.stringify(complex.render()));

var hello_world = <div>
	<div><p>hello</p></div><p>world</p>
</div>;

var fit_body = <body>
    <div class="rel">
        <div class="abs bottom-right">
            <div>最小化</div>
            <div style="width:10px;">a</div>
        </div>
    </div>
</body>;

var fit_stylesheet = {
	".rel": {
		"position": "relative",
        "width": "200px",
        "height": "200px",
        "background-color": "#aaa",
    },
	".abs": {
        "position": "absolute",
        //overflow: auto;
    },
    ".top-right": {
        "top": "10px",
        "right": "10px",
        "background-color": "#e8e",
    },
    ".bottom-right": {
        "bottom": "10px",
        "right": "10px",
        "background-color": "#e8e",
    }
};

app.showDialog(fit_body, fit_stylesheet);
