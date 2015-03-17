
TARGET :=
SUBDIR := buffer

ALL: $(SUBDIR)
	@make -C $^

clean: $(SUBDIR)
	@make -C $^ clean
	@rm *.o $(TARGET) -rf
