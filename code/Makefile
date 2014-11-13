
STD=-std=c++11
CPPFLAGS=`llvm-config --cppflags`
LDFLAGS=`llvm-config --ldflags`
LIBS=`llvm-config --libs --system-libs`

SOURCES=scanner.cxx ast.cxx parser.cxx basic.cxx 

all:
	clang++ $(STD) -gdwarf-2 -o basic $(SOURCES) $(CPPFLAGS) $(LDFLAGS) $(LIBS)

