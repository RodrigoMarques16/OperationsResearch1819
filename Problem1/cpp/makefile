CXX		 = g++
CPPFLAGS = -std=c++17 -Wall
SRCS 	 = *.cpp
BIN		 = ./bin
NAME	 = main.out
TARGET	 = $(BIN)/$(NAME)
OBJS	 = $(subst .cpp,.o,$(SRCS))
RM		 = rm -f

default: debug

release: $(OBS)
	$(CXX) $(CPPFLAGS) -Ofast $(SRCS) -o $(TARGET) $(OBJS)

debug: $(OBS)
	$(CXX) $(CPPFLAGS) -g $(SRCS) -o $(TARGET) $(OBS)

clean:
	$(RM) $(BIN)/*

exec:
	$(TARGET) ./inputcpp.txt

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

include .depend

