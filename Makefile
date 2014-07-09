CFLAGS=-g -Wall

all: amqp-value-test

clean:
	-rm amqp-value.tab.c amqp-value.tab.h amqp-value.lex.c amqp-value.lex.h amqp-value-test amqp-value.tab.o amqp-value.lex.o

amqp-value.tab.c amqp-value.tab.h: amqp-value.y
	bison -d $^

amqp-value.lex.c amqp-value.lex.h: amqp-value.flex
	flex -o amqp-value.lex.c --header-file=amqp-value.lex.h $^

amqp-value.tab.o: amqp-value.lex.h
amqp-value.lex.o: amqp-value.tab.h
amqp-value-test:: amqp-value.tab.o amqp-value.lex.o
	${CC} ${CFLAGS} -o $@ $^

