from selenium.webdriver import Chrome, ChromeOptions
from time import sleep
import re
import sys
import os

# example: `python main.py LA001.mdl LA001.nbt`

base_path = os.path.dirname(os.path.abspath(__file__))

desc = sys.argv[1]
sol = sys.argv[2]
url = 'file://' + base_path + '/content/checker.html'

if not os.path.isabs(desc):
    desc = os.path.normpath(os.path.join(base_path, desc))
else:
    desc = os.path.normpath(desc)

if not os.path.isabs(sol):
    sol = os.path.normpath(os.path.join(base_path, sol))
else:
    sol = os.path.normpath(sol)

options = ChromeOptions()
# ヘッドレスモードを有効にする（次の行をコメントアウトすると画面が表示される）。
options.add_argument('--headless')
# ChromeのWebDriverオブジェクトを作成する。
driver = Chrome(options=options)

try:
    driver.get(url)

    assert 'Checker' in driver.title

    # find input model file
    task_input = driver.find_element_by_id('submit_task')
    task_input.send_keys(desc);

    solution_input = driver.find_element_by_id('submit_solution')
    solution_input.send_keys(sol)
    
    button = driver.find_element_by_id('execute_solution')
    button.click()

    wait_limit = 3000
    stdout_text = ''
    while wait_limit > 0:
        stdout_text = driver.find_element_by_id('output').text
        if stdout_text.find('Success!') >= 0:
            break
        if stdout_text.find('Failed') >= 0 or stdout_text.find('Cannot check') >= 0:
            print('Found failure: ' + stdout_text)
            exit(1)
        sleep(1)
        wait_limit -= 1

    energy = re.search('Success! Your solution took ([0-9]+) time units.', stdout_text)
    if energy == None:
        print('timeout')
        exit(1)

    print(energy.group(1))
# スクリーンショットを撮る。
#driver.save_screenshot('tracer.png')

finally:
    driver.quit()  # ブラウザーを終了する

