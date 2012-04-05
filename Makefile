PROJECT_HOME=.

CC=gcc
CXX=g++
CFLAGS= -c -I./src
CXXFLAGS= -c -I./src
LDFLAGS=

DIRS=${PROJECT_HOME}/src/util
#PRUNE_TEST='-name *test*.cc -prune'
#SOURCES:=`find ${DIRS} ${PRUNE_TEST} -o -name '*.cc' -o -name '*.h' -print `
SOURCES=
LIBRARY=libjoeyutil.a
TEST=blockingqueue_test
PROGRAMS=${TEST}
LIBOBJECTS=${SOURCES:.cc=.o}

all:${LIBRARY}

check:${PROGRAMS} ${TEST}
	for t in ${TEST}; do echo "***** Running $$t"; 	./$$t || exit 1;	done

clean:
	-rm -f ${PROGRAMS} ${LIBRARY} ${LIBOBJECTS} ./src/*/*.o

${LIBRARY}:${LIBOBJECTS}
	rm -f $@
	${AR} -rsv $@ ${LIBOBJECTS}

blockingqueue_test: src/util/BlockingQueue_test.o ${LIBOBJECTS}
	${CXX} src/util/BlockingQueue_test.o ${LIBOBJECTS} -o $@ ${LDFLAGS} -lpthread

.cc.o:
	$(CXX) $(CXXFLAGS) $< -o $@
