CFLAGS=-g -Os -Wall -std=gnu99 -DNDEBUG

.PHONY: all

all: amqp-value-yacc amqp-value-lemon

CLEAN=\
	amqp-value.tab.c amqp-value.tab.h amqp-value.lex.c amqp-value.lex.h amqp-value.out \
	amqp-value.re.c amqp-value.c amqp-value.h \
	*.o \
	amqp-value-yacc amqp-value-lemon

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

amqp-value.c amqp-value.h: amqp-value.lemon
	lemon amqp-value.lemon

amqp-value.re.c: amqp-value.re
	re2c -o amqp-value.re.c  amqp-value.re

amqp-value.re.o: amqp-value.h
amqp-value-lemon: main.o amqp-value.re.o amqp-value.o
	${CC} ${CFLAGS} -o $@ $^ -lqpid-proton

