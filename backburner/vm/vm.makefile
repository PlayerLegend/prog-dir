test/assembler: src/vm/assembler.test.o src/vm/assembler.o src/paren-parser/paren-preprocessor.o src/paren-parser/paren-parser.o src/log/log.o src/immutable/immutable.o src/buffer_io/buffer_io.o src/vm/machine-code.o src/vm/text.o
test/vm: src/vm/vm.test.o src/vm/vm.o src/log/log.o src/vm/machine-code.o src/vm/text.o

#TESTS_C += test/assembler test/vm
