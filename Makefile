SRCS       := main.c sha1.c
OBJS       := $(SRCS:%.c=%.o)
DEPS       := $(SRCS:%.c=%.d)
EXECUTABLE := stablefinder
CFLAGS     := -Wall -Werror -c -MMD -MP -std=c99

all: $(EXECUTABLE)

ifneq ($(strip $(DEPS)),)
-include $(DEPS)
endif

$(EXECUTABLE): $(OBJS)
	gcc $(OBJS) -o $(EXECUTABLE)

%.o: %.c
	gcc $(CFLAGS) -MF "$(@:%.o=.%.d)" "$<" -o "$@" 

clean:
	rm -rf $(OBJS) $(DEPS) $(EXECUTABLE)
