#include <nlohmann/json.hpp>
using ::nlohmann::json;
#ifndef HEADER_H
#include "Header.h"
#endif

class QwenAI {
	std::string promt = R"(I need you to extract all variables, arguments, functions, the name of the current function, global variables.
Then you need to rename them so that the code is understandable to a person who does not know how to reverse engineer.
Then output json of approximately this type, found object: renamed object.
Return json, highlight in front and behind with these symbols | START_JSON | and | END_JSON | so that I can parse them later.
Output only json without a description.
Do not write anything except json.
Сheck yourself to make sure you renamed all functions, arguments, globals, the current function. Make sure everything is renamed and nothing is missed.
Example - |START_JSON|
{
'currentFunction': 'WinMain',
'args' : {
'hInstance': 'appInstance',
	'hPrevInstance' : 'prevAppInstance',
	'lpCmdLine' : 'commandLineArgs',
	'nShowCmd' : 'windowDisplayMode'
},
'variables': {
'CommandLineW': 'rawCommandLine',
	'v6' : 'mutexNameRef',
	'MutexW' : 'mutexHandle',
	'v8' : 'crashUploadResult',
	'v9' : 'finalResult',
	'lpName' : 'mutexNameBuffer',
	'v12' : 'bufferStorage'
},
'functions': {
'sub_142346208': 'checkCommandLineArgument',
	'sub_14010F330' : 'initMutexName',
	'sub_140103D40' : 'cleanupMutexName',
	'sub_1400D3AD0' : 'handleCrashUpload',
	'sub_141ACAA30' : 'runMainLogic',
	'sub_141ACA770' : 'initializeApplication',
	'sub_1400E33C0' : 'startMainApplication'
},
'globals': {
'byte_1439E1E17': 'applicationInitializedFlag'
}
}
|END_JSON|)";

	std::string replaceSumbol(const std::string& input, const std::string& sumbolA, const std::string& sumbolB) {
		std::string output = input;
		size_t pos = 0;
		while ((pos = output.find(sumbolA, pos)) != std::wstring::npos) {
			output.replace(pos, 1, sumbolB);
			pos += 2;
		}
		return output;
	}

	void removeWord(std::string& text, const std::string& word) {
		size_t pos = 0;
		// Ищем слово в тексте
		while ((pos = text.find(word, pos)) != std::string::npos) {
			// Удаляем слово, начиная с найденной позиции
			text.erase(pos, word.length());
		}
	}


	std::string sanitizeString(const std::string& input) {
		std::string output = input;
		output.erase(std::remove(output.begin(), output.end(), '\n'), output.end());
		output.erase(std::remove(output.begin(), output.end(), '\t'), output.end());
		output.erase(std::remove(output.begin(), output.end(), '\r'), output.end());
		size_t pos = 0;
		while ((pos = output.find("  ", pos)) != std::string::npos) {
			output.replace(pos, 2, " ");
		}
		return output;
	}

	std::string generatePromt(const std::string& decompiledCode) {
		return promt + R"( - |START_CODE|)" + decompiledCode + R"(|END_CODE|)";
	}
public:
	QwenAI() {}
	~QwenAI() {}

	std::string SendRequestToDeepseek(const std::string& decompiledCode)
	{
		std::string promt = generatePromt(decompiledCode);
		std::replace(promt.begin(), promt.end(), '\"', '\'');
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
		body = sanitizeString(body);

		// Отправка POST-запроса с cpr
		cpr::Response response = cpr::Post(
			cpr::Url{ "https://openrouter.ai/api/v1/chat/completions" },
			cpr::Header{
				{"Content-Type", "application/json"},
				{"Authorization", "Bearer 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"},
				{ "User-Agent", "PostmanRuntime/7.43.0" },
				{ "Accept", "*/*" },
			},
			cpr::Body{ body }
			);

		// Проверяем статус запроса
		if (response.status_code != 200)
		{
			warning("HTTP Error: %d\n", response.status_code);
			return "";
		}

		// Извлекаем content из json ответа
		nlohmann::json parsed_json = nlohmann::json::parse(response.text);

		// Извлекаем массив choices
		auto choices = parsed_json["choices"];
		std::string content;
		// Извлекаем контент из первого элемента массива choices
		if (!choices.empty() && choices[0].contains("message") && choices[0]["message"].contains("content")) {
			content = choices[0]["message"]["content"];
		}
		else {
			warning("Content not found in the JSON structure.");
		}

		removeWord(content, "|START_JSON|");
		removeWord(content, "|END_JSON|");
		std::replace(content.begin(), content.end(), '\'', '\"');
		msg("Content: %s\n", content.c_str());
		return content;
	}
};