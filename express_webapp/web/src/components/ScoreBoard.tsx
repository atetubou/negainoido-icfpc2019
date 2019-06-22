import React, {MouseEventHandler} from 'react';

export interface Solution {
    taskId: number;
    solver: string;
    score: number;
    solutionId: number;
    valid: boolean;
}

interface Props {
    solutions: Solution[];
    onValidate: (id: number) => void;
    onDownload: (id: number) => void;
    onRefresh: MouseEventHandler;
}

const ScoreBoard = (props: Props) => {
    return (
        <div>
            <button onClick={props.onRefresh}>Refresh</button>
            <table>
                <th>
                    <tr>
                        <td>Task ID</td>
                        <td>Solver</td>
                        <td>Score</td>
                        <td>isValid</td>
                        <td>Validate</td>
                        <td>Download</td>
                    </tr>
                </th>
                <tbody>
                    {props.solutions.map((task) => {
                        return (
                            <tr>
                                <td>{task.taskId}</td>
                                <td>{task.solver}</td>
                                <td>{task.score}</td>
                                <td>{task.valid ? 'Valid' : 'Not Valid'}</td>
                                <td>{task.score}</td>
                                <td>
                                    <button onClick={() => props.onValidate(task.solutionId)}>Validate</button>
                                </td>
                                <td>
                                    <button onClick={() => props.onValidate(task.solutionId)}>Download</button>
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