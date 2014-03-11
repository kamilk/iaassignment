import os
from random import choice
from subprocess import call

dir = '..\\IaAssignment\\samples\\all'
files = os.listdir(dir)
file = choice(files)

exe = '..\\Debug\\IaAssignment.exe'
path = os.path.join(dir, file)
print 'running ', exe, path
call([exe, path, '..\\IaAssignment\\data\\empty.png'])