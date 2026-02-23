#pragma once

#include "parser.hpp"
#include <string>
#include <sstream>
#include <variant>

class Generator {
public:
				inline explicit Generator(ProgNode prog): m_prog(std::move(prog)) {}

				void gen_expr(const ExprNode& expr) {
								struct ExprVisitor {
												Generator* gen;
												
												void operator() (const ExprNodeIntLit& expr_int_lit) const {
																gen->m_output << "    mov rax, " << expr_int_lit.int_lit.value.value() << "\n";
																gen->m_output << "    push rax\n";
												}

												void operator() (const ExprNodeIdent& expr_indent) const {
																// TODO
												}
								};

								ExprVisitor visitor({ .gen = this });
								std::visit(visitor, expr.var);
				}

				void gen_stmt(const StmtNode& stmt) {
								struct StmtVisitor {
												Generator* gen;
												void operator()(const StmtNodeExit& stmt_exit) const {
																gen->gen_expr(stmt_exit.expr);
																gen->m_output << "    mov rax, 60\n";
																
																gen->m_output << "    pop rdi\n";
																gen->m_output << "    syscall\n";
												}

												void operator()(const StmtNodeLet& stmt_let) {

												}
								};

								/*
								StmtVisitor visitor({ .gen = this });
								std::visit(visitor, stmt.var);
								*/
								StmtVisitor visitor{};
								visitor.gen = this;
								std::visit(visitor, stmt.var);
				}

				[[nodiscard]] inline std::string gen_prog() {
								m_output << "global _start\n_start:\n";
								
								for(const StmtNode& stmt: m_prog.statements) {
												gen_stmt(stmt);	
								}

								m_output << "    mov rax, 60\n";
								m_output << "    mov rdi, 0\n"; 
								m_output << "    syscall\n";


								return m_output.str();
				}

private:
				const ProgNode m_prog;
				std::stringstream m_output;
};

