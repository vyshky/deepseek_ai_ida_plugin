# FunctionRenamerDeepseekAI

## Комбинация клавиш ALT+SHIFT+1
## Плагин тестировался на этой версии IDA - https://t.me/osiesquad/47579

## Файлы и их функции

### FunctionRenamerDeepseekAI.cpp
Этот файл содержит основную реализацию плагина для декомпилятора Hex-Rays. Он декомпилирует текущую функцию, переименовывает переменные и функции с помощью DeepSeekAI и выводит результаты.

---

#### Сборка
```
cmake -B build -DIDA_SDK=path/to/idasdk
cmake --build build
```

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
- `mark_cfunc_dirty(ea_t start_ea)`: Помечает текущую функцию как грязную, то есть переменные были переименованны. Обновляет окошко и выводит новые имена после переименования

---

### DeepSeekAI.hpp  
Этот файл реализует класс для взаимодействия с AI-моделью DeepSeek через API Akash Network. Основная задача — анализ декомпилированного кода, автоматическое переименование элементов (функций, переменных, аргументов) и возврат структурированного JSON.  

---

#### Функции:  
- `SendRequestToDeepseek(const std::string& decompiledCode)`:  
  Основной метод для отправки декомпилированного кода в DeepSeek. Обрабатывает ответ: удаляет лишние символы, извлекает JSON между `|START_JSON|` и `|END_JSON|`, заменяет кавычки.  
- `generateBody(const std::string& decompiledCode)`:  
  Формирует тело запроса для AI: добавляет системный промпт, оборачивает код в `|START_CODE|`/`|END_CODE|`, заменяет двойные кавычки на одинарные и санирует строку.  
- `getSession()`:  
  Получает сессионные куки для аутентификации в API Akash Network через GET-запрос. Таймаут: 15 секунд.  
- `postToChat(std::string body, cpr::Cookies cookies)`:  
  Отправляет POST-запрос с телом и куками в API чата Akash. Таймаут: 10 минут. Обрабатывает HTTP-ошибки (например, статус ≠ 200).  

---

Пример вызова:  
```cpp  
DeepSeekAI ai;  
std::string jsonResult = ai.SendRequestToDeepseek(decompiled_code);  
```

---

### FunctionUtilityHexRay.hpp  
Этот файл предоставляет утилиты для работы с декомпилятором Hex-Rays в IDA. Он собирает информацию о функциях, переменных и аргументах, а также реализует их переименование на основе внешних данных (например, из JSON, полученного от DeepSeekAI).  

---

#### Функции:  
- `get_decompiled_code(func_t* pfn, std::string& out_code)`:  
  Декомпилирует функцию в псевдокод и сохраняет его в строку `out_code`. Обрабатывает ошибки декомпиляции (например, через `hexrays_failure_t`).
- `save_current_function_name(func_t* pfn)`:  
  Сохраняет имя текущей функции в глобальный словарь `current_function` для последующего переименования.
- `save_variables(func_t* pfn)`:  
  Собирает все локальные переменные (включая аргументы) и глобальные переменные, используемые в функции. Сохраняет их в `var_names`.
- `save_functions(func_t* pfn)`:  
  Находит все вызовы функций внутри текущей функции (например, через инструкции `call`) и сохраняет их имена в `function_names`.
- `rename_current_function(func_t* pfn, const std::string& new_name)`:  
  Переименовывает текущую функцию (через `set_name`), используя новое имя.
- `rename_all_lvars_and_globalvars(func_t* pfn)`:  
  Переименовывает все локальные и глобальные переменные из `var_names`. Для глобальных использует `set_name`, для локальных — `rename_lvar`.
- `rename_all_functions(func_t* pfn)`:  
  Переименовывает все функции из `function_names`, найденные в текущем контексте.  

---

#### Особенности:  
- Интеграция с Hex-Rays API:  
  Использует методы `decompile`, `get_lvars`, `func_item_iterator_t` для анализа функций и переменных.  
- Работа с глобальными переменными:  
  Определяет глобальные переменные через анализ инструкций (например, `o_mem` в `insn_t`).  
- Переименование:  
  Применяет методы IDA Pro (`set_name`, `rename_lvar`) для модификации имён.  

---

#### Пример использования:
```cpp
func_t* pfn = get_current_function();
std::string decompiled_code;
get_decompiled_code(pfn, decompiled_code);

// После получения JSON от DeepSeekAI:
rename_current_function(pfn, "newMain");
rename_all_lvars_and_globalvars(pfn);
rename_all_functions(pfn);
```  


📺 **YouTube-канал**: [Смотреть видео](https://www.youtube.com/watch?v=z6uhJats594)