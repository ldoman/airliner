#############################################################################
#
#   Copyright (c) 2017 Windhover Labs, L.L.C. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name Windhover Labs nor the names of its 
#    contributors may be used to endorse or promote products derived 
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
#############################################################################
 
SHELL := /bin/bash

TARGET_PATHS := $(shell find config -maxdepth 2 -mindepth 2 -type d -path 'config/shared/*' -prune -o -exec expr {} : '[^/]*/\(.*\)' \; )
TARGET_NAMES := $(shell echo ${TARGET_PATHS} )


help::
	@echo $(TARGET_PATHS)
	@echo 'Specify a target to build.  Available targets are:'
	@for name in $(TARGET_NAMES); do \
            echo $$name; \
        done
	

$(TARGET_NAMES)::
	@echo 'Updating submodules'
	git submodule update --init --recursive
	@echo 'Building '$@'.'
	@idx=1; \
	for name in $(TARGET_NAMES); do \
            if [ "$$name" == "$@" ] ; then \
                break; \
            fi; \
            ((idx++)); \
        done; \
	TARGET_PATH=$$(echo ${TARGET_PATHS} | cut -d " " -f $$idx); \
	mkdir -p build/$$TARGET_PATH; \
	(cd build/$$TARGET_PATH; cmake -DBUILDNAME:STRING=$$TARGET_PATH -G"Eclipse CDT4 - Unix Makefiles" -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE CMAKE_BUILD_TYPE=Debug ../../..; $(MAKE) --no-print-directory);
	

clean::
	rm -rf build


start:: start-typhoon-sitl


build-sitl::
	make typhoon_h480/sitl
	make setup-commander


start-typhoon-sitl:: check-env
	@echo "Launching simulations and flight software in SITL mode."
	@-(xterm -title "Gazebo Sim" -geometry 96x24+997+392 -e "cd build/typhoon_h480/sitl/host; ./start-gazebo") &
	@-(xterm -title "Airliner SITL Flight Software" -geometry 165x24+0+0 -e "cd build/typhoon_h480/sitl/target/exe; ./airliner") &
	make start-commander


setup-commander:: check-env
	cd tools/commanderjs; \
      npm install; \
      bower install;


start-commander:: check-env
	@-(xterm -title "YAMCS Server" -geometry 96x26+0+392 -e "cd /opt/yamcs; bin/yamcs-server.sh") &
	@-(xterm -title "CommanderJS Server" -geometry 96x26+0+784 -e "cd tools/commanderjs; bin/www") &
	firefox localhost:3000

check-env::
ifndef YAMCS_WORKSPACE
    $(error YAMCS_WORKSPACE is not set.  Suggested value is 'export YAMCS_WORKSPACE="<path-to-airliner>/airliner/config/typhoon_h480/sitl/commander_workspace"')
endif





