#include "cli.h"

#include "polynomial/polynomial.h"
#include "tic_tac_toe/tic_tac_toe.h"

#include <array>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <exception>
#include <functional>
#include <iomanip>
#include <ios>
#include <istream>
#include <limits>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace cli {
namespace {

constexpr int kInvalidChoice = -1;
constexpr int kMainMaxChoice = 2;
constexpr int kTicTacToeActionCount = 10;
constexpr int kPolynomialActionCount = 13;
constexpr std::string_view kInputClosedText = "Ввод завершён";

class UnknownOperationError : public std::invalid_argument {
public:
    UnknownOperationError() : std::invalid_argument("Unknown operation") {
    }
};

class InputClosedError : public std::runtime_error {
public:
    InputClosedError() : std::runtime_error(std::string(kInputClosedText)) {
    }
};

bool parse_integer(const std::string& text, int& value) {
    std::istringstream parser(text);
    if (!(parser >> value)) {
        return false;
    }
    parser >> std::ws;
    return parser.eof();
}
bool parse_double(const std::string& text, double& value) {
    std::istringstream parser(text);
    if (!(parser >> value)) {
        return false;
    }
    parser >> std::ws;
    return parser.eof() && std::isfinite(value);
}
bool parse_character(const std::string& text, char& value) {
    std::istringstream parser(text);
    if (!(parser >> value)) {
        return false;
    }
    parser >> std::ws;
    return parser.eof();
}
std::optional<tic_tac_toe::Position> parse_move(const std::string& line) {
    std::istringstream parser(line);
    int row = 0;
    int col = 0;
    std::string extra;
    if (!(parser >> row >> col) || row < 1 || col < 1 || (parser >> extra)) {
        return std::nullopt;
    }
    return tic_tac_toe::Position{static_cast<std::size_t>(row - 1), static_cast<std::size_t>(col - 1)};
}
char uppercase(char value) {
    return static_cast<char>(std::toupper(static_cast<unsigned char>(value)));
}
template <typename Error> bool is_current_exception() {
    try {
        throw;
    } catch (const Error&) {
        return true;
    } catch (...) {
        return false;
    }
}

struct ErrorRule {
    bool (*matches)();
    std::string_view message;
};

template <typename Error> constexpr ErrorRule make_rule(std::string_view message) {
    return {&is_current_exception<Error>, message};
}

constexpr std::array kErrorRules = {
    make_rule<UnknownOperationError>("Неизвестная операция. Используйте +, -, * или /."),
    make_rule<tic_tac_toe::InvalidBoardSizeError>("Недопустимый размер поля. Допустимый размер — от 3 до 10."),
    make_rule<tic_tac_toe::CellOccupiedError>("Клетка уже занята. Выберите другую клетку."),
    make_rule<tic_tac_toe::PositionOutOfRangeError>("Координаты выходят за пределы игрового поля."),
    make_rule<tic_tac_toe::InvalidMarkError>("Недопустимая метка. Используйте X, O или точку."),
    make_rule<tic_tac_toe::InvalidBoardFormatError>("Неверный формат игрового поля."),
    make_rule<polynomial::InvalidDegreeError>("Недопустимая степень многочлена. Допустимы значения от 0 до 1000."),
    make_rule<polynomial::CoefficientCountMismatchError>(
        "Количество коэффициентов не соответствует степени многочлена."),
    make_rule<polynomial::CoefficientIndexError>("Индекс коэффициента выходит за допустимые границы."),
    make_rule<polynomial::NonFiniteCoefficientError>("Коэффициент должен быть конечным числом."),
    make_rule<polynomial::DivisionByZeroPolynomialError>("Деление на нулевой многочлен невозможно."),
    make_rule<polynomial::InvalidPolynomialFormatError>(
        "Неверный формат многочлена. Проверьте степень и коэффициенты."),
    make_rule<InputClosedError>(kInputClosedText),
};

// Must be called from a catch handler: it inspects the exception being handled.
void print_error(std::ostream& output) {
    std::string_view message = "Не удалось выполнить действие. Проверьте введённые данные.";
    for (const ErrorRule& rule : kErrorRules) {
        if (rule.matches()) {
            message = rule.message;
            break;
        }
    }
    output << messages::kErrorPrefix << message << '\n';
}
int read_menu_choice(Console& console, int max_choice) {
    const std::string line = console.read_line(messages::kChoicePrompt);
    int choice = kInvalidChoice;
    if (!parse_integer(line, choice) || choice < 0 || choice > max_choice) {
        console.print_line(messages::kInvalidInput);
        return kInvalidChoice;
    }
    return choice;
}
int next_choice(Console& console, int max_choice) {
    try {
        return read_menu_choice(console, max_choice);
    } catch (const std::runtime_error&) {
        return kExitChoice;
    }
}
struct Term {
    double coefficient;
    std::size_t power;
};
void write_sign(std::ostream& output, const Term& term, bool has_previous) {
    if (has_previous) {
        output << (term.coefficient < 0.0 ? " - " : " + ");
    } else if (term.coefficient < 0.0) {
        output << '-';
    }
}
void write_magnitude(std::ostream& output, const Term& term) {
    const double magnitude = std::abs(term.coefficient);
    const bool unit = std::abs(magnitude - 1.0) <= polynomial::kCoefficientEpsilon;
    if (term.power == 0U || !unit) {
        output << magnitude;
    }
    if (term.power > 0U) {
        output << 'x';
    }
    if (term.power > 1U) {
        output << '^' << term.power;
    }
}
bool append_term(std::ostream& output, const Term& term, bool has_previous) {
    if (std::abs(term.coefficient) <= polynomial::kCoefficientEpsilon) {
        return has_previous;
    }
    write_sign(output, term, has_previous);
    write_magnitude(output, term);
    return true;
}
void require_single_line_end(std::istringstream& input) {
    input >> std::ws;
    if (!input.eof()) {
        std::string unexpected;
        input >> unexpected;
        throw polynomial::InvalidPolynomialFormatError("unexpected trailing token: '" + unexpected + "'");
    }
}
void print_game_result(std::ostream& output, const tic_tac_toe::TicTacToe& board) {
    const tic_tac_toe::Mark result = board.winner();
    if (result == tic_tac_toe::Mark::kCross) {
        output << "Победил игрок X\n";
    } else if (result == tic_tac_toe::Mark::kNought) {
        output << "Победил игрок O\n";
    } else if (board.is_draw()) {
        output << messages::kDraw << '\n';
    }
}

} // namespace

