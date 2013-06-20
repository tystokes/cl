CC = g++
MKDEP=/usr/X11R6/bin/makedepend -Y
ifeq ($(shell uname), "Linux")
	LIBS = -lOpenCL
	CFLAGS = -g -Wall -Wno-deprecated -Ipath-OpenCL-include -Lpath-OpenCL-libdir
else
	LIBS = -lOpenCL
	CFLAGS = -g -Wall -Wno-deprecated -Ipath-OpenCL-include -Lpath-OpenCL-libdir
endif

HDRS =
SRCS =
HDRS_SLN =
SRCS_SLN = test.cpp
OBJS = $(patsubst %.cpp,%.o,$(SRCS)) $(patsubst %.cpp,%.o,$(SRCS_SLN))

test: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean
clean: 
	-rm -f -r $(OBJS) *.o *~ *core* test

depend: $(SRCS) $(SRCS_SLN) $(HDRS) $(HDRS_SLN) Makefile
	$(MKDEP) $(CFLAGS) $(SRCS) $(SRCS_SLN) $(HDRS) $(HDRS_SLN) >& /dev/null
