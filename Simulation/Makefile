
include rules.make

.DEFAULT_GOAL := $(DEFAULT_CONFIGURATION)

ifeq ($(strip $(config)),)
	config = $(firstword $(filter $(BUILD_CONFIGURATIONS),$(MAKECMDGOALS)) $(DEFAULT_CONFIGURATION))
endif

TARGET_NAME = $(BIN_DIR)/$(TARGET)_$(config).elf

WORK_DIR = $(BUILD_DIR)/$(config)
OBJECTS = $(SOURCES:%.cpp=$(WORK_DIR)/%.o)
DEPS = $(SOURCES:%.cpp=$(BUILD_DIR)/.depend/%_$(config).d)
PCH = $(WORK_DIR)/$(SOURCE_DIR)/$(PCHCPP).gch

INCLUDE_FLAGS = -I$(dir $(PCH)) $(addprefix -I, $(LOCAL_INCLUDES))
CXXFLAGS += $(CXXFLAGS_$(config)) $(addprefix -isystem, $(SYSTEM_INCLUDES))
LINKFLAGS += $(addprefix -L, $(SYSTEM_LIBS))

.SECONDARY: $(OBJECTS) $(PCH)
.PHONY: docs

define make-cpp-target
$1:	$(WORK_DIR)/$(basename $(shell find $(SOURCE_DIR) -name "$1.cpp" | head -n 1)).o
endef
$(foreach d,$(SOURCES),$(eval $(call make-cpp-target,$(basename $(notdir $d)))))

$(BIN_DIR)/%.elf: $(OBJECTS)
	@/bin/echo -e "\e[1;37m[LINK] \e[1;92m$@ \e[0m$(CXXFLAGS) $(addprefix -l, $(SYSTEM_LIBRARIES)) $(LINKFLAGS)\e[0m"
	@/usr/bin/mkdir -p $(dir $@)
	@$(LINK) $(CXXFLAGS) -o $@ $^ $(addprefix -l, $(SYSTEM_LIBRARIES)) $(LINKFLAGS)
	@/bin/echo -e "\e[1;94mCompleted\e[0m"

$(WORK_DIR)/%.gch: %
	@/bin/echo -e "\e[1;94mCreating precompiled-header $@...\e[0m"
	@/usr/bin/rm -f $@
	@/usr/bin/mkdir -p $(dir $@)
	@$(COMPILE) -x c++ -c $^ -o $(WORK_DIR)/$*.gch -MMD -MP $(CXXFLAGS) $(INCLUDE_FLAGS)
	@/usr/bin/mkdir -p $(dir $(BUILD_DIR)/.depend/$*)
	@/usr/bin/mv $(WORK_DIR)/$*.d $(BUILD_DIR)/.depend/$*_$(config).d

$(WORK_DIR)/%.o: $(PCH) %.cpp
	@/bin/echo -e "\e[1;37m[CXX] \e[1;92m$*.cpp\e[0m $(CXXFLAGS)\e[0m"
	@/usr/bin/mkdir -p $(dir $@)
	@$(COMPILE) -c $*.cpp -o $@ -MMD -MP $(CXXFLAGS) $(INCLUDE_FLAGS)
	@/usr/bin/mkdir -p $(dir $(BUILD_DIR)/.depend/$*)
	@/usr/bin/mv $(WORK_DIR)/$*.d $(BUILD_DIR)/.depend/$*_$(config).d

$(WORK_DIR)/$(SOURCE_DIR)/$(PCHCPP).o:

$(BUILD_CONFIGURATIONS):	$(TARGET_NAME)

libs:
	@make -C third_party

help:
	@/usr/bin/echo '"$(TARGET)" makefile'
	@/usr/bin/echo "Possible build configurations: $(BUILD_CONFIGURATIONS)"
	@/usr/bin/echo "Additional targets: libs docs vars help clean clean_config"

vars:
	@/usr/bin/echo '"TARGET": $(TARGET)'
	@/usr/bin/echo '"config": $(config)'
	@/usr/bin/echo '"BUILD_DIR": $(BUILD_DIR)'
	@/usr/bin/echo '"WORK_DIR": $(WORK_DIR)'
	@/usr/bin/echo '"DOCS_DIR": $(DOCS_DIR)'
	@/usr/bin/echo '"PCH": $(PCH)'
	@/usr/bin/echo '"LOCAL_INCLUDES": $(LOCAL_INCLUDES)'
	@/usr/bin/echo '"LOCAL_LIBS": $(LOCAL_LIBS)'
	@/usr/bin/echo '"CXXFLAGS": $(CXXFLAGS)'
	@/usr/bin/echo '"SOURCES": $(SOURCES)'
	@/usr/bin/echo '"OBJECTS": $(OBJECTS)'
	@/usr/bin/echo '"DEPS": $(DEPS)'

docs: $(SOURCES) $(HEADERS)
	@/bin/echo -e "\e[1;94mBuilding docs...\e[0m"
	@doxygen $(DOXYGEN_CONFIG)
	cd $(DOCS_DIR)/latex && make;
	@mv $(DOCS_DIR)/latex/refman.pdf $(DOCS_DIR)/doc.pdf

clean:
	@/usr/bin/echo 'Cleaning all...'
	/usr/bin/rm -rf $(BUILD_DIR)
	/usr/bin/rm -rf $(BIN_DIR)/*
	/usr/bin/rm -rf $(DOCS_DIR)/html
	/usr/bin/rm -rf $(DOCS_DIR)/latex

clean_config:
	@/usr/bin/echo 'Cleaning $(WORK_DIR)...'
	/usr/bin/rm -rf $(WORK_DIR)
	/usr/bin/rm -f $(TARGET_NAME)
	/usr/bin/rm -rf $(DOCS_DIR)/html
	/usr/bin/rm -rf $(DOCS_DIR)/latex

-include $(DEPS)
