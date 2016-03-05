
include Rules.mk

.DEFAULT_GOAL := $(DEFAULT_CONFIGURATION)

BUILD_CONFIG = $(firstword $(filter $(BUILD_CONFIGURATIONS),$(MAKECMDGOALS)) $(DEFAULT_CONFIGURATION))

WORK_DIR = $(BUILD_DIR)/$(BUILD_CONFIG)
OBJECTS = $(SOURCES:%.cpp=$(WORK_DIR)/%.o)
PCH = $(WORK_DIR)/$(PCHCPP).pch

CXXFLAGS += $(CXXFLAGS_$(BUILD_CONFIG))
CXXFLAGS += $(addprefix -I, $(LOCAL_INCLUDES)) $(addprefix -L, $(LOCAL_LIBS))

.SECONDARY: $(OBJECTS) $(PCH)

$(BUILD_DIR)/%.elf: $(OBJECTS)
	@/bin/echo -e "\e[1;37m[LINK] \e[1;92m$@ \e[0m$(CXXFLAGS) $(addprefix -l, $(SYSTEM_LIBRARIES)) `pkg-config --static --libs glfw3`\e[0m"
	@/usr/bin/mkdir -p $(dir $@)
	@$(LINK) $(CXXFLAGS) -o $@ $^ $(addprefix -l, $(SYSTEM_LIBRARIES)) `pkg-config --static --libs glfw3`

$(BUILD_DIR)/$(TARGET)_release.elf:	$(BUILD_DIR)/$(TARGET).elf
	
TAGS: $(SOURCES) $(HEADERS)
	@/bin/echo -e "\e[1;94mCreating GTAGs...\e[0m"
	@/usr/bin/gtags ./

$(WORK_DIR)/%.pch: %.cpp
	@/bin/echo -e "\e[1;94mCreating precompiled-header $@...\e[0m"
	@/usr/bin/rm -f $@
	@/usr/bin/mkdir -p $(dir $@)
	@$(COMPILE) -c $^ -o $(WORK_DIR)/$*.o $(CXXFLAGS) -pch-create $@ |& icc_colorize
	
$(WORK_DIR)/%.o: $(PCH) %.cpp
	@/bin/echo -e "\e[1;37m[CXX] \e[1;92m$*.cpp\e[0m $(CXXFLAGS)\e[0m"
	@/usr/bin/mkdir -p $(dir $@)
	@$(COMPILE) -c $*.cpp -o $@ $(CXXFLAGS) -pch-use $(PCH)  |& icc_colorize

$(BUILD_CONFIGURATIONS):	TAGS $(BUILD_DIR)/$(TARGET)_$(BUILD_CONFIG).elf

help:
	@/usr/bin/echo '"$(TARGET)" makefile'
	@/usr/bin/echo "Possible build configurations: $(BUILD_CONFIGURATIONS)"

depend: $(BUILD_DIR)/.depend

$(BUILD_DIR)/.depend: $(SOURCES)
	@/usr/bin/mkdir -p $(BUILD_DIR)/
	@/bin/echo -e "\e[1;94mCalculating dependencies...\e[0m"
	@/usr/bin/rm -f $(BUILD_DIR)/.depend
	@$(COMPILE) $(CXXFLAGS) -MM $^ > $(BUILD_DIR)/.depend;

include $(BUILD_DIR)/.depend

clean:
	/usr/bin/rm -rf $(BUILD_DIR)
