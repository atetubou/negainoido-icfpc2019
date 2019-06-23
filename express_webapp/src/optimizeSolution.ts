import {getBestSolutionModel, getBestSolutionModels, getWhereOption} from "./LSolution";
import {problems} from "./data";
import {taskNum} from "./consts";

const estimateScore = (taskId: number, baseScore: number, score: number) => {
    const task = problems[taskId - 1];
    return 1000 * Math.log2(task.x * task.y)  * baseScore / score;
};

export const optimizeSolutions = async (budget: number) => {
    const baseSolutions = await getBestSolutionModels(getWhereOption(0, true));
    const baseScores = baseSolutions.map((solution) => solution ? solution.score : -1);
    let solutions = baseSolutions;

    let tmpBudget = budget;
    let currentTarget = 2010;
    while (tmpBudget > 0 && currentTarget < tmpBudget) {
        const solutionsWith = await getBestSolutionModels(getWhereOption(0, true, undefined, currentTarget));
        const improves = solutionsWith.map((s, i) => {
           if (s && baseScores[i] > 0) {
               return { id: i, score: estimateScore(s.task_id, baseScores[i], s.score) };
           }

           return { id: i, score: -1 };
        }).sort((a, b) => b.score - a.score);
        let idx = 0;
        while (tmpBudget > 0 && idx < improves.length) {
            const newSol = solutionsWith[improves[idx].id];

            if (newSol.cost <= tmpBudget) {
                solutions[newSol.task_id -1] = newSol;
                tmpBudget -= newSol.cost;
            }
            idx++;
        }
    }

    return solutions;
};
