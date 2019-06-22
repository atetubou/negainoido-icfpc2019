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

const getSolution = () => fetch('./solution').then((response) => {
    return response.json();
}).then((json) => {
    if (Array.isArray(json.solutions)) {
        return json.solutions.map(fromJsonToSolution);
    }
    throw 'invalid response';
});

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
}

class ScoreBoardWrapper extends React.Component<Props, State> {
    constructor(props: Props) {
        super(props);

        this.state = {
            loading: true,
        }
    }

    componentDidMount(): void {
        getSolution().then((solutions) => {
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
        this.setState({ loading: false }, () => {
            getSolution().then((solutions) => {
                this.setState({
                    solutions,
                    loading: false,
                    alert: undefined,
                });
            }).catch(() => this.setState({ loading: false }));
        });
    };

    render(): React.ReactNode {
        const { loading, solutions, alert } = this.state;
        if (!solutions) {
            return <div>loading</div>;
        }
        return (
            <div>
                {alert &&
                    <div className="alert alert-danger" role="alert">
                        {alert}
                    </div>
                }
                <ScoreBoard
                    loading={loading}
                    solutions={solutions!}
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
