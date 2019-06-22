import React, {MouseEventHandler} from 'react';

export interface Solution {
    taskId: number;
    solver: string;
    score: number;
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
        <div className="scoreBoard">
            {props.loading && <div> Loading </div>}
            <button onClick={props.onRefresh} disabled={props.loading}>Refresh</button>
            <button onClick={props.onDownloadZip} disabled={props.loading}>Download Zip</button>
            <table>
                <tr>
                    <th>Task ID</th>
                    <th>Solver</th>
                    <th>Score</th>
                    <th>isValid</th>
                    <th>Validate</th>
                    <th>Download</th>
                </tr>
                <tbody>
                    {props.solutions.map((task) => {
                        return (
                            <tr>
                                <td>{task.taskId}</td>
                                <td>{task.solver}</td>
                                <td>{task.score}</td>
                                <td>{task.valid ?
                                    <span className="scoreBoard-valid">Valid</span> :
                                    <span className="scoreBoard-invalid">Not Valid</span>}
                                </td>
                                <td>
                                    <button onClick={() => props.onValidate(task.solutionId)}>Validate</button>
                                </td>
                                <td>
                                    <button onClick={() => props.onDownload(task.solutionId)}>Download</button>
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