import filecmp
import os
import subprocess
import unittest


def sh(s):
    subprocess.Popen(s.split(' ')).wait()


# does the same thing as filecmp.cmp, but expectedPath file may contain an additional trailing newline character, as
# it seems to be impossible to flush stdout in JS without adding a newline
def compareFilesUpToTrailingNewline(outPath, expectedPath):
    with open(expectedPath, "r") as expectedFile, open(outPath, "r") as outFile:
        expected = expectedFile.read()
        out = outFile.read()
        if len(expected) == len(out):
            return expected == out
        if len(expected) + 1 == len(out) and out[-1] == '\n':
            return expected == out[:-1]
        return False


class Tester(object):
    def __init__(self, programsDir, bfCompilerOptions, llvm2targetCompiler, llvm2targetCompilerOptions, runPrefix,
                 assertTrue, binary, fileCaomparator):
        super().__init__()
        self.programsDir = programsDir
        self.bfCompilerOptions = bfCompilerOptions
        self.llvm2targetCompiler = llvm2targetCompiler
        self.llvm2targetCompilerOptions = llvm2targetCompilerOptions
        self.runPrefix = runPrefix
        self.assertTrue = assertTrue
        self.binary = binary
        self.fileComparator = fileCaomparator

    def before(self) -> None:
        self.programPaths = []
        for subdir, dirs, files in os.walk(self.programsDir):
            for file in files:
                if file.endswith("bfpp"):
                    self.programPaths.append(os.path.abspath(self.programsDir + file))

    def forProgram(self, programPath):
        print(f"Testing on source {programPath}")
        pathBinary, pathExpected, pathIn, pathLL, pathNoExtension, pathOut, _ = self.getPaths(programPath)
        sh(f"{self.binary} {programPath} -o {pathLL} {self.bfCompilerOptions}")
        self.assertTrue(os.path.isfile(pathLL))
        sh(f"{self.llvm2targetCompiler} {pathLL} -o {pathBinary} {self.llvm2targetCompilerOptions}")
        self.assertTrue(os.path.isfile(pathBinary))

        with open(pathIn, "r") as fileIn, open(pathOut, "w") as fileOut:
            subprocess.Popen((self.runPrefix + pathBinary).split(' '), stdin=fileIn, stdout=fileOut).wait()

        self.assertTrue(self.fileComparator(pathOut, pathExpected), msg=f'{pathNoExtension} wrong output')

    def getPaths(self, programPath):
        pathNoExtension = programPath[:programPath.find('.')]
        pathLL = pathNoExtension + ".ll"
        pathIn = pathNoExtension + ".in"
        pathBinary = pathNoExtension + ".bin"
        pathOut = pathNoExtension + ".out"
        pathExpected = pathNoExtension + ".expected"
        pathWASM = pathNoExtension + ".wasm"
        return pathBinary, pathExpected, pathIn, pathLL, pathNoExtension, pathOut, pathWASM

    def test(self):
        for file in self.programPaths:
            self.forProgram(file)

    def after(self) -> None:
        for file in self.programPaths:
            pathBinary, _, _, pathLL, pathNoExtension, pathOut, pathWASM = self.getPaths(file)
            for file in [pathBinary, pathLL, pathNoExtension, pathOut, pathWASM]:
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
        tester = Tester('test/programs/', '-t 3', "clang", "", "", self.assertTrue, self.binary,
                        lambda e, o: filecmp.cmp(e, o, shallow=False))
        self.general_test(tester)

    def test_legacy(self):
        tester = Tester('test/legacy/', '-t 3 -l', "clang", "", "", self.assertTrue, self.binary,
                        lambda e, o: filecmp.cmp(e, o, shallow=False))
        self.general_test(tester)

    def test_modernJS(self):
        tester = Tester('test/programs/', '-t 3 --target wasm32-unknown-emscripten', "/usr/lib/emscripten/emcc",
                        "-s EXIT_RUNTIME=1", "node ", self.assertTrue, self.binary,
                        compareFilesUpToTrailingNewline)
        self.general_test(tester)

    def test_legacyJS(self):
        tester = Tester('test/legacy/', '-t 3 -l --target wasm32-unknown-emscripten', "/usr/lib/emscripten/emcc",
                        "-s EXIT_RUNTIME=1", "node ", self.assertTrue, self.binary,
                        compareFilesUpToTrailingNewline)
        self.general_test(tester)


if __name__ == '__main__':
    unittest.main()
