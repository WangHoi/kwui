import { useState } from "Keact";

function UseStateExample(props) {
	let [n, setN] = useState(0);
	return <button onclick={() => setN(n + 1)}>{`Click ${n} times`}</button>;
}


app.showDialog({
	root: <UseStateExample />,
	stylesheet: `
	button { margin: 10px; padding: 4px; background-color: orange; }
	button:hover { background-color: orangered; }
	`
});
