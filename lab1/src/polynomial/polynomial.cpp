#include "polynomial/polynomial.h"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <ios>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace polynomial {
namespace {

std::string degree_message(int degree) {
    return "Invalid degree: " + std::to_string(degree) + " (allowed: 0.." + std::to_string(kMaxDegree) + ")";
}

std::string count_message(int degree, std::size_t count) {
    return "Coefficient count mismatch: degree " + std::to_string(degree) + " requires " +
           std::to_string(static_cast<std::size_t>(degree) + 1U) + " coefficients, got " + std::to_string(count);
}

// Shift of the divisor inside the remainder and the factor applied to it.
struct ShiftedScale {
    std::size_t shift;
    double scale;
};

// Intermediate state of the long division.
struct DivisionState {
    std::vector<double> quotient;
    std::vector<double> remainder;
};

std::vector<double> coefficients_of(const Polynomial& value) {
    std::vector<double> result;
    result.reserve(value.degree() + 1U);
    for (std::size_t power = 0; power <= value.degree(); ++power) {
        result.push_back(value[power]);
    }
    return result;
}

std::vector<double> multiply_raw(const Polynomial& lhs, const Polynomial& rhs) {
    std::vector<double> raw(lhs.degree() + rhs.degree() + 1U, 0.0);
    for (std::size_t left_power = 0; left_power <= lhs.degree(); ++left_power) {
        for (std::size_t right_power = 0; right_power <= rhs.degree(); ++right_power) {
            raw[left_power + right_power] += lhs[left_power] * rhs[right_power];
        }
    }
    return raw;
}

void subtract_scaled_divisor(std::vector<double>& remainder, const Polynomial& divisor, const ShiftedScale& step) {
    for (std::size_t power = 0; power <= divisor.degree(); ++power) {
        double& target = remainder[step.shift + power];
        target -= step.scale * divisor[power];
        if (!std::isfinite(target)) {
            throw NonFiniteCoefficientError(step.shift + power);
        }
    }
}

void divide_step(DivisionState& state, const Polynomial& divisor, const ShiftedScale& step) {
    const std::size_t leading_power = step.shift + divisor.degree();
    const double factor = state.remainder[leading_power] / step.scale;
    if (!std::isfinite(factor)) {
        throw NonFiniteCoefficientError(step.shift);
    }
    state.quotient[step.shift] = factor;
    subtract_scaled_divisor(state.remainder, divisor, ShiftedScale{step.shift, factor});
}

std::vector<double> divide_raw(const Polynomial& lhs, const Polynomial& rhs) {
    const std::size_t quotient_degree = lhs.degree() - rhs.degree();
    DivisionState state{std::vector<double>(quotient_degree + 1U, 0.0), coefficients_of(lhs)};
    const double divisor_lead = rhs[rhs.degree()];
    for (std::size_t step = quotient_degree + 1U; step > 0U; --step) {
        divide_step(state, rhs, ShiftedScale{step - 1U, divisor_lead});
    }
    return std::move(state.quotient);
}

class StreamState final {
public:
    explicit StreamState(std::ostream& output)
        : output_(output), flags_(output.flags()), precision_(output.precision()) {
    }
    ~StreamState() noexcept {
        output_.flags(flags_);
        output_.precision(precision_);
    }
    StreamState(const StreamState&) = delete;
    StreamState& operator=(const StreamState&) = delete;

private:
    std::ostream& output_;
    std::ios_base::fmtflags flags_;
    std::streamsize precision_;
};

bool is_digit(char character) {
    return character >= '0' && character <= '9';
}

std::size_t skip_sign(const std::string& token, std::size_t index) {
    const bool has_sign = index < token.size() && (token[index] == '+' || token[index] == '-');
    return has_sign ? index + 1U : index;
}

std::size_t skip_digits(const std::string& token, std::size_t index) {
    while (index < token.size() && is_digit(token[index])) {
        ++index;
    }
    return index;
}

// Returns the index after an optional exponent, or npos if the exponent is malformed.
std::size_t skip_exponent(const std::string& token, std::size_t index) {
    if (index >= token.size() || (token[index] != 'e' && token[index] != 'E')) {
        return index;
    }
    const std::size_t digits_start = skip_sign(token, index + 1U);
    const std::size_t digits_end = skip_digits(token, digits_start);
    return digits_end == digits_start ? std::string::npos : digits_end;
}

bool is_integer_token(const std::string& token) {
    const std::size_t start = skip_sign(token, 0U);
    return start < token.size() && skip_digits(token, start) == token.size();
}

bool is_coefficient_token(const std::string& token) {
    const std::size_t start = skip_sign(token, 0U);
    std::size_t index = skip_digits(token, start);
    std::size_t digits = index - start;
    if (index < token.size() && token[index] == '.') {
        const std::size_t fraction_start = index + 1U;
        index = skip_digits(token, fraction_start);
        digits += index - fraction_start;
    }
    if (digits == 0U) {
        return false;
    }
    return skip_exponent(token, index) == token.size();
}

int out_of_range_degree(bool negative) {
    return negative ? -1 : static_cast<int>(kMaxDegree + 1U);
}

long long parse_degree_token(const std::string& token) {
    try {
        return std::stoll(token);
    } catch (const std::out_of_range&) {
        throw InvalidDegreeError(out_of_range_degree(token.front() == '-'));
    }
}

int read_degree(std::istream& input) {
    std::string token;
    if (!(input >> token)) {
        throw InvalidPolynomialFormatError::degree_missing();
    }
    if (!is_integer_token(token)) {
        throw InvalidPolynomialFormatError::degree_not_integer(token);
    }
    const long long parsed = parse_degree_token(token);
    if (parsed < std::numeric_limits<int>::min() || parsed > std::numeric_limits<int>::max()) {
        throw InvalidDegreeError(out_of_range_degree(parsed < 0));
    }
    const int degree = static_cast<int>(parsed);
    if (degree < 0 || static_cast<std::size_t>(degree) > kMaxDegree) {
        throw InvalidDegreeError(degree);
    }
    return degree;
}

// Holds the end pointer as a member: strtod needs a mutable char**.
struct NumberParse {
    char* end = nullptr;
    double value = 0.0;
};

double parse_coefficient(const std::string& token, std::size_t power) {
    NumberParse parse;
    errno = 0;
    parse.value = std::strtod(token.c_str(), &parse.end);
    if (parse.end == token.c_str() || *parse.end != '\0') {
        throw InvalidPolynomialFormatError::coefficient_not_number(token);
    }
    if (!std::isfinite(parse.value)) {
        throw NonFiniteCoefficientError(power);
    }
    return parse.value;
}

std::vector<double> read_coefficients(std::istream& input, std::size_t expected) {
    std::vector<double> coefficients;
    coefficients.reserve(expected);
    for (std::size_t power = 0; power < expected; ++power) {
        std::string token;
        if (!(input >> token)) {
            throw InvalidPolynomialFormatError::coefficient_count_mismatch(expected, coefficients.size());
        }
        if (!is_coefficient_token(token)) {
            throw InvalidPolynomialFormatError::coefficient_not_number(token);
        }
        coefficients.push_back(parse_coefficient(token, power));
    }
    return coefficients;
}

} // namespace

