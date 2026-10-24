# Лабораторная работа №1 — основы ООП

Консольное приложение на C++17 с двумя независимыми классами:

- `tic_tac_toe::TicTacToe` — игровое поле «Крестики-нолики» размером от 3×3 до 10×10;
- `polynomial::Polynomial` — многочлен от одной переменной с арифметическими операциями.

Доменная логика отделена от консольного интерфейса (`lab1_cli`).

## Требования

- CMake 3.20 или новее;
- компилятор с поддержкой C++17 (g++ 11+ или clang++ 14+);
- Git и доступ к интернету для первоначальной загрузки GoogleTest.

## Сборка и запуск

Команды выполнять из корня репозитория.

```bash
cmake -S lab1 -B lab1/build
cmake --build lab1/build --parallel
./lab1/build/bin/lab1_cli
```

## Тесты

```bash
ctest --test-dir lab1/build --output-on-failure
```

Тесты находятся в `lab1/tests/`: `test_tic_tac_toe.cpp`, `test_polynomial.cpp` и `test_cli.cpp`.

## Документация API

Сгенерировать документацию Doxygen и открыть её на macOS:

```bash
doxygen lab1/Doxyfile
open lab1/docs/html/index.html
```

На Linux вместо `open` можно использовать `xdg-open lab1/docs/html/index.html`.

## Релизы и артефакты

Workflow в `.github/workflows/lab1.yml` собирает архив `lab1-cli-linux-x86_64.tar.gz` при отправке тега версии с префиксом `v`. Архив сохраняется в артефактах запуска GitHub Actions и прикрепляется к GitHub Release.

После проверки изменений и успешных CI-проверок релиз можно запустить тегом, например:

```bash
git tag v1.0.0
git push origin v1.0.0
```

Артефакт GitHub Actions хранится ограниченное время; прикреплённый к опубликованному релизу архив остаётся доступен в разделе **Releases**.
