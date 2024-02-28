import { useState, useEffect, useContext, createContext } from "Keact";

export function UseEffectExample(props) {
	let [n, setN] = useState(0);
	useEffect(() => {
		console.log("useEffect setup, n:", n);
		return () => console.log("useEffect cleanup, n:", n);
	}, [n]);
	return <button onclick={() => setN(n + 1)}>{`Click ${n} times`}</button>;
}

export function builder() {
	return {
		root: <UseEffectExample></UseEffectExample>,
		stylesheet: css`
button { margin: 10px; padding: 4px; background-color: orange; }
button.dark { background-color: black; color: white; }
button:hover { background-color: orangered; }
`
	}
}
