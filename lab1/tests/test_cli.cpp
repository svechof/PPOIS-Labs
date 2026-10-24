#include "cli.h"

#include <gtest/gtest.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace cli {
namespace {

TEST(CliTests, C01_Application_ExitImmediately) {
    std::istringstream input("0\n");
    std::ostringstream output;
    Application application(input, output);
    EXPECT_NO_THROW(application.run());
    EXPECT_NE(output.str().find("Работа завершена."), std::string::npos);
}
TEST(CliTests, C02_Application_InvalidMenuInput) {
    std::istringstream input("abc\n99\n0\n");
    std::ostringstream output;
    Application application(input, output);
    application.run();
    EXPECT_NE(output.str().find("Некорректный ввод"), std::string::npos);
    EXPECT_NE(output.str().find("Работа завершена."), std::string::npos);
}
TEST(CliTests, C03_Application_EofExitsCleanly) {
    std::istringstream input;
    std::ostringstream output;
    Application application(input, output);
    EXPECT_NO_THROW(application.run());
    EXPECT_NE(output.str().find("Работа завершена."), std::string::npos);
    EXPECT_EQ(output.str().find("Ошибка:"), std::string::npos);
}
TEST(CliTests, C04_TicTacToeMenu_FullGameCrossWins) {
    const std::string script = "1\n1\n3\n1 1\n1 1\n2 2\n1 2\n3 3\n1 3\n0\n0\n";
    std::istringstream input(script);
    std::ostringstream output;
    Application application(input, output);
    application.run();
    EXPECT_NE(output.str().find("Ход невозможен"), std::string::npos);
    EXPECT_NE(output.str().find("Победил игрок X"), std::string::npos);
}
TEST(CliTests, C05_TicTacToeMenu_FullGameDraw) {
    const std::string script = "1\n1\n3\n1 1\n1 2\n1 3\n2 2\n2 1\n2 3\n3 2\n3 1\n3 3\n0\n0\n";
    std::istringstream input(script);
    std::ostringstream output;
    Application application(input, output);
    application.run();
    EXPECT_NE(output.str().find("Ничья"), std::string::npos);
}
TEST(CliTests, C06_TicTacToeMenu_OccupiedMoveRetries) {
    std::istringstream input("1\n1\n3\n1 1\n1 1\nq\n0\n0\n");
    std::ostringstream output;
    Application application(input, output);
    application.run();
    EXPECT_NE(output.str().find("Ход невозможен"), std::string::npos);
    EXPECT_NE(output.str().find("Партия прервана"), std::string::npos);
}
TEST(CliTests, C07_TicTacToeMenu_QuitGame) {
    std::istringstream input("1\n1\n3\nq\n0\n0\n");
    std::ostringstream output;
    Application application(input, output);
    application.run();
    EXPECT_NE(output.str().find("Партия прервана"), std::string::npos);
}
TEST(CliTests, C08_TicTacToeMenu_AllItemsSmoke) {
    const std::string script = "1\n2\n4\n3\n4\n1\n1\nO\n5\n1\n1\n6\n1\n1\nX\n"
                               "7\n1\n1\nX\n8\n9\n10\n3\n....\n....\n....\n....\n0\n0\n";
    std::istringstream input(script);
    std::ostringstream output;
    Application application(input, output);
    application.run();
    const std::string rendered = output.str();
    EXPECT_NE(rendered.find("Можно"), std::string::npos);
    EXPECT_NE(rendered.find("Поле"), std::string::npos);
    EXPECT_NE(rendered.find("Клетка (1, 1) уже занята."), std::string::npos);
    EXPECT_NE(rendered.find("Текущий текст поля"), std::string::npos);
}
TEST(CliTests, C09_PolynomialMenu_FullScenario) {
    const std::string script = "2\n"
                               "1\nA\n2\n-1 0 1\n"
                               "2\nB\n1 -1 1\n"
                               "3\n"
                               "6\nA\n2\n"
                               "7\n8\n9\n10\n12\n"
                               "4\nA\n5\n"
                               "5\nA\n2\n0\n"
                               "1\nB\n0\n0\n"
                               "10\n0\n0\n";
    std::istringstream input(script);
    std::ostringstream output;
    Application application(input, output);
    application.run();
    const std::string text = output.str();
    EXPECT_NE(text.find("A: 2 -1 0 1, x^2 - 1"), std::string::npos);
    EXPECT_NE(text.find("B: 1 -1 1, x - 1"), std::string::npos);
    EXPECT_NE(text.find("p(2) = 3"), std::string::npos);
    EXPECT_NE(text.find("x^2 + x - 2"), std::string::npos);
    EXPECT_NE(text.find("x^2 - x"), std::string::npos);
    EXPECT_NE(text.find("x^3 - x^2 - x + 1"), std::string::npos);
    EXPECT_NE(text.find("остаток отброшен"), std::string::npos);
    EXPECT_NE(text.find("Деление на нулевой многочлен невозможно."), std::string::npos);
}
TEST(CliTests, C10_PolynomialMenu_DivideByZeroShowsError) {
    std::istringstream input("2\n10\n0\n0\n");
    std::ostringstream output;
    Application application(input, output);
    application.run();
    EXPECT_NE(output.str().find("Ошибка: Деление на нулевой многочлен невозможно."), std::string::npos);
    EXPECT_NE(output.str().find("Меню «Многочлены»"), std::string::npos);
}
TEST(CliTests, C11_PolynomialMenu_BadInputShowsError) {
    std::istringstream input("2\n1\nA\n1\n1 2\n2\nB\n1 invalid 3\n0\n0\n");
    std::ostringstream output;
    Application application(input, output);
    application.run();
    EXPECT_NE(output.str().find("Ошибка: Неверный формат многочлена. Проверьте степень и коэффициенты."),
              std::string::npos);
    EXPECT_NE(output.str().find("A: 1 1 2, 2x + 1"), std::string::npos);
}
TEST(CliTests, C12_BoardView_ExactRendering) {
    tic_tac_toe::TicTacToe board;
    (board[{0U, 0U}]) = tic_tac_toe::Mark::kCross;
    (board[{0U, 2U}]) = tic_tac_toe::Mark::kNought;
    (board[{1U, 1U}]) = tic_tac_toe::Mark::kCross;
    (board[{2U, 0U}]) = tic_tac_toe::Mark::kNought;
    (board[{2U, 2U}]) = tic_tac_toe::Mark::kCross;
    std::ostringstream output;
    BoardView view;
    view.render(output, board);
    EXPECT_EQ(output.str(), "    1  2  3\n 1  X  .  O\n 2  .  X  .\n 3  O  .  X\n");
    tic_tac_toe::TicTacToe large(10U);
    std::ostringstream large_output;
    view.render(large_output, large);
    EXPECT_NE(large_output.str().find(" 10"), std::string::npos);
}
TEST(CliTests, C13_PolynomialView_AlgebraicRendering) {
    const std::vector<std::pair<polynomial::Polynomial, std::string>> cases = {
        {polynomial::Polynomial(2, {-5.0, 0.0, 3.0}), "3x^2 - 5"},
        {polynomial::Polynomial(2, {0.0, -1.0, 1.0}), "x^2 - x"},
        {polynomial::Polynomial(1, {1.0, 1.0}), "x + 1"},
        {polynomial::Polynomial(), "0"},
        {polynomial::Polynomial(0, {2.5}), "2.5"}};
    PolynomialView view;
    for (const auto& item : cases) {
        EXPECT_EQ(view.algebraic(item.first), item.second);
    }
}
TEST(CliTests, C14_Console_ReadIntegerInRange) {
    std::istringstream input("abc\n99\n2\n");
    std::ostringstream output;
    Console console(input, output);
    EXPECT_EQ(console.read_integer_in_range("choice: ", 0, 2), 2);
    EXPECT_NE(output.str().find("Некорректный ввод"), std::string::npos);
    std::istringstream boundary_input("0\n");
    std::ostringstream boundary_output;
    Console boundary_console(boundary_input, boundary_output);
    EXPECT_EQ(boundary_console.read_integer_in_range("choice: ", 0, 2), 0);
}

} // namespace
} // namespace cli
