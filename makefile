#Murat Ulu
#"I pledge my honor that I have abided by the Stevens Honor System."

all:
	gcc -g -Wall -Werror -pedantic-errors -o chatclient chatclient.c
	gcc -g -Wall -Werror -pedantic-errors -o chatserver chatserver.c
clean:
	rm -f chatclient chatserver
