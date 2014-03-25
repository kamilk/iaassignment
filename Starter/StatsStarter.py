from __future__ import division
import Starter
import subprocess
import os
import sys
import re

if len(sys.argv) < 2:
    print 'Not enough arguments'
    exit()

dir = Starter.get_individual_path(sys.argv[1])
path = os.path.join(dir, '*.png')
proc = subprocess.Popen(['..\\Hand-in\\IaAssignment.exe', path], stdout = subprocess.PIPE)

r = re.compile('Event (\\d+)')
stats = []
for i in range(1, 6):
    stats.append(0)

count = 0
while True:
    line = proc.stdout.readline()
    if line == '': break
    line = line[(line.find(':')+1):]
    events = r.findall(line)
    print events
    for event in events:
        stats[int(event) - 1] += 1
    count += 1
        
descriptions = ['ontrack', 'entering', 'leaving', 'barrier', 'train']
print 'TOTAL: ', count
for event, occur in enumerate(stats):
    print 'Event ', event + 1, ' (', descriptions[event], ')\t: ', occur, ' (', occur/count * 100, '%)'
