import unittest
from subprocess import Popen, PIPE

class TestDatabase(unittest.TestCase):
    def setUp(self):
        print("[*] Setting up test.")

    def tearDown(self):
        print("[*] Ending test.")

    def run_db(self, commands):
        commands = '\n'.join(commands)
        commands += '\n'

        dbproc = Popen("./bin/simpledb", stdin=PIPE, stdout=PIPE, text=True)
        results = dbproc.communicate(commands)[0]
        return results.split("\n")

    def test_detectsUnrecognizedMetaCommands(self):
        commands = ['.not-a-command', '.exit']
        results = self.run_db(commands)
        self.assertEqual("db > Unrecognized command '.not-a-command'", results[0])
            

if __name__ == "__main__":
    unittest.main()