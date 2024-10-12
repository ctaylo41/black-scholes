#include "stockData.h"
#include <iostream>

void from_json(const nlohmann::json& j, StockResult& result) {
    result.c = j.value("c", 0.0);
    result.h = j.value("h", 0.0);
    result.l = j.value("l", 0.0);
    result.n = j.value("n", 0);
    result.o = j.value("o", 0.0);
    result.t = j.value("t", 0);
    result.v = j.value("v", 0);
    result.vw = j.value("vw", 0.0);
}

void from_json(const nlohmann::json& j, StockData& data) {
    data.adjusted = j.value("adjusted", false);
    data.queryCount = j.value("queryCount", 0);
    data.request_id = j.value("request_id", "");
    data.results = j.value("results", std::vector<StockResult>());
    data.resultsCount = j.value("resultsCount", 0);
    data.status = j.value("status", "");
    data.ticker = j.value("ticker", "");
    data.count = j.value("count", 0);
}

void StockResult::print() const {
    std::cout << "Close price: " << c << "\n"
              << "High price: " << h << "\n"
              << "Low price: " << l << "\n"
              << "Number of transactions: " << n << "\n"
              << "Open price: " << o << "\n"
              << "Timestamp: " << t << "\n"
              << "Volume: " << v << "\n"
              << "Volume-weighted average price: " << vw << "\n";
}

void StockData::print() const {
    std::cout << "Adjusted: " << adjusted << "\n"
              << "Query count: " << queryCount << "\n"
              << "Request ID: " << request_id << "\n"
              << "Results count: " << resultsCount << "\n"
              << "Status: " << status << "\n"
              << "Ticker: " << ticker << "\n"
              << "Count: " << count << "\n";

    std::cout << "Results:\n";
    for (const auto& result : results) {
        result.print();
        std::cout << "-------------------\n";
    }
}
