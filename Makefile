# A front end for the ninja build. configure.py holds the compiler flags for each unit,
# so this only runs it and ninja; it does not compile anything itself.
#
#   make                build main.dol and check it against the original (sha1)
#   make dol            build main.dol only
#   make progress       build and print the progress report
#   make unit U=camera/TCamera [F=draw__7TCameraFv]
#                       compare one unit (or one function) with the DOL
#   make configure      regenerate build.ninja
#   make format         run format-files.sh over src/ and include/
#   make format-check   list files that clang-format would change
#   make clean          remove the build outputs (keeps the downloaded tools)

VERSION ?= GEZE8P
PYTHON  ?= python
NINJA   ?= ninja

BUILD_DIR := build/$(VERSION)
DOL       := $(BUILD_DIR)/main.dol
DTK       := build/tools/dtk

# On Windows, a make without a shell runs dtk through CreateProcess, which can fail on a / path
ifneq ($(findstring MINGW,$(shell uname)),)
  DTK := build\tools\dtk.exe
endif
ifneq ($(findstring MSYS,$(shell uname)),)
  DTK := build\tools\dtk.exe
endif

.PHONY: all check dol progress unit configure format format-check clean

all: check

build.ninja: configure.py tools/project.py
	$(PYTHON) configure.py

configure:
	$(PYTHON) configure.py

dol: build.ninja
	$(NINJA) $(DOL)

check: dol
	$(DTK) shasum -c config/$(VERSION)/build.sha1

progress: build.ninja
	$(NINJA) progress

unit: build.ninja
ifeq ($(U),)
	$(error give the unit as U=dir/Name, e.g. make unit U=camera/TCamera)
endif
ifeq ($(F),)
	$(PYTHON) tools/check.py $(U)
else
	$(PYTHON) tools/check.py $(U) -d $(F)
endif

format:
	sh format-files.sh

format-check:
	sh format-files.sh --check

clean:
	-$(NINJA) -t clean
