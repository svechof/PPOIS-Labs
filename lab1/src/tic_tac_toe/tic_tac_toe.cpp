#include "tic_tac_toe/tic_tac_toe.h"

#include <algorithm>
#include <cstddef>
#include <istream>
#include <ostream>
#include <string>
#include <utility>

namespace tic_tac_toe {
namespace {
constexpr std::size_t kMaxSizeDigits = 18;

std::string format_position(Position position) {
    return "(" + std::to_string(position.row) + ", " + std::to_string(position.col) + ")";
}
std::size_t validated_size(std::size_t size) {
    if (size < kMinBoardSize || size > kMaxBoardSize) {
        throw InvalidBoardSizeError(size);
    }
    return size;
}
void require_real_mark(Mark mark) {
    if (mark != Mark::kCross && mark != Mark::kNought) {
        throw InvalidMarkError(mark_to_char(mark));
    }
}
std::string make_size_message(std::size_t size) {
    return "Invalid board size: " + std::to_string(size) + " (allowed: " +
           std::to_string(kMinBoardSize) + ".." + std::to_string(kMaxBoardSize) + ")";
}
bool is_digit(char symbol) { return symbol >= '0' && symbol <= '9'; }
bool is_size_token(const std::string& token) {
    return !token.empty() && token.size() <= kMaxSizeDigits &&
           std::all_of(token.begin(), token.end(), is_digit);
}
std::size_t read_size(std::istream& in) {
    std::string token;
    if (!(in >> token)) {
        throw InvalidBoardFormatError::size_missing();
    }
    if (!is_size_token(token)) {
        throw InvalidBoardFormatError::size_not_integer(token);
    }
    return static_cast<std::size_t>(std::stoull(token));
}
void read_row(std::istream& in, TicTacToe& board, std::size_t row) {
    std::string text;
    if (!(in >> text)) {
        throw InvalidBoardFormatError::row_missing(row + 1U);
    }
    if (text.size() != board.size()) {
        throw InvalidBoardFormatError::row_has_wrong_length(row + 1U, text.size(), board.size());
    }
    for (std::size_t col = 0; col < text.size(); ++col) {
        board[{row, col}] = mark_from_char(text[col]);
    }
}
}  // namespace

char mark_to_char(Mark mark) noexcept { return static_cast<char>(mark); }
Mark mark_from_char(char symbol) {
    for (const Mark mark : {Mark::kEmpty, Mark::kCross, Mark::kNought}) {
        if (mark_to_char(mark) == symbol) {
            return mark;
        }
    }
    throw InvalidMarkError(symbol);
}
TicTacToeError::TicTacToeError(const std::string& message) : std::runtime_error(message) {}
InvalidBoardSizeError::InvalidBoardSizeError(std::size_t size) : TicTacToeError(make_size_message(size)) {}
PositionOutOfRangeError::PositionOutOfRangeError(Position position, std::size_t board_size)
    : TicTacToeError("Position out of range: " + format_position(position) +
                     " for board size " + std::to_string(board_size)) {}
CellOccupiedError::CellOccupiedError(Position position)
    : TicTacToeError("Cell is already occupied: " + format_position(position)) {}
InvalidMarkError::InvalidMarkError(char symbol)
    : TicTacToeError(std::string("Invalid mark: '") + symbol + "'") {}
InvalidBoardFormatError::InvalidBoardFormatError(const std::string& details)
    : TicTacToeError("Invalid board format: " + details) {}
InvalidBoardFormatError InvalidBoardFormatError::size_missing() {
    return InvalidBoardFormatError("size is missing");
}
InvalidBoardFormatError InvalidBoardFormatError::size_not_integer(const std::string& token) {
    return InvalidBoardFormatError("size is not an integer: '" + token + "'");
}
InvalidBoardFormatError InvalidBoardFormatError::row_missing(std::size_t row_number) {
    return InvalidBoardFormatError("row " + std::to_string(row_number) + " is missing");
}
InvalidBoardFormatError InvalidBoardFormatError::row_has_wrong_length(
    std::size_t row_number, std::size_t actual_length, std::size_t expected_length) {
    return InvalidBoardFormatError("row " + std::to_string(row_number) + " has length " +
                                   std::to_string(actual_length) + ", expected " +
                                   std::to_string(expected_length));
}

TicTacToe::TicTacToe() : TicTacToe(kDefaultBoardSize) {}
TicTacToe::TicTacToe(std::size_t size) : size_(validated_size(size)), cells_(new Mark[size_ * size_]) {
    std::fill(cells_, cells_ + size_ * size_, Mark::kEmpty);
}
TicTacToe::TicTacToe(const TicTacToe& other)
    : size_(other.size_), cells_(new Mark[other.size_ * other.size_]) {
    std::copy(other.cells_, other.cells_ + size_ * size_, cells_);
}
TicTacToe::~TicTacToe() noexcept { delete[] cells_; }
TicTacToe& TicTacToe::operator=(const TicTacToe& other) {
    if (this != &other) {
        TicTacToe copy(other);
        swap(copy);
    }
    return *this;
}
bool TicTacToe::operator==(const TicTacToe& other) const noexcept {
    return size_ == other.size_ && std::equal(cells_, cells_ + size_ * size_, other.cells_);
}
bool TicTacToe::operator!=(const TicTacToe& other) const noexcept { return !(*this == other); }
std::size_t TicTacToe::size() const noexcept { return size_; }
const Mark& TicTacToe::operator[](Position position) const { return cells_[index_of(position)]; }
Mark& TicTacToe::operator[](Position position) { return cells_[index_of(position)]; }
bool TicTacToe::can_place(Position position, Mark mark) const noexcept {
    const bool real_mark = mark == Mark::kCross || mark == Mark::kNought;
    return real_mark && position.row < size_ && position.col < size_ &&
           cells_[position.row * size_ + position.col] == Mark::kEmpty;
}
void TicTacToe::place(Position position, Mark mark) {
    require_real_mark(mark);
    Mark& cell = (*this)[position];
    if (cell != Mark::kEmpty) {
        throw CellOccupiedError(position);
    }
    cell = mark;
}
bool TicTacToe::has_won(Mark mark) const {
    require_real_mark(mark);
    return has_full_line(mark);
}
Mark TicTacToe::winner() const noexcept {
    if (has_full_line(Mark::kCross)) { return Mark::kCross; }
    if (has_full_line(Mark::kNought)) { return Mark::kNought; }
    return Mark::kEmpty;
}
bool TicTacToe::is_full() const noexcept { return count(Mark::kEmpty) == 0U; }
bool TicTacToe::is_draw() const noexcept { return is_full() && winner() == Mark::kEmpty; }
std::size_t TicTacToe::count(Mark mark) const noexcept {
    return static_cast<std::size_t>(std::count(cells_, cells_ + size_ * size_, mark));
}
Mark TicTacToe::next_mark() const noexcept {
    return count(Mark::kCross) > count(Mark::kNought) ? Mark::kNought : Mark::kCross;
}
std::size_t TicTacToe::index_of(Position position) const {
    if (position.row >= size_ || position.col >= size_) {
        throw PositionOutOfRangeError(position, size_);
    }
    return position.row * size_ + position.col;
}
void TicTacToe::swap(TicTacToe& other) noexcept {
    std::swap(size_, other.size_);
    std::swap(cells_, other.cells_);
}
bool TicTacToe::is_row_owned_by(Mark mark, std::size_t row) const noexcept {
    const Mark* begin = cells_ + row * size_;
    return std::all_of(begin, begin + size_, [mark](Mark cell) { return cell == mark; });
}
bool TicTacToe::is_column_owned_by(Mark mark, std::size_t col) const noexcept {
    for (std::size_t row = 0; row < size_; ++row) {
        if (cells_[row * size_ + col] != mark) { return false; }
    }
    return true;
}
bool TicTacToe::is_main_diagonal_owned_by(Mark mark) const noexcept {
    for (std::size_t index = 0; index < size_; ++index) {
        if (cells_[index * size_ + index] != mark) { return false; }
    }
    return true;
}
bool TicTacToe::is_anti_diagonal_owned_by(Mark mark) const noexcept {
    for (std::size_t index = 0; index < size_; ++index) {
        if (cells_[index * size_ + size_ - 1U - index] != mark) { return false; }
    }
    return true;
}
bool TicTacToe::has_full_line(Mark mark) const noexcept {
    for (std::size_t index = 0; index < size_; ++index) {
        if (is_row_owned_by(mark, index) || is_column_owned_by(mark, index)) { return true; }
    }
    return is_main_diagonal_owned_by(mark) || is_anti_diagonal_owned_by(mark);
}
std::ostream& operator<<(std::ostream& out, const TicTacToe& board) {
    out << board.size() << '\n';
    for (std::size_t row = 0; row < board.size(); ++row) {
        for (std::size_t col = 0; col < board.size(); ++col) { out << mark_to_char(board[{row, col}]); }
        out << '\n';
    }
    return out;
}
std::istream& operator>>(std::istream& in, TicTacToe& board) {
    TicTacToe parsed(read_size(in));
    for (std::size_t row = 0; row < parsed.size(); ++row) { read_row(in, parsed, row); }
    board = parsed;
    return in;
}
}  // namespace tic_tac_toe
