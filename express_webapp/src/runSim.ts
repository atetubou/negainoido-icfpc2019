import * as webdriver from 'selenium-webdriver';
import * as chrome from 'selenium-webdriver/chrome';
import * as Path from "path";

const sleep = (ms: number) => {
    return new Promise((resolve) => {
        setTimeout(resolve, ms)
    });
};

const runSimulator = async (discPath: string, solPath: string, buyPath?: string) => {
    const driver = await new webdriver.Builder()
        .forBrowser('chrome')
        .setChromeOptions(new chrome.Options().headless()
            .addArguments("--no-sandbox")
            .addArguments("--disable-dev-shm-usage"))
        .build();

    const url = 'file://' + Path.join(__dirname, '../sim/checker.html');

    return await driver.get(url).then(() => {
        const task_input = driver.findElement({ id: "submit_task" });
        const solution_input = driver.findElement({ id: "submit_solution" });
        const booster_input = driver.findElement({ id: "submit_boosters" });
        const button_input = driver.findElement({ id: "execute_solution" });

        return task_input.sendKeys(discPath).then(async () => {
            for (let count = 0; count < 10; count++) {
                const output = await driver.findElement({id: "output"}).getText().catch(() => '');
                if (output.startsWith('Done uploading task')) {
                    return solution_input.sendKeys(solPath);
                }
                await sleep(10);
            }
            throw 'error';
        }).then( async () => {
            for (let count = 0; count < 10; count++) {
                const output = await driver.findElement({id: "output"}).getText().catch(() => '');
                if (output.startsWith('Done uploading solution')) {
                    return Promise.resolve();
                }
                await sleep(10);
            }
            throw 'error';
        }).then(async () => {
            if (buyPath) {
                await booster_input.sendKeys(buyPath);
                await sleep(10);
            }
            await button_input.click();
            for (let count = 0; count < 600; count++) {
                const output = await driver.findElement({id: "output"}).getText().catch(() => '');
                if (output.startsWith('Success!')) {
                    return output;
                }
                if (output.startsWith('Failed') || output.startsWith('Not all parts of') || output.startsWith('Cannot check')) {
                    throw 'invalid sol';
                }
                await sleep(1000);
            }
            throw 'timeout';
        });
    }).then((text) => {
        const result = /Success! Your solution took ([0-9]+) time units./.exec(text);
        driver.quit().catch(() => {});
        return result[1];
    }).catch((e) => {
        console.log('error in simulator: ' + e);
        driver.quit().catch(() => {});
        throw e;
    });
};

export default runSimulator;