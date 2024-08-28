function BoxShadowExample() {
    return <button class="inner">Click Me</button>;
}

var stylesheet = css`
    kml {
        width: 100%;
        height: 100%;
        background-color: #add8e6;
    }
    
    button.inner {
        position: absolute;
        left: 64px;
        top: 64px;
        width: 120px;
        height: 64px;
        background-color: #add8e6;
        border-radius: 24px;
        box-shadow: #c7f8ff 10px 10px 10px inset, #93b8c4 10px 10px 10px;

        line-height: 64px;
        text-align: center;
        font-size: 22px;
        color: lightslategray;
    }
    button.inner:hover {
        color: #c7f8ff;
    }

    button.inner:active {
        box-shadow: #93b8c4 10px 10px 10px inset,
        #c7f8ff -10px -10px 10px inset,
        #93b8c4 10px 10px 10px;
        color: aliceblue;
    }
`;

export function builder() {
    return {
        root: <BoxShadowExample />, stylesheet,
    }
}