TARGETS=coins pi pis 
CC=cc
all : $(TARGETS)

$(TARGETS): %: %.c

clean: 
	@rm -f $(TARGETS) a.out *.o

#try adding -lpthread at the end of the gcc line if it doesn't compile and it has threads