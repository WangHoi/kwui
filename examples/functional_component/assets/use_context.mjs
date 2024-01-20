import { useState, useContext, createContext } from "Keact";

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

export var root = <ThemeExample />;