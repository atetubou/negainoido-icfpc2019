import React from 'react';
import ScoreBoard, {Solution} from "./components/ScoreBoard";

const getSolution = () => fetch('./solution').then((response) => {
    return response.json();
}).then((json) => {
    if (Array.isArray(json.solutions)) {
        return json.solutions.map((s: any) => {
            return ({
                solutionId: s.id,
                taskId: s.task_id,
                solver: s.solver,
                valid: s.valid,
                score: s.score,
            } as Solution);
        });
    }
    throw 'invalid response';
});

interface Props {

}

interface State {
    loading: boolean;
    solutions?: Solution[];
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
        }).catch((e) => { console.log(e); })
    }

    render(): React.ReactNode {
        const { loading, solutions } = this.state;
        if (loading) {
            return <div>loading</div>;
        }
        return (
            <ScoreBoard
                solutions={solutions!}
                onValidate={() => {}}
                onRefresh={() => {}}
                onDownload={() => {}}
            />

        );
    }
}

export default ScoreBoardWrapper;
