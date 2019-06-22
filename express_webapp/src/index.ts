import * as express from 'express';
import * as path from 'path';
import * as morgan from 'morgan';
import * as timeout from 'connect-timeout'
import runSim from './runSim';

const normalizePort = (val: string) => {
    var port = parseInt(val, 10);
  
    if (port >= 0) {
      // port number
      return port;
    }
  
    return false;
};

const app = express();
const port = normalizePort(process.env.PORT || '3000');
const taskNum = 220;

app.use(timeout('180s'));

var morgan = require('morgan');
app.use(morgan('combined'));

console.log(path.join(__dirname, './static'));
app.use('/static', express.static(path.join(__dirname, './../static')))
app.use('/public', express.static(path.join(__dirname, './../web/dist')))

app.listen(port);
app.set('view engine', 'pug');

import { Sequelize } from 'sequelize-typescript';
import { Table, Column, Model, HasMany, DataType, CreatedAt, PrimaryKey } from 'sequelize-typescript';
import mysql2 from 'mysql2';

@Table({ timestamps: true, tableName: 'tbl_lsolutions'})
class LSolution extends Model<LSolution> {
    @Column
    solver: string;
    @Column(DataType.INTEGER)
    task_id: number;
    @Column(DataType.BIGINT)
    score: number;
    @Column(DataType.BOOLEAN)
    valid: boolean;
    @CreatedAt
    @Column
    created: Date;
}

const sequelize = new Sequelize({
    database: process.env.DB_DB || 'garasubodb',
    username: process.env.DB_USER || 'negainoido',
    password: process.env.DB_PASSWORD,
    host: process.env.DB_HOST || 'localhost',
    port: normalizePort(process.env.DB_PORT) || 3306,
    dialect: 'mysql',
    dialectModule: mysql2,
});

sequelize
    .authenticate()
    .then(() => console.log('DB connection has benn established successfully.'))
    .catch(err => console.error('Unable to connect DB!: ' + err));

sequelize.addModels([LSolution]);

sequelize.sync();


app.get('/', (req, res, next) => {
    res.render('index', {title: 'Express'});
});

app.get('/solution', async (req, res, next) => {
    const solutions = await LSolution.findAll();
    res.json({ solutions });
});

app.get('/stat/api', async (req, res, next) => {
    const solutions = await LSolution.findAll({raw: true, where: {'valid': true}});
    var stat_by_tasks = {};
    for (let sol of solutions) {
        let tid = sol['task_id'];
        if (stat_by_tasks[tid]) stat_by_tasks[tid].push(sol);
        else stat_by_tasks[tid] = [sol];
    }

    var stat = [];
    for (let tid in stat_by_tasks) {
        stat_by_tasks[tid].sort((a, b) => a['score'] - b['score']);
        for (let idx in stat_by_tasks[tid].slice(0, 10)) {
            stat.push(stat_by_tasks[tid][idx]);
        }
    }

    res.json({ values: stat });
});

app.get('/stat', (req, res, next) => {
    res.render('stat', {});
});


import fileUpload = require('express-fileupload');
import AWS = require('aws-sdk');
AWS.config.loadFromPath(process.env.KEY_JSON || './aws_key.json');
AWS.config.update({ region: 'us-east-2' });
const defaultBucket = process.env.S3_BUCKET || 'negainoido-icfpc2019-dev';

app.use(fileUpload({ limits: { fileSize: 50 * 1024 * 1024 } }));

const generateKey = (model: LSolution) => `solution_${model.solver}_${model.task_id}_${model.id}`;

// endpoint for submission
app.post('/solution', async (req, res, next) => {
    const solver = req.body['solver'] || 'unknown';
    const task_id = parseInt(req.body['task']) || 0;
    const data = (req.files!.file as fileUpload.UploadedFile).data;
    let valid = false;
    let score;
    if (!req.body['score']) {
        const desc = '';
        const sol = '';
        const tmp = os.tmpdir();
        const key = makeRandomId(`${solver}_${task_id}_`);
        const file = Path.join(tmp, key);

        await new Promise((resolve, reject) => {
            fs.writeFile(file, data, async (err) => {
                if (err) {
                    console.error('failed to write file: ' + err);
                    reject(err);
                } else {
                    runSim(getDescFile(task_id), file)
                        .then((s) => {
                            score = parseInt(s);
                            valid = true;
                            resolve();
                        })
                        .catch((e) => {
                            console.log('Failed to run sim : ' + e);
                            reject(e);
                        });
                }
            });
        });
    } else {
        score = parseInt(req.body['score']);
    }

    const solution = new LSolution({solver, task_id, score, valid});

    solution.save().then((model) => {
        console.log('created object ' + model);
        const params = {
            Bucket: defaultBucket,
            Key: generateKey(model),
            Body: data,
        };
        const s3 = new AWS.S3();

        s3.putObject(params, (err, data) => {
            if (err) {
                LSolution.destroy({where: {id: model.id}}).catch(() => {
                });
                console.error('faield to upload: ' + err);
                next(err);
            } else {
                console.log(data);
                res.json({
                    solution: model,
                    s3_data: data,
                });
            }
        });
    }).catch((e) => {
        console.error('error: ' + e);
        next(e);
    });
});

