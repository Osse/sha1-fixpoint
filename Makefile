SRCS       := main.c sha1.c
OBJS       := $(SRCS:%.c=%.o)
DEPS       := $(SRCS:%.c=%.d)
EXECUTABLE := sha1-fixpoint
CFLAGS     := -Wall -Werror -c -MMD -MP -std=c99
GITVERSION := git:$(shell git describe --always --tags 2>/dev/null)

ifneq ($(GITVERSION),git:)
CFLAGS     := $(CFLAGS) -D GITVERSION='"$(GITVERSION)"'
endif

all: $(EXECUTABLE)

ifneq ($(strip $(DEPS)),)
-include $(DEPS)
endif

$(EXECUTABLE): $(OBJS)
	gcc $(OBJS) -o $(EXECUTABLE)

%.o: %.c
	gcc $(CFLAGS) -MF "$(@:%.o=%.d)" "$<" -o "$@" 

clean:
	rm -rf $(OBJS) $(DEPS) $(EXECUTABLE)

version:
	echo $(GITVERSION)

