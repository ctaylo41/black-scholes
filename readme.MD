# Black-Scholes Project

## Overview

This project implements the Black-Scholes model, a mathematical model for pricing options. The model is widely used in financial markets to estimate the fair value of options.

## Prerequisites

To build and run this project, you need:

- C++ compiler
- CMake
- libcurl

## How It Works

The Black-Scholes model calculates the theoretical price of European call and put options based on the following parameters:
- **Stock Price (S)**: The current price of the underlying stock.
- **Strike Price (K)**: The price at which the option can be exercised.
- **Time to Maturity (T)**: The time remaining until the option's expiration date.
- **Risk-Free Rate (r)**: The risk-free interest rate.
- **Volatility (σ)**: The standard deviation of the stock's returns.

The model uses these inputs to compute the option price using the Black-Scholes formula.

## Usage

To run the project, use the following command:

```
./black_scholes_project <ticker>
```

Replace `<ticker>` with the stock ticker of your choosing. This is the only required argument.

## Building the Project

1. Ensure you have CMake and libcurl installed on your system.
2. Clone the repository.
3. Navigate to the project directory.
4. Run the following commands:

```
mkdir build
cd build
cmake ..
make
```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contact

For any inquiries, please contact [ctaylor7654@gmail.com](mailto:ctaylor7654@gmail.com).
