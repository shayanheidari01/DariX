# DariX Math Module

A comprehensive mathematical library for the DariX programming language, providing essential mathematical functions, constants, and utilities.

## Features

### 📊 Mathematical Constants
- `PI` - π (3.14159...)
- `E` - Euler's number (2.71828...)
- `SQRT2` - Square root of 2 (1.41421...)
- `GOLDEN_RATIO` - Golden ratio φ (1.61803...)

### ⚡ Core Mathematical Functions

#### Power and Root Functions
- `power(base, exponent)` - Power calculation using fast exponentiation
- `squareRoot(x)` - Square root using Newton's method

#### Trigonometric Functions
- `sine(x)` - Sine function using Taylor series
- `cosine(x)` - Cosine function derived from sine

#### Exponential and Logarithmic Functions
- `exponential(x)` - e^x using Taylor series
- `naturalLog(x)` - Natural logarithm (simplified implementation)

### 🔢 Number Theory Functions
- `fact(n)` - Factorial calculation
- `greatestCommonDivisor(a, b)` - GCD using Euclidean algorithm
- `isPrimeNumber(n)` - Prime number checking with optimization

### 📏 Rounding and Utility Functions
- `roundNumber(x)` - Round to nearest integer
- `floorNumber(x)` - Floor function
- `ceilNumber(x)` - Ceiling function

### 📊 Statistical Functions
- `average(array)` - Calculate mean of array
- `findMedian(array)` - Calculate median of sorted array

### 🌍 Angle Conversion
- `degreesToRadians(degrees)` - Convert degrees to radians
- `radiansToDegrees(radians)` - Convert radians to degrees

### 🎲 Random Number Generation
- `setRandomSeed(seed)` - Set random seed
- `randomNumber()` - Generate random float [0, 1)
- `randomInteger(min, max)` - Generate random integer in range

## Usage Examples

```darix
// Basic calculations
var result = power(2, 8)          // 256
var root = squareRoot(16)         // 4

// Trigonometry
var angle = degreesToRadians(90)  // π/2
var sinValue = sine(angle)        // ≈ 1

// Statistics
var data = [1, 2, 3, 4, 5]
var mean = average(data)          // 3
var median = findMedian(data)     // 3

// Practical applications
var circleArea = PI * power(radius, 2)
var hypotenuse = squareRoot(power(a, 2) + power(b, 2))
```

## Implementation Notes

- **Accuracy**: Functions use iterative methods (Newton's method, Taylor series) with sufficient precision for most applications
- **Performance**: Optimized algorithms with early termination conditions
- **Compatibility**: Designed specifically for DariX syntax and limitations
- **Error Handling**: Returns `null` for invalid inputs (negative square roots, etc.)

## Testing

The module includes comprehensive tests demonstrating:
- Mathematical accuracy with known values
- Edge case handling
- Practical application examples
- Performance with iterative calculations

## Files

- `math_module.dx` - Complete math module with demonstrations
- `math.dx` - Advanced version with additional functions
- `math_test.dx` - Comprehensive test suite

## Future Enhancements

Potential additions for future versions:
- Inverse trigonometric functions (arcsin, arccos, arctan)
- Hyperbolic functions (sinh, cosh, tanh)
- Matrix operations
- Complex number support
- Advanced statistical functions
- Numerical integration and differentiation

---

**Note**: This math module is designed to work within DariX's current language capabilities and provides a solid foundation for mathematical computing in DariX applications.