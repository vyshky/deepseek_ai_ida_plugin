/*
 *      Hex-Rays Decompiler project
 *      Copyright (c) 2007-2024 by Hex-Rays, support@hex-rays.com
 *      ALL RIGHTS RESERVED.
 *
 *      Sample plugin for Hex-Rays Decompiler.
 *      It decompiles the current function and prints it in the message window
 *
 */


#include "QwenAI.hpp"
#include "FunctionUtility.hpp"
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


///////////////////////////////////////////////////////////////////////////////////////////
void rename_specific_function(const std::string& old_name, const std::string& new_name) {
	ea_t func_ea = get_name_ea(BADADDR, old_name.c_str());
	if (func_ea != BADADDR) {
		set_name(func_ea, new_name.c_str(), SN_FORCE | SN_NODUMMY);
		function_names[old_name] = new_name;
		msg("Function %s renamed to %s\n", old_name.c_str(), new_name.c_str());
	}
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




// TODO :: 
// 1. Получаем Json
// 2. Записываем все значения мапы старый_нейм -> новый_нейм
// 3. Переименовываем функцю
// 4. Преименовываем переменные
// 5. Переименовываем все функции
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

	// Парсим функцию и сохраняем найденные переменные, функции и имя текущей функции
	save_current_function_name(pfn);
	save_variables(pfn);
	save_functions(pfn);


	// Подготовка промта для ai и получения переименованной таблицы функций и переменных в формате json
	std::string decompiled_code;
	get_decompiled_code(pfn, decompiled_code);
	QwenAI ai = QwenAI();
	std::string renamedElementsJson = ai.SendRequestToDeepseek(decompiled_code);
	nlohmann::json json = nlohmann::json::parse(renamedElementsJson);
	std::string currentFunctionName = json["currentFunction"];

	auto args = json["args"];
	for (auto& arg : args.items()) {
		std::string key = arg.key();
		std::string value = arg.value();
		var_names[key] = value;
	}

	auto variables = json["variables"];
	for (auto& variable : variables.items()) {
		std::string key = variable.key();
		std::string value = variable.value();
		var_names[key] = value;
	}

	auto functions = json["functions"];
	for (auto& function : functions.items()) {
		std::string key = function.key();
		std::string value = function.value();
		function_names[key] = value;
	}

	auto globals = json["globals"];
	for (auto& global : globals.items()) {
		std::string key = global.key();
		std::string value = global.value();
		var_names[key] = value;
	}

	// Переименование всех функций
	rename_current_function(pfn, currentFunctionName);
	rename_all_lvar(pfn);
	rename_all_functions(pfn);

	// Очищаем все сохраненные переменные и функции
	function_names.clear();
	var_names.clear();
	current_function.clear();

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