const getSolutionById = (id: number) => {
    return LSolution.findOne({
        attributes: ['id', 'solver', 'task_id', 'valid'],
        where: { id },
    });
};

const getBestSolutionModel = (taskId: number, valid = false) => {
    return LSolution.findOne({
        attributes: ['id', 'solver', 'task_id', 'valid'],
        where: { task_id: taskId, valid },
        order: [['score', 'ASC']]
    });
};

const getBestSolution = (taskId: number, valid = false) => {
    return getBestSolutionModel(taskId, valid).then((model) => {
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

import * as archiver from 'archiver';
import {GetObjectOutput} from "aws-sdk/clients/s3";
import {Readable} from "stream";
import * as os from "os";
import * as Path from "path";
import * as fs from "fs";
import {formatNumber, getDescFile, makeRandomId} from "./utils";


app.get('/solution/best/zip', async (req, res, next) => {
    const archive = archiver('zip');
    let promises: Promise<GetObjectOutput>[] = [];
    res.attachment('solutions.zip');
    archive.pipe(res);
    for (let i = 1; i <= taskNum; i++) {
        promises.push(getBestSolution(i, true));
    }
    await Promise.all(promises).then((solutions) => {
        solutions.forEach((solution, i) => {
            const name = `prob-${formatNumber(i+1)}.sol`;
            archive.append(solution.Body as Readable, { name });
        });
    })
        .then(() => {
            return archive.finalize();
        })
        .catch((e) => {
            console.error(e);
            next(e);
        });
});

app.get('/solution/best', async (req, res, next) => {
    let promises = [];
    for (let i = 1; i <= taskNum; i++) {
        promises.push(getBestSolutionModel(i));
    }
    Promise.all(promises).then((solutions) => {
        res.json({ solutions });
    })
        .catch((e) => {
            console.error(e);
            next(e);
        });
});

app.get('/solution/:id', async (req, res, next) => {
    const id = parseInt(req.params['id']);

    const model = await getSolutionById(id).catch((e) => {
        console.error("DB error" + e);
        res.status(500);
        res.json({ error: e });
    });

    if (!model) {
        console.error("error");
        res.status(404);
        res.render('not found solution');
        return;
    }
    const s3 = new AWS.S3();
    const key = generateKey(model);

    const params = {
        Bucket: defaultBucket,
        Key: key,
    };
    s3.getObject(params, (err, data) => {
        if (err) {
            console.error('failed to download: ' + err);
            next(err);
        } else {
            res.set('Content-Type', data.ContentType);
            res.set('Content-Disposition', 'attachment; filename='+key);
            res.set('Content-Length', data.ContentLength.toString());
            res.end(data.Body, 'binary');
        }
    });
});

app.get('/solution/:id/validate', async (req, res, next) => {
    const id = parseInt(req.params['id']);

    const target = await LSolution.findOne({
        attributes: ['id', 'solver', 'task_id'],
        where: { id: id },
    }).catch((e) => {
        console.error("DB error" + e);
        res.status(500);
        res.json({ error: e });
    });

    if (!target) {
        console.error("error");
        res.status(404);
        res.render('not found solution');
        return;
    }
    const s3 = new AWS.S3();
    const key = generateKey(target);

    const params = {
        Bucket: defaultBucket,
        Key: key,
    };
    s3.getObject(params, (err, data) => {
        if (err) {
            console.error('failed to download: ' + err);
            next(err);
        } else {
            const tmp = os.tmpdir();
            const file = Path.join(tmp, encodeURIComponent(key) + '.sol');
            fs.writeFile(file, data.Body, (err) => {
                if (err) {
                    console.error('failed to write file: ' + err);
                    next(err);
                } else {
                    runSim(getDescFile(target.task_id), file)
                        .then((score) => {
                            target.score = parseInt(score);
                            target.valid = true;
                            return target.save().then(() => {
                                res.json({ solution: target });
                            });
                        })
                        .catch((e) => {
                            console.log('Failed to run sim : ' + e);
                            next(e);
                        });
                }
            });

        }
    });
});


app.get('/solution/best/:id', async (req, res, next) => {
    const id = parseInt(req.params['id']);

    const best = await getBestSolutionModel(id).catch((e) => {
        console.error("DB error" + e);
        res.status(500);
        res.json({ error: e });
    });

    if (!best) {
        console.error("error");
        res.status(404);
        res.render('not found solution');
        return;
    }
    const s3 = new AWS.S3();
    const key = generateKey(best); 
    
    const params = {
        Bucket: defaultBucket,
        Key: key,
    };
    s3.getObject(params, (err, data) => {
        if (err) {
            console.error('failed to download: ' + err);
            next(err);
        } else {
            res.set('Content-Type', data.ContentType);
            res.set('Content-Disposition', 'attachment; filename='+key);
            res.set('Content-Length', data.ContentLength.toString());
            res.end(data.Body, 'binary');
        }
    });
});

const handleTimeout = (req, res, next) => {
    if (!req.timeout) {
        next();
    }
};

app.use(handleTimeout);
