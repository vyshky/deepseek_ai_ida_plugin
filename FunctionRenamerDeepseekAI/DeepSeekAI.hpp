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
		�heck yourself to make sure you renamed all functions, arguments, globals, the current function. Make sure everything is renamed and nothing is missed.
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
		  "model": "DeepSeek-V3.1",
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
	//**
	// * @brief �������� ������ ���� ����
	// * @return session_token
	//**
	cpr::Cookies getSession() {
		auto request_task = std::async(std::launch::async, []() {
			return cpr::Get(
				cpr::Url{ "https://chat.akash.network/api/auth/session" },
				cpr::Header{
					{"User-Agent","Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36" },
					{ "Origin","https://chat.akash.network" },
					{ "Referer","https://chat.akash.network/" }
				}
				);
			});
		if (request_task.wait_for(std::chrono::seconds(15)) == std::future_status::timeout) {
			throw std::runtime_error("Request timed out after 15 seconds.");
		}
		cpr::Response response = request_task.get();
		if (response.status_code != 200)
		{
			throw std::runtime_error("HTTP Error: " + std::to_string(response.status_code));
		}
		return response.cookies;
	}
	//**
	// * @brief ��������� ������ � ��� �����
	// * arg1 - body
	// * arg2 - cookies
	// * @return ����� ����
	//**
	std::string postToChat(std::string body, cpr::Cookies cookies) {
		auto it = cookies.begin();
		auto request_task = std::async(std::launch::async, [&body, &it]() {
			return cpr::Post(
				cpr::Url{ "https://chat.akash.network/api/chat" },
				cpr::Cookies(
					{ it->GetName() ,it->GetValue() }
				),
				cpr::Header{
					{"User-Agent","Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36" },
					{ "Origin","https://chat.akash.network" },
					{ "Referer","https://chat.akash.network/" }
				},
				cpr::Body{ body }
			);
		});
		// ������� ���������� ������ � ������� 10 �����
		if (request_task.wait_for(std::chrono::minutes(10)) == std::future_status::timeout) {
			throw std::runtime_error("Request timed out after 10 minutes.");
		}
		// �������� ��������� �������
		cpr::Response response = request_task.get();
		// ��������� ������ �������
		if (response.status_code != 200)
		{
			throw std::runtime_error("HTTP Error: " + std::to_string(response.status_code));
		}
		return response.text;
	}

public:
	DeepSeekAI() {}
	~DeepSeekAI() {}
	//**
	// * @brief ��������� ������ � ��� �����, �������� �����, �������� ��� � �������� ������� ��� ���������� ��������� ����� json ����������
	// * @return ������� json �����
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
};
