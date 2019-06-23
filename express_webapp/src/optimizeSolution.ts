import {getBestSolutionModel, getBestSolutionModels, getWhereOption} from "./LSolution";
import {problems} from "./data";
import {taskNum} from "./consts";

const estimateScore = (taskId: number, baseScore: number, score: number) => {
    const task = problems[taskId - 1];
    return 1000 * Math.log2(task.x * task.y)  * baseScore / score;
};

export const optimizeSolutions = async (budget: number) => {
    const baseSolutions = await getBestSolutionModels(getWhereOption(0, true, undefined, 0));
    const baseScores = baseSolutions.map((solution) => solution ? solution.score : -1);
    let solutions = baseSolutions;

    let tmpBudget = budget;
    let currentTarget = Math.min(2000, budget);
    while (tmpBudget > 0 && currentTarget <= tmpBudget) {
        const prevBudget = tmpBudget;
        const solutionsWith = await getBestSolutionModels(getWhereOption(0, true, undefined, currentTarget));
        const improves = solutionsWith.map((s, i) => {
           if (s && baseScores[i] > 0) {
               return { id: i, score: estimateScore(s.task_id, baseScores[i], s.score) };
           }

           return { id: i, score: -1 };
        }).sort((a, b) => b.score - a.score);
        console.log(improves);
        let idx = 0;
        while (tmpBudget > 0 && idx < improves.length && improves[idx].score > 0) {
            const newSol = solutionsWith[improves[idx].id];
            const taskId = newSol.task_id;
            const oldSol = solutions[taskId - 1];

            if (newSol.id !== oldSol.id && newSol.cost - oldSol.cost <= tmpBudget && newSol.score < oldSol.score) {
                solutions[taskId -1] = newSol;
                tmpBudget -= newSol.cost - oldSol.cost;
            }
            idx++;
        }
        if (currentTarget * 2 > tmpBudget && currentTarget !== prevBudget) {
            currentTarget = tmpBudget;
        } else {
            currentTarget *= 2;
        }
    }

    return solutions;
};
