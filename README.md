# EventSystem
Scalable and flexible eventSystem with easy interface

# Build
SFML used in example to handle user input.
C++20 required.
Visual Studio:
1. use vcpkg: vcpkg install sfml
2. create an empty project
3. paste code in main.cpp file
4. build
5. run

# Explain
class task - обеспечивает корутину;
class EventSystem - класс событийной системы;
class Event - интерфейс awaitable типа для корутины. Базовый для всех event-ов;
class EventInputSystem - базовый класс для пользовательского ввода. Наследники должны переопределить функцию update(), в которой будет использована resume();
class Gun реализует две функции с подпиской на событие;
Используйте корутину с co_await EventSystem::%НаследникEvent%::get(), чтобы ожидать событие.
Запускайте корутины в конструкторе своего класса.
