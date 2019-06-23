import React from 'react';
import ScoreBoard, {Solution} from "./components/ScoreBoard";
import {number} from "prop-types";

const fromJsonToSolution = (s: any) =>
    ({
        solutionId: s.id,
        taskId: s.task_id,
        solver: s.solver,
        valid: s.valid,
        score: s.score,
        cost: s.cost,
    } as Solution);

const optionToUrlParam = (options: any) => {
    let res = '';
    for (const key in options) {
        if (res) {
            res += '&';
        }
        res += key + '=' + encodeURIComponent(options[key] as string);
    }
    return res;
};

interface Options {
    taskId?: number;
    valid?: boolean;
    solver?: string;
    cost?: number;
    removeFailed?: boolean;
}

const getSolution  = (options: Options, onlyBest: boolean): Promise<Solution[]> => {
    const uri = onlyBest ? './solution/best' : './solution';
    return fetch(`${uri}?${optionToUrlParam(options)}`).then((response) => {
        return response.json();
    }).then((json) => {
        if (Array.isArray(json.solutions)) {
            return json.solutions.filter((s:any) => s).map(fromJsonToSolution);
        }
        throw 'invalid response';
    });
};

const validateSolution = (id: number) => {
    return fetch(`./solution/${id}/validate`).then((response) => {
        return response.json().then((json) => fromJsonToSolution(json.solution));
    });
};

const downloadSolution = (id: number) => {
    return fetch(`./solution/${id}`).then((response) => {
        return response.blob();
    })
        .then((blob) => {
            const url = window.URL.createObjectURL(blob);
            const link = document.createElement('a');
            link.href = url;
            document.body.appendChild(link);
            link.click();
            link.parentNode.removeChild(link);
        });
};

const downloadZip = () => {
    return fetch(`./solution/best/zip`).then((response) => {
        return response.blob();
    })
        .then((blob) => {
            const url = window.URL.createObjectURL(new Blob([blob]));
            const link = document.createElement('a');
            link.href = url;
            document.body.appendChild(link);
            link.click();
            link.parentNode.removeChild(link);
        });
};

interface Props {

}

interface State {
    loading: boolean;
    solutions?: Solution[];
    alert?: string;
    filterByTask: boolean;
    taskId: number;
    onlyValid: boolean;
    onlyBest: boolean;
    sortBy?: string;
    solver?: string;
    withBest: boolean;
    withCost: boolean;
    cost: number;
    removeFailed: boolean;
}

class ScoreBoardWrapper extends React.Component<Props, State> {
    constructor(props: Props) {
        super(props);

        this.state = {
            loading: true,
            filterByTask: false,
            taskId: 0,
            onlyValid: false,
            sortBy: undefined,
            onlyBest: false,
            withBest: false,
            withCost: false,
            cost: 0,
            removeFailed: false,
        }
    }

    componentDidMount(): void {
        getSolution({}, false).then((solutions) => {
            this.setState({
                solutions,
                loading: false,
            });
        }).catch((e) => {
            console.log(e);
            this.setState({ alert: 'Failed to fetch '});
        })
    }

    handleValidate = (id: number) => {
        this.setState({ loading: true });
        validateSolution(id).then((sol) => {
            this.setState((prevState) => {
                const solutions = prevState.solutions.concat();
                const idx = solutions.findIndex((cur) => cur.solutionId === sol.solutionId);
                solutions[idx] = sol;
                return ({
                    ...prevState,
                    loading: false,
                    solutions,
                    alert: undefined,
                });
            });
        }).catch(() => this.setState({
            loading: false,
            alert: 'failed to validate',
        }));
    };

