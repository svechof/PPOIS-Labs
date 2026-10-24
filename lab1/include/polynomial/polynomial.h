#pragma once

#include <cstddef>
#include <iosfwd>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

/// @defgroup polynomial Многочлены
/// @brief Класс многочлена и поддерживаемые арифметические операции.
/// @details
/// Класс polynomial::Polynomial представляет многочлен одной переменной
/// с вещественными коэффициентами типа double:
/// p(x) = a_0 + a_1*x + ... + a_n*x^n.
///
/// Коэффициенты хранятся по возрастанию степеней: индекс 0 соответствует
/// свободному члену, а индекс i — коэффициенту при x^i.
///
/// Поддерживаются вычисление значения многочлена, доступ к коэффициентам,
/// сложение, вычитание, умножение, деление и сравнение, а также текстовый
/// ввод и вывод.
///
/// При нормализации старшие коэффициенты, близкие к нулю, не увеличивают
/// эффективную степень. Операция деления возвращает частное, не сохраняя
/// остаток. Деление на нулевой многочлен вызывает исключение.
/// @{
namespace polynomial {

/// @brief Максимально допустимая степень многочлена.
inline constexpr std::size_t kMaxDegree = 1000;
/// @brief Абсолютный порог пренебрежимо малого коэффициента.
inline constexpr double kCoefficientEpsilon = 1e-9;
/// @brief Точность вывода, гарантирующая round-trip для конечного double.
inline constexpr int kOutputPrecision = std::numeric_limits<double>::max_digits10;

/// @brief Базовое исключение домена многочленов.
class PolynomialError : public std::runtime_error {
public:
    /// @brief Создаёт исключение с заданным сообщением.
    /// @param message Текст, возвращаемый `what()`.
    explicit PolynomialError(const std::string& message);
};

/// @brief Степень многочлена не входит в допустимый диапазон.
class InvalidDegreeError : public PolynomialError {
public:
    /// @brief Создаёт сообщение об ошибке степени.
    /// @param degree Недопустимая степень.
    /// @details Формат: `Invalid degree: <d> (allowed: 0..1000)`.
    explicit InvalidDegreeError(int degree);
};

/// @brief Размер вектора коэффициентов не соответствует объявленной степени.
class CoefficientCountMismatchError : public PolynomialError {
public:
    /// @brief Создаёт сообщение о несовпадении количества коэффициентов.
    /// @param degree Объявленная степень.
    /// @param count Фактическое количество коэффициентов.
    /// @details Формат: `Coefficient count mismatch: degree <d> requires <d+1> coefficients, got <k>`.
    CoefficientCountMismatchError(int degree, std::size_t count);
};

/// @brief Коэффициент или результат арифметики не является конечным числом.
class NonFiniteCoefficientError : public PolynomialError {
public:
    /// @brief Создаёт сообщение о неконечном коэффициенте.
    /// @param power Степень, при которой найден неконечный коэффициент.
    /// @details Формат: `Non-finite coefficient at power <i>`.
    explicit NonFiniteCoefficientError(std::size_t power);
};

/// @brief Запрошен коэффициент выше текущей степени многочлена.
class CoefficientIndexError : public PolynomialError {
public:
    /// @brief Создаёт сообщение о недопустимом индексе.
    /// @param power Запрошенная степень.
    /// @param degree Текущая степень многочлена.
    /// @details Формат: `Coefficient index out of range: <i> (degree is <d>)`.
    CoefficientIndexError(std::size_t power, std::size_t degree);
};

/// @brief Выполнено деление на нулевой многочлен.
class DivisionByZeroPolynomialError : public PolynomialError {
public:
    /// @brief Создаёт исключение с сообщением `Division by zero polynomial`.
    DivisionByZeroPolynomialError();
};

/// @brief Текстовое представление многочлена некорректно.
class InvalidPolynomialFormatError : public PolynomialError {
public:
    /// @brief Создаёт исключение с деталями нарушения формата.
    /// @param details Описание ошибки разбора.
    explicit InvalidPolynomialFormatError(const std::string& details);

