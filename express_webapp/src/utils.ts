import * as Path from "path";

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
