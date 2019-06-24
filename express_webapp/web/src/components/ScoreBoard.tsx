import React, {MouseEventHandler} from 'react';

export interface Solution {
    taskId: number;
    solver: string;
    score: number;
    cost: number;
    solutionId: number;
    valid: boolean;
}

interface Props {
    loading: boolean;
    solutions: Solution[];
    onValidate: (id: number) => void;
    onDownload: (id: number) => void;
    onRefresh: MouseEventHandler;
    onDownloadZip: MouseEventHandler;
}

const ScoreBoard = (props: Props) => {
    return (
        <div>
            {props.loading && <div> Loading </div>}
            <button onClick={props.onRefresh} disabled={props.loading}>Refresh</button>
            <a href="./solution/best/zip" target="_blank">Download Zip</a>
            <table className="table table-striped">
                <thead className="thead-dark">
                    <tr>
                        <th>Task ID</th>
                        <th>Solution ID</th>
                        <th>Solver</th>
                        <th>Score</th>
                        <th>Cost</th>
                        <th>isValid</th>
                        <th>Validate</th>
                        <th>Download Solution</th>
                        <th>Download Buy</th>
                    </tr>
                </thead>
                <tbody>
                    {props.solutions.map((task) => {
                        return (
                            <tr>
                                <td>{task.taskId}</td>
                                <td>{task.solutionId}</td>
                                <td>{task.solver}</td>
                                <td>{task.score}</td>
                                <td>{task.cost}</td>
                                <td>{task.valid ?
                                    <span className="scoreBoard-valid">Valid</span> :
                                    <span className="scoreBoard-invalid">Not Valid</span>}
                                </td>
                                <td>
                                    <button onClick={() => props.onValidate(task.solutionId)}>Validate</button>
                                </td>
                                <td>
                                    <a href={`./solution/${task.solutionId}/sol`}>Download .sol</a>
                                </td>
                                <td>
                                    {task.cost > 0 ?
                                        <a href={`./solution/${task.solutionId}/buy`}>Download .buy</a> :
                                        <span>-</span>
                                    }
                                </td>
                            </tr>
                        );
                    })}
                </tbody>
            </table>
        </div>
    );
};

export default ScoreBoard;