import Starter
import sys
import os

if len(sys.argv) < 2:
    print 'Not enough parameters'
    exit()

dir = '..\\IaAssignment\\samples\\all'
file_number = sys.argv[1]
file = 'lc-' + file_number.zfill(5) + '.png'
path = os.path.join(dir, file)

Starter.launch_for_file(path)
