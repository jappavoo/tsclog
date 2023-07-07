clinesize=$(shell cat /sys/devices/system/cpu/cpu0/cache/*/coherency_line_size | head -1)
JAVA_INCLUDE=$(shell dirname $$(find /usr/lib/jvm/java-11* -name jni.h))
TSCLOG_H=edu_bu_cs_sesa_tsclog_tsclog.h
TSCLOG_CLASS_DIR=edu/bu/cs/sesa/tsclog
HEADERS := now.h cacheline.h tsclogc.h buffer.h
CFLAGS := -D COHERENCY_LINE_SIZE=${clinesize} 
#CFLAGS += -DVERBOSE
#CFLAGS += -g
CFLAGS += -O3

.PHONY: clean all

all: run

run: libtsclog.so tsclog testtsclog.class
	java  -Djava.library.path=$(shell pwd) testtsclog
	./tsclog

testtsclog.class: testtsclog.java
	javac $<

$(TSCLOG_CLASS_DIR)/tsclog.class: $(TSCLOG_CLASS_DIR)/tsclog.java
	javac $<

$(TSCLOG_H): $(TSCLOG_CLASS_DIR)/tsclog.java
	javac -h . $<

tsclog.o: tsclog.c ${HEADERS} $(TSCLOG_H)
	gcc ${CFLAGS} -D __TSCLOG_LIB__  -c -fPIC -I${JAVA_INCLUDE} -I${JAVA_INCLUDE}/linux $< -o $@

libtsclog.so: tsclog.o
	gcc ${CFLAGS} -D __TSCLOG_LIB__ -shared -fPIC -o $@ $< -lc

tsclog: tsclog.c ${HEADERS}
	gcc ${CFLAGS} -o $@ $<

clean:
	rm -rf $(wildcard *.o *.s *.so *.class $(TSCLOG_H) $(TSCLOG_CLASS_DIR)/*.class tsclog.h tsclog)
 
