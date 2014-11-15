
STD=-std=c++11
DEB=-gdwarf-2
CPPFLAGS=`llvm-config --cppflags`
LDFLAGS=`llvm-config --ldflags`
LIBS=`llvm-config --libs --system-libs`

TARGET=basic

SOURCES=scanner.cxx ast.cxx parser.cxx basic.cxx
OBJECTS=scanner.o ast.o parser.o basic.o

all: $(OBJECTS)
	clang++ $(STD) $(DEB) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(LIBS)

ast.o: ast.cxx ast.hxx
	clang++ $(STD) $(DEB) -c $(CPPFLAGS) ast.cxx

scanner.o: scanner.cxx scanner.hxx tokens.hxx
	clang++ $(STD) $(DEB) -c $(CPPFLAGS) scanner.cxx

parser.o: parser.cxx parser.hxx tokens.hxx
	clang++ $(STD) $(DEB) -c $(CPPFLAGS) parser.cxx

basic.o: basic.cxx
	clang++ $(STD) $(DEB) -c $(CPPFLAGS) basic.cxx


clean:
	rm -rf *.o
	rm basic
	rm *~


