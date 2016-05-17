
PACKAGE_DIR = packages
INSTALL_DIR = $(CURDIR)

CMAKE_LIBS = glm gli glfw freetype2 libpng tinyobjloader zlib
MAKE_LIBS = libtga
AUTOGEN_LIBS = harfbuzz
CUSTOM_LIBS = glew libjpeg-turbo imgui

define gen-lib-target
all:: $1
update-$1:
	@/bin/echo -e "\e[1;94mPulling \e[1;92m$1\e[1;94m...\e[0m"
	@cd $(PACKAGE_DIR)/$1; git pull || true; git submodule update || true
update:: update-$1
endef

define gen-cmake-target
$1:
	@/bin/echo -e "\e[1;94mBuilding\e[1;92m $1\e[1;94m...\e[0m"
	@mkdir $(PACKAGE_DIR)/$1/build 2> /dev/null || true; cd $(PACKAGE_DIR)/$1/build; cmake -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) ..
	@make -C $(PACKAGE_DIR)/$1/build
	@make -C $(PACKAGE_DIR)/$1/build install

$(eval $(call gen-lib-target,$1))
clean::
	rm -rf $(PACKAGE_DIR)/$1/CMakeCache.txt $(PACKAGE_DIR)/$1/CMakeFiles/ $(PACKAGE_DIR)/$1/build
endef

define gen-make-target
$1:
	@/bin/echo -e "\e[1;94mBuilding\e[1;92m $1\e[1;94m...\e[0m"
	cd $(PACKAGE_DIR)/$1; ./configure --prefix=$(INSTALL_DIR)
	@make -C $(PACKAGE_DIR)/$1
	@make -C $(PACKAGE_DIR)/$1 install

$(eval $(call gen-lib-target,$1))
clean::
	@make -C $(PACKAGE_DIR)/$1 clean 2> /dev/null; true
endef

define gen-autogen-target
$1:
	@/bin/echo -e "\e[1;94mBuilding\e[1;92m $1\e[1;94m...\e[0m"
	@cd $(PACKAGE_DIR)/$1; ./autogen.sh --prefix=$(INSTALL_DIR)
	@make -C $(PACKAGE_DIR)/$1
	@make -C $(PACKAGE_DIR)/$1 install

$(eval $(call gen-lib-target,$1))
clean::
	@make -C $(PACKAGE_DIR)/$1 clean
endef

$(foreach l,$(CMAKE_LIBS),$(eval $(call gen-cmake-target,$(l))))
$(foreach l,$(MAKE_LIBS),$(eval $(call gen-make-target,$(l))))
$(foreach l,$(AUTOGEN_LIBS),$(eval $(call gen-autogen-target,$(l))))
$(foreach l,$(CUSTOM_LIBS),$(eval $(call gen-lib-target,$(l))))

glew:
	@/bin/echo -e "\e[1;94mBuilding\e[1;92m $@\e[1;94m...\e[0m"
	@make -C $(PACKAGE_DIR)/$@ extensions GLEW_DEST=$(INSTALL_DIR)
	@make -C $(PACKAGE_DIR)/$@ glew.bin GLEW_DEST=$(INSTALL_DIR)
	@make -C $(PACKAGE_DIR)/$@ install GLEW_DEST=$(INSTALL_DIR)
clean::
	@make -C $(PACKAGE_DIR)/glew clean

libjpeg-turbo:
	@/bin/echo -e "\e[1;94mBuilding\e[1;92m $@\e[1;94m...\e[0m"
	@mkdir $(PACKAGE_DIR)/$@/build 2> /dev/null || true
	@cd $(PACKAGE_DIR)/$@; autoreconf -fiv
	@cd $(PACKAGE_DIR)/$@/build; ../configure --prefix=$(INSTALL_DIR) CFLAGS='-O3'
	@make -C $(PACKAGE_DIR)/$@/build
	@make -C $(PACKAGE_DIR)/$@/build install
clean::
	rm -rf $(PACKAGE_DIR)/libjpeg-turbo/build

imgui:
	@/bin/echo -e "\e[1;94mBuilding\e[1;92m $@\e[1;94m...\e[0m"
	cd include; mkdir imgui 2> /dev/null || true && cp ../$(PACKAGE_DIR)/$@/*.cpp ./imgui/ && cp ../$(PACKAGE_DIR)/$@/*.h ./imgui/

clean::
	rm -rf include lib lib64 bin

init:
	rm -rf $(PACKAGE_DIR)/*
	@cd $(PACKAGE_DIR)/; git clone --single-branch --branch master http://git.savannah.gnu.org/r/freetype/freetype2.git
	@cd $(PACKAGE_DIR)/; git clone --single-branch --branch master https://github.com/madler/zlib.git
	@cd $(PACKAGE_DIR)/; git clone --single-branch --branch master https://github.com/nigels-com/glew.git
	@cd $(PACKAGE_DIR)/; git clone --single-branch --branch latest https://github.com/glfw/glfw.git
	@cd $(PACKAGE_DIR)/; git clone --single-branch --branch 1.4.x https://github.com/libjpeg-turbo/libjpeg-turbo.git
	@cd $(PACKAGE_DIR)/; git clone --single-branch --branch libpng16 https://github.com/glennrp/libpng.git
	@cd $(PACKAGE_DIR)/; git clone --single-branch --branch v0.9.23 https://github.com/syoyo/tinyobjloader.git
	@cd $(PACKAGE_DIR)/; git clone --single-branch --branch master https://github.com/ocornut/imgui.git
	@cd $(PACKAGE_DIR)/; wget -O libtga.tar.gz 'http://downloads.sourceforge.net/project/tgalib/libtga/1.0.1/libtga-1.0.1.tar.gz?r=https%3A%2F%2Fsourceforge.net%2Fprojects%2Ftgalib%2F&ts=1463234964&use_mirror=netix' && tar xvpf libtga.tar.gz && mv libtga-1.0.1 libtga && rm libtga.tar.gz
	@cd $(PACKAGE_DIR)/; git clone --single-branch --branch 1.2.7 https://github.com/behdad/harfbuzz
	@cd $(PACKAGE_DIR)/; git clone --single-branch --branch master https://github.com/g-truc/glm.git
	@cd $(PACKAGE_DIR)/; git clone --single-branch --branch master https://github.com/g-truc/gli.git

.PHONY: $(CMAKE_LIBS)
.PHONY: $(MAKE_LIBS)
.PHONY: $(AUTOGEN_LIBS)
.PHONY: $(CUSTOM_LIBS)