    /// @brief Сообщает, что степень отсутствует.
    [[nodiscard]] static InvalidPolynomialFormatError degree_missing();
    /// @brief Сообщает, что лексема степени не является целым числом.
    /// @param token Некорректная лексема.
    [[nodiscard]] static InvalidPolynomialFormatError degree_not_integer(const std::string& token);
    /// @brief Сообщает о недостаточном количестве коэффициентов.
    /// @param expected Ожидаемое количество коэффициентов.
    /// @param actual Фактическое количество коэффициентов.
    [[nodiscard]] static InvalidPolynomialFormatError coefficient_count_mismatch(
        std::size_t expected, std::size_t actual);
    /// @brief Сообщает, что лексема не является допустимым коэффициентом.
    /// @param token Некорректная лексема.
    [[nodiscard]] static InvalidPolynomialFormatError coefficient_not_number(const std::string& token);
};

/// @brief Многочлен от одной переменной с вещественными коэффициентами.
/// @details Элемент `coefficients_[i]` — коэффициент при `x^i`. Старшие
///          коэффициенты с модулем не больше `kCoefficientEpsilon` не входят
///          в эффективную степень. После конструирования, копирования и
///          арифметики вектор хранится в каноническом виде.
/// @invariant Коэффициенты не пусты и при создании/арифметике конечны.
/// @invariant `degree() == effective_size() - 1`.
/// @code
/// polynomial::Polynomial value(2, {-5.0, 0.0, 3.0});
/// double result = value(2.0); // 7.0
/// @endcode
class Polynomial {
public:
    /// @brief Создаёт нулевой многочлен.
    /// @post `degree() == 0`, `operator[](0) == 0.0`, `is_zero() == true`.
    Polynomial();

    /// @brief Создаёт многочлен по объявленной степени и коэффициентам.
    /// @param degree Степень от 0 до `kMaxDegree` включительно.
    /// @param coefficients Коэффициенты от свободного члена до старшего.
    /// @post Многочлен нормализован; фактическая степень может быть меньше `degree`.
    /// @throws InvalidDegreeError Если степень вне допустимого диапазона.
    /// @throws CoefficientCountMismatchError Если `coefficients.size() != degree + 1`.
    /// @throws NonFiniteCoefficientError Если коэффициент не является конечным.
    Polynomial(int degree, const std::vector<double>& coefficients);

    /// @brief Создаёт каноническую глубокую копию многочлена.
    /// @param other Копируемый объект.
    /// @post Копия равна `other`; коэффициенты выше эффективной степени отбрасываются.
    Polynomial(const Polynomial& other);

    /// @brief Уничтожает объект; ресурсы освобождает `std::vector`.
    ~Polynomial() = default;

    /// @brief Присваивает значение другого многочлена со строгой гарантией.
    /// @param other Источник копирования.
    /// @return Ссылка на `*this`.
    Polynomial& operator=(const Polynomial& other);

    /// @brief Возвращает нормализованную степень многочлена.
    /// @return `effective_size() - 1`; для нулевого многочлена — 0.
    [[nodiscard]] std::size_t degree() const noexcept;

    /// @brief Проверяет, является ли многочлен нулевым с учётом эпсилона.
    /// @return `true`, если степень равна нулю и свободный член пренебрежимо мал.
    [[nodiscard]] bool is_zero() const noexcept;

    /// @brief Получает коэффициент при указанной степени для чтения.
    /// @param power Степень слагаемого.
    /// @return Константная ссылка на коэффициент.
    /// @throws CoefficientIndexError Если `power > degree()`.
    [[nodiscard]] const double& operator[](std::size_t power) const;

    /// @brief Получает коэффициент для записи без увеличения степени.
    /// @param power Степень слагаемого.
    /// @return Ссылка на коэффициент.
    /// @details Запись пренебрежимо малого значения в старший коэффициент
    ///          уменьшает эффективную степень; после этого старший индекс недоступен.
    /// @throws CoefficientIndexError Если `power > degree()`.
    double& operator[](std::size_t power);

    /// @brief Вычисляет значение многочлена по схеме Горнера.
    /// @param point Точка вычисления.
    /// @return Значение многочлена; переполнение даёт IEEE 754 `+Inf`/`-Inf`.
    /// @note Метод `noexcept`; неконечный результат не вызывает исключение.
    [[nodiscard]] double operator()(double point) const noexcept;

    /// @brief Прибавляет `rhs` к текущему многочлену.
    /// @param rhs Второе слагаемое.
    /// @return Ссылка на `*this`.
    /// @throws NonFiniteCoefficientError Если коэффициент результата неконечен.
    Polynomial& operator+=(const Polynomial& rhs);

    /// @brief Вычитает `rhs` из текущего многочлена.
    /// @param rhs Вычитаемое.
    /// @return Ссылка на `*this`.
    /// @throws NonFiniteCoefficientError Если коэффициент результата неконечен.
    Polynomial& operator-=(const Polynomial& rhs);

