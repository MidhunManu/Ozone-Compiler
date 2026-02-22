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


std::string tokens_to_asm(const std::vector<Token>& tokens) {
				std::stringstream output;
				output << "global _start\n_start:\n";
				for(int i = 0; i < tokens.size(); i++) {
								const Token& token = tokens.at(i);
								if(token.type == TokenType::exit)  {
												if (i + 1 < tokens.size() && tokens.at(i + 1).type == TokenType::int_lit) {
																if (i + 2 < tokens.size() && tokens.at(i + 2).type == TokenType::semi) {
																				output << "    mov rax, 60\n";
																				output << "    mov rdi, " << tokens.at(i + 1).value.value() << "\n";
																				output << "    syscall\n";
																}
												}
								}
				}
				return output.str();
}

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
				{
								std::fstream asm_file("out.s", std::ios::out);
								asm_file << tokens_to_asm(tokenizer.tokenize());

				}

				return EXIT_SUCCESS;
}
