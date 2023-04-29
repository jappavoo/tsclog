clinesize=$(shell cat /sys/devices/system/cpu/cpu0/cache/*/coherency_line_size | head -1)
JAVA_INCLUDE=/usr/lib/jvm/java-11-openjdk/include

.PHONY: clean all

all: tsclog.so

tsclog.class: tsclog.java
	javac tsclog.java

tsclog.h: tsclog.class 
	javac -h . tsclog.java

tsclog.o: tsclog.c now.h cacheline.h tsclog.h
	gcc -D __TSCLOG_LIB__ -O2 -c -fPIC -I${JAVA_INCLUDE} -I${JAVA_INCLUDE}/linux $< -o $@

tsclog.so: tsclog.o
	gcc -D __TSCLOG_LIB__ -shared -fPIC -o $@ $< -lc

tsclog: tsclog.c now.h cacheline.h 
	gcc -O2 -D COHERENCY_LINE_SIZE=${clinesize} -o $@ $<

clean:
	rm -rf $(wildcard *.o *.s *.so *.class tsclog.h tsclog)
