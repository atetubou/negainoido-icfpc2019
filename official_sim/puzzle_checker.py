from selenium.webdriver import Chrome, ChromeOptions
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions
from selenium.webdriver.common.by import By
from time import sleep
import re
import sys
import os

# example: `python main.py LA001.desc LA001.sol`

script_path = os.path.dirname(os.path.abspath(__file__))

cond = sys.argv[1]
desc = sys.argv[2]
    
url = 'file://' + script_path + '/checker/checker.html'

base_path = os.getcwd()

if not os.path.isabs(cond):
    cond = os.path.normpath(os.path.join(base_path, cond))
else:
    cond = os.path.normpath(cond)

if not os.path.isabs(desc):
    desc = os.path.normpath(os.path.join(base_path, desc))
else:
    desc = os.path.normpath(desc)

options = ChromeOptions()
# ヘッドレスモードを有効にする（次の行をコメントアウトすると画面が表示される）。
options.add_argument('--headless')
# ChromeのWebDriverオブジェクトを作成する。
driver = Chrome(options=options)

try:
    driver.get(url)

    assert 'Puzzle' in driver.title

    # find input model file
    task_input = WebDriverWait(driver, 1000).until(
        expected_conditions.presence_of_element_located((By.ID, "submit_task"))
    )
    solution_input = WebDriverWait(driver, 1000).until(
        expected_conditions.presence_of_element_located((By.ID, "submit_solution"))
    )
    button = WebDriverWait(driver, 1000).until(
        expected_conditions.presence_of_element_located((By.ID, "execute_solution"))
    )

    task_input.send_keys(cond);
    sleep(1)
    solution_input.send_keys(desc)
    sleep(1)
    button.click()

    stdout_text = ''
    wait_limit = 3000
    while wait_limit > 0:
        stdout_text = WebDriverWait(driver, 1000).until(
            expected_conditions.presence_of_element_located((By.ID, "output"))
        ).text

        if stdout_text.find('Success!') >= 0:
            break
        if stdout_text.find('Failed') >= 0 or stdout_text.find('Cannot check') >= 0:
            print('Found failure: ' + stdout_text)
            exit(1)
        if stdout_text.find('Not all parts of the task were covered.') >= 0:
            print('Wrong solution: ' + stdout_text)
            exit(1)
        sleep(1)
        wait_limit -= 1

    if stdout_text.find('Success!') < 0:
        print('timeout')
        exit(1)

    print('Success')
# スクリーンショットを撮る。
#driver.save_screenshot('tracer.png')

finally:
    driver.quit()  # ブラウザーを終了する

