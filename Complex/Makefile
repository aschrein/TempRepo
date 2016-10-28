all : libComplex.a main.o
	gcc main.o libComplex.a -lm -o main
main.o: main.c Complex.h
	gcc -c main.c -o main.o
libComplex.a: Complex.o
	ar rcs libComplex.a Complex.o
Complex.o: Complex.c Complex.h
	gcc -c Complex.c -o Complex.o
clean:
	rm -f *.o *.a main
