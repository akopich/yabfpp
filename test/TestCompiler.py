import filecmp
import os
import subprocess
import unittest


def sh(s):
    subprocess.Popen(s.split(' ')).wait()


class Tester(object):
    def __init__(self, programsDir, compilerOptions, assertTrue, binary):
        super().__init__()
        self.programsDir = programsDir
        self.compilerOptions = compilerOptions
        self.assertTrue = assertTrue
        self.binary = binary

    def before(self) -> None:
        self.programPaths = []
        for subdir, dirs, files in os.walk(self.programsDir):
            for file in files:
                if file.endswith("bfpp"):
                    self.programPaths.append(os.path.abspath(self.programsDir + file))

    def forProgram(self, programPath):
        print(f"Testing on source {programPath}")
        pathBinary, pathExpected, pathIn, pathLL, pathNoExtension, pathOut = self.getPaths(programPath)
        sh(f"{self.binary} {programPath} -o {pathLL} {self.compilerOptions}")
        self.assertTrue(os.path.isfile(pathLL))
        sh(f"clang {pathLL} -o {pathBinary}")
        self.assertTrue(os.path.isfile(pathBinary))
        fileIn = open(pathIn, "r")
        fileOut = open(pathOut, "w")
        subprocess.Popen([pathBinary], stdin=fileIn, stdout=fileOut).wait()
        fileIn.close()
        fileOut.close()
        self.assertTrue(filecmp.cmp(pathOut, pathExpected, shallow=True), msg=f'{pathNoExtension} wrong output')

    def getPaths(self, programPath):
        pathNoExtension = programPath[:programPath.find('.')]
        pathLL = pathNoExtension + ".ll"
        pathIn = pathNoExtension + ".in"
        pathBinary = pathNoExtension + ".bin"
        pathOut = pathNoExtension + ".out"
        pathExpected = pathNoExtension + ".expected"
        return pathBinary, pathExpected, pathIn, pathLL, pathNoExtension, pathOut

    def test(self):
        for file in self.programPaths:
            self.forProgram(file)

    def after(self) -> None:
        for file in self.programPaths:
            pathBinary, _, _, pathLL, pathNoExtension, pathOut = self.getPaths(file)
            for file in [pathBinary, pathLL, pathNoExtension, pathOut]:
                if os.path.isfile(file):
                    os.remove(file)


class TestCompiler(unittest.TestCase):
    @classmethod
    def setUpClass(self) -> None:
        os.chdir('../')
        if os.path.isdir("build"):
            sh("rm -rf build")
        sh("cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Release")
        sh("cmake --build build --target all -j 20")
        self.binary = "build/yabfpp"

    def general_test(self, tester):
        tester.before()
        tester.test()
        tester.after()

    def test_modern(self):
        tester = Tester('test/programs/', '-t 3', self.assertTrue, self.binary)
        self.general_test(tester)

    def test_legacy(self):
        tester = Tester('test/legacy/', '-t 3 -l', self.assertTrue, self.binary)
        self.general_test(tester)


if __name__ == '__main__':
    unittest.main()
