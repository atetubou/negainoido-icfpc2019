import * as Path from "path";
import * as os from "os";
import * as fs from "fs";

export const formatNumber =(n: number) => {
    let tmp = n;
    let nnn = '';
    nnn +=  Math.floor(tmp / 100).toString();
    tmp %= 100;
    nnn += Math.floor(tmp / 10).toString();
    tmp %= 10;
    nnn += tmp.toString();
    return nnn;
};

export const getDescFile = (taskId: number) => Path.join(__dirname, '../descs', `prob-${formatNumber(taskId)}.desc`);

const characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
export const makeRandomId = (prefix: string) => {
    let result = prefix;
    const charactersLength = characters.length;
    for ( var i = 0; i < 5; i++ ) {
        result += characters.charAt(Math.floor(Math.random() * charactersLength));
    }
    return result;
};

export const putDataToS3 = (s3: AWS.S3, params: AWS.S3.PutObjectRequest) => {
    return new Promise<AWS.S3.PutObjectOutput>((resolve, reject) => {
        s3.putObject(params, (err, data) => {
            if (err) {
                console.error('faield to upload: ' + err);
                reject(err);
            } else {
                console.log(data);
                resolve(data);
            }
        });
    });
};

export const pullObjectS3ToTmp = (s3: AWS.S3, params: AWS.S3.GetObjectRequest) => {
    return new Promise<string>((resolve, reject) => {
        s3.getObject(params, (err, data) => {
            if (err) {
                console.error('failed to download: ' + err);
                reject(err);
                return;
            } else {
                const tmp = os.tmpdir();
                const file = Path.join(tmp, encodeURIComponent(params.Key));
                fs.writeFile(file, data.Body, async (err) => {
                    if (err) {
                        console.error('failed to write file: ' + err);
                        reject(err);
                    } else {
                        resolve(file);
                    }
                });

            }
        });
    });
};
