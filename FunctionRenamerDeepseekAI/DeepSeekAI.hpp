#include <nlohmann/json.hpp>
#include "StringUtils.hpp"
#ifndef HEADER_H
#include "Header.h"
#endif

class DeepSeekAI {
	std::string promt = R"(I need you to extract all variables, arguments, functions, the name of the current function, global variables.
		Then you need to rename them so that the code is understandable to a person who does not know how to reverse engineer.
		Then output json of approximately this type, found object: renamed object.
		Return json, highlight in front and behind with these symbols | START_JSON | and | END_JSON | so that I can parse them later.
		Output only json without a description.
		Do not write anything except json.
		Сheck yourself to make sure you renamed all functions, arguments, globals, the current function. Make sure everything is renamed and nothing is missed.
		Example - 
|START_JSON|
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

	std::string generateBody(const std::string& decompiledCode) {
		std::string result = promt + R"( - |START_CODE|)" + decompiledCode + R"(|END_CODE|)";
		std::replace(result.begin(), result.end(), '\"', '\'');
		std::string body = R"({
          "id": "Jr9GKaVHYnhWce9g",
		  "model": "DeepSeek-R1",
          "system": "You are a helpful assistant.",
		  "messages": [
			  {"role": "user", "content": ")" + result + R"("}
		  ],
		  "temperature": 0.6,
		  "topP": 0.95
	  })";
		body = sanitize_string(body);
		return body;
	}

public:
	DeepSeekAI() {}
	~DeepSeekAI() {}
	//**
	// * @brief Отправить запрос в чат акаша, получить ответ, привести его к удобному формату для дальнейшей обработки через json библиотеку
	// * @return Готовый json ответ
	//**
	std::string SendRequestToDeepseek(const std::string& decompiledCode)
	{
		std::string body = generateBody(decompiledCode);
		cpr::Cookies cookies = getSession();
		std::string responseText = postToChat(body, cookies);

		responseText = remove_line_containing(responseText, "f:{");
		responseText = remove_line_containing(responseText, "e:{");
		responseText = remove_line_containing(responseText, "d:{");
		responseText = remove_all_words(responseText, "0:\"");
		responseText = remove_all_words(responseText, "\"\n");
		responseText = remove_all_tags(responseText, R"(<think>[\s\S]*?</think>)");
		responseText = extract_tag_bodies(responseText, R"(\|START_JSON\|([\s\S]*?)\|END_JSON\|)");
		responseText = remove_all_words(responseText, "\\n");
		responseText = remove_all_words(responseText, "\\");
		responseText = replaceAll(responseText, "\'", "\"");
		return responseText;
	}

	//**
	// * @brief Получить сессию акаш чата
	// * @return session_token
	//**
	cpr::Cookies getSession() {
		auto request_task = std::async(std::launch::async, []() {
			return cpr::Get(cpr::Url{ "https://chat.akash.network/api/auth/session" });
			});
		if (request_task.wait_for(std::chrono::seconds(15)) == std::future_status::timeout) {
			throw std::runtime_error("Request timed out after 15 seconds.");
		}
		cpr::Response response = request_task.get();
		return response.cookies;
	}
	//**
	// * @brief Отправить вопрос в чат акаша
	// * arg1 - body
	// * arg2 - cookies
	// * @return Ответ чата
	//**
	std::string postToChat(std::string body, cpr::Cookies cookies) {
		auto it = cookies.begin();
		auto request_task = std::async(std::launch::async, [&body, &it]() {
			return cpr::Post(
				cpr::Url{ "https://chat.akash.network/api/chat" },
				cpr::Cookies(
					{ it->GetName() ,it->GetValue() }
				),
				cpr::Body{ body }
			);
			});
		// Ожидаем завершения задачи в течение 10 минут
		if (request_task.wait_for(std::chrono::minutes(10)) == std::future_status::timeout) {
			throw std::runtime_error("Request timed out after 10 minutes.");
		}
		// Получаем результат запроса
		cpr::Response response = request_task.get();
		// Проверяем статус запроса
		if (response.status_code != 200)
		{
			throw std::runtime_error("HTTP Error: " + std::to_string(response.status_code));
		}
		return response.text;
	}
};
