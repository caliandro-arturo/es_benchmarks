flash:
	pio run --target upload

.PHONY: test
test:
	@mkdir -p Test-build; \
	cd Test; \
	if [ $$(ls -1 | wc -l) -eq 0 ]; then \
		echo "Nothing to compile."; \
		exit; \
	fi; \
	for file in *; do \
		echo Compiling Test/"$$file"... ; \
		cc -o ../Test-build/$${file%.*} "$$file" ; \
	done; \

clean:
	pio run --target clean
	rm -rf Test-build/*)
