
include Rules.mk

.DEFAULT_GOAL := all

OBJECTS = $(SOURCES:%.cpp=%.o)
CXXFLAGS += $(addprefix -I, $(LOCAL_INCLUDES)) $(addprefix -L, $(LOCAL_LIBS))

.PRECIOUS: %.d
.SECONDARY: $(OBJECTS) $(PCHCPP).pch

%.elf: $(OBJECTS)
	@/bin/echo -e "\e[1;37m[LINK] \e[1;92m$@ \e[0m$^ $(CXXFLAGS) $(addprefix -l, $(SYSTEM_LIBRARIES)) `pkg-config --static --libs glfw3`\e[0m"
	@$(LINK) $(CXXFLAGS) -o $@ $^ $(addprefix -l, $(SYSTEM_LIBRARIES)) `pkg-config --static --libs glfw3`
	
TAGS:
	@/bin/echo -e "\e[1;94mCreating GTAGs...\e[0m"
	@gtags ./

$(PCHCPP).pch: $(PCHCPP).cpp
	@/bin/echo -e "\e[1;94mCreating precompiled-header...\e[0m"
	@rm -f $(PCHCPP).pch
	@$(COMPILE) -c $(PCHCPP).cpp -o $(PCHCPP).o $(CXXFLAGS) -pch-create $(PCHCPP).pch |& icc_colorize
	
%.o: $(PCHCPP).pch %.cpp
	@/bin/echo -e "\e[1;37m[CXX] \e[1;92m$*.cpp\e[0m -o $*.o $(CXXFLAGS)\e[0m"
	@$(COMPILE) -c $*.cpp -o $*.o $(CXXFLAGS) -pch-use $(PCHCPP).pch  |& icc_colorize

release:	CXXFLAGS += $(CXXRELEASEFLAGS)
release:	TAGS $(TARGET)

all:	debug

debug:	CXXFLAGS += $(CXXDEBUGFLAGS)
debug:	TAGS $(TARGET)

depend: .depend

.depend: $(SOURCES)
	@/bin/echo -e "\e[1;94mCalculating dependencies...\e[0m"
	@rm -f ./.depend
	@$(COMPILE) $(CXXFLAGS) -MM $^ > ./.depend;

include .depend

clean:
	rm -f ./.depend *~ *.o *.pch $(TARGET)
