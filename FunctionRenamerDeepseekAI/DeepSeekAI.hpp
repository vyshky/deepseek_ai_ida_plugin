#include <nlohmann/json.hpp>
#include "StringUtils.hpp"
#ifndef HEADER_H
#include "Header.h"
#endif

class DeepSeekAI {
	std::string promt =
R"(I need you to extract all variables, arguments, functions, the name of the current function, and global variables.
Then you need to rename them so that the code is understandable to a person who does not know how to reverse engineer.

**Renaming rules:**

1. Use Hungarian notation everywhere for variables, arguments, and globals.
2. Do not rename functions, variables, or globals that already have meaningful names (for example, `WinMain`, `RenderFrame`, `playerHealth`).
3. Don't rename Windows API, Linux API or other known external library functions/variables, but you should definitely return them in the response, double check that they are all in the response.
4. Keep the original letter case (lower and upper case) when renaming.
5. If the object has no meaningful name (like `v1`, `a2`, `sub_140123456`), rename it into a descriptive one.
6. The current function must also be renamed if it does not already have a meaningful name.
7. Adapt to context: Be prepared to adapt the notation to a specific context or task. Sometimes a simpler or more flexible naming system may be more appropriate. If there are more than 2 notations in one name, reduce it to 1.

**Output format:**

* Return JSON in this format:

|START_JSON|
        {
          'currentFunction': 'WinMain', // function WinMain
          'args': {
            'hInstance': 'hInstance', // handle Instance
            'hPrevInstance': 'hPrevInstance', // handle Previous Instance
            'lpCmdLine': 'lpCmdLine', // long pointer to zero-terminated string Command Line
            'nShowCmd': 'nShowCmd' // numeric Show Command
          },
          'variables': {
			'v3': 'phkResult' // pointer handler winapi argument passed to the function
            'CommandLineW': 'pwszRawCommandLine', // pointer to wide character string Raw Command Line
            'v6': 'pszMutexNameRef', // pointer to zero-terminated string Mutex Name Reference
            'MutexW': 'hMutexHandle', // handle Mutex
            'v8': 'iCrashUploadResult', // integer Crash Upload Result
            'v9': 'iFinalResult', // integer Final Result
            'lpName': 'pszMutexNameBuffer', // long pointer to zero-terminated string Mutex Name Buffer
            'v12': 'pBufferStorage', // pointer to Buffer Storage
            'dwErrorCode': 'dwError', // DWORD Error Code
            'lParam': 'lParamValue', // long parameter value
            'wParam': 'wParamValue', // word parameter value
            'lpRect': 'lprcRect', // long pointer to RECT structure
            'ptCursor': 'ptCursorPos', // POINT structure Cursor Position
			'array': 'aiCoords' // 'type' == 't' - type arr[10]; should be used as tArray
			'Class::var22': 'Class::iCounter' // Use 'Class::iCounter' instead of 'Class::var22' Class identifier should not be renamed
          },
          'functions': {
            'sub_142346208': 'CheckCommandLineArgument', // Check Command Line Argument
            'sub_14010F330': 'InitMutexName', // Initialize Mutex Name
            'sub_140103D40': 'CleanupMutexName', // Cleanup Mutex Name
            'sub_1400D3AD0': 'HandleCrashUpload', // Handle Crash Upload
            'sub_141ACAA30': 'RunMainLogic', // Run Main Logic
            'sub_141ACA770': 'InitApplication', // Initialize Application
            'sub_1400E33C0': 'StartMainApplication' // Start Main Application
          },
          'globals': {
            'byte_1439E1E17': 'gbApplicationInitializedFlag' // byte Application Initialized Flag
          }
        }
|END_JSON|

**Important:**

* Output **only JSON**, without descriptions, explanations, or extra text.
* Always return json between |START_JSON| and |END_JSON|.

** The code that needs to be renamed is below **)";

	std::string generateBody(const std::string& decompiledCode) {
		std::string result = promt + "\r\n" + R"(|START_CODE|)" + "\r\n" + decompiledCode + R"(|END_CODE|)";
		std::replace(result.begin(), result.end(), '\"', '\'');
		std::string body = R"({
          "id": "aYMA3tLVlZuW4fXN",
		  "model": "meta-llama-Llama-4-Maverick-17B-128E-Instruct-FP8",
          "system": "You are a helpful assistant.",
		  "messages": [
			  {"role": "user", "content": ")" + result + R"(",
									 "parts": [
									 {
									   "type": "text",
									   "text": ")" + result + R"("
									 }]

          }],
		  "temperature": 0.6,
		  "topP": 0.95
	  })";
		body = sanitize_string(body);
		return body;
	}
	//**
	// * @brief Получить сессию акаш чата
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
				cpr::Header{
					{"User-Agent","Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36" },
					{ "Origin","https://chat.akash.network" },
					{ "Referer","https://chat.akash.network/" }
				},
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
};
