import unittest
from subprocess import Popen, PIPE, run

class TestDatabase(unittest.TestCase):
    TESTING_DB_FILENAME = 'simpledbtesting.sdb'

    def setUp(self):
        pass

    def tearDown(self):
        run(['rm', "-f", self.TESTING_DB_FILENAME])

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

    def test_printsConstants(self):
        commands = ['.constants', '.exit']
        results = self.run_db(commands)
        expectedResults = [
            'ROW_SIZE: 293',
            'COMMON_NODE_HEADER_SIZE: 6',
            'LEAF_NODE_HEADER_SIZE: 14',
            'LEAF_NODE_CELL_SIZE: 297',
            'LEAF_NODE_SPACE_FOR_CELLS: 4082',
            'LEAF_NODE_MAX_CELLS: 13'
        ]
        for eres in expectedResults:
            self.assertIn(eres, results)

    def test_printsBtreeWithOneNode(self):
        commands = [
            "insert 3 user3 user3@website.com",
            "insert 1 user1 user1@website.com",
            "insert 2 user2 user2@website.com",
            ".btree",
            ".exit",
        ]
        results = self.run_db(commands)
        expectedResults = [
            "db > SimpleDB Tree:",
            "- leaf (size 3)",
            "    -1",
            "    -2",
            "    -3",
        ]
        for eres in expectedResults:
            self.assertIn(eres, results)

    def test_printBtreeWithMultipleNodes(self):
        commands = []
        for i in range(1, 15):
            commands.append("insert {0} user{0} user{0}@email.com".format(i))
        commands.append(".btree")
        results = self.run_db(commands)
        expectedResults = [
            "db > SimpleDB Tree:",
            "- internal (size 1)",
            "    - leaf (size 7)",
            "        -1",
            "        -2",
            "        -3",
            "        -4",
            "        -5",
            "        -6",
            "        -7",
            "    - key 7",
            "    - leaf (size 7)",
            "        -8",
            "        -9",
            "        -10",
            "        -11",
            "        -12",
            "        -13",
            "        -14",
        ]
        for eres in expectedResults:
            self.assertIn(eres, results)

    def test_selectStatementForMultiLevelTrees(self):
        commands = []
        for i in range(1, 16):
            commands.append("insert {0} user{0} user{0}@email.com".format(i))
        commands.append("select")
        commands.append(".exit")

        expectedResults = ["db > 1 user1 user1@email.com",]
        for i in range(2, 16):
            expectedResults.append("{0} user{0} user{0}@email.com".format(i))

        results = self.run_db(commands)

        for eres in expectedResults:
            self.assertIn(eres, results)

class TestErrors(unittest.TestCase):
    TESTING_DB_FILENAME = 'simpledbtesting.sdb'

    def setUp(self):
        pass

    def tearDown(self):
        run(['rm', "-f", self.TESTING_DB_FILENAME])

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
        self.assertIn("db > Need to implement splitting internal node", results)

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

    def test_detectsDuplicateKeys(self):
        commands = ['insert 1 user user@email.com', 'insert 1 anotheruser auser@email.com', '.exit']
        results = self.run_db(commands)
        self.assertIn("db > Error: Key already exits.", results)

if __name__ == "__main__":
    unittest.main()
