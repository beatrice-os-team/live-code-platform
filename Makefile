.PHONY : all help run

NODE := node

all: help

help:
	@echo "Makefile"
	@echo "Usage:"
	@echo "  make run    - Run the project"
	@echo "  make help   - Show this help message"

INDEX := index.js

run:
	@$(NODE) $(INDEX)