    handleRefresh = () => {
        const option: Options = {};
        const { filterByTask, taskId, onlyValid, onlyBest, solver, withBest, withCost, cost, removeFailed } = this.state;
        if (filterByTask && taskId > 0) {
            option['taskId'] = taskId;
        }
        if (onlyValid) {
            option.valid = true;
        }
        if (solver) {
            option.solver = solver;
        }
        if (withCost) {
            option.cost = cost;
        }
        if (removeFailed) {
            option.removeFailed = true;
        }
        this.setState({ loading: false }, () => {
            getSolution(option, onlyBest).then(async (sol) => {
                let solutions = sol;
                if (withBest) {
                    const {solver, ...otherOp} = option;
                    const bests = await getSolution(otherOp, true);
                    solutions = sol.concat(bests.filter((b) => sol.findIndex((s) => s.solutionId === b.solutionId) < 0));
                }
                this.setState({
                    solutions,
                    loading: false,
                    alert: undefined,
                });
            }).catch(() => this.setState({ loading: false }));
        });
    };

    render(): React.ReactNode {
        const { loading, solutions, alert, filterByTask, taskId, onlyValid, sortBy, onlyBest, solver, withBest, withCost, cost, removeFailed } = this.state;
        if (!solutions) {
            return <div>loading</div>;
        }
        let sorted;
        switch(sortBy) {
            case 'task':
                sorted = solutions.sort((a,b) => {
                    if(a.taskId - b.taskId != 0) {
                        return a.taskId - b.taskId;
                    }
                    return a.score - b.score;
                });
                break;
            case 'score':
                sorted = solutions.sort((a,b) => {
                    if (a.score - b.score != 0) {
                        return a.score - b.score
                    }
                    return a.taskId - b.taskId;
                });
                break;
            case 'solver':
                sorted = solutions.sort((a,b) => {
                    if (a.solver < b.solver) {
                        return -1;
                    }
                    if (b.solver < a.solver) {
                        return 1;
                    }
                    return a.score - b.score;
                });
                break;
            default:
                sorted = solutions.concat();
                break;
        }


        return (
            <div className="scoreBoard">
                {alert &&
                    <div className="alert alert-danger" role="alert">
                        {alert}
                    </div>
                }
                <div>
                    <label>Solver:</label>
                    <input type="text" value={solver} onChange={(e) => { this.setState({ solver: e.target.value })}} />
                </div>
                <div>
                    <input name="filterByTask" type="checkbox" checked={filterByTask} onChange={(e) => this.setState({ filterByTask: e.currentTarget.checked })}/>
                    <label> filter by Task Id</label>
                    <input type="number" disabled={!filterByTask} value={taskId} onChange={(e) => { this.setState({ taskId: parseInt(e.target.value) })}} />
                </div>
                <div>
                    <input name="onlyValid" type="checkbox" checked={onlyValid} onChange={(e) => this.setState({ onlyValid: e.currentTarget.checked })}/>
                    <label>Only Valid</label>
                </div>
                <div>
                    <input name="removeFailed" type="checkbox" checked={removeFailed} onChange={(e) => this.setState({ removeFailed: e.currentTarget.checked })}/>
                    <label>Remove Failed in Validation</label>
                </div>
                <div>
                    <input name="onlyBest" type="checkbox" checked={onlyBest} onChange={(e) => this.setState({ onlyBest: e.currentTarget.checked })}/>
                    <label>Only Best</label>
                </div>
                <div>
                    <input name="withBest" type="checkbox" checked={withBest} onChange={(e) => this.setState({ withBest: e.currentTarget.checked })}/>
                    <label>With Best</label>
                </div>
                <div>
                    <input name="withCost" type="checkbox" checked={withCost} onChange={(e) => this.setState({ withCost: e.currentTarget.checked })}/>
                    <label>Cost less than</label>
                    <input type="number" disabled={!withCost} value={cost} onChange={(e) => { this.setState({ cost: parseInt(e.target.value) })}} />
                </div>
                <div>
                    <select onChange={(e) => this.setState({ sortBy: e.currentTarget.value })}>
                        <option value="">None</option>
                        <option value="task">Task</option>
                        <option value="score">Score</option>
                        <option value="solver">Solver</option>
                    </select>
                </div>
                <ScoreBoard
                    loading={loading}
                    solutions={sorted}
                    onValidate={this.handleValidate}
                    onRefresh={this.handleRefresh}
                    onDownload={downloadSolution}
                    onDownloadZip={downloadZip}
                />
            </div>

        );
    }
}

export default ScoreBoardWrapper;
