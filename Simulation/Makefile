
include Rules.mk

OBJECTS = $(SOURCES:%.cpp=%.o)
CXXFLAGS += $(addprefix -I, $(LOCAL_INCLUDES)) $(addprefix -L, $(LOCAL_LIBS))

.PRECIOUS: %.d
.SECONDARY: $(OBJECTS)

%.elf: $(PCHCPP).o $(OBJECTS)
	$(LINK) $(CXXFLAGS) -o $@ $^ $(addprefix -l, $(SYSTEM_LIBRARIES)) `pkg-config --static --libs glfw3`

%.o: %.cpp
	$(COMPILE) -c $*.cpp -o $*.o $(CXXFLAGS) $(shell if [ "$*.cpp" = "$(PCHCPP).cpp" ]; then echo "-pch-create"; else echo "-pch-use"; fi) $(PCHCPP).pch |& icc_colorize

all:	CXXFLAGS += $(CXXRELEASEFLAGS)
all:	$(TARGET)

debug:	CXXFLAGS += $(CXXDEBUGFLAGS)
debug:	$(TARGET)

depend: .depend

.depend: $(SOURCES)
	rm -f ./.depend
	$(COMPILE) $(CXXFLAGS) -MM $^ > ./.depend;

include .depend

clean:
	rm -f ./.depend *~ *.o *.pch $(TARGET)
