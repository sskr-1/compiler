.PHONY: all clean build test examples

all: build

build:
	@mkdir -p build
	@cd build && cmake .. && make

clean:
	@rm -rf build
	@rm -f examples/*.ll
	@echo "Cleaned build directory and generated files"

test: build
	@echo "Testing simple example..."
	@./build/compiler examples/simple.c examples/simple.ll
	@echo ""
	@echo "Testing factorial example..."
	@./build/compiler examples/factorial.c examples/factorial.ll
	@echo ""
	@echo "Testing fibonacci example..."
	@./build/compiler examples/fibonacci.c examples/fibonacci.ll
	@echo ""
	@echo "All tests completed!"

examples: build
	@echo "Compiling all examples..."
	@for file in examples/*.c; do \
		echo "Compiling $$file..."; \
		./build/compiler $$file $${file%.c}.ll; \
	done
	@echo "All examples compiled!"

help:
	@echo "Available targets:"
	@echo "  make build    - Build the compiler"
	@echo "  make clean    - Clean build artifacts"
	@echo "  make test     - Run compiler on test examples"
	@echo "  make examples - Compile all example programs"
	@echo "  make help     - Show this help message"
