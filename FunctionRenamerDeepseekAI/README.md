# FunctionRenamerDeepseekAI

## Файлы и их функции

### FunctionRenamerDeepseekAI.cpp
Этот файл содержит основную реализацию плагина для декомпилятора Hex-Rays. Он декомпилирует текущую функцию, переименовывает переменные и функции с помощью DeepSeekAI и выводит результаты.

---

#### Функции:
- `init()`: Инициализирует плагин и проверяет доступность декомпилятора Hex-Rays.
- `plugin_ctx_t::run(size_t)`: Основная функция, которая выполняется при запуске плагина. Она декомпилирует текущую функцию, отправляет декомпилированный код в DeepSeekAI для переименования и применяет переименованные элементы.
- `save_current_function_name(func_t* pfn)`: Сохраняет имя текущей функции.
- `save_variables(func_t* pfn)`: Сохраняет переменные, найденные в текущей функции.
- `save_functions(func_t* pfn)`: Сохраняет функции, найденные в текущей функции.
- `get_decompiled_code(func_t* pfn, std::string& decompiled_code)`: Получает декомпилированный код текущей функции.
- `rename_current_function(func_t* pfn, const std::string& new_name)`: Переименовывает текущую функцию.
- `rename_all_lvars_and_globalvars(func_t* pfn)`: Переименовывает все локальные и глобальные переменные.
- `rename_all_functions(func_t* pfn)`: Переименовывает все функции.
- `mark_cfunc_dirty(ea_t start_ea)`: Помечает декомпилированную функцию как измененную для повторной декомпиляции.

---

### DeepSeekAI.hpp  
Этот файл реализует класс для взаимодействия с AI-моделью DeepSeek через API Akash Network. Основная задача — анализ декомпилированного кода, автоматическое переименование элементов (функций, переменных, аргументов) и возврат структурированного JSON.  

---

#### Функции:  
- **`SendRequestToDeepseek(const std::string& decompiledCode)`**:  
  Основной метод для отправки декомпилированного кода в DeepSeek. Обрабатывает ответ: удаляет лишние символы, извлекает JSON между `|START_JSON|` и `|END_JSON|`, заменяет кавычки.  
- **`generateBody(const std::string& decompiledCode)`**:  
  Формирует тело запроса для AI: добавляет системный промпт, оборачивает код в `|START_CODE|`/`|END_CODE|`, заменяет двойные кавычки на одинарные и санирует строку.  
- **`getSession()`**:  
  Получает сессионные куки для аутентификации в API Akash Network через GET-запрос. Таймаут: 15 секунд.  
- **`postToChat(std::string body, cpr::Cookies cookies)`**:  
  Отправляет POST-запрос с телом и куками в API чата Akash. Таймаут: 10 минут. Обрабатывает HTTP-ошибки (например, статус ≠ 200).  

---

Пример вызова:  
```cpp  
DeepSeekAI ai;  
std::string jsonResult = ai.SendRequestToDeepseek(decompiled_code);  
// Парсинг jsonResult для применения переименований в декомпиляторе.  
```
