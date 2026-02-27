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
struct NodeTermParen;

struct NodeTermIntLit {
				Token int_lit;
};

struct NodeTermIdent {
				Token ident;
};

struct NodeTerm {
				std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> var;
};

struct ExprNode {
				std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeTermParen {
				ExprNode* expr;
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

struct BinExprSub {
				ExprNode* lhs;
				ExprNode* rhs;
};

struct BinExprDiv {
				ExprNode* lhs;
				ExprNode* rhs;
};

struct NodeBinExpr {
				std::variant<BinExprAdd*, BinExprMul*, BinExprSub*, BinExprDiv*> var;
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
								} 
								else if (auto open_paren = try_consume(TokenType::open_paren)) {
												auto expr = parse_expr();
												if (!expr.value()) {
																std::cerr << "expected expression" << std::endl;
																exit(EXIT_FAILURE);
												}

												try_consume(TokenType::close_paren);
												auto term_expr = m_allocator.alloc<NodeTermParen>();
												term_expr->expr = expr.value();
												auto term = m_allocator.alloc<NodeTerm>();
												term->var = term_expr;
												return term;
								}
								else {
												return {};
								}
				}

				std::optional<ExprNode*> parse_expr(int min_prec = 0) {
								std::optional<NodeTerm*> term_lhs = parse_term();
								if(!term_lhs.has_value()) {
												return {};
								}


								auto expr_lhs = m_allocator.alloc<ExprNode>();
								expr_lhs->var = term_lhs.value();

								/*
								while(true) {
												std::optional<Token> current_tok = peek();
												std::optional<int> prec;
												if (!current_tok.has_value()) {
																prec = bin_prec(current_tok->type);
																if (!prec.has_value() || prec < min_prec) {
																				break;
																}
												} else {
																break;
												}

												int next_min_prec = prec.value() + 1;
												auto expr_rhs = parse_expr(next_min_prec);

												if (!expr_rhs.value()) {
																std::cerr << "Can't parse expression, absent RHS expr" << std::endl;
																exit(EXIT_FAILURE);
												}

												auto expr = m_allocator.alloc<NodeBinExpr>();
												

												Token op = consume();


												if (op.type == TokenType::plus) {
																auto add = m_allocator.alloc<BinExprAdd>();
																add->lhs = expr_lhs;
																add->rhs = expr_rhs.value();
																expr->var = add;
												}
												else if (op.type == TokenType::mul) {
																auto mul = m_allocator.alloc<BinExprMul>();
																mul->lhs = expr_lhs;
																mul->rhs = expr_rhs.value();
																expr->var = mul;
												}

												expr_lhs->var = expr;
								}
								*/

								while (true) {
												std::optional<Token> current_tok = peek();
												if (!current_tok.has_value()) break;

												std::optional<int> prec = bin_prec(current_tok->type);
												if (!prec.has_value() || prec.value() < min_prec) break;

												Token op = consume();
												int next_min_prec = prec.value() + 1;
												auto expr_rhs = parse_expr(next_min_prec);

												if (!expr_rhs.has_value()) {
																std::cerr << "Can't parse expression, absent RHS expr" << std::endl;
																exit(EXIT_FAILURE);
												}

												auto bin_expr = m_allocator.alloc<NodeBinExpr>();

												auto lhs_copy = m_allocator.alloc<ExprNode>();
												lhs_copy->var = expr_lhs->var;

												if (op.type == TokenType::plus) {
																auto add = m_allocator.alloc<BinExprAdd>();
																add->lhs = lhs_copy;
																add->rhs = expr_rhs.value();
																bin_expr->var = add;
												} 
												else if (op.type == TokenType::mul) {
																auto mul = m_allocator.alloc<BinExprMul>();
																mul->lhs = lhs_copy;
																mul->rhs = expr_rhs.value();
																bin_expr->var = mul;
												}
												else if (op.type == TokenType::sub) {
																auto sub = m_allocator.alloc<BinExprSub>();
																sub->lhs = lhs_copy;
																sub->rhs = expr_rhs.value();
																bin_expr->var = sub;
												}
												else if (op.type == TokenType::div) {
																auto div = m_allocator.alloc<BinExprDiv>();
																div->lhs = lhs_copy;
																div->rhs = expr_rhs.value();
																bin_expr->var = div;
												}

												expr_lhs->var = bin_expr;
								}
								return expr_lhs;

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
				if (peek().has_value() && peek().value().type == type) {
								return consume();
				} else {
								return {};
				}
}


const std::vector<Token> m_tokens;
size_t m_index = 0;
ArenaAllocator m_allocator;
};
