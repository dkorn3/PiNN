.PHONY: all build smoke-test clean

all: build

build:
	@./scripts/build.sh

smoke-test: build
	@./scripts/run-smoke-test.sh

clean:
	@make -C third_party/CmdParameter clean
	@make -C third_party/V3DLib clean
