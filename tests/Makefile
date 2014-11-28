
STD=-std=c++11
DEB=-gdwarf-2 -O0
COV=--coverage

#
all: scanner
	

# 
scanner: scanner.o testScanner.o
	clang++ $(STD) $(DEB) $(COV) -o testScanner scanner.o testScanner.o 

testScanner.o: testScanner.cxx
	clang++ $(STD) $(DEB) $(COV) -c -o testScanner.o testScanner.cxx 

scanner.o: 
	clang++ $(STD) $(DEB) $(COV) -c -o scanner.o ../code/scanner.cxx 



clean:
	rm -rf *.o
	rm -rf *.gcov *.gcda *.gcno
	rm -rf testScanner


