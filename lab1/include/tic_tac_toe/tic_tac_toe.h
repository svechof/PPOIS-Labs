#pragma once

#include <cstddef>
#include <iosfwd>
#include <stdexcept>
#include <string>

/// @defgroup tic_tac_toe Крестики-нолики
/// @brief Игровое поле от 3×3 до 10×10, правила, исключения и текстовый формат.
/// @details Игровой домен не зависит от пользовательского интерфейса.
/// @{
namespace tic_tac_toe {

/// @brief Минимально допустимая сторона игрового поля.
inline constexpr std::size_t kMinBoardSize = 3;
/// @brief Максимально допустимая сторона игрового поля.
inline constexpr std::size_t kMaxBoardSize = 10;
/// @brief Размер поля, создаваемого конструктором по умолчанию.
inline constexpr std::size_t kDefaultBoardSize = 3;

/// @brief Содержимое одной клетки игрового поля.
/// @details Значения перечисления совпадают с символами текстового формата.
enum class Mark : char {
    kEmpty = '.',  ///< Клетка свободна.
    kCross = 'X',  ///< В клетке стоит крестик.
    kNought = 'O'  ///< В клетке стоит нолик.
};

/// @brief Преобразует метку в символ текстового формата.
/// @param mark Преобразуемая метка.
/// @return `'.'`, `'X'` или `'O'`.
char mark_to_char(Mark mark) noexcept;

/// @brief Преобразует символ текстового формата в метку.
/// @param symbol Символ `'.'`, `'X'` или `'O'`; регистр важен.
/// @return Соответствующая метка.
/// @throws InvalidMarkError Если символ не входит в допустимый набор.
Mark mark_from_char(char symbol);

/// @brief Позиция клетки на игровом поле.
/// @details Индексы API отсчитываются с нуля, строка 0 расположена сверху.
struct Position {
    std::size_t row;  ///< Номер строки, начиная с 0.
    std::size_t col;  ///< Номер столбца, начиная с 0.
};

/// @brief Базовый класс исключений домена «Крестики-нолики».
class TicTacToeError : public std::runtime_error {
public:
    /// @brief Создаёт исключение с готовым сообщением.
    /// @param message Текст, возвращаемый `what()`.
    explicit TicTacToeError(const std::string& message);
};

/// @brief Размер поля не входит в диапазон [3, 10].
class InvalidBoardSizeError : public TicTacToeError {
public:
    /// @brief Создаёт исключение о недопустимом размере.
    /// @param size Указанный размер поля.
    /// @details Сообщение: `Invalid board size: <size> (allowed: 3..10)`.
    explicit InvalidBoardSizeError(std::size_t size);
};

/// @brief Позиция лежит за границами поля.
class PositionOutOfRangeError : public TicTacToeError {
public:
    /// @brief Создаёт исключение о недопустимой позиции.
    /// @param position Недопустимые координаты.
    /// @param board_size Сторона поля.
    /// @details Сообщение: `Position out of range: (<row>, <col>) for board size <n>`.
    PositionOutOfRangeError(Position position, std::size_t board_size);
};

/// @brief Попытка поставить фигуру в занятую клетку.
class CellOccupiedError : public TicTacToeError {
public:
    /// @brief Создаёт исключение о занятой клетке.
    /// @param position Координаты занятой клетки.
    explicit CellOccupiedError(Position position);
};

/// @brief Передана недопустимая метка.
class InvalidMarkError : public TicTacToeError {
public:
    /// @brief Создаёт исключение о недопустимой метке.
    /// @param symbol Символ, который не удалось принять как метку.
    explicit InvalidMarkError(char symbol);
};

/// @brief Текстовое представление поля не соответствует формату ввода.
class InvalidBoardFormatError : public TicTacToeError {
public:
    /// @brief Создаёт исключение с подробностями нарушения формата.
    /// @param details Описание ошибки.
    explicit InvalidBoardFormatError(const std::string& details);

