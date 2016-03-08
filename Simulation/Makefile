
include Rules.mk

.DEFAULT_GOAL := $(DEFAULT_CONFIGURATION)

ifeq ($(strip $(BUILD_CONFIG)),)
	BUILD_CONFIG = $(firstword $(filter $(BUILD_CONFIGURATIONS),$(MAKECMDGOALS)) $(DEFAULT_CONFIGURATION))
endif

WORK_DIR = $(BUILD_DIR)/$(BUILD_CONFIG)
OBJECTS = $(SOURCES:%.cpp=$(WORK_DIR)/%.o)
DEPS = $(SOURCES:%.cpp=$(BUILD_DIR)/.depend/%_$(BUILD_CONFIG).d)
PCH = $(WORK_DIR)/$(SOURCE_DIR)/$(PCHCPP).pch

CXXFLAGS += $(CXXFLAGS_$(BUILD_CONFIG))
CXXFLAGS += $(addprefix -L, $(LOCAL_LIBS))
INCLUDE_FLAGS = $(addprefix -I, $(LOCAL_INCLUDES))

.SECONDARY: $(OBJECTS) $(PCH)

$(BUILD_DIR)/%.elf: $(OBJECTS)
	@/bin/echo -e "\e[1;37m[LINK] \e[1;92m$@ \e[0m$(CXXFLAGS) $(addprefix -l, $(SYSTEM_LIBRARIES)) $(LINKFLAGS)\e[0m"
	@/usr/bin/mkdir -p $(dir $@)
	@$(LINK) $(CXXFLAGS) -o $@ $^ $(addprefix -l, $(SYSTEM_LIBRARIES)) $(LINKFLAGS)

$(BUILD_DIR)/$(TARGET)_release.elf:	$(BUILD_DIR)/$(TARGET).elf
	
TAGS: $(SOURCES) $(HEADERS)
	@/bin/echo -e "\e[1;94mCreating GTAGs...\e[0m"
	@/usr/bin/gtags ./

$(WORK_DIR)/%.pch: %.cpp
	@/bin/echo -e "\e[1;94mCreating precompiled-header $@...\e[0m"
	@/usr/bin/rm -f $@
	@/usr/bin/mkdir -p $(dir $@)
	@$(COMPILE) -c $^ -o $(WORK_DIR)/$*.o -MMD -MP $(CXXFLAGS) $(INCLUDE_FLAGS) -pch-create $@ |& icc_colorize
	@/usr/bin/mkdir -p $(dir $(BUILD_DIR)/.depend/$*)
	@/usr/bin/mv $(WORK_DIR)/$*.d $(BUILD_DIR)/.depend/$*_$(BUILD_CONFIG).d
	
$(WORK_DIR)/%.o: $(PCH) %.cpp
	@/bin/echo -e "\e[1;37m[CXX] \e[1;92m$*.cpp\e[0m $(CXXFLAGS)\e[0m"
	@/usr/bin/mkdir -p $(dir $@)
	@$(COMPILE) -c $*.cpp -o $@ -MMD -MP $(CXXFLAGS) $(INCLUDE_FLAGS) -pch-use $(PCH)  |& icc_colorize
	@/usr/bin/mkdir -p $(dir $(BUILD_DIR)/.depend/$*)
	@/usr/bin/mv $(WORK_DIR)/$*.d $(BUILD_DIR)/.depend/$*_$(BUILD_CONFIG).d
	
$(WORK_DIR)/$(SOURCE_DIR)/$(PCHCPP).o:

$(BUILD_CONFIGURATIONS):	TAGS $(BUILD_DIR)/$(TARGET)_$(BUILD_CONFIG).elf

help:
	@/usr/bin/echo '"$(TARGET)" makefile'
	@/usr/bin/echo "Possible build configurations: $(BUILD_CONFIGURATIONS)"
	
vars:
	@/usr/bin/echo '"TARGET": $(TARGET)'
	@/usr/bin/echo '"BUILD_CONFIG": $(BUILD_CONFIG)'
	@/usr/bin/echo '"BUILD_DIR": $(BUILD_DIR)'
	@/usr/bin/echo '"WORK_DIR": $(WORK_DIR)'
	@/usr/bin/echo '"PCH": $(PCH)'
	@/usr/bin/echo '"LOCAL_INCLUDES": $(LOCAL_INCLUDES)'
	@/usr/bin/echo '"LOCAL_LIBS": $(LOCAL_LIBS)'
	@/usr/bin/echo '"CXXFLAGS": $(CXXFLAGS)'
	@/usr/bin/echo '"SOURCES": $(SOURCES)'
	@/usr/bin/echo '"OBJECTS": $(OBJECTS)'
	@/usr/bin/echo '"DEPS": $(DEPS)'

-include $(DEPS)

clean:
	@/usr/bin/echo 'Cleaning all...'
	/usr/bin/rm -rf $(BUILD_DIR)

clean_config:
	@/usr/bin/echo 'Cleaning $(WORK_DIR)...'
	/usr/bin/rm -rf $(WORK_DIR)
