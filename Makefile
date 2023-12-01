all: crustashell.o ligne_commande.o
	gcc -ansi -pedantic -Wall -Wextra -Werror crustashell.o ligne_commande.o -o crustashell

%.o : %.c
	gcc -ansi -pedantic -Wall -Wextra -Werror -c -o $@ $<

clean:
	rm -f *.o crustashell