PolynomialError::PolynomialError(const std::string& message) : std::runtime_error(message) {
}
InvalidDegreeError::InvalidDegreeError(int degree) : PolynomialError(degree_message(degree)) {
}
CoefficientCountMismatchError::CoefficientCountMismatchError(int degree, std::size_t count)
    : PolynomialError(count_message(degree, count)) {
}
NonFiniteCoefficientError::NonFiniteCoefficientError(std::size_t power)
    : PolynomialError("Non-finite coefficient at power " + std::to_string(power)) {
}
CoefficientIndexError::CoefficientIndexError(std::size_t power, std::size_t degree)
    : PolynomialError("Coefficient index out of range: " + std::to_string(power) + " (degree is " +
                      std::to_string(degree) + ")") {
}
DivisionByZeroPolynomialError::DivisionByZeroPolynomialError() : PolynomialError("Division by zero polynomial") {
}
InvalidPolynomialFormatError::InvalidPolynomialFormatError(const std::string& details)
    : PolynomialError("Invalid polynomial format: " + details) {
}
InvalidPolynomialFormatError InvalidPolynomialFormatError::degree_missing() {
    return InvalidPolynomialFormatError("degree is missing");
}
InvalidPolynomialFormatError InvalidPolynomialFormatError::degree_not_integer(const std::string& token) {
    return InvalidPolynomialFormatError("degree is not an integer: '" + token + "'");
}
InvalidPolynomialFormatError InvalidPolynomialFormatError::coefficient_count_mismatch(std::size_t expected,
                                                                                      std::size_t actual) {
    return InvalidPolynomialFormatError("expected " + std::to_string(expected) + " coefficients, got " +
                                        std::to_string(actual));
}
InvalidPolynomialFormatError InvalidPolynomialFormatError::coefficient_not_number(const std::string& token) {
    return InvalidPolynomialFormatError("coefficient is not a number: '" + token + "'");
}

