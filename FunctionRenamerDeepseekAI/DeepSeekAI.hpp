#ifndef HEADER_H
#include "Header.h"
#endif

class DeepSeekAI {
	std::string promt = R"(
Мне нужно, чтобы ты вытащил все переменные, аргументы, функции, название текущей функции, глобальные переменные.
Потом нужно их переименовать, чтобы код был понятен человеку который не умеет реверсить.
Потом вывести json примерно такого типа, найденный обект : переименованный объект.
Верни json, выделить спереди и с зади вот такими символами | START_JSON | и | END_JSON | , чтобы я их смог потом спарсить.
Выводи только json без описания.
Пример - |START_JSON|
{
"currentFunction": "WinMain",
	"args" : {
	"hInstance": "appInstance",
		"hPrevInstance" : "prevAppInstance",
		"lpCmdLine" : "commandLineArgs",
		"nShowCmd" : "windowDisplayMode"
},
	"variables": {
	"CommandLineW": "rawCommandLine",
		"v6" : "mutexNameRef",
		"MutexW" : "mutexHandle",
		"v8" : "crashUploadResult",
		"v9" : "finalResult",
		"lpName" : "mutexNameBuffer",
		"v12" : "bufferStorage"
},
	"functions": {
	"sub_142346208": "checkCommandLineArgument",
		"sub_14010F330" : "initMutexName",
		"sub_140103D40" : "cleanupMutexName",
		"sub_1400D3AD0" : "handleCrashUpload",
		"sub_141ACAA30" : "runMainLogic",
		"sub_141ACA770" : "initializeApplication",
		"sub_1400E33C0" : "startMainApplication"
},
	"globals": {
	"byte_1439E1E17": "applicationInitializedFlag"
}
}
|END_JSON|)";
public:
	DeepSeekAI() {}
	~DeepSeekAI() {}

	void SendRequestToDeepseek(const std::string& decompiledCode)
	{
		// Отправка POST-запроса с cpr
		cpr::Response response = cpr::Post(
			cpr::Url{ "https://openrouter.ai/api/v1/chat/completions" },
			cpr::Header{
				{"Content-Type", "application/json"},
				{"Authorization", "Bearer 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"}
			},
			cpr::Body{ R"({
           "model": "deepseek/deepseek-r1:free",
           "messages": [
               {"role": "user", "content": ")" + generatePromt(decompiledCode) + R"("}
           ],
           "temperature": 0,
           "top_p": 1,
           "top_k": 0,
           "frequency_penalty": 0,
           "presence_penalty": 0,
           "repetition_penalty": 1,
           "min_p": 0,
           "top_a": 0
       })" }
		);

		// Проверяем статус запроса
		if (response.status_code != 200)
		{
			warning("HTTP Error: %d\n", response.status_code);
			return;
		}

		// Выводим первые 100 символов в warning
		warning("Response: %.100s\n", response.text.c_str());
	}

	std::string generatePromt(const std::string& decompiledCode) {
		 return promt + R"( - |START_CODE|)" + decompiledCode + R"(|END_CODE|)";
	}
};