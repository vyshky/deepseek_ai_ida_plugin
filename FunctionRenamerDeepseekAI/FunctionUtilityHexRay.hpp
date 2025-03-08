#ifndef HEADER_H
#include "Header.h"
#endif

std::map<std::string, std::string> function_names;
std::map<std::string, std::string> var_names;
std::map<std::string, std::string> current_function;


void get_decompiled_code(func_t* pfn, std::string& out_code) {
	hexrays_failure_t hf;
	cfuncptr_t cfunc = decompile(pfn, &hf, DECOMP_WARNINGS);
	if (cfunc == nullptr) {
		warning("#error \"%a: %s", hf.errea, hf.desc().c_str());
		return;
	}
	const strvec_t& sv = cfunc->get_pseudocode();
	for (int i = 0; i < sv.size(); i++) {
		qstring buf;
		tag_remove(&buf, sv[i].line);
		out_code += buf.c_str();
		out_code += "\n";
	}
}

void save_current_function_name(func_t* pfn) {
	qstring func_name;
	if (get_func_name(&func_name, pfn->start_ea) > 0) {
		current_function[func_name.c_str()] = "";
		msg("Current function name: %s\n", func_name.c_str());
	}
	else {
		msg("Failed to get the current function name\n");
	}
}

void save_variables(func_t* pfn) {
	hexrays_failure_t hf;
	cfuncptr_t cfunc = decompile(pfn, &hf, DECOMP_WARNINGS);
	if (cfunc == nullptr) {
		warning("#error \"%a: %s", hf.errea, hf.desc().c_str());
		return;
	}

	// Перебор локальных переменных  
	const lvars_t& lvars = *cfunc->get_lvars();
	for (auto& lvar : lvars) {
		if (!lvar.name.empty()) {
			if (lvar.is_arg_var()) {
				msg("Argument: %s\n", lvar.name.c_str());
			}
			else {
				msg("Local variable: %s\n", lvar.name.c_str());
			}
			var_names[lvar.name.c_str()] = ""; // Save the variable name  
		}
	}

	// Перебор всех элементов функции  
	func_item_iterator_t fii(pfn);
	for (bool ok = fii.set(pfn); ok; ok = fii.next_addr()) {
		ea_t ea = fii.current();
		if (is_code(get_flags(ea))) {
			// Проверка операндов инструкции на глобальные переменные  
			insn_t insn;
			decode_insn(&insn, ea);
			for (int i = 0; i < UA_MAXOP; i++) {
				if (insn.ops[i].type == o_mem) {
					qstring var_name;
					if (get_name(&var_name, insn.ops[i].addr) > 0) {
						msg("Global variable used at address: %a, name: %s\n", insn.ops[i].addr, var_name.c_str());
					}
					else {
						msg("Global variable used at address: %a\n", insn.ops[i].addr);
					}
					var_names[var_name.c_str()] = ""; // Save the global variable name  
				}
			}
		}
	}
}

void save_functions(func_t* pfn) {
	func_item_iterator_t fii(pfn);
	for (bool ok = fii.set(pfn); ok; ok = fii.next_addr()) {
		ea_t ea = fii.current();
		insn_t insn;
		if (decode_insn(&insn, ea)) {
			if (insn.itype == 18 || insn.itype == 86 || insn.itype == 16) {
				qstring callee_name;
				if (get_name(&callee_name, insn.ops[0].addr) > 0) {
					msg("Call to function at address: %a, name: %s\n", insn.ops[0].addr, callee_name.c_str());
					function_names[callee_name.c_str()] = ""; // Save the function name  
				}
			}
		}
	}
}

void rename_current_function(func_t* pfn, const std::string& new_name) {
	qstring old_name;
	if (get_func_name(&old_name, pfn->start_ea) > 0) {
		set_name(pfn->start_ea, new_name.c_str(), SN_FORCE | SN_NODUMMY);
		msg("Function %s renamed to %s\n", old_name.c_str(), new_name.c_str());
	}
	else {
		msg("Failed to get the current function name\n");
	}
}

// lvar и globalvar
void rename_all_lvars_and_globalvars(func_t* pfn) {
	qstring name;
	for (const auto& var : var_names) {
		const std::string& old_name = var.first;
		const std::string& new_name = var.second;
		
		ea_t address_var = get_name_ea(pfn->start_ea, var.first.c_str());
		ssize_t result = get_ea_name(&name, address_var, GN_LOCAL);
		if (result != -1) {
			// globalVar
			set_name(address_var, new_name.c_str(), SN_FORCE | SN_NODUMMY);
		}
		else {
			// lvar
			rename_lvar(pfn->start_ea, old_name.c_str(), new_name.c_str());
		}
	}
}

void rename_all_functions(func_t* pfn) {
	for (const auto& entry : function_names) {
		const std::string& old_name = entry.first;
		const std::string& new_name = entry.second;
		ea_t func_ea = get_name_ea(BADADDR, old_name.c_str());
		if (func_ea != BADADDR) {
			set_name(func_ea, new_name.c_str(), SN_FORCE | SN_NODUMMY);
			msg("Function %s renamed to %s\n", old_name.c_str(), new_name.c_str());
		}
	}
}
