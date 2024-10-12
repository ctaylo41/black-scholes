#ifndef STOCKDATA_H
#define STOCKDATA_H

#include <string>
#include <vector>
#include "include/json.hpp"

struct StockResult {
    double c;
    double h;
    double l;
    int n;
    double o;
    int64_t t;
    int v;
    double vw;
    void print() const;
};

struct StockData {
    bool adjusted;
    int queryCount;
    std::string request_id;
    std::vector<StockResult> results;
    int resultsCount;
    std::string status;
    std::string ticker;
    int count;
    void print() const;
    double calculateVolatility() const;
};

void from_json(const nlohmann::json& j, StockResult& result);
void from_json(const nlohmann::json& j, StockData& data);
#endif