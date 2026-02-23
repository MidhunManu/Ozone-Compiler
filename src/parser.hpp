#pragma once

#include "tokenization.hpp"
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

// namespace node {
struct ExprNode {
				Token int_lit;
};

struct ExitNode {
				ExprNode expr;
};

//};

class Parser {
public:
				inline explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {} 
				std::optional<ExprNode> parse_expr() {
							if (peek().has_value() && peek().value().type == TokenType::int_lit) {
											return ExprNode{.int_lit = consume()};
							} else {
											return {};
							}
				}

				/*
				std::optional<ExitNode> parse() {
								std::optional<ExitNode> exit_node;
								while(peek().has_value()) {
												consume();
												if (peek().value().type == TokenType::exit) {
																if (auto node_expr = parse_expr()) {
																				exit_node = ExitNode { .expr = node_expr.value() };				
																} else {
																				std::cerr << "Invalid Expression" << std::endl;
																				exit(EXIT_FAILURE);
																}
																if (peek().has_value() && peek().value().type != TokenType::semi) {
																				consume();
																} else {
																				std::cerr << "Invalid Expression" << std::endl;
																				exit(EXIT_FAILURE);
																}
												}
								}
								m_index = 0;
								return exit_node;
				}
				*/
				std::optional<ExitNode> parse() {

								if (!peek().has_value() || peek()->type != TokenType::exit 
								    && peek(1).value().type == TokenType::open_paren) {
												return {};
								}

								consume(); // consume 'exit'
								consume();

								auto expr = parse_expr();
								if (!expr.has_value()) {
												std::cerr << "Invalid Expression\n";
												exit(EXIT_FAILURE);
								}
								if (peek().has_value() && peek().value().type == TokenType::close_paren) {
												consume();
								} else {
												std::cerr << "close parenthesis expected" << std::endl;
												exit(EXIT_FAILURE);
								}

								if (!peek().has_value() || peek()->type != TokenType::semi) {
												std::cerr << "Expected semicolon\n";
												exit(EXIT_FAILURE);
								}

								consume(); // consume ';'

								return ExitNode{ .expr = expr.value() };
				}
private: 
				inline std::optional<Token> peek(int ahead = 0) const {
								if (m_index + ahead >= m_tokens.size()) {
												return {};
								} else {
												return m_tokens.at(m_index + ahead);
								}
				}

				inline Token consume() {
								return m_tokens.at(m_index++);
				}
				const std::vector<Token> m_tokens;
				size_t m_index = 0;
};
