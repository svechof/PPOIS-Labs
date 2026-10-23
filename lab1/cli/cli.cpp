#include "cli.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <istream>
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace cli {
namespace {

bool parse_integer(const std::string& text, int& value) {
    std::istringstream parser(text);
    if (!(parser >> value)) { return false; }
    parser >> std::ws;
    return parser.eof();
}
bool parse_double(const std::string& text, double& value) {
    std::istringstream parser(text);
    if (!(parser >> value)) { return false; }
    parser >> std::ws;
    return parser.eof() && std::isfinite(value);
}
char uppercase(char value) { return static_cast<char>(std::toupper(static_cast<unsigned char>(value))); }
void print_error(std::ostream& output, const std::exception& error) {
    const std::string details = error.what();
    std::string message = "Не удалось выполнить действие. Проверьте введённые данные.";

    if (details.find("Unknown operation") != std::string::npos) {
        message = "Неизвестная операция. Используйте +, -, * или /.";
    } else if (details.find("Invalid board size:") != std::string::npos) {
        message = "Недопустимый размер поля. Допустимый размер — от 3 до 10.";
    } else if (details.find("Cell is already occupied:") != std::string::npos) {
        message = "Клетка уже занята. Выберите другую клетку.";
    } else if (details.find("Position out of range:") != std::string::npos) {
        message = "Координаты выходят за пределы игрового поля.";
    } else if (details.find("Invalid mark:") != std::string::npos) {
        message = "Недопустимая метка. Используйте X, O или точку.";
    } else if (details.find("Invalid board format:") != std::string::npos) {
        message = "Неверный формат игрового поля.";
    } else if (details.find("Invalid degree:") != std::string::npos ||
               details.find("Invalid polynomial degree:") != std::string::npos) {
        message = "Недопустимая степень многочлена. Допустимы значения от 0 до 1000.";
    } else if (details.find("Coefficient count mismatch:") != std::string::npos ||
               details.find("Invalid coefficient count:") != std::string::npos) {
        message = "Количество коэффициентов не соответствует степени многочлена.";
    } else if (details.find("Coefficient index out of range:") != std::string::npos) {
        message = "Индекс коэффициента выходит за допустимые границы.";
    } else if (details.find("Non-finite coefficient") != std::string::npos) {
        message = "Коэффициент должен быть конечным числом.";
    } else if (details.find("Division by zero polynomial") != std::string::npos) {
        message = "Деление на нулевой многочлен невозможно.";
    } else if (details.find("Invalid polynomial format:") != std::string::npos) {
        message = "Неверный формат многочлена. Проверьте степень и коэффициенты.";
    } else if (details == "Ввод завершён") {
        message = details;
    }

    output << messages::kErrorPrefix << message << '\n';
}
void require_single_line_end(std::istringstream& input) {
    input >> std::ws;
    if (!input.eof()) {
        std::string unexpected;
        input >> unexpected;
        throw polynomial::InvalidPolynomialFormatError(
            "unexpected trailing token: '" + unexpected + "'");
    }
}
void print_game_result(std::ostream& output, const tic_tac_toe::TicTacToe& board) {
    const tic_tac_toe::Mark result = board.winner();
    if (result == tic_tac_toe::Mark::kCross) { output << "Победил игрок X\n"; }
    else if (result == tic_tac_toe::Mark::kNought) { output << "Победил игрок O\n"; }
    else if (board.is_draw()) { output << messages::kDraw << '\n'; }
}

}  // namespace

Console::Console(std::istream& input, std::ostream& output) : input_(input), output_(output) {}
std::string Console::read_line(std::string_view prompt) {
    output_ << prompt;
    std::string line;
    if (!std::getline(input_, line)) { throw std::runtime_error("Ввод завершён"); }
    return line;
}
int Console::read_integer(std::string_view prompt) {
    while (true) {
        const std::string text = read_line(prompt);
        int value = 0;
        if (parse_integer(text, value)) { return value; }
        print_invalid_input();
    }
}
int Console::read_integer_in_range(std::string_view prompt, int minimum, int maximum) {
    while (true) {
        const int value = read_integer(prompt);
        if (value >= minimum && value <= maximum) { return value; }
        print_invalid_input();
    }
}
double Console::read_double(std::string_view prompt) {
    while (true) {
        const std::string text = read_line(prompt);
        double value = 0.0;
        if (parse_double(text, value)) { return value; }
        print_invalid_input();
    }
}
char Console::read_character(std::string_view prompt) {
    while (true) {
        const std::string text = read_line(prompt);
        std::istringstream parser(text);
        char value = '\0';
        if (parser >> value) {
            parser >> std::ws;
            if (parser.eof()) { return value; }
        }
        print_invalid_input();
    }
}
void Console::print(std::string_view text) { output_ << text; }
void Console::print_line(std::string_view text) { output_ << text << '\n'; }
void Console::print_invalid_input() { print_line(messages::kInvalidInput); }

void BoardView::render(std::ostream& output, const tic_tac_toe::TicTacToe& board) const {
    output << std::setw(kIndexWidth) << ' ';
    for (std::size_t col = 0; col < board.size(); ++col) {
        output << std::setw(kCellWidth) << col + 1U;
    }
    output << '\n';
    for (std::size_t row = 0; row < board.size(); ++row) {
        output << std::setw(kIndexWidth) << row + 1U;
        for (std::size_t col = 0; col < board.size(); ++col) {
            output << std::setw(kCellWidth) << tic_tac_toe::mark_to_char(board[{row, col}]);
        }
        output << '\n';
    }
}
void PolynomialView::render(std::ostream& output, char name, const polynomial::Polynomial& value) const {
    output << name << ": " << value << ", " << algebraic(value) << '\n';
}
std::string PolynomialView::algebraic(const polynomial::Polynomial& value) const {
    std::ostringstream output;
    bool has_previous = false;
    const auto old_flags = output.flags();
    const std::streamsize old_precision = output.precision();
    output << std::defaultfloat << std::setprecision(kDisplayPrecision);
    for (std::size_t power = value.degree() + 1U; power > 0U; --power) {
        append_term(output, value, power - 1U, has_previous);
    }
    output.flags(old_flags);
    output.precision(old_precision);
    return has_previous ? output.str() : "0";
}
void PolynomialView::append_term(std::ostream& output, const polynomial::Polynomial& value,
                                 std::size_t power, bool& has_previous) const {
    const double coefficient = value[power];
    if (std::abs(coefficient) <= polynomial::kCoefficientEpsilon) { return; }
    if (has_previous) { output << (coefficient < 0.0 ? " - " : " + "); }
    else if (coefficient < 0.0) { output << '-'; }
    const double magnitude = std::abs(coefficient);
    const bool variable = power > 0U;
    const bool unit = std::abs(magnitude - 1.0) <= polynomial::kCoefficientEpsilon;
    if (!variable || !unit) { output << magnitude; }
    if (variable) { output << 'x'; }
    if (power > 1U) { output << '^' << power; }
    has_previous = true;
}

TicTacToeMenu::TicTacToeMenu(std::istream& input, std::ostream& output)
    : input_(input), output_(output), console_(input, output), board_view_(), game_() {}
void TicTacToeMenu::run() {
    while (input_) {
        show_menu();
        int choice = -1;
        try {
            const std::string line = console_.read_line(messages::kChoicePrompt);
            if (!parse_integer(line, choice) || choice < 0 || choice > 10) {
                console_.print_line(messages::kInvalidInput);
                continue;
            }
        } catch (const std::runtime_error&) {
            return;
        }
        if (choice == 0) { return; }
        try {
            switch (choice) {
                case 1: play_full_game(); break;
                case 2: create_board(); break;
                case 3: show_board(); break;
                case 4: check_placement(); break;
                case 5: read_cell(); break;
                case 6: write_cell(); break;
                case 7: place_mark(); break;
                case 8: check_winner(); break;
                case 9: demonstrate_copy(); break;
                case 10: demonstrate_text_format(); break;
                default: console_.print_line(messages::kInvalidInput); break;
            }
        } catch (const std::exception& error) {
            if (!input_) { return; }
            print_error(output_, error);
        }
    }
}
void TicTacToeMenu::show_menu() const { output_ << messages::kTicTacToeTitle << '\n' << messages::kTicTacToeMenu; }
tic_tac_toe::Position TicTacToeMenu::read_position() {
    const int row = console_.read_integer(messages::kRowPrompt);
    const int col = console_.read_integer(messages::kColumnPrompt);
    const std::size_t row_index = row < 1 ? game_.size() : static_cast<std::size_t>(row - 1);
    const std::size_t col_index = col < 1 ? game_.size() : static_cast<std::size_t>(col - 1);
    if (row < 1 || col < 1 || row_index >= game_.size() || col_index >= game_.size()) {
        throw tic_tac_toe::PositionOutOfRangeError({row_index, col_index}, game_.size());
    }
    return {row_index, col_index};
}
tic_tac_toe::Mark TicTacToeMenu::read_mark(bool allow_empty) {
    const std::string_view prompt = allow_empty ? messages::kAnyMarkPrompt : messages::kMarkPrompt;
    const char symbol = uppercase(console_.read_character(prompt));
    const tic_tac_toe::Mark mark = tic_tac_toe::mark_from_char(symbol);
    if (!allow_empty && mark == tic_tac_toe::Mark::kEmpty) {
        throw tic_tac_toe::InvalidMarkError(symbol);
    }
    return mark;
}
void TicTacToeMenu::play_full_game() {
    const int size = console_.read_integer(messages::kBoardSizePrompt);
    if (size < 0) { throw tic_tac_toe::InvalidBoardSizeError(0U); }
    game_ = tic_tac_toe::TicTacToe(static_cast<std::size_t>(size));
    show_board();
    while (input_) {
        const std::string line = console_.read_line(messages::kMovePrompt);
        if (uppercase(line.empty() ? '\0' : line.front()) == 'Q') {
            console_.print_line(messages::kGameInterrupted);
            return;
        }
        play_one_turn(line);
        if (game_.winner() != tic_tac_toe::Mark::kEmpty || game_.is_draw()) {
            print_game_result(output_, game_);
            return;
        }
    }
}
void TicTacToeMenu::create_board() {
    const int size = console_.read_integer(messages::kBoardSizePrompt);
    if (size < 0) { throw tic_tac_toe::InvalidBoardSizeError(0U); }
    game_ = tic_tac_toe::TicTacToe(static_cast<std::size_t>(size));
    console_.print_line(messages::kBoardCreated);
    show_board();
}
void TicTacToeMenu::show_board() const { board_view_.render(output_, game_); }
void TicTacToeMenu::check_placement() {
    const tic_tac_toe::Position position = read_position();
    const tic_tac_toe::Mark mark = read_mark();
    console_.print_line(game_.can_place(position, mark) ? messages::kMovePossible : messages::kMoveImpossible);
}
void TicTacToeMenu::read_cell() {
    const tic_tac_toe::Position position = read_position();
    output_ << tic_tac_toe::mark_to_char(game_[position]) << '\n';
}
void TicTacToeMenu::write_cell() {
    const tic_tac_toe::Position position = read_position();
    game_[position] = read_mark(true);
    show_board();
}
void TicTacToeMenu::place_mark() {
    const tic_tac_toe::Position position = read_position();
    const tic_tac_toe::Mark mark = read_mark();

    if (!game_.can_place(position, mark)) {
        output_ << messages::kErrorPrefix
                << "Клетка (" << position.row + 1U
                << ", " << position.col + 1U
                << ") уже занята.\n";
        return;
    }

    game_.place(position, mark);
    show_board();
}
void TicTacToeMenu::check_winner() const {
    print_game_result(output_, game_);
    if (game_.winner() == tic_tac_toe::Mark::kEmpty && !game_.is_draw()) {
        output_ << messages::kNoWinner << '\n';
    }
    output_ << (game_.is_full() ? "Поле заполнено\n" : "Поле не заполнено\n");
}
void TicTacToeMenu::demonstrate_copy() {
    tic_tac_toe::TicTacToe copy(game_);
    output_ << (copy == game_ ? messages::kCopyEqual : messages::kCopyDifferent) << '\n';
    copy[{0U, 0U}] = copy[{0U, 0U}] == tic_tac_toe::Mark::kCross
                          ? tic_tac_toe::Mark::kNought : tic_tac_toe::Mark::kCross;
    output_ << (copy != game_ ? messages::kCopyDifferent : messages::kCopyEqual) << '\n';
    console_.print_line(messages::kOriginalUnchanged);
    tic_tac_toe::TicTacToe assigned;
    assigned = game_;
    output_ << (assigned == game_ ? messages::kAssignmentEqual : messages::kCopyDifferent) << '\n';
    tic_tac_toe::TicTacToe& same_board = assigned;
    assigned = same_board;
}
void TicTacToeMenu::demonstrate_text_format() {
    output_ << "Текущий текст поля:\n" << game_;
    console_.print("Введите текст нового поля (размер и строки): ");
    tic_tac_toe::TicTacToe parsed;
    input_ >> parsed;
    if (!input_) { throw tic_tac_toe::InvalidBoardFormatError("input failed"); }
    input_.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    game_ = parsed;
    show_board();
}

void TicTacToeMenu::play_one_turn(const std::string& line) {
    std::istringstream parser(line);
    int row = 0;
    int col = 0;
    if (!(parser >> row >> col) || row < 1 || col < 1) {
        console_.print_line(messages::kMoveRejected);
        return;
    }
    std::string extra;
    if (parser >> extra) {
        console_.print_line(messages::kMoveRejected);
        return;
    }
    const tic_tac_toe::Position position{static_cast<std::size_t>(row - 1), static_cast<std::size_t>(col - 1)};
    const tic_tac_toe::Mark mark = game_.next_mark();
    if (!game_.can_place(position, mark)) {
        console_.print_line(messages::kMoveRejected);
        return;
    }
    game_.place(position, mark);
    show_board();
}

PolynomialMenu::PolynomialMenu(std::istream& input, std::ostream& output)
    : input_(input), output_(output), console_(input, output), polynomial_view_(), polynomial_a_(), polynomial_b_() {}
void PolynomialMenu::run() {
    while (input_) {
        show_menu();
        int choice = -1;
        try {
            const std::string line = console_.read_line(messages::kChoicePrompt);
            if (!parse_integer(line, choice) || choice < 0 || choice > 13) {
                console_.print_line(messages::kInvalidInput);
                continue;
            }
        } catch (const std::runtime_error&) {
            return;
        }
        if (choice == 0) { return; }
        try {
            switch (choice) {
                case 1: set_polynomial(); break;
                case 2: read_polynomial_text(); break;
                case 3: show_polynomials(); break;
                case 4: read_coefficient(); break;
                case 5: write_coefficient(); break;
                case 6: evaluate_polynomial(); break;
                case 7: perform_binary_operation('+'); break;
                case 8: perform_binary_operation('-'); break;
                case 9: perform_binary_operation('*'); break;
                case 10: perform_binary_operation('/'); break;
                case 11: perform_compound_operation(); break;
                case 12: compare_polynomials(); break;
                case 13: demonstrate_copy(); break;
                default: console_.print_line(messages::kInvalidInput); break;
            }
        } catch (const std::exception& error) {
            if (!input_) { return; }
            print_error(output_, error);
        }
    }
}
void PolynomialMenu::show_menu() const { output_ << messages::kPolynomialTitle << '\n' << messages::kPolynomialMenu; }
char PolynomialMenu::read_target_name(std::string_view prompt) {
    while (true) {
        const char name = uppercase(console_.read_character(prompt));
        if (name == 'A' || name == 'B') { return name; }
        console_.print_line(messages::kInvalidInput);
    }
}
polynomial::Polynomial& PolynomialMenu::target(char name) {
    if (name == 'A') { return polynomial_a_; }
    if (name == 'B') { return polynomial_b_; }
    throw std::invalid_argument("Unknown polynomial name");
}
void PolynomialMenu::set_polynomial() {
    const char name = read_target_name(messages::kTargetPrompt);
    polynomial::Polynomial parsed = read_structured_polynomial();
    target(name) = parsed;
    show_polynomials();
}
polynomial::Polynomial PolynomialMenu::read_structured_polynomial() {
    const int degree = console_.read_integer(messages::kDegreePrompt);
    const std::string line = console_.read_line(messages::kCoefficientsPrompt);
    std::istringstream parser(std::to_string(degree) + " " + line);
    polynomial::Polynomial parsed;
    parser >> parsed;
    require_single_line_end(parser);
    return parsed;
}
void PolynomialMenu::read_polynomial_text() {
    const char name = read_target_name(messages::kTargetPrompt);
    const std::string line = console_.read_line(messages::kPolynomialLinePrompt);
    std::istringstream parser(line);
    polynomial::Polynomial parsed;
    parser >> parsed;
    require_single_line_end(parser);
    target(name) = parsed;
    polynomial_view_.render(output_, name, target(name));
}
void PolynomialMenu::show_polynomials() const {
    polynomial_view_.render(output_, 'A', polynomial_a_);
    polynomial_view_.render(output_, 'B', polynomial_b_);
}
void PolynomialMenu::read_coefficient() {
    const char name = read_target_name(messages::kTargetPrompt);
    const int index = console_.read_integer(messages::kIndexPrompt);
    if (index < 0) {
        throw polynomial::CoefficientIndexError(std::numeric_limits<std::size_t>::max(), target(name).degree());
    }
    output_ << target(name)[static_cast<std::size_t>(index)] << '\n';
}
void PolynomialMenu::write_coefficient() {
    const char name = read_target_name(messages::kTargetPrompt);
    const int index = console_.read_integer(messages::kIndexPrompt);
    if (index < 0) {
        throw polynomial::CoefficientIndexError(std::numeric_limits<std::size_t>::max(), target(name).degree());
    }
    const double coefficient = console_.read_double(messages::kCoefficientPrompt);
    target(name)[static_cast<std::size_t>(index)] = coefficient;
    polynomial_view_.render(output_, name, target(name));
}
void PolynomialMenu::evaluate_polynomial() {
    const char name = read_target_name(messages::kTargetPrompt);
    const double point = console_.read_double(messages::kArgumentPrompt);
    output_ << "p(" << point << ") = " << target(name)(point) << '\n';
}
void PolynomialMenu::perform_binary_operation(char operation) {
    polynomial::Polynomial result;
    switch (operation) {
        case '+': result = polynomial_a_ + polynomial_b_; break;
        case '-': result = polynomial_a_ - polynomial_b_; break;
        case '*': result = polynomial_a_ * polynomial_b_; break;
        case '/': result = polynomial_a_ / polynomial_b_; console_.print_line(messages::kRemainderDiscarded); break;
        default: throw std::invalid_argument("Unknown operation");
    }
    output_ << result << ", " << polynomial_view_.algebraic(result) << '\n';
}
void PolynomialMenu::perform_compound_operation() {
    const char operation = console_.read_character(messages::kOperationPrompt);
    switch (operation) {
        case '+': polynomial_a_ += polynomial_b_; break;
        case '-': polynomial_a_ -= polynomial_b_; break;
        case '*': polynomial_a_ *= polynomial_b_; break;
        case '/': polynomial_a_ /= polynomial_b_; console_.print_line(messages::kRemainderDiscarded); break;
        default: throw std::invalid_argument("Unknown operation");
    }
    polynomial_view_.render(output_, 'A', polynomial_a_);
}
void PolynomialMenu::compare_polynomials() const {
    output_ << (polynomial_a_ == polynomial_b_ ? messages::kEqual : messages::kNotEqual) << '\n';
}
void PolynomialMenu::demonstrate_copy() {
    polynomial::Polynomial copy(polynomial_a_);
    output_ << (copy == polynomial_a_ ? "Копия равна A\n" : "Копия не равна A\n");
    copy[0U] += 1.0;
    output_ << (copy == polynomial_a_ ? "A не изменился\n" :
                "Копия изменена; A не изменился\n");
    polynomial_b_ = copy;
    polynomial::Polynomial& same_polynomial = polynomial_b_;
    polynomial_b_ = same_polynomial;
    output_ << (polynomial_b_ == copy ? "Присваивание и самоприсваивание выполнены\n" :
                "Ошибка копирования\n");
}

Application::Application(std::istream& input, std::ostream& output) : input_(input), output_(output) {}
void Application::run() {
    output_ << messages::kApplicationTitle << '\n';
    while (input_) {
        show_menu();
        int choice = 0;
        try { choice = read_choice(); }
        catch (const std::runtime_error&) { output_ << messages::kGoodbye << '\n'; return; }
        if (choice < 0 || choice > 2) {
            output_ << messages::kInvalidInput << '\n';
            continue;
        }
        try {
            if (choice == 0) { output_ << messages::kGoodbye << '\n'; return; }
            if (choice == 1) { run_tic_tac_toe(); }
            else if (choice == 2) { run_polynomial(); }
        } catch (const std::exception& error) {
            print_error(output_, error);
            if (!input_) { output_ << messages::kGoodbye << '\n'; return; }
        }
    }
    output_ << messages::kGoodbye << '\n';
}
void Application::show_menu() const { output_ << messages::kMainMenu; }
int Application::read_choice() {
    Console console(input_, output_);
    const std::string line = console.read_line(messages::kChoicePrompt);
    int choice = -1;
    if (!parse_integer(line, choice)) { return -1; }
    return choice;
}
void Application::run_tic_tac_toe() { TicTacToeMenu menu(input_, output_); menu.run(); }
void Application::run_polynomial() { PolynomialMenu menu(input_, output_); menu.run(); }

}  // namespace cli
