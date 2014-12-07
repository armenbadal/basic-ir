
STD=-std=c++11
DEB=-gdwarf-2
CPPFLAGS=`llvm-config --cppflags`
LDFLAGS=`llvm-config --ldflags`
LIBS=`llvm-config --libs --system-libs`
COVFLAGS=-fprofile-arcs -ftest-coverage

TARGET=basic

SOURCES=scanner.cxx ast.cxx codeir.cxx lispast.cxx parser.cxx symtab.cxx compiler.cxx basic.cxx
OBJECTS=scanner.o ast.o codeir.o lispast.o parser.o symtab.o compiler.o basic.o

all: $(OBJECTS)
	clang++ $(STD) $(DEB) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(LIBS)

%.o: %.cxx support.hxx
	clang++ $(STD) $(DEB) -c $(CPPFLAGS) -o $@ $<

support.hxx: support.c
	clang -S -emit-llvm -o support.ll support.c
	gawk -f irtoc.awk support.ll > support.hxx

clean:
	rm -rf *.o
	rm -rf basic
	rm -rf *~
	rm -rf support.hxx support.ll


