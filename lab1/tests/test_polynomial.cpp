#include "polynomial/polynomial.h"

#include <gtest/gtest.h>

#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <new>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace {
std::atomic<bool> fail_next_allocation{false};
}

void* operator new(std::size_t size) {
    if (fail_next_allocation.exchange(false)) { throw std::bad_alloc(); }
    if (void* memory = std::malloc(size)) { return memory; }
    throw std::bad_alloc();
}
void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }

namespace polynomial {
namespace {

TEST(PolynomialTests, P01_Construction_DefaultIsZero) {
    const Polynomial value;
    EXPECT_EQ(value.degree(), 0U);
    EXPECT_DOUBLE_EQ(value[0U], 0.0);
    EXPECT_TRUE(value.is_zero());
}
TEST(PolynomialTests, P02_Construction_FromDegreeAndCoefficients) {
    const Polynomial value(2, {1.0, 2.0, 3.0});
    EXPECT_EQ(value.degree(), 2U);
    EXPECT_DOUBLE_EQ(value[0U], 1.0);
    EXPECT_DOUBLE_EQ(value[1U], 2.0);
    EXPECT_DOUBLE_EQ(value[2U], 3.0);
}
TEST(PolynomialTests, P03_Construction_Constant) {
    const Polynomial value(0, {5.0});
    EXPECT_EQ(value.degree(), 0U);
    EXPECT_DOUBLE_EQ(value[0U], 5.0);
}
TEST(PolynomialTests, P04_Construction_NormalizesTrailingZeros) {
    const Polynomial value(3, {1.0, 2.0, 0.0, 0.0});
    EXPECT_EQ(value.degree(), 1U);
}
TEST(PolynomialTests, P05_Construction_AllZerosIsZero) {
    const Polynomial value(2, {0.0, 0.0, 0.0});
    EXPECT_TRUE(value.is_zero());
    EXPECT_EQ(value.degree(), 0U);
    EXPECT_DOUBLE_EQ(value[0U], 0.0);
}
TEST(PolynomialTests, P06_Construction_EpsilonTrimming) {
    EXPECT_EQ(Polynomial(1, {1.0, 1e-10}).degree(), 0U);
    EXPECT_EQ(Polynomial(1, {1.0, 2e-9}).degree(), 1U);
}
TEST(PolynomialTests, P07_Construction_NegativeDegreeThrows) {
    try { static_cast<void>(Polynomial(-1, {0.0})); FAIL(); }
    catch (const InvalidDegreeError& error) {
        EXPECT_STREQ(error.what(), "Invalid degree: -1 (allowed: 0..1000)");
    }
}
TEST(PolynomialTests, P08_Construction_DegreeLimit) {
    std::vector<double> maximum(kMaxDegree + 1U, 0.0);
    EXPECT_NO_THROW(static_cast<void>(Polynomial(1000, maximum)));
    std::vector<double> too_large(kMaxDegree + 2U, 0.0);
    EXPECT_THROW(static_cast<void>(Polynomial(1001, too_large)), InvalidDegreeError);
}
TEST(PolynomialTests, P09_Construction_CountMismatchThrows) {
    try { static_cast<void>(Polynomial(2, {1.0, 2.0})); FAIL(); }
    catch (const CoefficientCountMismatchError& error) {
        EXPECT_STREQ(error.what(), "Coefficient count mismatch: degree 2 requires 3 coefficients, got 2");
    }
    EXPECT_THROW(static_cast<void>(Polynomial(1, {1.0, 2.0, 3.0})), CoefficientCountMismatchError);
}
TEST(PolynomialTests, P10_Construction_NonFiniteThrows) {
    const double infinity = std::numeric_limits<double>::infinity();
    const double nan = std::numeric_limits<double>::quiet_NaN();
    try { static_cast<void>(Polynomial(1, {1.0, infinity})); FAIL(); }
    catch (const NonFiniteCoefficientError& error) { EXPECT_STREQ(error.what(), "Non-finite coefficient at power 1"); }
    EXPECT_THROW(static_cast<void>(Polynomial(1, {nan, 1.0})), NonFiniteCoefficientError);
}
TEST(PolynomialTests, P11_Construction_CheckOrder) {
    EXPECT_THROW(static_cast<void>(Polynomial(-1, {1.0, 2.0})), InvalidDegreeError);
}
TEST(PolynomialTests, P12_Access_ConstReadsAll) {
    const Polynomial value(2, {-5.0, 0.0, 3.0});
    EXPECT_DOUBLE_EQ(value[0U], -5.0);
    EXPECT_DOUBLE_EQ(value[1U], 0.0);
    EXPECT_DOUBLE_EQ(value[2U], 3.0);
}
TEST(PolynomialTests, P13_Access_ConstOutOfRangeThrows) {
    const Polynomial value(2, {1.0, 2.0, 3.0});
    try { static_cast<void>(value[3U]); FAIL(); }
    catch (const CoefficientIndexError& error) {
        EXPECT_STREQ(error.what(), "Coefficient index out of range: 3 (degree is 2)");
    }
}
TEST(PolynomialTests, P14_Access_NonConstWrites) {
    Polynomial value(2, {1.0, 2.0, 3.0});
    value[1U] = 4.0;
    EXPECT_DOUBLE_EQ(value[1U], 4.0);
    EXPECT_DOUBLE_EQ(value(2.0), 21.0);
}
TEST(PolynomialTests, P15_Access_WriteZeroToLeadingLowersDegree) {
    Polynomial value(2, {1.0, 2.0, 3.0});
    value[2U] = 0.0;
    EXPECT_EQ(value.degree(), 1U);
    EXPECT_THROW(static_cast<void>(value[2U]), CoefficientIndexError);
}
TEST(PolynomialTests, P16_Access_NonConstOutOfRangeThrows) {
    Polynomial value(1, {1.0, 2.0});
    EXPECT_THROW(static_cast<void>(value[2U]), CoefficientIndexError);
}
TEST(PolynomialTests, P17_Access_BoundaryPower) {
    Polynomial value(1, {1.0, 2.0});
    EXPECT_DOUBLE_EQ(value[0U], 1.0);
    EXPECT_DOUBLE_EQ(value[value.degree()], 2.0);
    const Polynomial zero;
    EXPECT_DOUBLE_EQ(zero[0U], 0.0);
    EXPECT_THROW(static_cast<void>(zero[1U]), CoefficientIndexError);
}
TEST(PolynomialTests, P18_Evaluate_HornerValues) {
    const Polynomial value(2, {-5.0, 0.0, 3.0});
    EXPECT_DOUBLE_EQ(value(2.0), 7.0);
    EXPECT_DOUBLE_EQ(value(0.0), -5.0);
    EXPECT_DOUBLE_EQ(value(-2.0), 7.0);
    EXPECT_DOUBLE_EQ(value(1.0), -2.0);
}
TEST(PolynomialTests, P19_Evaluate_ZeroAndConstant) {
    EXPECT_DOUBLE_EQ(Polynomial()(45.0), 0.0);
    EXPECT_DOUBLE_EQ(Polynomial(0, {4.25})(-100.0), 4.25);
}
TEST(PolynomialTests, P20_Evaluate_OverflowGivesInfinity) {
    const Polynomial value(2, {0.0, 0.0, 2.0});
    EXPECT_TRUE(std::isinf(value(1e200)));
    Polynomial with_dead(2, {1.0, 2.0, 3.0});
    with_dead[2U] = 0.0;
    EXPECT_DOUBLE_EQ(with_dead(2.0), 5.0);
}
TEST(PolynomialTests, P21_Add_Cases) {
    EXPECT_EQ(Polynomial(1, {1.0, 1.0}) + Polynomial(2, {0.0, 1.0, 1.0}),
              Polynomial(2, {1.0, 2.0, 1.0}));
    EXPECT_EQ(Polynomial(2, {0.0, 1.0, 1.0}) + Polynomial(2, {0.0, 0.0, -1.0}),
              Polynomial(1, {0.0, 1.0}));
    const Polynomial value(1, {1.0, 2.0});
    EXPECT_TRUE((value + (Polynomial(1, {-1.0, -2.0}))).is_zero());
}
TEST(PolynomialTests, P22_Add_CompoundAndAliasing) {
    Polynomial value(1, {1.0, 2.0});
    Polynomial* address = &value;
    EXPECT_EQ(&(value += value), address);
    EXPECT_EQ(value, Polynomial(1, {2.0, 4.0}));
}
TEST(PolynomialTests, P23_Subtract_Cases) {
    const Polynomial value(2, {1.0, 2.0, 3.0});
    EXPECT_TRUE((value - value).is_zero());
    EXPECT_EQ(Polynomial(2, {0.0, 0.0, 1.0}) - Polynomial(2, {1.0, 0.0, 1.0}),
              Polynomial(0, {-1.0}));
    Polynomial compound(value);
    compound -= value;
    EXPECT_TRUE(compound.is_zero());
    compound = value;
    compound -= compound;
    EXPECT_TRUE(compound.is_zero());
}
TEST(PolynomialTests, P24_Multiply_Cases) {
    EXPECT_EQ(Polynomial(1, {1.0, 1.0}) * Polynomial(1, {-1.0, 1.0}),
              Polynomial(2, {-1.0, 0.0, 1.0}));
    EXPECT_TRUE((Polynomial(2, {1.0, 2.0, 3.0}) * Polynomial()).is_zero());
    EXPECT_EQ(Polynomial(1, {2.0, 3.0}) * Polynomial(0, {4.0}),
              Polynomial(1, {8.0, 12.0}));
}
TEST(PolynomialTests, P25_Multiply_CompoundAliasingAndLimit) {
    Polynomial value(1, {1.0, 1.0});
    value *= value;
    EXPECT_EQ(value, Polynomial(2, {1.0, 2.0, 1.0}));
    std::vector<double> coefficients(kMaxDegree + 1U, 0.0);
    coefficients.back() = 1.0;
    Polynomial large(1000, coefficients);
    const Polynomial original(large);
    EXPECT_THROW(large *= Polynomial(1, {0.0, 1.0}), InvalidDegreeError);
    EXPECT_EQ(large, original);
}
TEST(PolynomialTests, P26_Divide_ExactCases) {
    EXPECT_EQ(Polynomial(2, {-1.0, 0.0, 1.0}) / Polynomial(1, {-1.0, 1.0}),
              Polynomial(1, {1.0, 1.0}));
    EXPECT_EQ(Polynomial(3, {-6.0, 11.0, -6.0, 1.0}) / Polynomial(1, {-1.0, 1.0}),
              Polynomial(2, {6.0, -5.0, 1.0}));
}
TEST(PolynomialTests, P27_Divide_RemainderDiscarded) {
    EXPECT_EQ(Polynomial(2, {1.0, 0.0, 1.0}) / Polynomial(1, {0.0, 1.0}),
              Polynomial(1, {0.0, 1.0}));
    EXPECT_EQ(Polynomial(3, {1.0, 2.0, 0.0, 1.0}) / Polynomial(2, {1.0, 0.0, 1.0}),
              Polynomial(1, {0.0, 1.0}));
}
TEST(PolynomialTests, P28_Divide_LowerDegreeAndZeroDividend) {
    EXPECT_TRUE((Polynomial(1, {1.0, 1.0}) / Polynomial(2, {0.0, 0.0, 1.0})).is_zero());
    EXPECT_TRUE((Polynomial() / Polynomial(1, {1.0, 1.0})).is_zero());
}
TEST(PolynomialTests, P29_Divide_ByConstant) {
    EXPECT_EQ(Polynomial(2, {4.0, 0.0, 2.0}) / Polynomial(0, {2.0}),
              Polynomial(2, {2.0, 0.0, 1.0}));
}
TEST(PolynomialTests, P30_Divide_ByZeroThrows) {
    Polynomial dividend(1, {3.0, 4.0});
    const Polynomial original(dividend);
    EXPECT_THROW(static_cast<void>(dividend / Polynomial()), DivisionByZeroPolynomialError);
    EXPECT_THROW(dividend /= Polynomial(0, {1e-12}), DivisionByZeroPolynomialError);
    EXPECT_EQ(dividend, original);
}
TEST(PolynomialTests, P31_Divide_SelfDivision) {
    Polynomial value(2, {1.0, 2.0, 3.0});
    value /= value;
    EXPECT_EQ(value, Polynomial(0, {1.0}));
}
TEST(PolynomialTests, P32_Arithmetic_NonFiniteResultThrows) {
    Polynomial value(0, {1e200});
    const Polynomial original(value);
    EXPECT_THROW(value *= Polynomial(0, {1e200}), NonFiniteCoefficientError);
    EXPECT_EQ(value, original);
}
TEST(PolynomialTests, P33_Arithmetic_ValueSemantics) {
    Polynomial first(1, {1.0, 1.0});
    Polynomial second(1, {-1.0, 1.0});
    const Polynomial result = first + second;
    first[0U] = 20.0;
    second[0U] = 30.0;
    EXPECT_EQ(result, Polynomial(1, {0.0, 2.0}));
}
TEST(PolynomialTests, P34_Arithmetic_AlgebraicLaws) {
    const Polynomial first(2, {2.0, -1.0, 1.0});
    const Polynomial second(1, {1.0, 2.0});
    const Polynomial third(1, {-3.0, 1.0});
    EXPECT_EQ((first + second) - second, first);
    EXPECT_EQ((first * second) / second, first);
    EXPECT_EQ(first * (second + third), first * second + first * third);
}
TEST(PolynomialTests, P35_Equality_Cases) {
    EXPECT_EQ(Polynomial(3, {1.0, 2.0, 0.0, 0.0}), Polynomial(1, {1.0, 2.0}));
    EXPECT_EQ(Polynomial(0, {0.0}), Polynomial(0, {5e-10}));
    EXPECT_NE(Polynomial(0, {0.0}), Polynomial(0, {2e-9}));
    EXPECT_EQ(Polynomial(), Polynomial(0, {1e-12}));
    EXPECT_EQ(Polynomial(1, {1.0, 2.0}) != Polynomial(1, {1.0, 2.0}), false);
}
TEST(PolynomialTests, P36_Copy_ConstructorEqualAndIndependent) {
    Polynomial original(2, {1.0, 2.0, 3.0});
    Polynomial copy(original);
    EXPECT_EQ(copy, original);
    copy[0U] = 99.0;
    EXPECT_DOUBLE_EQ(original[0U], 1.0);
}
TEST(PolynomialTests, P37_Copy_DropsDeadCoefficients) {
    Polynomial original(2, {1.0, 2.0, 3.0});
    original[2U] = 0.0;
    const Polynomial copy(original);
    EXPECT_EQ(copy.degree(), 1U);
    EXPECT_EQ(copy, original);
    EXPECT_THROW(static_cast<void>(original[2U]), CoefficientIndexError);
}
TEST(PolynomialTests, P38_Copy_AssignDifferentDegrees) {
    Polynomial target(0, {9.0});
    const Polynomial source(3, {1.0, 2.0, 3.0, 4.0});
    target = source;
    EXPECT_EQ(target, source);
    target = Polynomial(0, {2.0});
    EXPECT_EQ(target.degree(), 0U);
}
TEST(PolynomialTests, P39_Copy_SelfAssignment) {
    Polynomial value(1, {1.0, 2.0});
    value = value;
    EXPECT_EQ(value, Polynomial(1, {1.0, 2.0}));
}
TEST(PolynomialTests, P40_Copy_ChainedAssignment) {
    Polynomial first;
    Polynomial second;
    Polynomial third(2, {3.0, 2.0, 1.0});
    first = second = third;
    EXPECT_EQ(first, third);
    EXPECT_EQ(second, third);
}
TEST(PolynomialTests, P41_Copy_AssignStrongGuarantee) {
    Polynomial target(1, {3.0, 4.0});
    const Polynomial original(target);
    const Polynomial source(1000, std::vector<double>(1001U, 1.0));
    fail_next_allocation = true;
    EXPECT_THROW(target = source, std::bad_alloc);
    EXPECT_EQ(target, original);
}
TEST(PolynomialTests, P42_Io_WriteFormat) {
    const Polynomial value(2, {-5.0, 0.0, 3.0});
    std::ostringstream output;
    output << value;
    EXPECT_EQ(output.str(), "2 -5 0 3");
    EXPECT_EQ(output.str().back(), '3');
    std::ostringstream zero;
    zero << Polynomial();
    EXPECT_EQ(zero.str(), "0 0");
    std::ostringstream fixed;
    fixed << std::fixed << std::setprecision(2);
    const auto flags = fixed.flags();
    const auto precision = fixed.precision();
    fixed << Polynomial(1, {0.1, 2.0});
    EXPECT_EQ(fixed.flags(), flags);
    EXPECT_EQ(fixed.precision(), precision);
}
TEST(PolynomialTests, P43_Io_WritePrecisionAndRoundTrip) {
    const Polynomial original(1, {0.1, 2.0});
    std::ostringstream output;
    output << original;
    EXPECT_NE(output.str().find("0.10000000000000001"), std::string::npos);
    Polynomial restored;
    std::istringstream input(output.str());
    input >> restored;
    EXPECT_EQ(restored, original);
}
TEST(PolynomialTests, P44_Io_ReadValidVariants) {
    const std::string variants[] = {"2 -5 0 3", "+1 2 3", "0 7", "1 .5 -1e2", "1 5. 2", "2\n-5\n0\n3"};
    for (const std::string& text : variants) {
        Polynomial value;
        std::istringstream input(text);
        input >> value;
        EXPECT_GE(value.degree(), 0U);
    }
}
TEST(PolynomialTests, P45_Io_ReadInvalidTable) {
    struct Case { std::string text; std::string message; int category; };
    const Case cases[] = {
        {"", "Invalid polynomial format: degree is missing", 0},
        {"x", "Invalid polynomial format: degree is not an integer: 'x'", 0},
        {"1.5 1 2", "Invalid polynomial format: degree is not an integer: '1.5'", 0},
        {"1e2 1 2", "Invalid polynomial format: degree is not an integer: '1e2'", 0},
        {"-1 0", "Invalid degree: -1 (allowed: 0..1000)", 1},
        {"1001", "Invalid degree: 1001 (allowed: 0..1000)", 1},
        {"2 1 2", "Invalid polynomial format: expected 3 coefficients, got 2", 0},
        {"1 a 2", "Invalid polynomial format: coefficient is not a number: 'a'", 0},
        {"1 1e 2", "Invalid polynomial format: coefficient is not a number: '1e'", 0},
        {"1 0x10 2", "Invalid polynomial format: coefficient is not a number: '0x10'", 0},
        {"1 nan 2", "Invalid polynomial format: coefficient is not a number: 'nan'", 0},
        {"1 inf 2", "Invalid polynomial format: coefficient is not a number: 'inf'", 0},
        {"1 1,5 2", "Invalid polynomial format: coefficient is not a number: '1,5'", 0},
        {"1 1e999 2", "Non-finite coefficient at power 0", 2}
    };
    for (const Case& item : cases) {
        Polynomial value;
        std::istringstream input(item.text);
        try { input >> value; FAIL() << item.text; }
        catch (const InvalidDegreeError& error) {
            EXPECT_EQ(item.category, 1);
            EXPECT_EQ(error.what(), item.message);
        } catch (const NonFiniteCoefficientError& error) {
            EXPECT_EQ(item.category, 2);
            EXPECT_EQ(error.what(), item.message);
        } catch (const InvalidPolynomialFormatError& error) {
            EXPECT_EQ(item.category, 0);
            EXPECT_EQ(error.what(), item.message);
        }
    }
}
TEST(PolynomialTests, P46_Io_ReadFailureKeepsTarget) {
    Polynomial value(1, {3.0, 4.0});
    const Polynomial original(value);
    std::istringstream input("2 1 invalid 3");
    EXPECT_THROW(input >> value, InvalidPolynomialFormatError);
    EXPECT_EQ(value, original);
}
TEST(PolynomialTests, P47_Io_ReadTwoFromOneStream) {
    std::istringstream input("1 1 2 2 3 4 5");
    Polynomial first;
    Polynomial second;
    input >> first >> second;
    EXPECT_EQ(first, Polynomial(1, {1.0, 2.0}));
    EXPECT_EQ(second, Polynomial(2, {3.0, 4.0, 5.0}));
}
TEST(PolynomialTests, P48_Io_ReadNormalizes) {
    Polynomial value;
    std::istringstream input("3 1 2 0 0");
    input >> value;
    EXPECT_EQ(value.degree(), 1U);
}
TEST(PolynomialTests, P49_Exceptions_Hierarchy) {
    static_assert(std::is_base_of_v<PolynomialError, InvalidDegreeError>);
    static_assert(std::is_base_of_v<PolynomialError, CoefficientIndexError>);
    static_assert(std::is_base_of_v<std::runtime_error, DivisionByZeroPolynomialError>);
    EXPECT_STREQ(DivisionByZeroPolynomialError().what(), "Division by zero polynomial");
}
TEST(PolynomialTests, P50_Structure_HeadersSelfContained) {
    static_assert(std::is_copy_constructible_v<Polynomial>);
    static_assert(std::is_copy_assignable_v<Polynomial>);
    static_assert(!std::is_convertible_v<double, Polynomial>);
    EXPECT_TRUE(std::is_default_constructible_v<Polynomial>);
}

}  // namespace
}  // namespace polynomial
