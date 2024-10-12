#ifndef OPTIONDATA_H
#define OPTIONDATA_H

#include <string>
#include <vector>
#include "include/json.hpp"
#include "stockData.h"


struct AdditionalUnderlying {
    double amount;
    std::string type;
    std::string underlying;
};

struct Result {
    std::string cfi;
    std::string contract_type;
    std::string exercise_style;
    std::string expiration_date;
    std::string primary_contract;
    std::string primary_exchange;
    int shares_per_contract;
    double strike_price;
    std::string ticker;
    std::string underlying_ticker;
    std::vector<AdditionalUnderlying> additional_underlyings;
};

struct OptionData {
    std::string request_id;
    Result results;
    std::string status;
    StockData stockData;
};

struct blackScholesParams {
    double S;
    double K;
    double T;
    double r;
    double sigma;
    bool isCall;
    void print() const;
    double callPrice() const;
    double putPrice() const;
    double delta() const;
    double gamma() const;
    double theta() const;
    double vega() const;
    double rho() const;
    double impliedVolatility(double marketPrice);
};

void from_json(const nlohmann::json& j, AdditionalUnderlying& au);
void from_json(const nlohmann::json& j, Result& result);
void from_json(const nlohmann::json& j, OptionData& data);

void printAdditionalUnderlying(const AdditionalUnderlying& au);
void printResult(const Result& result);
void printOptionData(const OptionData& data);

#endif