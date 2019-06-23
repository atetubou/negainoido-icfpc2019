import {getBestSolutionModels, getWhereOption, LSolution} from "./LSolution";
import {problems} from "./data";
import {taskNum} from "./consts";

const estimateScore = (taskId: number, baseScore: number, score: number) => {
    const task = problems[taskId - 1];
    return 1000 * Math.log2(task.x * task.y)  * baseScore / score;
};

export const getSolutionStats = (solutions: LSolution[], baseScores: number[]) => {
    let totalCost = 0;
    let totalScore = 0;
    solutions.forEach((solution) => {
        if(!solution) return;
        totalCost += solution.cost;
        const taskId = solution.task_id;
        totalScore += estimateScore(taskId, baseScores[taskId-1], solution.score);
    });
    return { cost: totalCost, eScore: totalScore };
};

export const optimizeSolutions = async (budget: number, baseSolutions: LSolution[]) => {
    const baseScores = baseSolutions.map((solution) => solution ? solution.score : -1);
    let solutions = baseSolutions.concat([]);

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

export const optimizeSolutions2 = async (budget: number, baseSolutions: LSolution[]) => {
    const baseScores = baseSolutions.map((solution) => solution ? solution.score : -1);
    let solutions = baseSolutions.concat([]);

    const unit = 500;
    let targetBudget = unit;
    const improves: { score: number, solution: LSolution }[] = [];
    while (targetBudget <= budget) {
        const solutionsWith = await getBestSolutionModels(getWhereOption(0, true, undefined, targetBudget));
        solutionsWith.filter((s) => !!s).forEach((solution, i) => {
            if (improves.findIndex((s) => s.solution.id === solution.id) >= 0) return;

            if (baseScores[i] > 0) {
                const score = estimateScore(solution.task_id, baseScores[i], solution.score) - solution.cost;
                if (score > 0) {
                    improves.push({ score, solution });
                }
            }
        });
        targetBudget += unit;
    }
    let tmpBudget = budget;
    const improved = Array(taskNum).fill(0);
    improves.sort((a, b) => b.score - a.score).forEach((improve) => {
        const newSol = improve.solution;
        const taskId = newSol.task_id;
        const oldSol = solutions[taskId - 1];
        if (newSol.cost - oldSol.cost >= tmpBudget) {
            return;
        }
        if (improved[taskId - 1] >= improve.score) {
            return;
        }

        tmpBudget -= newSol.cost - oldSol.cost;
        solutions[taskId - 1] = newSol;
        improved[taskId - 1] = improve.score;
    });

    return solutions;
};

export const optimizeSolutions3 = async (budget: number, baseSolutions: LSolution[]) => {
    const baseScores = baseSolutions.map((solution) => solution ? solution.score : -1);
    let solutions = baseSolutions.concat([]);

    const unit = 500;
    let targetBudget = unit;
    const improves: { score: number, solution: LSolution }[] = [];
    while (targetBudget <= budget) {
        const solutionsWith = await getBestSolutionModels(getWhereOption(0, true, undefined, targetBudget));
        solutionsWith.filter((s) => !!s).forEach((solution, i) => {
            if (improves.findIndex((s) => s.solution.id === solution.id) >= 0) return;

            if (baseScores[i] > 0) {
                const score = (estimateScore(solution.task_id, baseScores[i], solution.score) - solution.cost)/solution.cost;
                if (score > 0) {
                    improves.push({ score, solution });
                }
            }
        });
        targetBudget += unit;
    }
    let tmpBudget = budget;
    const improved = Array(taskNum).fill(0);
    improves.sort((a, b) => b.score - a.score).forEach((improve) => {
        const newSol = improve.solution;
        const taskId = newSol.task_id;
        const oldSol = solutions[taskId - 1];
        if (newSol.cost - oldSol.cost >= tmpBudget) {
            return;
        }
        if (improved[taskId - 1] >= improve.score * newSol.cost) {
            return;
        }

        tmpBudget -= newSol.cost - oldSol.cost;
        solutions[taskId - 1] = newSol;
        improved[taskId - 1] = improve.score * newSol.cost;
    });

    return solutions;
};
