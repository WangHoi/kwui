import { useState, useEffect, useContext, createContext } from "Keact";

export function UseEffectExample(props) {
	let [n, setN] = useState(0);
	useEffect(() => {
		console.log("n changed:", n);
		return () => {
			console.log("use Effect dtor");
		}
	}, [n]);
	return <button onclick={() => setN(n + 1)}>{`Click ${n} times`}</button>;
}

export var root = <UseEffectExample></UseEffectExample>;
export var stylesheet = css`
button { margin: 10px; padding: 4px; background-color: orange; }
button.dark { background-color: black; color: white; }
button:hover { background-color: orangered; }
`;