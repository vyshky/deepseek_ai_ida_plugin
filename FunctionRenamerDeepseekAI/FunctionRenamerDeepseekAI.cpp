#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include <kernwin.hpp>

//--------------------------------------------------------------------------
struct plugin_ctx_t : public plugmod_t
{
	virtual bool idaapi run(size_t) override;
};

//--------------------------------------------------------------------------
bool idaapi plugin_ctx_t::run(size_t)
{
	msg("Hello, world! (cpp)\n");
	return true;
}

//--------------------------------------------------------------------------
static plugmod_t* idaapi init()
{
	return new plugin_ctx_t;
}

//--------------------------------------------------------------------------
plugin_t PLUGIN =
{
  IDP_INTERFACE_VERSION,
  PLUGIN_UNL            // Unload the plugin immediately after calling 'run'
  | PLUGIN_MULTI,       // The plugin can work with multiple idbs in parallel
  init,                 // initialize
  nullptr,
  nullptr,
  "Hello World Plugin",        // comment
  "This plugin shows a message", // help
  "HelloPlugin",               // wanted_name (обязательное поле)
  "Ctrl+Alt+H"                 // wanted_hotkey (не nullptr!)
};
