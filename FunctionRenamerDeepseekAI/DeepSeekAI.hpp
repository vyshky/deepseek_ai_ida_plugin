#include <nlohmann/json.hpp>
using ::nlohmann::json;
#ifndef HEADER_H
#include "Header.h"
#endif
using namespace std;


class DeepSeekAI {
	nlohmann::json jsonData = {
  {"happy", 2},
	};


	std::string promt = R"(
Мне нужно, чтобы ты вытащил все переменные, аргументы, функции, название текущей функции, глобальные переменные.
Потом нужно их переименовать, чтобы код был понятен человеку который не умеет реверсить.
Потом вывести json примерно такого типа, найденный обект : переименованный объект.
Верни json, выделить спереди и с зади вот такими символами | START_JSON | и | END_JSON | , чтобы я их смог потом спарсить.
Выводи только json без описания.
Не пиши ничего, кроме json.
Отвечай на русском.
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

	std::string sanitizeString(const std::string& input) {
		std::string output = input;
		output.erase(std::remove(output.begin(), output.end(), '\n'), output.end());
		output.erase(std::remove(output.begin(), output.end(), '\t'), output.end());
		size_t pos = 0;
		while ((pos = output.find('"', pos)) != std::string::npos) {
			output.replace(pos, 1, "'");
			pos += 2;
		}
		return output;
	}
	std::string generatePromt(const std::string& decompiledCode) {
		return sanitizeString(promt + R"( - |START_CODE|)" + decompiledCode + R"(|END_CODE|)");
	}
public:
	DeepSeekAI() {}
	~DeepSeekAI() {}




	void SendRequestToDeepseek(const std::string& decompiledCode)
	{
		std::string promt = generatePromt(decompiledCode);
		std::string body = R"({
           "model": "deepseek/deepseek-r1:free",
           "messages": [
               {"role": "user", "content": ")" + promt + R"("}
           ],
           "temperature": 0,
           "top_p": 1,
           "top_k": 0,
           "frequency_penalty": 0,
           "presence_penalty": 0,
           "repetition_penalty": 1,
           "min_p": 0,
           "top_a": 0
       })";

		// Отправка POST-запроса с cpr
		cpr::Response response = cpr::Post(
			cpr::Url{ "https://openrouter.ai/api/v1/chat/completions" },
			cpr::Header{
				{"Content-Type", "application/json"},
				{"Authorization", "Bearer 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"}
			},
			cpr::Body{ body }
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

};