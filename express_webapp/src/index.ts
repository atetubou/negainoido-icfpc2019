import * as express from 'express';

const normalizePort = (val: string) => {
    var port = parseInt(val, 10);
  
    if (port >= 0) {
      // port number
      return port;
    }
  
    return false;
}

const app = express();
const port = normalizePort(process.env.PORT || '3000');

app.listen(port);
app.set('view engine', 'pug');

import { Sequelize } from 'sequelize-typescript';
import { Table, Column, Model, HasMany, DataType, CreatedAt, PrimaryKey } from 'sequelize-typescript';
import mysql2 from 'mysql2';

@Table({ timestamps: true, tableName: 'Solutions'})
class Solution extends Model<Solution> {
    @Column
    solver: string;
    @Column(DataType.INTEGER)
    program_id: number
    @Column(DataType.DOUBLE)
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

sequelize.addModels([Solution]);

sequelize.sync();


app.get('/', (req, res, next) => {
    res.render('index', {title: 'Express'});
});

app.get('/solution', async (req, res, next) => {
    const solutions = await Solution.findAll();
    res.json({ solutions });
});



import fileUpload = require('express-fileupload');
import AWS = require('aws-sdk');
AWS.config.loadFromPath(process.env.KEY_JSON || './aws_key.json');
AWS.config.update({ region: 'us-east-2' });
const defaultBucket = process.env.S3_BUCKET || 'negainoido-icfpc-2019-dev';

app.use(fileUpload({ limits: { fileSize: 50 * 1024 * 1024 } }));

app.use('/public', express.static('web/dist'));

app.post('/solution', (req, res, next) => {
    const solver = req.body['solver'] ||  'unknown';
    const program_id = req.body['program'] || 0;
    const score = req.body['score'] || 0.0;

    const solution = new Solution({ solver, program_id, score });

    solution.save().then((model) => {
        console.log('created object ' + model);
        const params = {
            Bucket: defaultBucket,
            Key: `solution_${solver}_${program_id}_${model.id}`,
            Body: (req.files!.file as fileUpload.UploadedFile).data,
        }
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

app.use((err, req, res, next) =>  {
    console.error(err.stack);
    res.status(500).send('internal server error');
});