Polynomial::Polynomial() : coefficients_{0.0} {
}
Polynomial::Polynomial(int degree, const std::vector<double>& coefficients) {
    if (degree < 0 || static_cast<std::size_t>(degree) > kMaxDegree) {
        throw InvalidDegreeError(degree);
    }
    if (coefficients.size() != static_cast<std::size_t>(degree) + 1U) {
        throw CoefficientCountMismatchError(degree, coefficients.size());
    }
    require_finite(coefficients);
    coefficients_ = coefficients;
    normalize(coefficients_);
}
Polynomial::Polynomial(const Polynomial& other)
    : coefficients_(other.coefficients_.begin(),
                    other.coefficients_.begin() + static_cast<std::ptrdiff_t>(other.effective_size())) {
    normalize(coefficients_);
}
Polynomial& Polynomial::operator=(const Polynomial& other) {
    if (this != &other) {
        Polynomial copy(other);
        swap(copy);
    }
    return *this;
}
std::size_t Polynomial::degree() const noexcept {
    return effective_size() - 1U;
}
bool Polynomial::is_zero() const noexcept {
    return degree() == 0U && std::abs(coefficients_[0]) <= kCoefficientEpsilon;
}
const double& Polynomial::operator[](std::size_t power) const {
    if (power > degree()) {
        throw CoefficientIndexError(power, degree());
    }
    return coefficients_[power];
}
double& Polynomial::operator[](std::size_t power) {
    if (power > degree()) {
        throw CoefficientIndexError(power, degree());
    }
    return coefficients_[power];
}
double Polynomial::operator()(double point) const noexcept {
    double result = 0.0;
    for (std::size_t power = effective_size(); power > 0U; --power) {
        result = (result * point) + coefficients_[power - 1U];
    }
    return result;
}
std::size_t Polynomial::effective_size() const noexcept {
    std::size_t size = coefficients_.size();
    while (size > 1U && std::abs(coefficients_[size - 1U]) <= kCoefficientEpsilon) {
        --size;
    }
    return size;
}
double Polynomial::coefficient_or_zero(std::size_t power) const noexcept {
    return power < effective_size() ? coefficients_[power] : 0.0;
}
void Polynomial::normalize(std::vector<double>& coefficients) noexcept {
    while (coefficients.size() > 1U && std::abs(coefficients.back()) <= kCoefficientEpsilon) {
        coefficients.pop_back();
    }
    if (coefficients.size() == 1U && std::abs(coefficients.front()) <= kCoefficientEpsilon) {
        coefficients.front() = 0.0;
    }
}
void Polynomial::require_finite(const std::vector<double>& coefficients) {
    for (std::size_t power = 0; power < coefficients.size(); ++power) {
        if (!std::isfinite(coefficients[power])) {
            throw NonFiniteCoefficientError(power);
        }
    }
}
Polynomial Polynomial::from_raw(std::vector<double> raw) {
    if (raw.empty()) {
        raw.push_back(0.0);
    }
    if (raw.size() - 1U > kMaxDegree) {
        throw InvalidDegreeError(static_cast<int>(raw.size() - 1U));
    }
    require_finite(raw);
    normalize(raw);
    Polynomial result;
    result.coefficients_.swap(raw);
    return result;
}
void Polynomial::swap(Polynomial& other) noexcept {
    coefficients_.swap(other.coefficients_);
}
bool Polynomial::operator==(const Polynomial& other) const noexcept {
    if (degree() != other.degree()) {
        return false;
    }
    for (std::size_t power = 0; power <= degree(); ++power) {
        if (std::abs(coefficient_or_zero(power) - other.coefficient_or_zero(power)) > kCoefficientEpsilon) {
            return false;
        }
    }
    return true;
}
bool Polynomial::operator!=(const Polynomial& other) const noexcept {
    return !(*this == other);
}

