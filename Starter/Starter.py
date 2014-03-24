import os
from subprocess import call

def get_categories_for_image(file):
    result = []
    individual_path = '..\\IaAssignment\\samples\\individual'
    categories = os.listdir(individual_path)
    for category in categories:
        path = os.path.join(individual_path, category)
        if file in os.listdir(path):
            result.append(category)
    return result
    
def launch_for_file(path):
    exe = '..\\Debug\\IaAssignment.exe'
    print 'running ', exe, path
    print get_categories_for_image(file)

    exit_code = call([exe, path, '..\\IaAssignment\\data\\empty.png'])
    return exit_code
