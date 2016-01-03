
all:
	@echo "Read the instructions in the INSTALL.txt file"

.PHONY: clean
clean:
	@find . -name "*~" -exec rm \{} \;
	@rm -f ezdebt

.PHONY: bin
bin: 
	g++ -o ezdebt -Wall -Isrc -DezdebtDATA_DIR=\".\" src/ezdebt.cc


.PHONY: install
install: bin
	cp ezdebt ezdebt.css $(WORKSPACE)
