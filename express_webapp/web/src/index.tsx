import React from 'react';
import ReactDom from 'react-dom';

const Hello = () => <div>hello react!</div>;

ReactDom.render(<Hello />, document.getElementById('content'));