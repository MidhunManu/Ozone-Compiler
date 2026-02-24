#pragma once

#include "parser.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <variant>

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
																offset << "QWORD [rsp + " << (gen->m_stack_size - var.stack_location - 1) * 8 << "]\n";
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
																const auto name = stmt_let.ident.value.value();
																if(gen->m_vars.contains(name)) {
																				std::cerr << "Identifier already used : "
																								  << stmt_let.ident.value.value()
																									<< std::endl;

																				exit(EXIT_FAILURE);
																}

																gen->gen_expr(stmt_let.expr);
																gen->m_vars.insert({
																								name,
																								Var { .stack_location = gen->m_stack_size - 1 }
																});
																/*
																gen->m_vars.insert({
																												stmt_let.ident.value.value(),
																												Var { .stack_location = gen->m_stack_size }
																								});

																gen->gen_expr(stmt_let.expr);
																*/
												}

												void operator()(const StmtNodePrint& stmt_print) const {
																gen->gen_expr(stmt_print.expr);
																gen->gen_expr(stmt_print.expr);
																gen->pop("rdi");
																gen->m_output << "    call print_u64\n";

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


								m_output << run_time_print();
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

				static std::string run_time_print() {
								std::stringstream ss;

								ss << "    default rel\n";
								ss << "    section .data\n";
								ss << "    print_buf: times 32 db 0\n";
								ss << "\n";
								ss << "    section .text\n";
								ss << "    print_u64:\n";
								ss << "        lea rsi, [print_buf + 31]\n";
								ss << "        xor rcx, rcx\n";
								ss << "\n";
								ss << "        mov rax, rdi\n";
								ss << "        cmp rax, 0\n";
								ss << "        jne .loop\n";
								ss << "\n";
								ss << "        mov byte [rsi], '0'\n";
								ss << "        mov rcx, 1\n";
								ss << "        jmp .write\n";
								ss << "\n";
								ss << "    .loop:\n";
								ss << "        xor rdx, rdx\n";
								ss << "        mov rbx, 10\n";
								ss << "        div rbx\n";
								ss << "        add dl, '0'\n";
								ss << "        mov [rsi], dl\n";
								ss << "        dec rsi\n";
								ss << "        inc rcx\n";
								ss << "        test rax, rax\n";
								ss << "        jne .loop\n";
								ss << "\n";
								ss << "        inc rsi\n";
								ss << "\n";
								ss << "    .write:\n";
								ss << "        mov rax, 1\n";
								ss << "        mov rdi, 1\n";
								ss << "        mov rdx, rcx\n";
								ss << "        syscall\n";
								ss << "        ret\n";

								return ss.str();
				}
};

