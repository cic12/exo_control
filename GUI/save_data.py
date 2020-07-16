import os
import datetime
from shutil import copyfile
import pandas as pd

ts_day = '{:%Y-%m-%d}'.format(datetime.datetime.now())
ts_test = '{:%Y-%m-%d_%H.%M.%S}'.format(datetime.datetime.now())

config = pd.read_csv('../res/config.txt',header=None)
cfg = config.values[0,:]

test = (ts_test + '_DEV' + str(cfg[0]) + 
        '_HUM' + str(cfg[1]) + 
        '_AIN' + str(cfg[2]) + 
        '_CTL' + str(cfg[3]) + 
        '_CFG' + str(cfg[4]) + 
        '_TRJ' + str(cfg[5]) +
        '_CON' + str(cfg[6]))

test_path = '../../exo_results/data/' + ts_day + '/' + test + '/'
os.makedirs(os.path.dirname(test_path), exist_ok=True)

def copy_test_file(test_path,file_path):
    copyfile('../res/' + file_path, test_path + file_path)
    
for file in os.listdir('../res/'): # for all .txt files
    if file.endswith(".txt"):
        copy_test_file(test_path,file) # copy to test directory
        