    /// @brief Создаёт исключение, если размер поля отсутствует.
    [[nodiscard]] static InvalidBoardFormatError size_missing();
    /// @brief Создаёт исключение, если лексема размера не является целым числом.
    /// @param token Прочитанная лексема.
    [[nodiscard]] static InvalidBoardFormatError size_not_integer(const std::string& token);
    /// @brief Создаёт исключение, если отсутствует строка поля.
    /// @param row_number Номер ожидаемой строки, начиная с 1.
    [[nodiscard]] static InvalidBoardFormatError row_missing(std::size_t row_number);
    /// @brief Создаёт исключение, если у строки неверная длина.
    /// @param row_number Номер строки, начиная с 1.
    /// @param actual_length Фактическая длина строки.
    /// @param expected_length Ожидаемая длина строки — размер поля.
    [[nodiscard]] static InvalidBoardFormatError row_has_wrong_length(
        std::size_t row_number, std::size_t actual_length, std::size_t expected_length);
};

/// @brief Квадратное поле «Крестики-нолики» размером от 3×3 до 10×10.
/// @details Класс единолично владеет одномерным динамическим массивом клеток.
///          Реализовано правило трёх: деструктор, копирующий конструктор,
///          копирующее присваивание. Перемещение не объявляется.
/// @invariant `kMinBoardSize <= size() <= kMaxBoardSize`.
/// @invariant Буфер содержит `size() * size()` допустимых меток.
/// @invariant Очерёдность ходов и баланс X/O не являются инвариантами:
///            доступ через неконстантный `operator[]` предназначен для прямой записи.
class TicTacToe {
public:
    /// @brief Создаёт пустое поле размера `kDefaultBoardSize`.
    /// @post Поле 3×3, все клетки `Mark::kEmpty`.
    /// @throws std::bad_alloc Если выделить память не удалось.
    TicTacToe();

    /// @brief Создаёт пустое поле указанного размера.
    /// @param size Сторона поля в диапазоне [3, 10].
    /// @post Все клетки равны `Mark::kEmpty`.
    /// @throws InvalidBoardSizeError Если размер вне диапазона.
    /// @throws std::bad_alloc Если выделить память не удалось.
    /// @code
    /// tic_tac_toe::TicTacToe board(5);
    /// @endcode
    explicit TicTacToe(std::size_t size);

    /// @brief Создаёт глубокую копию поля.
    /// @param other Поле-источник.
    /// @post Клетки равны `other`, а буферы независимы.
    /// @throws std::bad_alloc Если выделить память не удалось.
    TicTacToe(const TicTacToe& other);

    /// @brief Освобождает принадлежащий объекту массив клеток.
    ~TicTacToe() noexcept;

    /// @brief Присваивает содержимое другого поля со строгой гарантией.
    /// @param other Поле-источник.
    /// @return Ссылка на `*this`.
    /// @details Поддерживает самоприсваивание и разные размеры полей.
    /// @throws std::bad_alloc Если копия не может быть выделена; объект не изменяется.
    TicTacToe& operator=(const TicTacToe& other);

    /// @brief Сравнивает поля на равенство.
    /// @param other Сравниваемое поле.
    /// @return `true`, если размеры и все соответствующие клетки равны.
    [[nodiscard]] bool operator==(const TicTacToe& other) const noexcept;

    /// @brief Сравнивает поля на неравенство.
    /// @param other Сравниваемое поле.
    /// @return Отрицание результата `operator==`.
    [[nodiscard]] bool operator!=(const TicTacToe& other) const noexcept;

    /// @brief Возвращает сторону квадратного поля.
    /// @return Размер N.
    [[nodiscard]] std::size_t size() const noexcept;

    /// @brief Получает клетку только для чтения.
    /// @param position Координаты клетки, индексы с нуля.
    /// @return Константная ссылка на метку клетки.
    /// @throws PositionOutOfRangeError Если координаты вне поля.
    [[nodiscard]] const Mark& operator[](Position position) const;

    /// @brief Получает клетку для прямой записи без проверки правил хода.
    /// @param position Координаты клетки, индексы с нуля.
    /// @return Ссылка на метку клетки.
    /// @details Занятость клетки и очерёдность не проверяются; `Mark::kEmpty` допустима.
    /// @throws PositionOutOfRangeError Если координаты вне поля.
    Mark& operator[](Position position);

    /// @brief Проверяет, можно ли поставить указанную фигуру.
    /// @param position Координаты клетки.
    /// @param mark Метка игрока.
    /// @return `true`, если метка допустима, клетка существует и свободна.
    /// @post Поле не изменяется.
    [[nodiscard]] bool can_place(Position position, Mark mark) const noexcept;

