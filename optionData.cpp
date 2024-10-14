#include "optionData.h"
#include <iostream>
#include "include/json.hpp"
#include <cstdint>
#include "stockData.h"


void printAdditionalUnderlying(const AdditionalUnderlying& au) {
    std::cout << "Amount: " << au.amount << "\n";
    std::cout << "Type: " << au.type << "\n";
    std::cout << "Underlying: " << au.underlying << "\n";
}

void printResult(const Result& result) {
    std::cout << "CFI: " << result.cfi << "\n";
    std::cout << "Contract Type: " << result.contract_type << "\n";
    std::cout << "Exercise Style: " << result.exercise_style << "\n";
    std::cout << "Expiration Date: " << result.expiration_date << "\n";
    std::cout << "Primary Exchange: " << result.primary_exchange << "\n";
    std::cout << "Shares per Contract: " << result.shares_per_contract << "\n";
    std::cout << "Strike Price: " << result.strike_price << "\n";
    std::cout << "Ticker: " << result.ticker << "\n";
    std::cout << "Underlying Ticker: " << result.underlying_ticker << "\n";
    for (const auto& au : result.additional_underlyings) {
        printAdditionalUnderlying(au);
    }
}

void printOptionData(const OptionData& data) {
    std::cout << "Request ID: " << data.request_id << "\n";
    printResult(data.results);
    std::cout << "Status: " << data.status << "\n";
}

void from_json(const nlohmann::json& j, OptionData& data) {
    j.at("request_id").get_to(data.request_id);
    j.at("results").get_to(data.results);
    j.at("status").get_to(data.status);
}

void from_json(const nlohmann::json& j, AdditionalUnderlying& au) {
    j.at("amount").get_to(au.amount);
    j.at("type").get_to(au.type);
    j.at("underlying").get_to(au.underlying);
}

void from_json(const nlohmann::json& j, Result& result) {
    j.at("cfi").get_to(result.cfi);
    j.at("contract_type").get_to(result.contract_type);
    j.at("exercise_style").get_to(result.exercise_style);
    j.at("expiration_date").get_to(result.expiration_date);
    j.at("primary_exchange").get_to(result.primary_exchange);
    j.at("shares_per_contract").get_to(result.shares_per_contract);
    j.at("strike_price").get_to(result.strike_price);
    j.at("ticker").get_to(result.ticker);
    j.at("underlying_ticker").get_to(result.underlying_ticker);
    if (j.find("additional_underlyings") != j.end()) {
        j.at("additional_underlyings").get_to(result.additional_underlyings);
    }
}

double StockData::calculateVolatility() const {
    if(results.size()<2) {
        return 0.0;
    }
    std::vector<double> dailyReturns;
    for(uint8_t i = 1; i<results.size();i++) {
        double previousClose = results[i-1].c;
        double currentClose = results[i].c;
        double dailyReturn = (currentClose-previousClose)/previousClose;
        dailyReturns.push_back(dailyReturn);
    }

    double mean = std::accumulate(dailyReturns.begin(),dailyReturns.end(),0.0)/dailyReturns.size();
    double variance = 0.0;
    for(double returnVal : dailyReturns) {
        variance += (returnVal-mean) * (returnVal-mean);
    }

    variance /= dailyReturns.size();
    double yearlyVolatility = std::sqrt(variance);
    return yearlyVolatility * std::sqrt(252);
}

double norm_cdf(double x) {
    return 0.5 * std::erfc(-x * std::sqrt(0.5));
}

double norm_pdf(double x) {
    return std::exp(-0.5*x*x) / std::sqrt(2*M_PI);
}

double blackScholesParams::callPrice() const {
    double d1 = (std::log(S/K) + (r + 0.5 * sigma * sigma) * T) / (sigma *std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    return S * norm_cdf(d1) - K * std::exp(-r * T) * norm_cdf(d2);
}

double blackScholesParams::putPrice() const {
    double d1 = (std::log(S/K) + (r + 0.5 * sigma * sigma) * T) / (sigma*std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    return K * std::exp(-r * T) * norm_cdf(-d2) - S * norm_cdf(-d1);
}

double blackScholesParams::delta() const {
    double d1 = (std::log(S/K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    if(isCall) {
        return norm_cdf(d1);
    } else {
        return norm_cdf(d1) - 1;
    }
}

double blackScholesParams::gamma() const {
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    return norm_pdf(d1) / (S*sigma*std::sqrt(T));
}

double blackScholesParams::theta() const {
    double d1 = (std::log(S/K) + (r+0.5*sigma*sigma)*T) / (sigma*std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    double term1 = -S * norm_pdf(d1) * sigma/(2*std::sqrt(T));
    double term2 = r * K * std::exp(-r*T) * norm_cdf(d2);
    if (isCall) {
        return term1 - term2;
    } else {
        return term1 + term2;
    }
}

double blackScholesParams::vega() const {
    double d1 = (std::log(S/K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    return S * norm_pdf(d1) * std::sqrt(T);
}

double blackScholesParams::rho() const {
    double d1 = (std::log(S/K) + (r + 0.5 * sigma * sigma) * T) / (sigma*std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    if (isCall) {
        return K*T * std::exp(-r * T) * norm_cdf(d2);
    } else {
        return -K * T * std::exp(-r * T) * norm_cdf(-d2);
    }
}

double blackScholesParams::impliedVolatility(double marketPrice) {
    double epsilon = 1e-5; // Convergence threshold
    double maxIterations = 100; // Maximum number of iterations
    double sigma = 0.2; // Initial guess for volatility
    double sigmaMin = 1e-5; // Minimum reasonable volatility
    double sigmaMax = 5.0; // Maximum reasonable volatility

    for (int i = 0; i < maxIterations; ++i) {
        this->sigma = sigma;
        double price = isCall ? callPrice() : putPrice();
        double vega = this->vega();

        // Check for division by zero or very small vega
        if (vega < epsilon) {
            return sigma;
        }

        double diff = price - marketPrice;

        if (std::fabs(diff) < epsilon) {
            return sigma;
        }

        sigma -= diff / vega;

        if (sigma < sigmaMin) {
            sigma = sigmaMin;
        } else if (sigma > sigmaMax) {
            sigma = sigmaMax;
        }
        std::cout << sigma << std::endl;
    }

    return sigma; 
}