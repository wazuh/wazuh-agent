default_target: all
.PHONY : default_target

OSSEC_CC = gcc
OSSEC_LINK = ar -crus
OSSEC_RANLIB = ranlib
EXTERNAL_PROCPS=./
PROCPS_LIB  = $(EXTERNAL_PROCPS)/libproc.a
PROCPS_INCLUDE=-I./${EXTERNAL_PROCPS}

procps_c := $(wildcard ${EXTERNAL_PROCPS}*.c)
procps_o := $(procps_c:.c=.o)

${EXTERNAL_PROCPS}%.o: ${EXTERNAL_PROCPS}%.c
	${OSSEC_CC} -fPIC -c $^ -o $@

$(PROCPS_LIB): ${procps_o}
	${OSSEC_LINK} $@ $^
	${OSSEC_RANLIB} $@

all: $(PROCPS_LIB)
