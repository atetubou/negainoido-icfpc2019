import * as express from 'express';

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
    res.json({ ['hello'] });
});

app.get('/stat', (req, res, next) => {
    res.render('index', {title: 'Express'});
});


import fileUpload = require('express-fileupload');
import AWS = require('aws-sdk');
AWS.config.loadFromPath(process.env.KEY_JSON || './aws_key.json');
AWS.config.update({ region: 'us-east-2' });
const defaultBucket = process.env.S3_BUCKET || 'negainoido-icfpc2019-dev';

app.use(fileUpload({ limits: { fileSize: 50 * 1024 * 1024 } }));

app.use('/public', express.static('web/dist'));

const generateKey = (model: LSolution) => `solution_${model.solver}_${model.task_id}_${model.id}`;

app.post('/solution', (req, res, next) => {
    const solver = req.body['solver'] ||  'unknown';
    const task_id = parseInt(req.body['task']) || 0;
    const score = req.body['score'] || Infinity;
    const data = (req.files!.file as fileUpload.UploadedFile).data;

    const solution = new LSolution({ solver, task_id, score });

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
                LSolution.destroy({ where: { id: model.id }}).catch(() => {});
                console.error('faield to upload: ' + err);
                res.status(500);
                res.json({ error: err });
            } else {
                console.log(data);
                res.json({ data });
            }
        });
    }).catch((e) => {
        console.error('error: '+ e);
        res.status(500);

        res.json({ error: e });
    });
});

const getBestSolutionModel = (taskId: number) => {
    return LSolution.findOne({
        attributes: ['id', 'solver', 'task_id'],
        where: { task_id: taskId },
        order: [['score', 'ASC']]
    });
};

const getBestSolution = (taskId: number) => {
    return getBestSolutionModel(taskId).then((model) => {
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

const formatNumber =(n: number) => {
    let tmp = n;
    let nnn = '';
    nnn +=  Math.floor(tmp / 100).toString();
    tmp %= 100;
    nnn += Math.floor(tmp / 10).toString();
    tmp %= 10;
    nnn += tmp.toString();
    return nnn;
};

app.get('/solution/best/zip', async (req, res, next) => {
    const archive = archiver('zip');
    let promises: Promise<GetObjectOutput>[] = [];
    res.attachment('solutions.zip');
    archive.pipe(res);
    for (let i = 1; i <= 150; i++) {
        promises.push(getBestSolution(i));
    }
    Promise.all(promises).then((solutions) => {
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
            res.status(500);
            res.json({ error: e });
        });
});

app.get('/solution/best', async (req, res, next) => {
    let promises = [];
    for (let i = 1; i <= 150; i++) {
        promises.push(getBestSolutionModel(i));
    }
    Promise.all(promises).then((solutions) => {
        res.json({ solutions });
    })
        .catch((e) => {
            console.error(e);
            res.status(500);
            res.json({ error: e });
        });
});

app.get('/solution/best/:id', async (req, res, next) => {
    const id = parseInt(req.params['id']);

    const best = await LSolution.findOne({
        attributes: ['id', 'solver', 'task_id'],
        where: { task_id: id },
        order: [['score', 'ASC']]
    }).catch((e) => {
        console.error("DB error" + e);
        res.status(500);
        res.json({ error: e });
    });

    if (!best) {
        console.error("error");
        res.status(500);
        res.json({});
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
            res.status(500);
            res.json({ error: err });
        } else {
            res.set('Content-Type', data.ContentType);
            res.set('Content-Disposition', 'attachment; filename='+key);
            res.set('Content-Length', data.ContentLength.toString());
            res.end(data.Body, 'binary');
        }
    });
});

app.use((err, req, res, next) =>  {
    console.error(err.stack);
    res.status(500).send('internal server error');
});
