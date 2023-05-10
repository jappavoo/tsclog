clinesize=$(shell cat /sys/devices/system/cpu/cpu0/cache/*/coherency_line_size | head -1)
JAVA_INCLUDE=$(shell dirname $$(find /usr/lib/jvm/java-11* -name jni.h))
HEADERS := now.h cacheline.h tsclogc.h buffer.h
CFLAGS := -D COHERENCY_LINE_SIZE=${clinesize} -DVERBOSE
CFLAGS += -g
CFLAGS += -O0

.PHONY: clean all

all: run

run: libtsclog.so
	java  -Djava.library.path=$(shell pwd) tsclog

tsclog.class: tsclog.java
	javac tsclog.java

tsclog.h: tsclog.class 
	javac -h . tsclog.java

tsclog.o: tsclog.c ${HEADERS} tsclog.h
	gcc ${CFLAGS} -D __TSCLOG_LIB__  -c -fPIC -I${JAVA_INCLUDE} -I${JAVA_INCLUDE}/linux $< -o $@

libtsclog.so: tsclog.o
	gcc ${CFLAGS} -D __TSCLOG_LIB__ -shared -fPIC -o $@ $< -lc

tsclog: tsclog.c ${HEADERS}
	gcc ${CFLAGS} -o $@ $<

clean:
	rm -rf $(wildcard *.o *.s *.so *.class tsclog.h tsclog)
