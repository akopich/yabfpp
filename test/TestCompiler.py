import filecmp
import os
import subprocess
import unittest


class MyTestCase(unittest.TestCase):
    def sh(self, s):
        subprocess.Popen(s.split(' ')).wait()

    def setUp(self) -> None:
        os.chdir('../')
        if os.path.isdir("build"):
            self.sh("rm -rf build")
        self.sh("cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Release")
        self.sh("cmake --build build --target all -j 20")
        self.binary = "build/yabfpp"
        programsDir = 'test/programs/'
        self.programPaths = []
        for subdir, dirs, files in os.walk(programsDir):
            for file in files:
                if file.endswith("bfpp"):
                    self.programPaths.append(os.path.abspath(programsDir + file))

    def forProgram(self, programPath):
        pathBinary, pathExpected, pathIn, pathLL, pathNoExtension, pathOut = self.getPaths(programPath)
        self.sh(f"{self.binary} {programPath} -o {pathLL} -t 3")
        self.assertTrue(os.path.isfile(pathLL))
        self.sh(f"clang {pathLL} -o {pathBinary}")
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

    def test_something(self):
        for file in self.programPaths:
            self.forProgram(file)

    def tearDown(self) -> None:
        for file in self.programPaths:
            pathBinary, _, _, pathLL, pathNoExtension, pathOut = self.getPaths(file)
            for file in [pathBinary, pathLL, pathNoExtension, pathOut]:
                if os.path.isfile(file):
                    os.remove(file)


if __name__ == '__main__':
    unittest.main()
