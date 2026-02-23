#pragma once

#include "tokenization.hpp"
#include <cstdlib>
#include <iostream>
#include <optional>
#include <variant>
#include <vector>

struct ExprNodeIntLit {
				Token int_lit;
};

struct ExprNodeIdent {
				Token ident;
};

struct ExprNode {
				std::variant<ExprNodeIntLit, ExprNodeIdent> var;
};

struct StmtNodeExit {
				ExprNode expr;
};

struct StmtNodeLet {
				Token ident;
				ExprNode expr;
};

struct StmtNode {
				std::variant<StmtNodeExit, StmtNodeLet> var;
};

struct ProgNode {
				std::vector<StmtNode> statements;
};

/*
struct ExitNode {
				ExprNode expr;
};
*/


class Parser {
public:
				inline explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {} 
				std::optional<ExprNode> parse_expr() {
							if (peek().has_value() && peek().value().type == TokenType::int_lit) {
											return ExprNode{.var = ExprNodeIntLit{ .int_lit = consume() } };
							} 
							else if (peek().has_value() && peek().value().type == TokenType::ident) {
											return ExprNode{.var = ExprNodeIdent{ .ident = consume() } };
							}
							else {
											return {};
							}
				}

				std::optional<StmtNode> parse_stmt() {
								if (peek().has_value() && peek()->type == TokenType::exit &&
																peek(1).has_value() && peek(1)->type == TokenType::open_paren) {

												consume(); // exit
												consume(); // (

												StmtNodeExit stmt_exit;
												if (auto node_expr = parse_expr()) {
																stmt_exit.expr = node_expr.value();
												} else {
																std::cerr << "Invalid Expression" << std::endl;
																exit(EXIT_FAILURE);
												}

												if (peek().has_value() && peek()->type == TokenType::close_paren) {
																consume();
												} else {
																std::cerr << "expected )" << std::endl;
																exit(EXIT_FAILURE);
												}

												if (peek().has_value() && peek()->type == TokenType::semi) {
																consume();
												} else {
																std::cerr << "expected ;" << std::endl;
																exit(EXIT_FAILURE);
												}

												return StmtNode{ .var = stmt_exit };
								}
								else if(peek().has_value() && peek().value().type == TokenType::let
																&& peek(1).has_value() && peek(1).value().type == TokenType::ident
																&& peek(2).has_value() && peek(2).value().type == TokenType::eq) {
												consume();
												auto stmt_let = StmtNodeLet { .ident = consume() };
												consume();
												if (auto expr = parse_expr()) {
																stmt_let.expr = expr.value();
												} else {
																std::cerr << "Invalid Expression" << std::endl;
																exit(EXIT_FAILURE);
												}
												if (peek().has_value() && peek().value().type == TokenType::semi) {
																consume();
												} else {
																std::cerr << "Expected ;" << std::endl;
																exit(EXIT_FAILURE);
												}

												return StmtNode{ .var = stmt_let };
								} else {
												return {};
								}
				}

				std::optional<ProgNode> parse_prog() {
								ProgNode prog;
								while(peek().has_value()) {
												if (auto stml = parse_stmt()) {
																prog.statements.push_back(stml.value());
												} else {
																std::cerr << "Invalid Statement" << std::endl;
																exit(EXIT_FAILURE);

												}
								}
								return prog;
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
