import Starter
import os
import sys

if len(sys.argv) < 2:
    print 'Not enough arguments'
    exit()

dir = Starter.get_individual_path(sys.argv[1])
for file in os.listdir(dir):
    path = os.path.join(dir, file)
    exit_code = Starter.launch_for_file(path)
    if exit_code == 1: break
