import * as express from 'express';
import * as path from 'path';
import * as Path from 'path';
import * as timeout from 'connect-timeout'
import {Sequelize} from 'sequelize-typescript';
import mysql2 from 'mysql2';
import * as archiver from 'archiver';
import {GetObjectOutput} from "aws-sdk/clients/s3";
import {Readable} from "stream";
import * as os from "os";
import * as fs from "fs";
import {formatNumber, makeRandomId, pullObjectS3ToTmp, putDataToS3} from "./utils";
import {FindOptions, Op} from "sequelize";
import {getTotalCost} from "./validateBuy";
import {
    AWS,
    defaultBucket,
    generateBuyKey,
    generateKey,
    getBestSolution,
    getBestSolutionModel,
    getSolutionById,
    getWhereOption,
    LSolution, pullBuyDataFromS3, pullDataFromS3,
    validateModel
} from "./LSolution";
import {taskNum} from "./consts";

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

app.use(timeout('180s'));

var morgan = require('morgan');
app.use(morgan('combined'));

console.log(path.join(__dirname, './static'));
app.use('/static', express.static(path.join(__dirname, './../static')))
app.use('/public', express.static(path.join(__dirname, './../web/dist')))

app.listen(port);
app.set('view engine', 'pug');

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
    const valid = req.query.valid || false;
    const checked = req.query.checked || false;
    const limit = req.query.limit;
    const taskId = parseInt(req.query.taskId || '0');
    const page = parseInt(req.query.page || '1');
    const solver = req.query.solver;
    const cost = parseInt(req.query.cost || '-1');
    const options: FindOptions = {
        attributes: ['id', 'solver', 'task_id', 'score', 'valid', 'has_buy', 'cost'],
    };
    const where = getWhereOption(taskId, valid, solver, cost);
    if (valid) {
        where.valid = true;
    }
    if (checked) {
        where.checked = true;
    }
    if (solver) {
        where.solver = { [Op.like]: solver };
    }
    if (taskId > 0) {
        where.task_id = taskId;
    }
    if (where) {
        options.where = where;
    }
    if (limit) {
        options.limit = limit;
    }
    if (page > 1) {
        options.offset = limit * (page - 1);
    }
    const solutions = await LSolution.findAll(options);
    res.json({ solutions });
});

app.get('/stat/api', async (req, res, next) => {
    let num = req.query.num | 0;
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
        for (let idx in stat_by_tasks[tid].slice(0, num)) {
            stat.push(stat_by_tasks[tid][idx]);
        }
    }

    res.json({ values: stat });
});

app.get('/stat', (req, res, next) => {
    res.render('stat', {});
});


import fileUpload = require('express-fileupload');
import {optimizeSolutions, optimizeSolutions2} from "./optimizeSolution";

AWS.config.loadFromPath(process.env.KEY_JSON || './aws_key.json');
AWS.config.update({ region: 'us-east-2' });

app.use(fileUpload({ limits: { fileSize: 50 * 1024 * 1024 } }));

