
TARGET := meplatform.a
TAR_TEST :=platform_test
CC := g++
ARLAGS := rcs

#=====================
TEST := platform_test 
CUR_DIR :=$(shell pwd)
TEST_INCLUDE :=-I$(CUR_DIR)
LDFLAG := -shared -fPIC
CFLAGS := -g -fPIC
#=====================

OBJECTS += $(patsubst %.cpp, %.o, $(wildcard *.cpp))
%.o:%.cpp
	$(CC) $(CFLAGS) $(DFLAGS) $(INCLUDE) -c $^ -o $@

$(TEST) : $(TARGET) platfrom_test.o
	g++ $(TEST_INCLUDE) $(LDFLAG) -o $(TEST) platfrom_test.o $(TARGET)

platfrom_test.o: test/platfrom_test.cpp
	g++ $(CFLAGS) $(DFLAGS) $(INCLUDE) $(TEST_INCLUDE) -c test/platfrom_test.cpp -o platfrom_test.o


$(TARGET):  $(OBJECTS) 
	$(AR) $(INCLUDE) $(ARLAGS) $@  $^ $(LIBS)

clean:
	@rm *.o $(TARGET) -rf
