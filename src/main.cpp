#include <cctype>
#include <cstdlib>
#include <cwctype>
#include <ios>
#include <iostream>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "tokenization.hpp"
#include "parser.hpp"
#include "generation.hpp"


int main(int argc, char* argv[]) {
				if (argc < 2) {
								std::cerr << "incorrect useage" << std::endl;
								return EXIT_FAILURE;
				}
				std::cout << argv[0] << std::endl;

				std::string contents;
				{
								std::stringstream buffer;
								std::fstream file(argv[1], std::ios::in);
								buffer << file.rdbuf();
								contents = buffer.str();
				}

				Tokenizer tokenizer(std::move(contents));
				std::vector<Token> tokens = tokenizer.tokenize();
				Parser parser(std::move(tokens));
				std::optional<ProgNode> prog = parser.parse_prog();

				if (!prog.has_value()) {
								std::cerr << "Invalid Program" << std::endl;
								exit(EXIT_FAILURE);
				}

				Generator generator(prog.value());
				{
								std::fstream asm_file("out.s", std::ios::out);
								asm_file << generator.gen_prog();

				}

				// assemble and link
				system("nasm -f elf64 out.s");
				system("ld -o a.out out.o");

				return EXIT_SUCCESS;
}
