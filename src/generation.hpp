#pragma once

#include "parser.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <variant>

class Generator {
public:
				inline explicit Generator(ProgNode prog): m_prog(std::move(prog)) {}

				void genTerm(const NodeTerm* term) {
								struct TermVisitor {
												Generator* gen;
												void operator() (const NodeTermIntLit* term_int_lit) const {
																gen->m_output << "    mov rax, " << term_int_lit->int_lit.value.value() << "\n";
																gen->push("rax");

												}

												void operator() (const NodeTermIdent* term_int_inden) const {
																if(!gen->m_vars.contains(term_int_inden->ident.value.value())) {
																				std::cerr << "Undeclared Identifier `" << term_int_inden->ident.value.value() << "`\n";
																				exit(EXIT_FAILURE);
																}
																const auto& var = gen->m_vars.at(term_int_inden->ident.value.value());
																std::stringstream offset;
																offset << "QWORD [rsp + " << (gen->m_stack_size - var.stack_location - 1) * 8 << "]\n";
																gen->push(offset.str());

												}

												void operator() (const NodeTermParen* term_paren) const {
																gen->gen_expr(term_paren->expr);
												}
								};

								TermVisitor visitor({.gen = this});
								std::visit(visitor, term->var);
				}

				void gen_bin_expr(const NodeBinExpr* bin_expr) {
								struct BinExprAddVisitor {
												Generator* gen;
												void operator() (BinExprAdd* add) const {
																gen->gen_expr(add->rhs);
																gen->gen_expr(add->lhs);
																gen->pop("rax");
																gen->pop("rbx");
																gen->m_output << "    add rax, rbx\n";
																gen->push("rax");
												}

												void operator() (BinExprMul* mul) const {
																gen->gen_expr(mul->rhs);
																gen->gen_expr(mul->lhs);
																gen->pop("rax");
																gen->pop("rbx");
																gen->m_output << "    mul rbx\n";
																gen->push("rax");
												}

												void operator() (BinExprSub* sub) const {
																gen->gen_expr(sub->rhs);
																gen->gen_expr(sub->lhs);
																gen->pop("rax");
																gen->pop("rbx");
																gen->m_output << "    sub rax, rbx\n";
																gen->push("rax");
												}

												void operator() (BinExprDiv* div) const {
																gen->gen_expr(div->rhs);
																gen->gen_expr(div->lhs);
																gen->pop("rax");
																gen->pop("rbx");
																gen->m_output << "    div rbx\n";
																gen->push("rax");

												}
								};

								BinExprAddVisitor visitor {.gen = this};
								std::visit(visitor, bin_expr->var);
				}

				void gen_expr(const ExprNode* expr) {
								struct ExprVisitor {
												Generator* gen;
												
												void operator() (const NodeTerm* term) const {
																gen->genTerm(term);
												}

												void operator() (const NodeBinExpr* bin_expr) const {
																gen->gen_bin_expr(bin_expr);
																
												}
								};

								ExprVisitor visitor({ .gen = this });
								std::visit(visitor, expr->var);
				}

				void gen_stmt(const StmtNode* stmt) {
								struct StmtVisitor {
												Generator* gen;
												void operator()(const StmtNodeExit* stmt_exit) const {
																gen->gen_expr(stmt_exit->expr);
																gen->m_output << "    mov rax, 60\n";
																
																gen->pop("rdi");
																gen->m_output << "    syscall\n";
												}

												void operator()(const StmtNodeLet* stmt_let) {
																// not an error, cuz linter is at c++17 tho cmake is at c++20
																if(gen->m_vars.contains(stmt_let->ident.value.value())) {
																				std::cerr << "Identifier already used : "
																								  << stmt_let->ident.value.value()
																									<< std::endl;

																				exit(EXIT_FAILURE);
																}

																gen->m_vars.insert({
																												stmt_let->ident.value.value(),
																												Var { .stack_location = gen->m_stack_size }
																								});

																gen->gen_expr(stmt_let->expr);
												}

												void operator() (const StmtNodeStdOut* stmt_std_out) {
																/*
																gen->gen_expr(stmt_std_out->expr);
																gen->m_output << "    mov rax, 60\n";

																gen->pop("rdi");
																gen->m_output << "    syscall\n";
																*/

																gen->gen_expr(stmt_std_out->expr);
																gen->pop("rdi");
																gen->m_output << "    call print_u64\n";
																// missing! add these:
																gen->m_output << "    mov rax, 1\n";
																gen->m_output << "    mov rdi, 1\n";
																gen->m_output << "    lea rsi, [newline]\n";
																gen->m_output << "    mov rdx, 1\n";
																gen->m_output << "    syscall\n";
																gen->m_uses_print = true;
												}
								};

								StmtVisitor visitor{};
								visitor.gen = this;
								std::visit(visitor, stmt->var);
				}

				[[nodiscard]] inline std::string gen_prog() {
								/*
								if (m_uses_print) {
												m_output << run_time_print();
								}

								m_output << "global _start\n_start:\n";

								for(auto stmt: m_prog.statements) {
												gen_stmt(stmt);	
								}

								m_output << "    mov rax, 60\n";
								m_output << "    mov rdi, 0\n"; 
								m_output << "    syscall\n";


								return m_output.str();
								*/
								m_output << "global _start\n_start:\n";

								for(auto stmt: m_prog.statements) {
												gen_stmt(stmt);	
								}

								m_output << "    mov rax, 60\n";
								m_output << "    mov rdi, 0\n"; 
								m_output << "    syscall\n";

								if(m_uses_print) {
												return run_time_print() + m_output.str();
								}

								return m_output.str();
				}

private:
				bool m_uses_print = false;

				inline void push(const std::string& reg) {
								m_output << "    push " << reg << "\n";
								m_stack_size++;
				}

				inline void pop(const std::string& reg) {
								m_output << "    pop " << reg << "\n";
								m_stack_size--;
				}

				static std::string run_time_print() {
								std::stringstream ss;
								ss << "    default rel\n";
								ss << "    section .data\n";
								ss << "    print_buf: times 32 db 0\n";
								ss << "    newline: db 10\n";          // <-- added
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

				struct Var {
								size_t stack_location;
				};

				const ProgNode m_prog;
				std::stringstream m_output;
				size_t m_stack_size = 0;
				std::unordered_map<std::string, Var> m_vars {};
};

