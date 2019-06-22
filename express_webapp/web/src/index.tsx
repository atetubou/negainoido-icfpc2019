import React from 'react';
import ReactDom from 'react-dom';
import ScoreBoardWrapper from "./ScoreBoardWrapper";

const Hello = () => <div>hello react!</div>;

ReactDom.render(<ScoreBoardWrapper />, document.getElementById('content'));
