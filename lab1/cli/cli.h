#pragma once

#include "polynomial/polynomial.h"
#include "tic_tac_toe/tic_tac_toe.h"

#include <cstddef>
#include <iosfwd>
#include <string>
#include <string_view>

/// @brief Консольный интерфейс лабораторной работы.
/// @details Объединяет адаптеры потоков, представления и меню; доменные классы
///          остаются независимыми от пользовательского интерфейса.
namespace cli {

/// @brief Именованная константа домена или интерфейса.
inline constexpr int kDisplayPrecision = 6;
/// @brief Именованная константа домена или интерфейса.
inline constexpr int kIndexWidth = 2;
/// @brief Именованная константа домена или интерфейса.
inline constexpr int kCellWidth = 3;
/// @brief Именованная константа домена или интерфейса.
inline constexpr int kExitChoice = 0;

/// @brief Тексты меню, приглашений и результатов операций.
namespace messages {
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kApplicationTitle = "=== Лабораторная работа №1 ===";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kMainMenu =
    "\nГлавное меню:\n1. Крестики-нолики\n2. Многочлен от одной переменной\n0. Выход\n";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kChoicePrompt = "Ваш выбор: ";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kInvalidInput = "Некорректный ввод";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kErrorPrefix = "Ошибка: ";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kGoodbye = "Работа завершена.";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kTicTacToeTitle = "\n=== Крестики-нолики ===";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kTicTacToeMenu =
    "\nМеню «Крестики-нолики»:\n"
    "1. Играть партию целиком\n"
    "2. Создать новое поле\n"
    "3. Показать поле\n"
    "4. Проверить возможность хода\n"
    "5. Прочитать клетку\n"
    "6. Записать клетку напрямую\n"
    "7. Поставить фигуру по правилам\n"
    "8. Проверить победителя\n"
    "9. Копирование и сравнение\n"
    "10. Текстовый формат поля\n"
    "0. Назад\n";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kBoardSizePrompt = "Введите размер поля (3..10): ";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kRowPrompt = "Введите номер строки (1..N): ";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kColumnPrompt = "Введите номер столбца (1..N): ";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kMarkPrompt = "Введите метку (X или O): ";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kAnyMarkPrompt = "Введите метку (., X или O): ";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kMovePrompt = "Введите строку и столбец (1..N) или q: ";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kMovePossible = "Можно";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kMoveImpossible = "Нельзя";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kMoveRejected = "Ход невозможен";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kMoveMade = "Ход выполнен";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kGameInterrupted = "Партия прервана";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kNoWinner = "Победителя нет";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kDraw = "Ничья";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kBoardCreated = "Поле создано";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kCopyEqual = "Копия равна оригиналу";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kCopyDifferent =
    "После изменения копия не равна оригиналу";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kOriginalUnchanged = "Оригинал не изменён";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kAssignmentEqual = "После присваивания объекты равны";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kPolynomialTitle = "\n=== Многочлены ===";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kPolynomialMenu =
    "\nМеню «Многочлены»:\n"
    "1. Задать многочлен\n"
    "2. Ввести в текстовом формате\n"
    "3. Показать A и B\n"
    "4. Прочитать коэффициент\n"
    "5. Изменить коэффициент\n"
    "6. Вычислить значение\n"
    "7. A + B\n"
    "8. A - B\n"
    "9. A * B\n"
    "10. A / B\n"
    "11. Составная операция A op= B\n"
    "12. Сравнить A и B\n"
    "13. Копирование и присваивание\n"
    "0. Назад\n";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kTargetPrompt = "Выберите многочлен (A/B): ";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kDegreePrompt = "Введите степень многочлена: ";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kCoefficientsPrompt =
    "Введите коэффициенты c0 ... cn одной строкой: ";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kPolynomialLinePrompt = "Введите строку degree c0 ... cn: ";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kIndexPrompt = "Введите степень коэффициента: ";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kCoefficientPrompt = "Введите значение коэффициента: ";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kArgumentPrompt = "Введите аргумент x: ";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kOperationPrompt = "Введите операцию (+, -, *, /): ";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kRemainderDiscarded = "остаток отброшен";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kEqual = "равны";
/// @brief Именованная константа домена или интерфейса.
inline constexpr std::string_view kNotEqual = "не равны";
}  // namespace messages

/// @brief Консольный ввод и вывод через заданные потоки.
class Console {
public:
/// @brief Создаёт объект.
    Console(std::istream& input, std::ostream& output);
/// @brief Читает строку после приглашения.
    [[nodiscard]] std::string read_line(std::string_view prompt);
/// @brief Читает целое число.
    [[nodiscard]] int read_integer(std::string_view prompt);
/// @brief Читает целое число из заданного диапазона.
    [[nodiscard]] int read_integer_in_range(std::string_view prompt, int minimum, int maximum);
/// @brief Читает конечное вещественное число.
    [[nodiscard]] double read_double(std::string_view prompt);
/// @brief Читает один символ.
    [[nodiscard]] char read_character(std::string_view prompt);
/// @brief Выводит текст без перевода строки.
    void print(std::string_view text);
/// @brief Выводит текст с переводом строки.
    void print_line(std::string_view text);
private:
/// @brief Выполняет операцию класса.
    void print_invalid_input();
    std::istream& input_;
    std::ostream& output_;
};

/// @brief Отображение игрового поля.
class BoardView {
public:
/// @brief Выводит представление объекта.
    void render(std::ostream& output, const tic_tac_toe::TicTacToe& board) const;
};

/// @brief Отображение многочлена в текстовой и алгебраической форме.
class PolynomialView {
public:
/// @brief Выводит представление объекта.
    void render(std::ostream& output, char name, const polynomial::Polynomial& value) const;
/// @brief Формирует алгебраическую запись многочлена.
    [[nodiscard]] std::string algebraic(const polynomial::Polynomial& value) const;
private:
    void append_term(std::ostream& output, const polynomial::Polynomial& value,
                     std::size_t power, bool& has_previous) const;
};

/// @brief Меню игры крестики-нолики.
class TicTacToeMenu {
public:
/// @brief Создаёт объект.
    TicTacToeMenu(std::istream& input, std::ostream& output);
/// @brief Запускает цикл меню.
    void run();
private:
/// @brief Выводит пункты меню.
    void show_menu() const;
/// @brief Читает позицию клетки.
    [[nodiscard]] tic_tac_toe::Position read_position();
/// @brief Читает метку клетки.
    [[nodiscard]] tic_tac_toe::Mark read_mark(bool allow_empty = false);
/// @brief Выполняет операцию класса.
    void play_full_game();
/// @brief Выполняет операцию класса.
    void play_one_turn(const std::string& line);
/// @brief Выполняет операцию класса.
    void create_board();
/// @brief Выполняет операцию класса.
    void show_board() const;
/// @brief Выполняет операцию класса.
    void check_placement();
/// @brief Выполняет операцию класса.
    void read_cell();
/// @brief Выполняет операцию класса.
    void write_cell();
/// @brief Выполняет операцию класса.
    void place_mark();
/// @brief Выполняет операцию класса.
    void check_winner() const;
/// @brief Демонстрирует семантику копирования.
    void demonstrate_copy();
/// @brief Выполняет операцию класса.
    void demonstrate_text_format();

