import { useState, useContext, createContext } from "Keact";

export function UseStateExample(props) {
	let [n, setN] = useState(0);
	return <button onclick={() => setN(n + 1)}>{`Click ${n} times`}</button>;
}

export var root = <UseStateExample></UseStateExample>;
export var stylesheet = css`
button { margin: 10px; padding: 4px; background-color: orange; }
button.dark { background-color: black; color: white; }
button:hover { background-color: orangered; }
`;