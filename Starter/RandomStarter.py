import os
from random import choice
from subprocess import call
import Starter

dir = '..\\IaAssignment\\samples\\all'
files = os.listdir(dir)

while True:
    file = choice(files)

    path = os.path.join(dir, file)
    
    exit_code = Starter.launch_for_file(path)
    if exit_code == 1: break
