clinesize=$(shell cat /sys/devices/system/cpu/cpu0/cache/*/coherency_line_size | head -1)
JAVA_INCLUDE=$(shell dirname $$(find /usr/lib/jvm/java-11* -name jni.h))

.PHONY: clean all

all: run

run: libtsclog.so
	java  -Djava.library.path=$(pwd) tsclog

tsclog.class: tsclog.java
	javac tsclog.java

tsclog.h: tsclog.class 
	javac -h . tsclog.java

tsclog.o: tsclog.c now.h cacheline.h tsclog.h
	gcc -D COHERENCY_LINE_SIZE=${clinesize} -D __TSCLOG_LIB__ -O2 -c -fPIC -I${JAVA_INCLUDE} -I${JAVA_INCLUDE}/linux $< -o $@

libtsclog.so: tsclog.o
	gcc -D __TSCLOG_LIB__ -shared -fPIC -o $@ $< -lc

tsclog: tsclog.c now.h cacheline.h 
	gcc -O2 -D COHERENCY_LINE_SIZE=${clinesize} -o $@ $<

clean:
	rm -rf $(wildcard *.o *.s *.so *.class tsclog.h tsclog)
