from __future__ import division
import Starter
import subprocess
import os
import sys
import re

if len(sys.argv) < 2:
    print 'Not enough arguments'
    exit()

event_name = sys.argv[1]
    
dir = Starter.get_individual_path(event_name)
path = os.path.join(dir, '*.png')
proc = subprocess.Popen(['..\\Hand-in\\IaAssignment.exe', path], stdout = subprocess.PIPE)

r = re.compile('Event (\\d+)')
stats = []
for i in range(1, 6):
    stats.append(0)

descriptions = ['ontrack', 'entering', 'leaving', 'barrier', 'train']
event_number = descriptions.index(event_name) + 1
wrongly_classified = []    
    
count = 0
while True:
    line = proc.stdout.readline()
    if line == '': break
    print line.strip()
    colon_idx = line.find(':')
    file_name = line[0:colon_idx]
    line = line[(colon_idx+1):]
    events = [int(event) for event in r.findall(line)]
    for event in events:
        stats[event - 1] += 1
    count += 1
    if event_number not in events:
        wrongly_classified.append(file_name)

print '\nImages not classified as \'', event_name, '\':'
for image in wrongly_classified:
    print image

print
        
print 'TOTAL: ', count
for event, occur in enumerate(stats):
    print 'Event ', event + 1, ' (', descriptions[event], ')\t: ', occur, '\t(', occur/count * 100, '%)'