Console::Console(std::istream& input, std::ostream& output) : input_(input), output_(output) {
}
std::string Console::read_line(std::string_view prompt) {
    output_ << prompt;
    std::string line;
    if (!std::getline(input_, line)) {
        throw InputClosedError();
    }
    return line;
}
int Console::read_integer(std::string_view prompt) {
    while (true) {
        const std::string text = read_line(prompt);
        int value = 0;
        if (parse_integer(text, value)) {
            return value;
        }
        print_invalid_input();
    }
}
int Console::read_integer_in_range(std::string_view prompt, int minimum, int maximum) {
    while (true) {
        const int value = read_integer(prompt);
        if (value >= minimum && value <= maximum) {
            return value;
        }
        print_invalid_input();
    }
}
double Console::read_double(std::string_view prompt) {
    while (true) {
        const std::string text = read_line(prompt);
        double value = 0.0;
        if (parse_double(text, value)) {
            return value;
        }
        print_invalid_input();
    }
}
char Console::read_character(std::string_view prompt) {
    while (true) {
        const std::string text = read_line(prompt);
        char value = '\0';
        if (parse_character(text, value)) {
            return value;
        }
        print_invalid_input();
    }
}
void Console::print(std::string_view text) {
    output_ << text;
}
void Console::print_line(std::string_view text) {
    output_ << text << '\n';
}
void Console::print_invalid_input() {
    print_line(messages::kInvalidInput);
}

