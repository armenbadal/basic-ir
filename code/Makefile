
STD=-std=c++11
DEB=-gdwarf-2
CPPFLAGS=`llvm-config --cppflags`
LDFLAGS=`llvm-config --ldflags`
LIBS=`llvm-config --libs core --system-libs`

TARGET=basic

SOURCES=scanner.cxx ast.cxx codeir.cxx parser.cxx basic.cxx
OBJECTS=scanner.o ast.o codeir.o parser.o basic.o

all: $(OBJECTS)
	clang++ $(STD) $(DEB) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(LIBS)

ast.o: ast.cxx ast.hxx
	clang++ $(STD) $(DEB) -c $(CPPFLAGS) ast.cxx

codeir.o: codeir.cxx ast.hxx
	clang++ $(STD) $(DEB) -c $(CPPFLAGS) codeir.cxx

scanner.o: scanner.cxx scanner.hxx tokens.hxx
	clang++ $(STD) $(DEB) -c $(CPPFLAGS) scanner.cxx

parser.o: parser.cxx parser.hxx tokens.hxx
	clang++ $(STD) $(DEB) -c $(CPPFLAGS) parser.cxx

basic.o: basic.cxx
	clang++ $(STD) $(DEB) -c $(CPPFLAGS) basic.cxx


clean:
	rm -rf *.o
	rm -rf basic
	rm -rf *~


