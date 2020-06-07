import unittest
from subprocess import Popen, PIPE, run

class TestDatabase(unittest.TestCase):
    TESTING_DB_FILENAME = 'simpledbtesting.sdb'

    def setUp(self):
        pass

    def tearDown(self):
        run(['rm', self.TESTING_DB_FILENAME])

    def run_db(self, commands):
        commands = '\n'.join(commands)
        commands += '\n'

        dbproc = Popen(["./bin/simpledb", self.TESTING_DB_FILENAME], stdin=PIPE, stdout=PIPE, text=True)
        results = dbproc.communicate(commands)[0]
        return results.split("\n")

    def test_insertsAndRetrievesARow(self):
        commands = ['insert 1 user user@example.com', 'select', '.exit']
        results = self.run_db(commands)
        self.assertIn("db > 1 user user@example.com", results)

    def test_allowsStringsThatAreMaximumLength(self):
        """
        Currently it is defined that username should be 32 bytes and email should be 255 bytes
        """
        long_username = "a"*32
        long_email = "b"*255
        long_insert = "insert 1 {} {}".format(long_username, long_email)
        commands = [long_insert, "select", ".exit"]
        results = self.run_db(commands)
        self.assertIn("db > 1 {} {}".format(long_username, long_email), results)

    def test_persistsDataWhenClosed(self):
        commands1 = ['insert 1 user user@email.com', '.exit']
        commands2 = ['select', '.exit']

        self.run_db(commands1)
        results = self.run_db(commands2)
        self.assertIn('db > 1 user user@email.com', results)

class TestErrors(unittest.TestCase):
    TESTING_DB_FILENAME = 'simpledbtesting.sdb'

    def setUp(self):
        pass

    def tearDown(self):
        run(['rm', self.TESTING_DB_FILENAME])

    def run_db(self, commands):
        commands = '\n'.join(commands)
        commands += '\n'

        dbproc = Popen(["./bin/simpledb", self.TESTING_DB_FILENAME], stdin=PIPE, stdout=PIPE, text=True)
        results = dbproc.communicate(commands)[0]
        return results.split("\n")

    def test_detectsUnrecognizedMetaCommands(self):
        commands = ['.not-a-command', '.exit']
        results = self.run_db(commands)
        self.assertEqual("db > Unrecognized command '.not-a-command'", results[0])

    def test_detectsArgumentCountErrorsInInsert(self):
        commands = ['insert 1 user', '.exit']
        results = self.run_db(commands)
        self.assertIn("db > Syntax error. Could not parse statement.", results)

    def test_detectsUnrecognizedStatements(self):
        commands = ['not-a-statement', '.exit']
        results = self.run_db(commands)
        self.assertIn("db > Unrecognized keyword at start of 'not-a-statement'", results)

    def test_detectsWhenTableIsFull(self):
        """ 
        Currently a page can hold 15 rows.
        A table has 100 pages. Therefore total rows in the table = 15*100 = 1500
        """
        commands = []
        for i in range(0, 1501):
            commands.append("insert {0} user{0} user{0}@email.com".format(i))
        commands.append(".exit")
        results = self.run_db(commands)
        self.assertIn("db > Error: Table full.", results)

    def test_detectsWhenStringsAreTooLong(self):
        long_username = "a"*33
        long_email = "b"*256
        long_insert = "insert 1 {} {}".format(long_username, long_email)
        commands = [long_insert, "select", ".exit"]
        results = self.run_db(commands)
        self.assertIn("db > Maximum string length exceeded.", results)

    def test_detectsWhenIDIsNegative(self):
        commands = ['insert -1 user user@email.com', 'select', '.exit']
        results = self.run_db(commands)
        self.assertIn("db > ID cannot be negative.", results)

if __name__ == "__main__":
    unittest.main()
