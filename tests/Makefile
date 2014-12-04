
STD=-std=c++11
DEB=-gdwarf-2 -O0
COV=--coverage
LLVMC=`llvm-config --cppflags`
LLVML=`llvm-config --ldflags --libs core --system-libs`

SRC=*.cxx
HDR=*.hxx
OBJ=scanner.o parser.o ast.o codeir.o lispast.o symtab.o

#
all: scanner parser

# 
scanner: copysrc scanner.o testScanner.o
	clang++ $(STD) $(DEB) $(COV) -o testScanner scanner.o testScanner.o 

#
parser: copysrc $(OBJ)
	clang++ $(STD) $(DEB) $(COV) -o testParser $(OBJ) testParser.o $(LLVML)

#
%.o: %.cxx
	clang++ $(STD) $(DEB) $(COV) $(LLVMC) -c -o $@ $<

copysrc:
	cp ../code/$(SRC) .
	cp ../code/$(HDR) . 

clean:
	rm -rf $(SRC) $(HDR)
	rm -rf *.o
	rm -rf *.gcov *.gcda *.gcno
	rm -rf testScanner testParser


