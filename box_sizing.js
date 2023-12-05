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

var simple_css = `
body {
	margin: 8px;
}
.button {
    display: inline-block;
    border: 1px solid black;
    padding: 4px;
    width: 80px;
    height: 40px;
    box-sizing:border-box;
}
`;

var two_button = (<body><div class="button">最小化</div><div class="button" style="box-sizing:content-box;">最小化</div></body>);

app.showDialog(two_button, simple_css);
