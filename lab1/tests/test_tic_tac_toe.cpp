#include "tic_tac_toe/tic_tac_toe.h"

#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <new>
#include <sstream>
#include <string>
#include <type_traits>

namespace {
std::atomic<bool> fail_next_array_allocation{false};
}

void* operator new[](std::size_t size) {
    if (fail_next_array_allocation.exchange(false)) { throw std::bad_alloc(); }
    if (void* memory = std::malloc(size)) { return memory; }
    throw std::bad_alloc();
}
void operator delete[](void* memory) noexcept { std::free(memory); }
void operator delete[](void* memory, std::size_t) noexcept { std::free(memory); }

namespace tic_tac_toe {
namespace {

TEST(TicTacToeTests, T01_Construction_DefaultIs3x3Empty) {
    const TicTacToe board;
    EXPECT_EQ(board.size(), 3U);
    EXPECT_EQ(board.count(Mark::kEmpty), 9U);
}
TEST(TicTacToeTests, T02_Construction_CustomSizeEmpty) {
    const TicTacToe board(5U);
    EXPECT_EQ(board.size(), 5U);
    EXPECT_EQ(board.count(Mark::kEmpty), 25U);
}
TEST(TicTacToeTests, T03_Construction_BoundarySizesAccepted) {
    EXPECT_EQ(TicTacToe(3U).size(), 3U);
    EXPECT_EQ(TicTacToe(10U).size(), 10U);
}
TEST(TicTacToeTests, T04_Construction_SizeBelowMinThrows) {
    for (std::size_t size = 0U; size < 3U; ++size) {
        EXPECT_THROW(static_cast<void>(TicTacToe(size)), InvalidBoardSizeError);
    }
    try { static_cast<void>(TicTacToe(2U)); }
    catch (const InvalidBoardSizeError& error) {
        EXPECT_STREQ(error.what(), "Invalid board size: 2 (allowed: 3..10)");
    }
}
TEST(TicTacToeTests, T05_Construction_SizeAboveMaxThrows) {
    EXPECT_THROW(TicTacToe(11U), InvalidBoardSizeError);
    EXPECT_THROW(TicTacToe(static_cast<std::size_t>(-1)), InvalidBoardSizeError);
}
TEST(TicTacToeTests, T06_Access_ConstIndexReadsEmpty) {
    const TicTacToe board;
    EXPECT_EQ((board[{0U, 0U}]), Mark::kEmpty);
    EXPECT_EQ((board[{2U, 2U}]), Mark::kEmpty);
}
TEST(TicTacToeTests, T07_Access_ConstIndexReadsPlacedMark) {
    TicTacToe board;
    board.place({1U, 2U}, Mark::kNought);
    const TicTacToe& view = board;
    EXPECT_EQ((view[{1U, 2U}]), Mark::kNought);
}
TEST(TicTacToeTests, T08_Access_NonConstIndexWritesCell) {
    TicTacToe board;
    (board[{0U, 0U}]) = Mark::kCross;
    EXPECT_EQ((board[{0U, 0U}]), Mark::kCross);
}
TEST(TicTacToeTests, T09_Access_NonConstIndexOverwritesAndClears) {
    TicTacToe board;
    (board[{0U, 0U}]) = Mark::kCross;
    (board[{0U, 0U}]) = Mark::kNought;
    EXPECT_EQ((board[{0U, 0U}]), Mark::kNought);
    (board[{0U, 0U}]) = Mark::kEmpty;
    EXPECT_EQ((board[{0U, 0U}]), Mark::kEmpty);
}
TEST(TicTacToeTests, T10_Access_OutOfRangeRowThrows) {
    TicTacToe board;
    const TicTacToe& view = board;
    try { static_cast<void>((view[{3U, 0U}])); FAIL(); }
    catch (const PositionOutOfRangeError& error) {
        EXPECT_STREQ(error.what(), "Position out of range: (3, 0) for board size 3");
    }
    EXPECT_THROW(static_cast<void>((board[{3U, 0U}])), PositionOutOfRangeError);
}
TEST(TicTacToeTests, T11_Access_OutOfRangeColThrows) {
    TicTacToe board;
    const TicTacToe& view = board;
    EXPECT_THROW(static_cast<void>((view[{0U, 3U}])), PositionOutOfRangeError);
    EXPECT_THROW(static_cast<void>((board[{0U, 3U}])), PositionOutOfRangeError);
}
TEST(TicTacToeTests, T12_Access_OutOfRangeBothThrows) {
    TicTacToe board;
    EXPECT_THROW(static_cast<void>((board[{3U, 3U}])), PositionOutOfRangeError);
}
TEST(TicTacToeTests, T13_Access_BoundaryCells) {
    TicTacToe board(5U);
    (board[{0U, 0U}]) = Mark::kCross;
    (board[{4U, 4U}]) = Mark::kNought;
    EXPECT_EQ((board[{0U, 0U}]), Mark::kCross);
    EXPECT_EQ((board[{4U, 4U}]), Mark::kNought);
}
TEST(TicTacToeTests, T14_CanPlace_EmptyCellBothMarks) {
    const TicTacToe board;
    EXPECT_TRUE(board.can_place({1U, 1U}, Mark::kCross));
    EXPECT_TRUE(board.can_place({1U, 1U}, Mark::kNought));
}
TEST(TicTacToeTests, T15_CanPlace_OccupiedCell) {
    TicTacToe board;
    board.place({1U, 1U}, Mark::kCross);
    EXPECT_FALSE(board.can_place({1U, 1U}, Mark::kNought));
}
TEST(TicTacToeTests, T16_CanPlace_EmptyMark) {
    const TicTacToe board;
    EXPECT_FALSE(board.can_place({0U, 0U}, Mark::kEmpty));
}
TEST(TicTacToeTests, T17_CanPlace_OutOfRangeNoThrow) {
    const TicTacToe board;
    EXPECT_FALSE(board.can_place({3U, 0U}, Mark::kCross));
    EXPECT_FALSE(board.can_place({0U, 3U}, Mark::kNought));
}
TEST(TicTacToeTests, T18_Place_PlacesMarks) {
    TicTacToe board;
    board.place({0U, 0U}, Mark::kCross);
    board.place({1U, 1U}, Mark::kNought);
    EXPECT_EQ((board[{0U, 0U}]), Mark::kCross);
    EXPECT_EQ((board[{1U, 1U}]), Mark::kNought);
}
TEST(TicTacToeTests, T19_Place_OccupiedThrowsAndKeepsBoard) {
    TicTacToe board;
    board.place({0U, 0U}, Mark::kCross);
    const TicTacToe original(board);
    EXPECT_THROW(board.place({0U, 0U}, Mark::kNought), CellOccupiedError);
    EXPECT_EQ(board, original);
}
TEST(TicTacToeTests, T20_Place_OutOfRangeThrows) {
    TicTacToe board;
    EXPECT_THROW(board.place({0U, 3U}, Mark::kCross), PositionOutOfRangeError);
}
TEST(TicTacToeTests, T21_Place_EmptyMarkThrows) {
    TicTacToe board;
    EXPECT_THROW(board.place({0U, 0U}, Mark::kEmpty), InvalidMarkError);
}
TEST(TicTacToeTests, T22_Place_CheckOrder) {
    TicTacToe board;
    EXPECT_THROW(board.place({3U, 0U}, Mark::kEmpty), InvalidMarkError);
}
TEST(TicTacToeTests, T23_NextMark_AlternatesFromCross) {
    TicTacToe board;
    EXPECT_EQ(board.next_mark(), Mark::kCross);
    board.place({0U, 0U}, Mark::kCross);
    EXPECT_EQ(board.next_mark(), Mark::kNought);
    (board[{1U, 1U}]) = Mark::kNought;
    EXPECT_EQ(board.next_mark(), Mark::kCross);
}
TEST(TicTacToeTests, T24_Win_RowForEachRow) {
    for (const std::size_t size : {3U, 4U, 10U}) {
        for (std::size_t row = 0U; row < size; ++row) {
            TicTacToe board(size);
            for (std::size_t col = 0U; col < size; ++col) { (board[{row, col}]) = Mark::kCross; }
            EXPECT_TRUE(board.has_won(Mark::kCross));
            EXPECT_EQ(board.winner(), Mark::kCross);
        }
    }
}
TEST(TicTacToeTests, T25_Win_ColumnForEachColumn) {
    for (const std::size_t size : {3U, 4U, 10U}) {
        for (std::size_t col = 0U; col < size; ++col) {
            TicTacToe board(size);
            for (std::size_t row = 0U; row < size; ++row) { (board[{row, col}]) = Mark::kNought; }
            EXPECT_TRUE(board.has_won(Mark::kNought));
            EXPECT_EQ(board.winner(), Mark::kNought);
        }
    }
}
TEST(TicTacToeTests, T26_Win_MainDiagonal) {
    TicTacToe board(4U);
    for (std::size_t index = 0U; index < board.size(); ++index) { (board[{index, index}]) = Mark::kCross; }
    EXPECT_TRUE(board.has_won(Mark::kCross));
}
TEST(TicTacToeTests, T27_Win_AntiDiagonal) {
    TicTacToe board(4U);
    for (std::size_t index = 0U; index < board.size(); ++index) {
        (board[{index, board.size() - 1U - index}]) = Mark::kNought;
    }
    EXPECT_TRUE(board.has_won(Mark::kNought));
}
TEST(TicTacToeTests, T28_Win_IncompleteLineIsNotWin) {
    TicTacToe board;
    (board[{0U, 0U}]) = Mark::kCross;
    (board[{0U, 1U}]) = Mark::kCross;
    EXPECT_FALSE(board.has_won(Mark::kCross));
}
TEST(TicTacToeTests, T29_Win_MixedLineIsNotWin) {
    TicTacToe board;
    (board[{0U, 0U}]) = Mark::kCross;
    (board[{0U, 1U}]) = Mark::kCross;
    (board[{0U, 2U}]) = Mark::kNought;
    EXPECT_FALSE(board.has_won(Mark::kCross));
}
TEST(TicTacToeTests, T30_Win_HasWonEmptyMarkThrows) {
    TicTacToe board;
    EXPECT_THROW(static_cast<void>(board.has_won(Mark::kEmpty)), InvalidMarkError);
}
TEST(TicTacToeTests, T31_Win_WinnerAndPriority) {
    TicTacToe board;
    for (std::size_t index = 0U; index < 3U; ++index) {
        (board[{0U, index}]) = Mark::kCross;
        (board[{1U, index}]) = Mark::kNought;
    }
    EXPECT_EQ(board.winner(), Mark::kCross);
    EXPECT_FALSE(board.is_draw());
    TicTacToe empty;
    EXPECT_EQ(empty.winner(), Mark::kEmpty);
}
TEST(TicTacToeTests, T32_Draw_FullBoardCases) {
    TicTacToe draw;
    const Mark layout[3][3] = {{Mark::kCross, Mark::kNought, Mark::kCross},
                               {Mark::kCross, Mark::kNought, Mark::kNought},
                               {Mark::kNought, Mark::kCross, Mark::kCross}};
    for (std::size_t row = 0U; row < 3U; ++row) {
        for (std::size_t col = 0U; col < 3U; ++col) { (draw[{row, col}]) = layout[row][col]; }
    }
    EXPECT_TRUE(draw.is_full());
    EXPECT_TRUE(draw.is_draw());
    TicTacToe full;
    for (std::size_t row = 0U; row < 3U; ++row) {
        for (std::size_t col = 0U; col < 3U; ++col) { (full[{row, col}]) = Mark::kCross; }
    }
    EXPECT_TRUE(full.is_full());
    EXPECT_FALSE(full.is_draw());
    EXPECT_FALSE(TicTacToe().is_full());
}
TEST(TicTacToeTests, T33_Count_PerMark) {
    TicTacToe board;
    (board[{0U, 0U}]) = Mark::kCross;
    (board[{1U, 1U}]) = Mark::kNought;
    EXPECT_EQ(board.count(Mark::kCross), 1U);
    EXPECT_EQ(board.count(Mark::kNought), 1U);
    EXPECT_EQ(board.count(Mark::kEmpty), 7U);
}
TEST(TicTacToeTests, T34_Copy_ConstructorDeepAndEqual) {
    TicTacToe original;
    (original[{0U, 0U}]) = Mark::kCross;
    TicTacToe copy(original);
    EXPECT_EQ(copy, original);
    (copy[{0U, 0U}]) = Mark::kNought;
    EXPECT_NE(copy, original);
    EXPECT_EQ((original[{0U, 0U}]), Mark::kCross);
}
TEST(TicTacToeTests, T35_Copy_AssignDifferentSizes) {
    TicTacToe target(3U);
    TicTacToe source(5U);
    (source[{4U, 4U}]) = Mark::kCross;
    target = source;
    EXPECT_EQ(target, source);
    (target[{4U, 4U}]) = Mark::kNought;
    EXPECT_EQ((source[{4U, 4U}]), Mark::kCross);
}
TEST(TicTacToeTests, T36_Copy_SelfAssignment) {
    TicTacToe board;
    (board[{0U, 0U}]) = Mark::kCross;
    board = board;
    EXPECT_EQ((board[{0U, 0U}]), Mark::kCross);
}
TEST(TicTacToeTests, T37_Copy_ChainedAssignment) {
    TicTacToe first;
    TicTacToe second(4U);
    TicTacToe third(5U);
    (third[{4U, 4U}]) = Mark::kNought;
    first = second = third;
    EXPECT_EQ(first, third);
    EXPECT_EQ(second, third);
}
TEST(TicTacToeTests, T38_Copy_AssignStrongGuarantee) {
    TicTacToe target(3U);
    (target[{0U, 0U}]) = Mark::kCross;
    const TicTacToe original(target);
    TicTacToe source(5U);
    fail_next_array_allocation = true;
    EXPECT_THROW(target = source, std::bad_alloc);
    EXPECT_EQ(target, original);
}
TEST(TicTacToeTests, T39_Equality_EqualValues) {
    const TicTacToe first;
    const TicTacToe second;
    EXPECT_EQ(first, second);
}
TEST(TicTacToeTests, T40_Equality_DifferentSize) {
    EXPECT_NE(TicTacToe(3U), TicTacToe(4U));
}
TEST(TicTacToeTests, T41_Equality_DifferentCell) {
    TicTacToe first;
    TicTacToe second;
    (second[{0U, 0U}]) = Mark::kCross;
    EXPECT_NE(first, second);
}
TEST(TicTacToeTests, T42_Equality_NotEqualIsNegation) {
    const TicTacToe first;
    TicTacToe second;
    (second[{2U, 2U}]) = Mark::kNought;
    EXPECT_EQ(first != second, !(first == second));
}
TEST(TicTacToeTests, T43_Equality_Reflexive) {
    TicTacToe board;
    EXPECT_EQ(board, board);
}
TEST(TicTacToeTests, T44_Equality_Copy) {
    TicTacToe first(6U);
    (first[{3U, 4U}]) = Mark::kNought;
    const TicTacToe second(first);
    EXPECT_TRUE(first == second);
}
TEST(TicTacToeTests, T45_Io_WriteCanonicalText) {
    TicTacToe board;
    (board[{0U, 0U}]) = Mark::kCross;
    (board[{0U, 2U}]) = Mark::kNought;
    (board[{1U, 1U}]) = Mark::kCross;
    (board[{2U, 0U}]) = Mark::kNought;
    (board[{2U, 2U}]) = Mark::kCross;
    std::ostringstream output;
    output << board;
    EXPECT_EQ(output.str(), "3\nX.O\n.X.\nO.X\n");
}
TEST(TicTacToeTests, T46_Io_ReadValidVariants) {
    for (const std::string text : {"3 X.O .X. O.X", "3\r\nX.O\r\n.X.\r\nO.X\r\n", " 3 \t X.O  .X.  O.X "}) {
        TicTacToe board;
        std::istringstream input(text);
        input >> board;
        EXPECT_EQ((board[{0U, 0U}]), Mark::kCross);
        EXPECT_EQ((board[{2U, 2U}]), Mark::kCross);
    }
}
TEST(TicTacToeTests, T47_Io_RoundTrip) {
    for (const std::size_t size : {3U, 5U, 10U}) {
        TicTacToe original(size);
        (original[{0U, 0U}]) = Mark::kCross;
        (original[{size - 1U, size - 1U}]) = Mark::kNought;
        std::ostringstream output;
        output << original;
        TicTacToe restored;
        std::istringstream input(output.str());
        input >> restored;
        EXPECT_EQ(original, restored);
    }
}
TEST(TicTacToeTests, T48_Io_ReadInvalidTable) {
    struct Case { std::string text; std::string expected; bool size_error; };
    const Case cases[] = {
        {"", "Invalid board format: size is missing", false},
        {"abc", "Invalid board format: size is not an integer: 'abc'", false},
        {"-3", "Invalid board format: size is not an integer: '-3'", false},
        {"3.5", "Invalid board format: size is not an integer: '3.5'", false},
        {"1234567890123456789", "Invalid board format: size is not an integer: '1234567890123456789'", false},
        {"2 .. ..", "Invalid board size: 2 (allowed: 3..10)", true},
        {"11 ...", "Invalid board size: 11 (allowed: 3..10)", true},
        {"3 X.O .X.", "Invalid board format: row 3 is missing", false},
        {"3 X.O .X O.X", "Invalid board format: row 2 has length 2, expected 3", false},
        {"3 X.O .Z. O.X", "Invalid mark: 'Z'", false},
        {"3 x.o ... ...", "Invalid mark: 'x'", false}
    };
    for (const Case& item : cases) {
        TicTacToe board;
        std::istringstream input(item.text);
        try {
            input >> board;
            FAIL() << item.text;
        } catch (const InvalidBoardSizeError& error) {
            EXPECT_TRUE(item.size_error);
            EXPECT_EQ(error.what(), item.expected);
        } catch (const TicTacToeError& error) {
            EXPECT_FALSE(item.size_error);
            EXPECT_EQ(error.what(), item.expected);
        }
    }
}
TEST(TicTacToeTests, T49_Io_ReadFailureKeepsTarget) {
    TicTacToe board;
    (board[{0U, 0U}]) = Mark::kCross;
    const TicTacToe original(board);
    std::istringstream input("3 ... .... ...");
    EXPECT_THROW(input >> board, InvalidBoardFormatError);
    EXPECT_EQ(board, original);
}
TEST(TicTacToeTests, T50_Io_ReadTwoBoardsFromOneStream) {
    std::istringstream input("3 X.O .X. O.X 3 ... ... ...");
    TicTacToe first;
    TicTacToe second;
    input >> first >> second;
    EXPECT_EQ((first[{0U, 0U}]), Mark::kCross);
    EXPECT_FALSE(second.is_full());
    EXPECT_EQ(second.count(Mark::kEmpty), 9U);
}
TEST(TicTacToeTests, T51_Io_ReadEmptyStream) {
    TicTacToe board;
    std::istringstream input;
    EXPECT_THROW(input >> board, InvalidBoardFormatError);
}
TEST(TicTacToeTests, T52_Exceptions_Hierarchy) {
    static_assert(std::is_base_of_v<TicTacToeError, InvalidBoardSizeError>);
    static_assert(std::is_base_of_v<std::runtime_error, InvalidMarkError>);
    EXPECT_NE(std::string(InvalidBoardSizeError(2U).what()).size(), 0U);
}
TEST(TicTacToeTests, T53_Mark_CharConversion) {
    EXPECT_EQ(mark_to_char(Mark::kEmpty), '.');
    EXPECT_EQ(mark_to_char(Mark::kCross), 'X');
    EXPECT_EQ(mark_from_char('O'), Mark::kNought);
    EXPECT_THROW(mark_from_char('x'), InvalidMarkError);
}
TEST(TicTacToeTests, T54_Structure_HeadersSelfContained) {
    static_assert(std::is_copy_constructible_v<TicTacToe>);
    static_assert(std::is_copy_assignable_v<TicTacToe>);
    static_assert(!std::is_convertible_v<std::size_t, TicTacToe>);
    SUCCEED();
}

}  // namespace
}  // namespace tic_tac_toe
