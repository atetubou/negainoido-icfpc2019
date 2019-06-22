import React from 'react';
import Table from '@material-ui/core/Table';
import TableBody from '@material-ui/core/TableBody';
import TableCell from '@material-ui/core/TableCell';
import TableHead from '@material-ui/core/TableHead';
import TablePagination from '@material-ui/core/TablePagination';
import TableRow from '@material-ui/core/TableRow';
import TableSortLabel from '@material-ui/core/TableSortLabel';
import {Button} from "@material-ui/core";

export interface Task {
    id: number;
    best_solver: string;
    score: number;
}

interface Props {
    tasks: Task[];
}

const ScoreBoard = (props: Props) => {
    return (
        <div/>
        /*
        <Table>
            <TableHead>
                <TableRow>
                    <TableCell>Task ID</TableCell>
                    <TableCell>Best Solver</TableCell>
                    <TableCell>Best Score</TableCell>
                    <TableCell>Download</TableCell>
                </TableRow>
            </TableHead>
            <TableBody>
                {props.tasks.map((task) => {
                    return (
                        <TableRow>
                            <TableCell>{task.id}</TableCell>
                            <TableCell>{task.best_solver}</TableCell>
                            <TableCell>{task.score}</TableCell>
                            <TableCell>
                                <Button variant="contained" className="scoreBoard-downloadButton">Download</Button>
                            </TableCell>
                        </TableRow>
                    );
                })}
            </TableBody>
        </Table>
        */
    );
};