void BoardView::render(std::ostream& output, const tic_tac_toe::TicTacToe& board) {
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
void PolynomialView::render(std::ostream& output, char name, const polynomial::Polynomial& value) {
    output << name << ": " << value << ", " << algebraic(value) << '\n';
}
std::string PolynomialView::algebraic(const polynomial::Polynomial& value) {
    std::ostringstream output;
    bool has_previous = false;
    output << std::defaultfloat << std::setprecision(kDisplayPrecision);
    for (std::size_t power = value.degree() + 1U; power > 0U; --power) {
        has_previous = append_term(output, Term{value[power - 1U], power - 1U}, has_previous);
    }
    return has_previous ? output.str() : "0";
}

TicTacToeMenu::TicTacToeMenu(std::istream& input, std::ostream& output)
    : input_(input), output_(output), console_(input, output) {
}
void TicTacToeMenu::run() {
    while (input_) {
        show_menu();
        const int choice = next_choice(console_, kTicTacToeActionCount);
        if (choice == kExitChoice) {
            return;
        }
        if (choice != kInvalidChoice && !execute(choice)) {
            return;
        }
    }
}
bool TicTacToeMenu::execute(int choice) {
    try {
        dispatch(choice);
    } catch (const std::exception&) {
        if (!input_) {
            return false;
        }
        print_error(output_);
    }
    return true;
}
void TicTacToeMenu::dispatch(int choice) {
    const std::array<std::function<void()>, kTicTacToeActionCount> actions = {
        [this] { play_full_game(); },   [this] { create_board(); },
        [this] { show_board(); },       [this] { check_placement(); },
        [this] { read_cell(); },        [this] { write_cell(); },
        [this] { place_mark(); },       [this] { check_winner(); },
        [this] { demonstrate_copy(); }, [this] { demonstrate_text_format(); },
    };
    actions.at(static_cast<std::size_t>(choice - 1))();
}
void TicTacToeMenu::show_menu() const {
    output_ << messages::kTicTacToeTitle << '\n' << messages::kTicTacToeMenu;
}
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
    if (size < 0) {
        throw tic_tac_toe::InvalidBoardSizeError(0U);
    }
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
    if (size < 0) {
        throw tic_tac_toe::InvalidBoardSizeError(0U);
    }
    game_ = tic_tac_toe::TicTacToe(static_cast<std::size_t>(size));
    console_.print_line(messages::kBoardCreated);
    show_board();
}
void TicTacToeMenu::show_board() const {
    BoardView::render(output_, game_);
}
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
        output_ << messages::kErrorPrefix << "Клетка (" << position.row + 1U << ", " << position.col + 1U
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
    copy[{0U, 0U}] =
        copy[{0U, 0U}] == tic_tac_toe::Mark::kCross ? tic_tac_toe::Mark::kNought : tic_tac_toe::Mark::kCross;
    output_ << (copy != game_ ? messages::kCopyDifferent : messages::kCopyEqual) << '\n';
    console_.print_line(messages::kOriginalUnchanged);
    tic_tac_toe::TicTacToe assigned;
    assigned = game_;
    output_ << (assigned == game_ ? messages::kAssignmentEqual : messages::kCopyDifferent) << '\n';
    const tic_tac_toe::TicTacToe& same_board = assigned;
    assigned = same_board;
}
void TicTacToeMenu::demonstrate_text_format() {
    output_ << "Текущий текст поля:\n" << game_;
    console_.print("Введите текст нового поля (размер и строки): ");
    tic_tac_toe::TicTacToe parsed;
    input_ >> parsed;
    if (!input_) {
        throw tic_tac_toe::InvalidBoardFormatError("input failed");
    }
    input_.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    game_ = parsed;
    show_board();
}

void TicTacToeMenu::play_one_turn(const std::string& line) {
    const std::optional<tic_tac_toe::Position> position = parse_move(line);
    if (!position) {
        console_.print_line(messages::kMoveRejected);
        return;
    }
    const tic_tac_toe::Mark mark = game_.next_mark();
    if (!game_.can_place(*position, mark)) {
        console_.print_line(messages::kMoveRejected);
        return;
    }
    game_.place(*position, mark);
    show_board();
}

