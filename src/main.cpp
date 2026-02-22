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

enum class TokenType {
				_return,
				int_lit,
				semi
};

struct Token {
				TokenType type;
				std::optional<std::string> value;
};

std::vector<Token> tokenize(const std::string& str) {
				std::vector<Token> tokens {};
				std::string buffer;
				for(int i = 0; i < str.size(); i++) {
								char c = str.at(i);

								if (std::isalpha(c)) {
												buffer.push_back(c);
												i++;
												while(std::isalnum(str.at(i))) {
																buffer.push_back(str.at(i));
																i++;
												}
												i--;

												if (buffer == "return") {
																tokens.push_back({.type = TokenType::_return});
																buffer.clear();
																continue;
												} else {
																std::cerr << "error" << std::endl;
																exit(EXIT_FAILURE);
												}
								}
								

								else if (std::isdigit(c)) {
												buffer.push_back(c);
												i++;
												while(std::isdigit(str.at(i))) {
																buffer.push_back(str.at(i));
																i++;
												}
												i--;
												tokens.push_back({.type = TokenType::int_lit, .value = buffer});
												buffer.clear();
								}
								else if (c == ';') {
												tokens.push_back({.type = TokenType::semi});
								}
								else if (std::isspace(c)) {
												continue;
								}
								else {
												std::cerr << "error" << std::endl;
								}
				}
				return tokens;
}

std::string tokens_to_asm(const std::vector<Token>& tokens) {
				std::stringstream output;
				output << "global _start\n_start:\n";
				for(int i = 0; i < tokens.size(); i++) {
								const Token& token = tokens.at(i);
								if(token.type == TokenType::_return)  {
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

				{
								std::fstream asm_file("out.s", std::ios::out);
								asm_file << tokens_to_asm(tokenize(contents));

				}

				return EXIT_SUCCESS;
}
