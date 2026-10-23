# Лабораторная работа №1 — проектирование ПО интеллектуальных систем

## Шпаргалка для быстрой проверки

Все команды выполняются из корня репозитория.

| Действие | Команда |
|---|---|
| Сборка | `cmake -S lab1 -B lab1/build && cmake --build lab1/build` |
| Тесты | `ctest --test-dir lab1/build --output-on-failure` |
| Запуск CLI | `./lab1/build/bin/lab1_cli` |
| Генерация документации | `doxygen lab1/Doxyfile` |
| Открыть документацию | `lab1/docs/html/index.html` |

**Инструкция для проверяющего.** Скачайте `lab1-cli-linux-x86_64.tar.gz` из раздела Releases на GitHub, распакуйте командой `tar -xzf lab1-cli-linux-x86_64.tar.gz` и запустите `./lab1_cli`. Бинарный файл собирается со статической линковкой; автономность проверяется smoke-тестом в чистом контейнере Ubuntu 22.04.

## Репозиторий

| Каталог | Содержание |
|---|---|
| `lab1/` | Активная лабораторная работа: C++17, крестики-нолики, многочлены и CLI |
| `lab2/` | Лабораторная работа не начата |
| `lab3/` | Лабораторная работа не начата |
| `lab4/` | Лабораторная работа не начата |

## Лабораторная работа №1

Реализованы два независимых домена: `tic_tac_toe::TicTacToe` (вариант 1.5.1) и `polynomial::Polynomial` (вариант 1.6.2). CLI не переносит бизнес-логику в меню: правила игры и арифметика реализованы в доменных классах.

### Требования

- CMake ≥ 3.20;
- g++ ≥ 11 или clang++ ≥ 14 с поддержкой C++17;
- Git и доступ к интернету при первой загрузке GoogleTest через `FetchContent`;
- для статического анализа: `clang-format`, `clang-tidy`, `run-clang-tidy`;
- для документации и покрытия: Doxygen, Graphviz и gcovr.

### Сборка, тесты, санитайзеры

```bash
cmake -S lab1 -B lab1/build -DCMAKE_BUILD_TYPE=Debug
cmake --build lab1/build --parallel
ctest --test-dir lab1/build --output-on-failure

cmake -S lab1 -B lab1/build-asan -DCMAKE_BUILD_TYPE=Debug -DLAB1_SANITIZE=ON
cmake --build lab1/build-asan --parallel
ctest --test-dir lab1/build-asan --output-on-failure
```

### CMake-опции

| Опция | Значение по умолчанию | Назначение |
|---|---|---|
| `LAB1_BUILD_TESTS` | `ON` | собирать модульные тесты |
| `LAB1_SANITIZE` | `OFF` | включить ASan и UBSan |
| `LAB1_COVERAGE` | `OFF` | включить измерение покрытия |
| `LAB1_STATIC` | `OFF` | статически слинковать CLI на Linux |

### Тесты

- `lab1/tests/test_tic_tac_toe.cpp`: T01–T54;
- `lab1/tests/test_polynomial.cpp`: P01–P50;
- `lab1/tests/test_cli.cpp`: C01–C14.

Полная инструкция и разбор GitHub Actions для защиты находятся в `lab1/GETTING_STARTED.md`. Спецификация исходных требований находится в `specification.md`.
