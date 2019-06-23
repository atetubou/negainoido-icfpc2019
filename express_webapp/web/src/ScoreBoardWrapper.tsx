import React from 'react';
import ScoreBoard, {Solution} from "./components/ScoreBoard";

const fromJsonToSolution = (s: any) =>
    ({
        solutionId: s.id,
        taskId: s.task_id,
        solver: s.solver,
        valid: s.valid,
        score: s.score,
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
}

const getSolution = (options: Options, onlyBest: boolean) => {
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
        const { filterByTask, taskId, onlyValid, onlyBest } = this.state;
        if (filterByTask && taskId > 0) {
            option['taskId'] = taskId;
        }
        if (onlyValid) {
            option.valid = true;
        }
        this.setState({ loading: false }, () => {
            getSolution(option, onlyBest).then((solutions) => {
                this.setState({
                    solutions,
                    loading: false,
                    alert: undefined,
                });
            }).catch(() => this.setState({ loading: false }));
        });
    };

    render(): React.ReactNode {
        const { loading, solutions, alert, filterByTask, taskId, onlyValid, sortBy, onlyBest } = this.state;
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
            <div>
                {alert &&
                    <div className="alert alert-danger" role="alert">
                        {alert}
                    </div>
                }
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
                    <input name="onlyBest" type="checkbox" checked={onlyBest} onChange={(e) => this.setState({ onlyBest: e.currentTarget.checked })}/>
                    <label>Only Best</label>
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
