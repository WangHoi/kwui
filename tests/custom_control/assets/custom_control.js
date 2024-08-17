function PlotExample() {
    return <plot></plot>;
}

var stylesheet = css`
    kml {
        width: 100%;
        height: 100%;
        background-color: darkgray;
    }
    
    plot {
        position: absolute;
        inset: 20px;
        background-color: #202020;
        border-radius: 16px;
        box-shadow: lemonchiffon 0px 0px 16px;
    }

    div.inner {
        position: absolute;
        left: 64px;
        top: 64px;
        width: 128px;
        height: 128px;
        background-color: #add8e6;
        border-radius: 24px;
        box-shadow: #93b8c4 10px 10px 10px inset,
        #c7f8ff -10px -10px 10px inset,
        #93b8c4 10px 10px 10px,
        #c7f8ff -10px -10px 10px;
    }
`;

export function builder() {
    return {
        root: <PlotExample />, stylesheet,
    }
}