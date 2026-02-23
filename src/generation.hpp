#pragma once

#include "parser.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <variant>
#include <algorithm>

class Generator {
public:
				inline explicit Generator(ProgNode prog): m_prog(std::move(prog)) {}

				void gen_expr(const ExprNode& expr) {
								struct ExprVisitor {
												Generator* gen;
												
												void operator() (const ExprNodeIntLit& expr_int_lit) const {
																gen->m_output << "    mov rax, " << expr_int_lit.int_lit.value.value() << "\n";
																gen->push("rax");
												}

												void operator() (const ExprNodeIdent& expr_indent) {
																if(!gen->m_vars.contains(expr_indent.ident.value.value())) {
																				std::cerr << "Undeclared Identifier `" << expr_indent.ident.value.value() << "`\n";
																				exit(EXIT_FAILURE);
																}
																const auto& var = gen->m_vars.at(expr_indent.ident.value.value());
																std::stringstream offset;
																offset << "QWORD [rsp + " << (gen->m_stack_size - var.stack_location - 1) * 4 << "]\n";
																gen->push(offset.str());
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
																
																gen->pop("rdi");
																gen->m_output << "    syscall\n";
												}

												void operator()(const StmtNodeLet& stmt_let) {
																// not an error, cuz linter is at c++17 tho cmake is at c++20
																if(gen->m_vars.contains(stmt_let.ident.value.value())) {
																				std::cerr << "Identifier already used : "
																								  << stmt_let.ident.value.value()
																									<< std::endl;

																				exit(EXIT_FAILURE);
																}

																gen->m_vars.insert({
																												stmt_let.ident.value.value(),
																												Var { .stack_location = gen->m_stack_size }
																								});

																gen->gen_expr(stmt_let.expr);
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
				inline void push(const std::string& reg) {
								m_output << "    push " << reg << "\n";
								m_stack_size++;
				}

				inline void pop(const std::string& reg) {
								m_output << "    pop " << reg << "\n";
								m_stack_size--;
				}

				struct Var {
								size_t stack_location;
				};

				const ProgNode m_prog;
				std::stringstream m_output;
				size_t m_stack_size = 0;
				std::unordered_map<std::string, Var> m_vars {};
};