    /// @brief Делает ход по правилам игры.
    /// @param position Координаты клетки.
    /// @param mark Метка игрока.
    /// @details Проверки идут в порядке: допустимость метки, границы, занятость.
    /// @throws InvalidMarkError Если передана `Mark::kEmpty` или недопустимая метка.
    /// @throws PositionOutOfRangeError Если координаты вне поля.
    /// @throws CellOccupiedError Если клетка уже занята.
    void place(Position position, Mark mark);

    /// @brief Проверяет, есть ли у игрока полностью занятая линия.
    /// @param mark Метка игрока; пустая метка не допускается.
    /// @return `true`, если совпадают все клетки строки, столбца или диагонали.
    /// @throws InvalidMarkError Если метка недопустима.
    [[nodiscard]] bool has_won(Mark mark) const;

    /// @brief Определяет победителя.
    /// @return `Mark::kCross`, `Mark::kNought` или `Mark::kEmpty`, если победителя нет.
    /// @details Если полные линии одновременно есть у обеих меток, приоритет у X.
    [[nodiscard]] Mark winner() const noexcept;

    /// @brief Проверяет, что все клетки заняты.
    /// @return `true`, если пустых клеток нет.
    [[nodiscard]] bool is_full() const noexcept;

    /// @brief Проверяет, завершилась ли партия вничью.
    /// @return `true`, если поле заполнено и победителя нет.
    [[nodiscard]] bool is_draw() const noexcept;

    /// @brief Считает количество клеток с заданной меткой.
    /// @param mark Искомая метка.
    /// @return Количество клеток от 0 до `size() * size()`.
    [[nodiscard]] std::size_t count(Mark mark) const noexcept;

    /// @brief Определяет следующую метку на основе количества фигур.
    /// @return `Mark::kNought`, если X больше, чем O; иначе `Mark::kCross`.
    [[nodiscard]] Mark next_mark() const noexcept;

private:
    /// @brief Преобразует координаты в индекс с проверкой границ.
    /// @param position Координаты клетки.
    /// @return `row * size_ + col`.
    /// @throws PositionOutOfRangeError Если координаты вне поля.
    [[nodiscard]] std::size_t index_of(Position position) const;
    /// @brief Обменивает размер и буфер с другим полем без исключений.
    /// @param other Второе поле.
    void swap(TicTacToe& other) noexcept;
    /// @brief Проверяет, целиком ли строка принадлежит метке.
    /// @param mark Проверяемая метка.
    /// @param row Индекс строки.
    [[nodiscard]] bool is_row_owned_by(Mark mark, std::size_t row) const noexcept;
    /// @brief Проверяет, целиком ли столбец принадлежит метке.
    /// @param mark Проверяемая метка.
    /// @param col Индекс столбца.
    [[nodiscard]] bool is_column_owned_by(Mark mark, std::size_t col) const noexcept;
    /// @brief Проверяет главную диагональ.
    /// @param mark Проверяемая метка.
    [[nodiscard]] bool is_main_diagonal_owned_by(Mark mark) const noexcept;
    /// @brief Проверяет побочную диагональ.
    /// @param mark Проверяемая метка.
    [[nodiscard]] bool is_anti_diagonal_owned_by(Mark mark) const noexcept;
    /// @brief Проверяет наличие хотя бы одной полной линии.
    /// @param mark Проверяемая метка.
    [[nodiscard]] bool has_full_line(Mark mark) const noexcept;

    std::size_t size_;
    Mark* cells_;
};

/// @brief Записывает поле в каноническом текстовом формате.
/// @param out Выходной поток.
/// @param board Выводимое поле.
/// @return Ссылка на `out`.
/// @details Формат: N, `\n`, затем N строк по N символов, каждая завершается `\n`.
std::ostream& operator<<(std::ostream& out, const TicTacToe& board);

/// @brief Читает поле во временный объект, присваивая цель только при полном успехе.
/// @param in Входной поток.
/// @param board Целевой объект.
/// @return Ссылка на `in`.
/// @details Принимает ровно размер и N строк; остаток потока не потребляется.
/// @throws InvalidBoardFormatError При отсутствии размера/строки, неверной длине или лексеме.
/// @throws InvalidBoardSizeError Если размер вне [3, 10].
/// @throws InvalidMarkError Если встретился недопустимый символ клетки.
std::istream& operator>>(std::istream& in, TicTacToe& board);

}  // namespace tic_tac_toe
/// @}
