/*
 *      Hex-Rays Decompiler project
 *      Copyright (c) 2007-2024 by Hex-Rays, support@hex-rays.com
 *      ALL RIGHTS RESERVED.
 *
 *      Sample plugin for Hex-Rays Decompiler.
 *      It decompiles the current function and prints it in the message window
 *
 */

#include <hexrays.hpp>

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

void renameFunction(func_t* pfn, const char* funcName) {
	func_item_iterator_t fii(pfn);
	ea_t ea = fii.current();
	//set_name(ea, funcName, SN_NON_PUBLIC | SN_NON_WEAK | SN_NOLIST | SN_FORCE | SN_NODUMMY);
	set_name(ea, funcName,  SN_FORCE | SN_NODUMMY);
	mark_cfunc_dirty(ea);
}


// TODO :: 
// 1. Написать передачу декомпилированного кода в deepseekAI
// 2. Переименовать все переменные в функции и саму функцию
// 3. Вывести на экран измененую функцию перед переименовывание, с подтверждением и возможность изменить функцию 
//--------------------------------------------------------------------------
bool idaapi plugin_ctx_t::run(size_t)
{
	func_t* pfn = get_func(get_screen_ea());

	std::string funcName = "new_function_name";
	renameFunction(pfn, funcName.c_str());

	/////////////////////////////////////////////////////////
	//	 // Инициализация итератора для элементов функции
	//func_item_iterator_t fii(pfn);
	////for (bool ok = fii.set(pfn); ok; ok = fii.next_addr())
	////{
	//ea_t ea = fii.current();
	//// Пример изменения имени переменной
	//set_name(ea, "new_function_name", SN_NON_PUBLIC | SN_NON_WEAK | SN_NOLIST | SN_FORCE | SN_NODUMMY);
	////}
	//mark_cfunc_dirty(ea);
	//// Пример изменения имени функции
	////set_name(pfn->start_ea, "new_function_name_test", SN_NON_PUBLIC | SN_NON_WEAK | SN_NOLIST | SN_FORCE | SN_NODUMMY);
	/////////////////////////////////////////////////////////

	if (pfn == nullptr)
	{
		warning("Please position the cursor within a function");
		return true;
	}
	hexrays_failure_t hf;
	cfuncptr_t cfunc = decompile(pfn, &hf, DECOMP_WARNINGS);
	if (cfunc == nullptr)
	{
		warning("#error \"%a: %s", hf.errea, hf.desc().c_str());
		return true;
	}
	msg("%a: successfully decompiled\n", pfn->start_ea);

	const strvec_t& sv = cfunc->get_pseudocode();
	for (int i = 0; i < sv.size(); i++)
	{
		qstring buf;
		tag_remove(&buf, sv[i].line);
		msg("%s\n", buf.c_str());
	}
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
  "Decompile & Print",  // the preferred short name of the plugin
  nullptr,              // the preferred hotkey to run the plugin
};
