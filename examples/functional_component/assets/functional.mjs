import { useState, useContext, createContext } from "Keact";

function UseStateExample(props) {
	let [n, setN] = useState(0);
	return <button onclick={() => setN(n + 1)}>{`Click ${n} times`}</button>;
}

let [Theme, ThemeProvider] = createContext();

function Sub1(props, kids) {
	return kids;
}

function Sub2(props, kids) {
	let theme = useContext(Theme);
	return <button class={theme}>Hello</button>
}

function ThemeExample() {
	return <ThemeProvider value="dark">
		<Sub1>
			<Sub2></Sub2>
		</Sub1>
	</ThemeProvider>
}

app.showDialog({
	root: <ThemeExample />,
	stylesheet: css`
	button { margin: 10px; padding: 4px; background-color: orange; }
	button.dark { background-color: black; color: white; }
	button:hover { background-color: orangered; }
	`
});
