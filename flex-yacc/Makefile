CFLAGS=-g -Os -Wall -std=gnu99 -DNDEBUG

.PHONY: all

all: amqp-value-yacc

CLEAN=\
	amqp-value.tab.c amqp-value.tab.h amqp-value.lex.c amqp-value.lex.h \
	*.o \
	amqp-value-yacc \

clean:
	-rm $(CLEAN)

amqp-value.tab.c amqp-value.tab.h: amqp-value.y
	bison -d $^

amqp-value.lex.c amqp-value.lex.h: amqp-value.flex
	flex -o amqp-value.lex.c --header-file=amqp-value.lex.h $^

amqp-value.tab.o: amqp-value.tab.c amqp-value.lex.h
amqp-value.lex.o: amqp-value.lex.c amqp-value.tab.h
amqp-value-yacc: amqp-value.tab.o amqp-value.lex.o main.o
	${CC} ${CFLAGS} -o $@ $^ -lqpid-proton