    std::istream& input_;
    std::ostream& output_;
    Console console_;
    BoardView board_view_;
    tic_tac_toe::TicTacToe game_;
};

/// @brief Меню операций с многочленами.
class PolynomialMenu {
public:
/// @brief Создаёт объект.
    PolynomialMenu(std::istream& input, std::ostream& output);
/// @brief Запускает цикл меню.
    void run();
private:
/// @brief Выводит пункты меню.
    void show_menu() const;
/// @brief Читает имя многочлена.
    [[nodiscard]] char read_target_name(std::string_view prompt);
/// @brief Возвращает выбранный многочлен.
    [[nodiscard]] polynomial::Polynomial& target(char name);
/// @brief Задаёт выбранный многочлен.
    void set_polynomial();
/// @brief Читает многочлен в текстовом формате.
    void read_polynomial_text();
/// @brief Читает степень и коэффициенты многочлена.
    [[nodiscard]] polynomial::Polynomial read_structured_polynomial();
/// @brief Выводит оба многочлена.
    void show_polynomials() const;
/// @brief Читает коэффициент.
    void read_coefficient();
/// @brief Изменяет коэффициент.
    void write_coefficient();
/// @brief Вычисляет значение многочлена.
    void evaluate_polynomial();
/// @brief Выполняет бинарную операцию.
    void perform_binary_operation(char operation);
/// @brief Выполняет составную операцию.
    void perform_compound_operation();
/// @brief Сравнивает многочлены.
    void compare_polynomials() const;
/// @brief Демонстрирует семантику копирования.
    void demonstrate_copy();

    std::istream& input_;
    std::ostream& output_;
    Console console_;
    PolynomialView polynomial_view_;
    polynomial::Polynomial polynomial_a_;
    polynomial::Polynomial polynomial_b_;
};

/// @brief Главное приложение лабораторной работы.
class Application {
public:
/// @brief Создаёт объект.
    Application(std::istream& input, std::ostream& output);
/// @brief Запускает цикл меню.
    void run();
private:
/// @brief Выводит пункты меню.
    void show_menu() const;
/// @brief Читает пункт главного меню.
    [[nodiscard]] int read_choice();
/// @brief Выполняет операцию класса.
    void run_tic_tac_toe();
/// @brief Выполняет операцию класса.
    void run_polynomial();
    std::istream& input_;
    std::ostream& output_;
};

}  // namespace cli
