/*
 *      Hex-Rays Decompiler project
 *      Copyright (c) 2007-2024 by Hex-Rays, support@hex-rays.com
 *      ALL RIGHTS RESERVED.
 *
 *      Sample plugin for Hex-Rays Decompiler.
 *      It decompiles the current function and prints it in the message window
 *
 */


#include "DeepSeekAI.hpp"
#ifndef HEADER_H
#include "Header.h"
#endif


 //--------------------------------------------------------------------------
struct plugin_ctx_t : public plugmod_t
{
	~plugin_ctx_t()
	{
		term_hexrays_plugin();
	}
	virtual bool idaapi run(size_t) override;
};

//--------------------------------------------------------------------------
static plugmod_t* idaapi init()
{
	if (!init_hexrays_plugin())
		return nullptr; // no decompiler
	const char* hxver = get_hexrays_version();
	msg("Hex-rays version %s has been detected, %s ready to use\n",
		hxver, PLUGIN.wanted_name);
	return new plugin_ctx_t;
}

std::map<std::string, std::string> function_names;
std::map<std::string, std::string> var_names;
std::map<std::string, std::string> current_function;
void save_current_function_name(func_t* pfn) {
	qstring func_name;
	if (get_func_name(&func_name, pfn->start_ea) > 0) {
		current_function[func_name.c_str()] = ""; // Save the current function name
		msg("Current function name: %s\n", func_name.c_str());
	}
	else {
		msg("Failed to get the current function name\n");
	}
}

void save_variables(func_t* pfn) {
	// Получение декомпилированного кода  
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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool rename_lvar2(ea_t func_ea, const char* oldname, const char* newname)
{
	lvar_saved_info_t info;
	if (!locate_lvar(&info.ll, func_ea, oldname))
		return false;
	info.name = newname;
	return modify_user_lvar_info(func_ea, MLI_NAME, info);
}

///////////////////////////////////////////////////////////////////////////////////////////
void rename_specific_function(const std::string& old_name, const std::string& new_name) {
	ea_t func_ea = get_name_ea(BADADDR, old_name.c_str());
	if (func_ea != BADADDR) {
		set_name(func_ea, new_name.c_str(), SN_FORCE | SN_NODUMMY);
		function_names[old_name] = new_name;
		msg("Function %s renamed to %s\n", old_name.c_str(), new_name.c_str());
	}
}

void rename_all_functions(func_t* pfn) {
	int counter = 1;
	for (auto& entry : function_names) {
		/*	if (entry.first == "print") {
				rename_specific_function("print", "print_print");
			}
			else {*/
		qstring new_name = qstring("function_") + std::to_string(counter).c_str();
		ea_t func_ea = get_name_ea(BADADDR, entry.first.c_str());
		if (func_ea != BADADDR) {
			set_name(func_ea, new_name.c_str(), SN_FORCE | SN_NODUMMY);
			entry.second = new_name.c_str();
			counter++;
		}
		//}
	}
	mark_cfunc_dirty(pfn->start_ea);
}
///////////////////////////////////////////////////////////////////////////////////////////


void rename_variables(func_t* pfn, const char* funcName) {
	func_item_iterator_t fii(pfn);
	for (bool ok = fii.set(pfn); ok; ok = fii.next_addr())
	{
		ea_t ea = fii.current();
		set_name(ea, "new_function_name", SN_NON_PUBLIC | SN_NON_WEAK | SN_NOLIST | SN_FORCE | SN_NODUMMY);
	}
	mark_cfunc_dirty(pfn->start_ea);
}


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


// Функция для отправки запроса в Deepseek API
// TODO :: создать промт для переданных данных и для данных которрые должны возвращаться
//void SendRequestToDeepseek(const std::string& content)
//{
//	// Отправка POST-запроса с cpr
//	cpr::Response response = cpr::Post(
//		cpr::Url{ "https://openrouter.ai/api/v1/chat/completions" },
//		cpr::Header{
//			{"Content-Type", "application/json"},
//			{"Authorization", "Bearer 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"}
//		},
//		cpr::Body{ R"({
//            "model": "deepseek/deepseek-r1:free",
//            "messages": [
//                {"role": "user", "content": ")" + content + R"("}
//            ],
//            "temperature": 0,
//            "top_p": 1,
//            "top_k": 0,
//            "frequency_penalty": 0,
//            "presence_penalty": 0,
//            "repetition_penalty": 1,
//            "min_p": 0,
//            "top_a": 0
//        })" }
//	);
//
//	// Проверяем статус запроса
//	if (response.status_code != 200)
//	{
//		warning("HTTP Error: %d\n", response.status_code);
//		return;
//	}
//
//	// Выводим первые 100 символов в warning
//	warning("Response: %.100s\n", response.text.c_str());
//}


// TODO :: 
// 1. Написать передачу декомпилированного кода в deepseekAI - print_current_decompiled_code()
// 2. Переименовать все переменные в функции и саму функцию -  find_function, fund_variables,deepseekAI получаем переименованную табличку из функций и переменных, rename_current_function()
// 3. Вывести на экран измененую функцию перед переименовывание, с подтверждением и возможность изменить функцию 
//--------------------------------------------------------------------------
bool idaapi plugin_ctx_t::run(size_t)
{
	ea_t screen_ea = get_screen_ea();
	func_t* pfn = get_func(get_screen_ea());
	if (pfn == nullptr)
	{
		warning("Please position the cursor within a function");
		return true;
	}
	msg("Current function start address: %a\n", pfn->start_ea);
	qstring funcName = "new_function_name";
	//renameFunction(pfn, funcName.c_str());
	//renameVariables(pfn, "sss");
	//analyzeFunctionElements(pfn);
	//getDecompiledCode(pfn);
	save_current_function_name(pfn);
	save_variables(pfn);
	save_functions(pfn);

	/*const char* oldname = "v4";
	const char* newname = "var_444444";*/

	/*if (rename_lvar2(screen_ea, oldname, newname)) {
		msg("variable %s succes renamed %s\n", oldname, newname);
	}
	else {
		msg("error %s\n", oldname);
	}*/

	std::string decompiled_code;
	get_decompiled_code(pfn, decompiled_code);

	DeepSeekAI deepSeekAI = DeepSeekAI();

	deepSeekAI.SendRequestToDeepseek(decompiled_code);

	mark_cfunc_dirty(pfn->start_ea);
	return true;
}

//--------------------------------------------------------------------------
static char comment[] = "Sample1 plugin for Hex-Rays decompiler";

//--------------------------------------------------------------------------
//
//      PLUGIN DESCRIPTION BLOCK
//
//--------------------------------------------------------------------------
plugin_t PLUGIN =
{
  IDP_INTERFACE_VERSION,
  PLUGIN_MULTI,         // The plugin can work with multiple idbs in parallel
  init,                 // initialize
  nullptr,
  nullptr,
  comment,              // long comment about the plugin
  nullptr,              // multiline help about the plugin
  "Deepseek Decompile",  // the preferred short name of the plugin
  nullptr,              // the preferred hotkey to run the plugin
};
