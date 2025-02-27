#include "cpr/cpr.h"
#include "nlohmann/json.hpp"
#include "string"


class DeepSeekAI {
	std::string url = "https://openrouter.ai/api/v1/chat/completions";
	std::string api_key = "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";
	std::string body = R"({
    "model": "deepseek/deepseek-r1:free",
    "messages": [
      {"role": "user", "content": "Напиши код helloworld на асм"}
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
	cpr::Header headers ={ { "Content-Type", "application/json" },
						   {"Authorization", "Bearer " + api_key } };
public:
	DeepSeekAI() {	}
	~DeepSeekAI() {
	}

	void execute_msg() {
		cpr::Response response = cpr::Post(cpr::Url{ url },
			cpr::Body{ body },
			cpr::Header{ headers });
	}
}