// endpoint for submission
app.post('/solution', async (req, res, next) => {
    const solver = req.body['solver'] || 'unknown';
    const task_id = parseInt(req.body['task']) || 0;
    const data = (req.files!.file as fileUpload.UploadedFile).data;
    const buyData = req.files!.buyFile ? (req.files!.buyFile as fileUpload.UploadedFile).data : undefined;
    const cost = buyData ? getTotalCost(buyData.toString()) : 0;
    if (cost < 0) {
        next();
        return;
    }
    let valid = false;
    const score = parseInt(req.body['score']) || -1;

    const solution = new LSolution({solver, task_id, score, valid, cost, has_buy: !!buyData });

    solution.save().then(async (model) => {
        console.log('created object ' + model);
        const params = {
            Bucket: defaultBucket,
            Key: generateKey(model),
            Body: data,
        };
        const s3 = new AWS.S3();

        putDataToS3(s3, params).then((data) => {
            if (buyData) {
                const params = {
                    Bucket: defaultBucket,
                    Key: generateBuyKey(model),
                    Body: buyData,
                };

                return putDataToS3(s3, params);
            }
        }).then(() => {
            res.json({
                solution: model,
            });
        }).catch((e) => {
            LSolution.destroy({where: {id: model.id}}).catch(() => {});
            console.error('faield to upload: ' + e);
            next(e);
        });
        const tmp = os.tmpdir();
        const key = encodeURIComponent(makeRandomId(`${solver}_${task_id}_`));
        const file = Path.join(tmp, key);
        const buyFile = buyData ? Path.join(tmp, key + '_buy') : undefined;

        await new Promise((resolve, reject) => {
            fs.writeFile(file, data, async (err) => {
                if (err) {
                    console.error('failed to write file: ' + err);
                    reject(err);
                } else {
                    resolve();
                }
            });
        }).then(() => {
            if (buyFile) {
                return new Promise((resolve, reject) => {
                    fs.writeFile(buyFile, buyData, async (err) => {
                        if (err) {
                            console.error('failed to write file: ' + err);
                            reject(err);
                        } else {
                            resolve();
                        }
                    });
                });
            }
        }).then(() => {
            return validateModel(model.task_id, model, file);
        }).catch((e) => {
            console.log('error in validate: ' + e);
        });
    }).catch((e) => {
        console.error('error: ' + e);
        next(e);
    });
});


app.get('/solution/best/zip', async (req, res, next) => {
    const budget = parseInt(req.query.budget || '0');
    const archive = archiver('zip');
    let promises: Promise<GetObjectOutput>[] = [];
    res.attachment('solutions.zip');
    archive.pipe(res);
    archive.on("error", next);
    await optimizeSolutions2(budget)
        .then((solutions) => {
            return solutions.map((solution) => {
                if (solution) {
                    return [pullDataFromS3(solution), pullBuyDataFromS3(solution)];
                }
                return [];
            });
        })
        .then((dataArray) => {
            return Promise.all(dataArray.map(async (data, i) => {
                if (data.length >= 2) {
                    console.log(data);
                    const sol = await data[0];
                    const buy = await data[1];
                    if (sol) {
                        const name = `prob-${formatNumber(i + 1)}.sol`;
                        archive.append(sol.Body as Readable, {name});
                    }
                    if (buy) {
                        const name = `prob-${formatNumber(i + 1)}.buy`;
                        archive.append(buy.Body as Readable, {name});
                    }
                }
            }));
        })
        .then(() => {
            archive.finalize();
        })
        .catch((e) => {
            console.error(e);
            next(e);
        });
});

app.get('/solution/best', async (req, res, next) => {
    const valid = !!req.query.valid;
    const taskId = parseInt(req.query.taskId || '0');
    const solver = req.query.solver;
    let promises = [];
    if (taskId === 0) {
        for (let i = 1; i <= taskNum; i++) {
            promises.push(getBestSolutionModel(getWhereOption(i, valid, solver)));
        }
    } else {
        promises.push(getBestSolutionModel(getWhereOption(taskId, valid, solver)));
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
        attributes: ['id', 'solver', 'task_id', 'has_buy'],
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
    pullObjectS3ToTmp(s3, params).then((file) => {
        if (target.has_buy) {
            console.log('pull buy file');
            const buyParams = {
                Bucket: defaultBucket,
                Key: generateBuyKey(target),
            };
            return pullObjectS3ToTmp(s3, buyParams).then((buyFile) => ({ file, buyFile }));
        }
        return Promise.resolve({ file, buyFile: undefined });
    }).then(({ file, buyFile }) => {
            return validateModel(target.task_id, target, file, buyFile)
                .then(() => res.json({solution: target}));
    }) .catch((e) => {
        console.error('failed to download ' + e);
        next(e);
    });
});


app.get('/solution/best/:id', async (req, res, next) => {
    const id = parseInt(req.params['id']);
    const valid = req.query.valid || false;
    const solver = req.query.solver;

    const best = await getBestSolutionModel(getWhereOption(id, valid, solver)).catch((e) => {
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
