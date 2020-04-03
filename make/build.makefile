include $(STUB)

OUTPUT = $(TMP_PREFIX)/$(PROGRAM_NAME)



LD = gcc

OBJ != printf 'src/%s.o ' $(PROGRAM_OBJ)

all: $(OUTPUT)
	@echo built $(OUTPUT)
	@echo

$(OUTPUT): $(OBJ)
	@mkdir -p $(@D)
	@echo objects: $(OBJ)
	$(LD) -o $(@) $(OBJ) $(LDLIBS)



include make/depend.makefile
