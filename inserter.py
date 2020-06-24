import sys
from subprocess import Popen, PIPE, run

TESTING_DB_FILENAME = 'simpledbtesting.sdb'

commands = []
for i in range(45):
    commands.append("insert {0} user{0} user{0}@email.com".format(i))
commands.append(".btree")
commands.append(".exit")

commands = '\n'.join(commands)
commands +='\n'
dbproc = Popen(["./bin/simpledb", TESTING_DB_FILENAME], stdin=PIPE, stdout=PIPE, text=True)
results = dbproc.communicate(commands)[0]
for res in results.split('\n'):
    print(res)

if '-r' in sys.argv:
    run(['rm', '-f', TESTING_DB_FILENAME])