Polynomial& Polynomial::operator+=(const Polynomial& rhs) {
    const std::size_t result_size = std::max(effective_size(), rhs.effective_size());
    std::vector<double> raw(result_size, 0.0);
    for (std::size_t power = 0; power < result_size; ++power) {
        raw[power] = coefficient_or_zero(power) + rhs.coefficient_or_zero(power);
    }
    require_finite(raw);
    Polynomial result = from_raw(std::move(raw));
    swap(result);
    return *this;
}

Polynomial& Polynomial::operator-=(const Polynomial& rhs) {
    const std::size_t result_size = std::max(effective_size(), rhs.effective_size());
    std::vector<double> raw(result_size, 0.0);
    for (std::size_t power = 0; power < result_size; ++power) {
        raw[power] = coefficient_or_zero(power) - rhs.coefficient_or_zero(power);
    }
    require_finite(raw);
    Polynomial result = from_raw(std::move(raw));
    swap(result);
    return *this;
}

Polynomial& Polynomial::operator*=(const Polynomial& rhs) {
    const std::size_t result_degree = degree() + rhs.degree();
    if (result_degree > kMaxDegree) {
        throw InvalidDegreeError(static_cast<int>(result_degree));
    }
    if (is_zero() || rhs.is_zero()) {
        Polynomial zero;
        swap(zero);
        return *this;
    }
    std::vector<double> raw = multiply_raw(*this, rhs);
    require_finite(raw);
    Polynomial result = from_raw(std::move(raw));
    swap(result);
    return *this;
}

Polynomial& Polynomial::operator/=(const Polynomial& rhs) {
    if (rhs.is_zero()) {
        throw DivisionByZeroPolynomialError();
    }
    if (is_zero() || degree() < rhs.degree()) {
        Polynomial zero;
        swap(zero);
        return *this;
    }
    std::vector<double> quotient = divide_raw(*this, rhs);
    require_finite(quotient);
    Polynomial result = from_raw(std::move(quotient));
    swap(result);
    return *this;
}

Polynomial operator+(Polynomial lhs, const Polynomial& rhs) {
    lhs += rhs;
    return lhs;
}
Polynomial operator-(Polynomial lhs, const Polynomial& rhs) {
    lhs -= rhs;
    return lhs;
}
Polynomial operator*(Polynomial lhs, const Polynomial& rhs) {
    lhs *= rhs;
    return lhs;
}
Polynomial operator/(Polynomial lhs, const Polynomial& rhs) {
    lhs /= rhs;
    return lhs;
}

std::ostream& operator<<(std::ostream& out, const Polynomial& value) {
    const StreamState state(out);
    out << std::defaultfloat << std::setprecision(kOutputPrecision) << value.degree();
    for (std::size_t power = 0; power <= value.degree(); ++power) {
        out << ' ' << value[power];
    }
    return out;
}

std::istream& operator>>(std::istream& input, Polynomial& value) {
    const int degree = read_degree(input);
    const Polynomial parsed(degree, read_coefficients(input, static_cast<std::size_t>(degree) + 1U));
    value = parsed;
    return input;
}

} // namespace polynomial
