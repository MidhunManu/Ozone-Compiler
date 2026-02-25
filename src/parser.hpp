#pragma once

#include "arena.hpp"
#include "tokenization.hpp"
#include <cstdlib>
#include <iostream>
#include <optional>
#include <variant>
#include <vector>


struct NodeBinExpr;

struct ExprNodeIntLit {
				Token int_lit;
};

struct ExprNodeIdent {
				Token ident;
};

struct ExprNode {
				std::variant<ExprNodeIntLit*, ExprNodeIdent*, NodeBinExpr*> var;
};

struct StmtNodeExit {
				ExprNode* expr;
};

struct StmtNodeLet {
				Token ident;
				ExprNode* expr;
};

struct StmtNode {
				std::variant<StmtNodeExit*, StmtNodeLet*> var;
};

struct ProgNode {
				std::vector<StmtNode*> statements;
};

struct BinExprAdd {
				ExprNode* lhs;
				ExprNode* rhs;
};

struct BinExprMul {
				ExprNode* lhs;
				ExprNode* rhs;
};

struct NodeBinExpr {
				std::variant<BinExprAdd*, BinExprMul*> var;
};


/*
struct ExitNode {
				ExprNode expr;
};
*/


class Parser {
public:
				inline explicit Parser(std::vector<Token> tokens) 
								: m_tokens(std::move(tokens)),
								m_allocator(1024 * 1024 * 4)
								{} 


				std::optional<ExprNode*> parse_expr() {
							if (peek().has_value() && peek().value().type == TokenType::int_lit) {
											auto expr_lit = m_allocator.alloc<ExprNodeIntLit>();
											expr_lit->int_lit = consume();
											auto expr = m_allocator.alloc<ExprNode>();
											expr->var = expr_lit;
											return expr;
							}
							else if (peek().has_value() && peek().value().type == TokenType::ident) {
											auto expr_ident = m_allocator.alloc<ExprNodeIdent>();
											expr_ident->ident = consume();
											auto expr = m_allocator.alloc<ExprNode>();
											expr->var = expr_ident;
											return expr;
							}
							else {
											return {};
							}
				}

				std::optional<StmtNode*> parse_stmt() {
								if (peek().has_value() && peek()->type == TokenType::exit &&
																peek(1).has_value() && peek(1)->type == TokenType::open_paren) {

												consume(); // exit
												consume(); // (

												auto* stmt_exit = m_allocator.alloc<StmtNodeExit>();

												if (auto expr = parse_expr()) {
																stmt_exit->expr = *expr;
												} else {
																std::cerr << "Invalid Expression\n";
																std::exit(EXIT_FAILURE);
												}

												if (!peek().has_value() || peek()->type != TokenType::close_paren) {
																std::cerr << "expected )\n";
																std::exit(EXIT_FAILURE);
												}
												consume();

												if (!peek().has_value() || peek()->type != TokenType::semi) {
																std::cerr << "expected ;\n";
																std::exit(EXIT_FAILURE);
												}
												consume();

												auto* stmt = m_allocator.alloc<StmtNode>();
												stmt->var = stmt_exit;
												return stmt;
								}

								else if (peek().has_value() && peek()->type == TokenType::let &&
																peek(1).has_value() && peek(1)->type == TokenType::ident &&
																peek(2).has_value() && peek(2)->type == TokenType::eq) {

												consume(); // let

												auto* stmt_let = m_allocator.alloc<StmtNodeLet>();
												stmt_let->ident = consume();

												consume(); // =

												if (auto expr = parse_expr()) {
																stmt_let->expr = *expr;
												} else {
																std::cerr << "Invalid Expression\n";
																std::exit(EXIT_FAILURE);
												}

												if (!peek().has_value() || peek()->type != TokenType::semi) {
																std::cerr << "Expected ;\n";
																std::exit(EXIT_FAILURE);
												}
												consume();

												auto* stmt = m_allocator.alloc<StmtNode>();
												stmt->var = stmt_let;
												return stmt;
								}

								return {};
				}
				std::optional<ProgNode> parse_prog() {
								ProgNode prog;
								while (peek().has_value()) {
												if (auto stmt = parse_stmt()) {
																prog.statements.push_back(*stmt);
												} else {
																std::cerr << "Invalid Statement\n";
																std::exit(EXIT_FAILURE);
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
				ArenaAllocator m_allocator;
};