PolynomialMenu::PolynomialMenu(std::istream& input, std::ostream& output)
    : input_(input), output_(output), console_(input, output) {
}
void PolynomialMenu::run() {
    while (input_) {
        show_menu();
        const int choice = next_choice(console_, kPolynomialActionCount);
        if (choice == kExitChoice) {
            return;
        }
        if (choice != kInvalidChoice && !execute(choice)) {
            return;
        }
    }
}
bool PolynomialMenu::execute(int choice) {
    try {
        dispatch(choice);
    } catch (const std::exception&) {
        if (!input_) {
            return false;
        }
        print_error(output_);
    }
    return true;
}
void PolynomialMenu::dispatch(int choice) {
    const std::array<std::function<void()>, kPolynomialActionCount> actions = {
        [this] { set_polynomial(); },
        [this] { read_polynomial_text(); },
        [this] { show_polynomials(); },
        [this] { read_coefficient(); },
        [this] { write_coefficient(); },
        [this] { evaluate_polynomial(); },
        [this] { perform_binary_operation('+'); },
        [this] { perform_binary_operation('-'); },
        [this] { perform_binary_operation('*'); },
        [this] { perform_binary_operation('/'); },
        [this] { perform_compound_operation(); },
        [this] { compare_polynomials(); },
        [this] { demonstrate_copy(); },
    };
    actions.at(static_cast<std::size_t>(choice - 1))();
}
void PolynomialMenu::show_menu() const {
    output_ << messages::kPolynomialTitle << '\n' << messages::kPolynomialMenu;
}
char PolynomialMenu::read_target_name(std::string_view prompt) {
    while (true) {
        const char name = uppercase(console_.read_character(prompt));
        if (name == 'A' || name == 'B') {
            return name;
        }
        console_.print_line(messages::kInvalidInput);
    }
}
polynomial::Polynomial& PolynomialMenu::target(char name) {
    if (name == 'A') {
        return polynomial_a_;
    }
    if (name == 'B') {
        return polynomial_b_;
    }
    throw std::invalid_argument("Unknown polynomial name");
}
void PolynomialMenu::set_polynomial() {
    const char name = read_target_name(messages::kTargetPrompt);
    const polynomial::Polynomial parsed = read_structured_polynomial();
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
    PolynomialView::render(output_, name, target(name));
}
void PolynomialMenu::show_polynomials() const {
    PolynomialView::render(output_, 'A', polynomial_a_);
    PolynomialView::render(output_, 'B', polynomial_b_);
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
    PolynomialView::render(output_, name, target(name));
}
void PolynomialMenu::evaluate_polynomial() {
    const char name = read_target_name(messages::kTargetPrompt);
    const double point = console_.read_double(messages::kArgumentPrompt);
    output_ << "p(" << point << ") = " << target(name)(point) << '\n';
}
void PolynomialMenu::perform_binary_operation(char operation) {
    polynomial::Polynomial result;
    switch (operation) {
    case '+':
        result = polynomial_a_ + polynomial_b_;
        break;
    case '-':
        result = polynomial_a_ - polynomial_b_;
        break;
    case '*':
        result = polynomial_a_ * polynomial_b_;
        break;
    case '/':
        result = polynomial_a_ / polynomial_b_;
        console_.print_line(messages::kRemainderDiscarded);
        break;
    default:
        throw UnknownOperationError();
    }
    output_ << result << ", " << PolynomialView::algebraic(result) << '\n';
}
void PolynomialMenu::perform_compound_operation() {
    const char operation = console_.read_character(messages::kOperationPrompt);
    switch (operation) {
    case '+':
        polynomial_a_ += polynomial_b_;
        break;
    case '-':
        polynomial_a_ -= polynomial_b_;
        break;
    case '*':
        polynomial_a_ *= polynomial_b_;
        break;
    case '/':
        polynomial_a_ /= polynomial_b_;
        console_.print_line(messages::kRemainderDiscarded);
        break;
    default:
        throw UnknownOperationError();
    }
    PolynomialView::render(output_, 'A', polynomial_a_);
}
void PolynomialMenu::compare_polynomials() const {
    output_ << (polynomial_a_ == polynomial_b_ ? messages::kEqual : messages::kNotEqual) << '\n';
}
void PolynomialMenu::demonstrate_copy() {
    polynomial::Polynomial copy(polynomial_a_);
    output_ << (copy == polynomial_a_ ? "Копия равна A\n" : "Копия не равна A\n");
    copy[0U] += 1.0;
    output_ << (copy == polynomial_a_ ? "A не изменился\n" : "Копия изменена; A не изменился\n");
    polynomial_b_ = copy;
    const polynomial::Polynomial& same_polynomial = polynomial_b_;
    polynomial_b_ = same_polynomial;
    output_ << (polynomial_b_ == copy ? "Присваивание и самоприсваивание выполнены\n" : "Ошибка копирования\n");
}

Application::Application(std::istream& input, std::ostream& output) : input_(input), output_(output) {
}
void Application::run() {
    output_ << messages::kApplicationTitle << '\n';
    bool running = true;
    while (running && input_) {
        running = step();
    }
    output_ << messages::kGoodbye << '\n';
}
bool Application::step() {
    show_menu();
    int choice = kInvalidChoice;
    try {
        choice = read_choice();
    } catch (const std::runtime_error&) {
        return false;
    }
    if (choice < 0 || choice > kMainMaxChoice) {
        output_ << messages::kInvalidInput << '\n';
        return true;
    }
    return choice != kExitChoice && run_choice(choice);
}
bool Application::run_choice(int choice) {
    try {
        if (choice == 1) {
            run_tic_tac_toe();
        } else {
            run_polynomial();
        }
    } catch (const std::exception&) {
        print_error(output_);
        return static_cast<bool>(input_);
    }
    return true;
}
void Application::show_menu() const {
    output_ << messages::kMainMenu;
}
int Application::read_choice() {
    Console console(input_, output_);
    const std::string line = console.read_line(messages::kChoicePrompt);
    int choice = kInvalidChoice;
    if (!parse_integer(line, choice)) {
        return kInvalidChoice;
    }
    return choice;
}
void Application::run_tic_tac_toe() {
    TicTacToeMenu menu(input_, output_);
    menu.run();
}
void Application::run_polynomial() {
    PolynomialMenu menu(input_, output_);
    menu.run();
}

} // namespace cli
