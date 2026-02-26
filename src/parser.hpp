#pragma once

#include "arena.hpp"
#include "tokenization.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <variant>
#include <vector>


struct NodeBinExpr;

struct NodeTermIntLit {
				Token int_lit;
};

struct NodeTermIdent {
				Token ident;
};

struct NodeTerm {
				std::variant<NodeTermIntLit*, NodeTermIdent*> var;
};

struct ExprNode {
				std::variant<NodeTerm*, NodeBinExpr*> var;
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

/*
struct BinExprMul {
				ExprNode* lhs;
				ExprNode* rhs;
};
*/

struct NodeBinExpr {
				// std::variant<BinExprAdd*, BinExprMul*> var;
				BinExprAdd* add;
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

				/*
					 std::optional<NodeBinExpr*> parse_bin_expr() {
				// TODO: implment adddition fix for RHS.
				if (auto lhs = parse_expr()) {
				}
				} else {
				std::cerr << "unsupported operator" << std::endl;
				exit(EXIT_FAILURE);
				}
				return {};
				}
				*/

				std::optional<NodeTerm*> parse_term() {
								if (auto int_lit = try_consume(TokenType::int_lit)) {
												auto term_int_literal = m_allocator.alloc<NodeTermIntLit>();
												term_int_literal->int_lit = int_lit.value();
												auto term = m_allocator.alloc<NodeTerm>();
												term->var = term_int_literal;
												return term;
								}
								else if (auto iden = try_consume(TokenType::ident)) {
												auto expr_ident = m_allocator.alloc<NodeTermIdent>();
												expr_ident->ident = iden.value();
												auto term = m_allocator.alloc<NodeTerm>();
												term->var = expr_ident;
												return term;
								} else {
												return {};
								}
				}

				std::optional<ExprNode*> parse_expr() {
								if (auto term = parse_term()) {
												if (try_consume(TokenType::plus).has_value()) {
																auto bin_expr = m_allocator.alloc<NodeBinExpr>();
																auto bin_expr_add = m_allocator.alloc<BinExprAdd>();
																auto lhs_expr = m_allocator.alloc<ExprNode>();
																lhs_expr->var = term.value();
																bin_expr_add->lhs = lhs_expr;

																if (auto rhs = parse_expr()) {
																				bin_expr_add->rhs = rhs.value();
																				bin_expr->add = bin_expr_add;
																				auto expr = m_allocator.alloc<ExprNode>();
																				expr->var = bin_expr;
																				return expr;
																} else {
																				std::cout << "expected expression" << std::endl;
																				exit(EXIT_FAILURE);
																}
												} else {
																auto expr = m_allocator.alloc<ExprNode>();
																expr->var = term.value();
																return expr;
												}
								} else {
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


								else {
												return {};
								}
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

inline std::optional<Token> try_consume(TokenType type) {
				if (peek().has_value() && peek().value().type == TokenType::semi) {
								return consume();
				} else {
								return {};
				}
}


const std::vector<Token> m_tokens;
size_t m_index = 0;
ArenaAllocator m_allocator;
};
