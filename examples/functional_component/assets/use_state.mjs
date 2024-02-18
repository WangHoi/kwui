import { useState, useContext, createContext } from "Keact";

export function UseStateExample(props) {
	let [n, setN] = useState(0);
	return <button onclick={() => setN(n + 1)}>{`Click ${n} times`}</button>;
}

export function builder() {
	return {
		root: <UseStateExample></UseStateExample>,
		stylesheet: css`
button { margin: 10px; padding: 4px; background-color: orange; }
button.dark { background-color: black; color: white; }
button:hover { background-color: orangered; }
`
	};
}