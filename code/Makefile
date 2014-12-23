
STD=-std=c++11
DEB=-gdwarf-2
CPPFLAGS=$(shell llvm-config --cppflags)
LDFLAGS=$(shell llvm-config --ldflags)
LIBS=$(shell llvm-config --libs --system-libs)
#COV=-fprofile-arcs -ftest-coverage
COV=

TARGET=basic

SOURCES=scanner.cxx ast.cxx codeir.cxx lispast.cxx parser.cxx symtab.cxx compiler.cxx basic.cxx
OBJECTS=scanner.o ast.o codeir.o lispast.o parser.o symtab.o compiler.o basic.o

all: $(OBJECTS)
	clang++ $(STD) $(DEB) $(COV) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(LIBS)

%.o: %.cxx support.hxx
	clang++ $(STD) $(DEB) $(COV) -c $(CPPFLAGS) -o $@ $<

support.hxx: support.c
	clang -S -emit-llvm -o support.ll support.c
	gawk -f irtoc.awk support.ll > support.hxx

clean:
	rm -rf *.o
	rm -rf basic
	rm -rf *~
	rm -rf support.hxx support.ll
	rm -rf *.gcno *.gcda *.gcov


