
STD=-std=c++11
DEB=-gdwarf-2 -O0
COV=--coverage
LLVMC=`llvm-config --cppflags`
LLVML=`llvm-config --ldflags --libs core --system-libs`

OBJ=scanner.o parser.o ast.o codeir.o lispast.o symtab.o

#
all: scanner parser

# 
scanner: scanner.o testScanner.o
	clang++ $(STD) $(DEB) $(COV) -o testScanner $^ 


testScanner.o: testScanner.cxx
	clang++ $(STD) $(DEB) $(COV) $(LLVMC) -c -o $@ $<

#
parser: $(OBJ) testParser.o
	clang++ $(STD) $(DEB) $(COV) -o testParser $^ $(LLVML)

testParser.o: testParser.cxx
	clang++ $(STD) $(DEB) $(COV) $(LLVMC) -c -o $@ $<

#
%.o: ../code/%.cxx
	clang++ $(STD) $(DEB) $(COV) $(LLVMC) -c -o $@ $<


clean:
	rm -rf *.o
	rm -rf *.gcov *.gcda *.gcno
	rm -rf testScanner testParser


