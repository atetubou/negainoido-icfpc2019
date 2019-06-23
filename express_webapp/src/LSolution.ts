import {Column, CreatedAt, DataType, Model, Table} from "sequelize-typescript";
import {Op, WhereOptions} from "sequelize";
import runSim from "./runSim";
import {getDescFile} from "./utils";

@Table({timestamps: true, tableName: 'tbl_lsolutions'})
export class LSolution extends Model<LSolution> {
    @Column
    solver: string;
    @Column(DataType.INTEGER)
    task_id: number;
    @Column(DataType.BIGINT)
    score: number;
    @Column(DataType.BOOLEAN)
    valid: boolean;
    @Column(DataType.BOOLEAN)
    checked: boolean;
    @Column(DataType.BOOLEAN)
    has_buy: boolean;
    @Column(DataType.BIGINT)
    cost: number;
    @CreatedAt
    @Column
    created: Date;
}

export const getSolutionById = (id: number) => {
    return LSolution.findOne({
        attributes: ['id', 'solver', 'task_id', 'valid', 'score', 'has_buy', 'cost'],
        where: {id},
    });
};
export const getWhereOption = (taskId: number, valid = false, solver?: string, cost?: number) => {
    const option: WhereOptions = {};
    if (taskId) {
        option.task_id = taskId;
    }
    if (valid) {
        option.valid = valid;
    }
    if (solver) {
        option.solver = {[Op.like]: solver};
    }
    if (cost >= 0) {
        option.cost = {[Op.lte] : cost };
    }
    return option;
};
export import AWS = require('aws-sdk');
import {taskNum} from "./consts";

export const defaultBucket = process.env.S3_BUCKET || 'negainoido-icfpc2019-dev';
export const generateKey = (model: LSolution) => `solution_${model.solver}_${model.task_id}_${model.id}`;
export const generateBuyKey = (model: LSolution) => `buy_${model.solver}_${model.task_id}_${model.id}`;
export const validateModel = (taskId: number, model: LSolution, file: string, buyFile?: string) => {
    return runSim(getDescFile(taskId), file, buyFile)
        .then((s) => {
            model.score = parseInt(s);
            model.valid = true;
            model.checked = true;
            return model.save();
        })
        .catch((e) => {
            console.log('Failed to run sim : ' + e);
            model.valid = false;
            model.checked = true;
            return model.save().then(() => {
                throw e;
            });
        });
};

export const getBestSolutionModels = (where: WhereOptions) => {
    const promises: Promise<LSolution|null>[] = [];
    for (let i = 0; i < taskNum; i++) {
        promises.push(getBestSolutionModel({ ...where, task_id: i+1 }));
    }
    return Promise.all(promises);
};
export const getBestSolutionModel = (where: WhereOptions) => {
    return LSolution.findOne({
        attributes: ['id', 'solver', 'task_id', 'valid', 'score', 'has_buy', 'cost'],
        where: {score: {[Op.gt]: 0}, ...where},
        order: [['score', 'ASC']]
    });
};

export const pullDataFromS3 = (model: LSolution): Promise<AWS.S3.GetObjectOutput> => {
    const s3 = new AWS.S3();
    const key = generateKey(model);

    const params = {
        Bucket: defaultBucket,
        Key: key,
    };

    return new Promise((resolve, reject) => {
        s3.getObject(params, (err, data) => {
            if (err) {
                reject(err);
            } else {
                resolve(data);
            }
        });
    });
};

export const pullBuyDataFromS3 = (model: LSolution): Promise<AWS.S3.GetObjectOutput|null> => {
    if (!model.has_buy) {
        return Promise.resolve(null);
    }
    const s3 = new AWS.S3();
    const key = generateBuyKey(model);

    const params = {
        Bucket: defaultBucket,
        Key: key,
    };

    return new Promise((resolve, reject) => {
        s3.getObject(params, (err, data) => {
            if (err) {
                reject(err);
            } else {
                resolve(data);
            }
        });
    });
};

export const getBestSolution = (taskId: number, valid = false, solver?: string) => {
    return getBestSolutionModel(getWhereOption(taskId, valid, solver)).then((model) => {
        if (!model) {
            return Promise.resolve();
        }
        const s3 = new AWS.S3();
        const key = generateKey(model);

        const params = {
            Bucket: defaultBucket,
            Key: key,
        };
        return new Promise((resolve, reject) => {
            s3.getObject(params, (err, data) => {
                if (err) {
                    reject(err);
                } else {
                    resolve(data);
                }
            });
        });
    });
};