    /// @brief Умножает текущий многочлен на `rhs`.
    /// @param rhs Второй множитель.
    /// @return Ссылка на `*this`.
    /// @throws InvalidDegreeError Если степень произведения выше `kMaxDegree`.
    /// @throws NonFiniteCoefficientError Если коэффициент результата неконечен.
    Polynomial& operator*=(const Polynomial& rhs);

    /// @brief Делит текущий многочлен на `rhs`, отбрасывая остаток.
    /// @param rhs Делитель.
    /// @return Ссылка на `*this`.
    /// @throws DivisionByZeroPolynomialError Если делитель нулевой.
    /// @throws NonFiniteCoefficientError Если частное содержит неконечный коэффициент.
    Polynomial& operator/=(const Polynomial& rhs);

    /// @brief Сравнивает нормализованные коэффициенты с абсолютным допуском.
    /// @param other Сравниваемый многочлен.
    /// @return `true`, если степени равны и все коэффициенты отличаются не более чем на эпсилон.
    [[nodiscard]] bool operator==(const Polynomial& other) const noexcept;

    /// @brief Проверяет неравенство двух многочленов.
    /// @param other Сравниваемый многочлен.
    /// @return Отрицание результата `operator==`.
    [[nodiscard]] bool operator!=(const Polynomial& other) const noexcept;

private:
    /// @brief Возвращает эффективное количество коэффициентов, не меньше одного.
    [[nodiscard]] std::size_t effective_size() const noexcept;
    /// @brief Возвращает коэффициент или 0 для степени выше эффективной.
    /// @param power Индекс коэффициента.
    [[nodiscard]] double coefficient_or_zero(std::size_t power) const noexcept;
    /// @brief Удаляет старшие коэффициенты, пренебрежимо близкие к нулю.
    /// @param coefficients Вектор, нормализуемый на месте.
    static void normalize(std::vector<double>& coefficients) noexcept;
    /// @brief Проверяет конечность коэффициентов.
    /// @param coefficients Проверяемый вектор.
    /// @throws NonFiniteCoefficientError Для первого неконечного коэффициента.
    static void require_finite(const std::vector<double>& coefficients);
    /// @brief Создаёт нормализованный объект из сырого вектора коэффициентов.
    /// @param raw Вектор коэффициентов результата.
    /// @return Канонический многочлен.
    static Polynomial from_raw(std::vector<double> raw);
    /// @brief Обменивает содержимое с другим многочленом без исключений.
    /// @param other Второй объект.
    void swap(Polynomial& other) noexcept;

    std::vector<double> coefficients_;
};

/// @brief Складывает два многочлена.
/// @param lhs Первый операнд, переданный по значению.
/// @param rhs Второй операнд.
/// @return Копия `lhs` после применения `+= rhs`.
[[nodiscard]] Polynomial operator+(Polynomial lhs, const Polynomial& rhs);
/// @brief Вычитает второй многочлен из первого.
/// @param lhs Уменьшаемое, переданное по значению.
/// @param rhs Вычитаемое.
/// @return Копия `lhs` после применения `-= rhs`.
[[nodiscard]] Polynomial operator-(Polynomial lhs, const Polynomial& rhs);
/// @brief Умножает два многочлена.
/// @param lhs Первый множитель, переданный по значению.
/// @param rhs Второй множитель.
/// @return Копия `lhs` после применения `*= rhs`.
[[nodiscard]] Polynomial operator*(Polynomial lhs, const Polynomial& rhs);
/// @brief Делит многочлены и отбрасывает остаток.
/// @param lhs Делимое, переданное по значению.
/// @param rhs Делитель.
/// @return Копия `lhs` после применения `/= rhs`.
/// @throws DivisionByZeroPolynomialError Если делитель нулевой.
[[nodiscard]] Polynomial operator/(Polynomial lhs, const Polynomial& rhs);

/// @brief Выводит степень и коэффициенты в каноническом текстовом формате.
/// @param out Выходной поток.
/// @param value Выводимый многочлен.
/// @return Ссылка на поток.
/// @details Между лексемами ровно один пробел; нет завершающего пробела и `\n`.
///          Форматирование и точность потока восстанавливаются после вывода.
std::ostream& operator<<(std::ostream& out, const Polynomial& value);

/// @brief Читает степень и коэффициенты, обновляя цель только при полном успехе.
/// @param in Входной поток.
/// @param value Целевой объект.
/// @return Ссылка на поток.
/// @throws InvalidPolynomialFormatError Для неверного текстового формата.
/// @throws InvalidDegreeError Для степени вне допустимого диапазона.
/// @throws NonFiniteCoefficientError Для неконечного коэффициента.
std::istream& operator>>(std::istream& in, Polynomial& value);

}  // namespace polynomial
/// @}
