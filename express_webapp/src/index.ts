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

    const solution = new LSolution({ solver, task_id, score });

    solution.save().then((model) => {
        console.log('created object ' + model);
        const params = {
            Bucket: defaultBucket,
            Key: generateKey(model),
            Body: (req.files!.file as fileUpload.UploadedFile).data,
        };
        const s3 = new AWS.S3();

        s3.putObject(params, (err, data) => {
            if (err) {
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
            res.set('Content-Type', data.ContentType)
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
