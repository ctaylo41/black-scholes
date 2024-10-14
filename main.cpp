#include <iostream>
#include <string>
#include <curl/curl.h>
#include "include/json.hpp"
#include <vector>
#include "optionData.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <ctime>
#include <iomanip>
#include <chrono>
#include "stockData.h"

void getDateInfo(std::string& nowStr, std::string& pastStr) {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);

    std::ostringstream dateStream;
    dateStream << std::put_time(localTime, "%Y-%m-%d");
    nowStr = dateStream.str();

    localTime->tm_mday -= 30;
    std::mktime(localTime);

    std::ostringstream pastStream;
    pastStream << std::put_time(localTime, "%Y-%m-%d");
    pastStr = pastStream.str();
}

std::tm parseDate(const std::string& dateStr) {
    std::tm tm = {};
    std::istringstream ss(dateStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    return tm;
}

double dateDifference(std::string& t1,std::string& t2) {
    std::tm tm1 = parseDate(t1);
    std::tm tm2 = parseDate(t2);

    auto time_point1 = std::chrono::system_clock::from_time_t(std::mktime(&tm1));
    auto time_point2 = std::chrono::system_clock::from_time_t(std::mktime(&tm2));

    auto duration = std::chrono::duration_cast<std::chrono::hours>(time_point2 - time_point1);
    double days = duration.count() / 24.0;
    return days/365.0; 
}


std::unordered_map<std::string, std::string> readEnv(const std::string& envFilePath) {
    std::unordered_map<std::string, std::string> envMap;
    std::ifstream file(envFilePath);
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream lineStream(line);
        std::string key, value;
        if (std::getline(lineStream, key, '=') && std::getline(lineStream, value)) {
            envMap[key] = value;
        }
    }

    return envMap;
}

using namespace std;
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool performCurlRequest(const string& url, string& response) {
    CURL *curl = curl_easy_init();
    if(!curl) {
        cerr << "Failed to initialize curl" << endl;
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if(res != CURLE_OK) {
        cerr << "Failed to perform curl request: " << curl_easy_strerror(res) << endl;
        return false;
    }
    return true;
}



int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <option_ticker>" << std::endl;
        return 1;
    }

    std::string optionTicker = argv[1];
    auto env = readEnv("../.env");
    std::string apiKey = env["API_KEY"];
    double riskFree = stod(env["RISK_FREE_RATE"]);
    string optionsUrl = "https://api.polygon.io/v3/reference/options/contracts/O:" + optionTicker + "?apiKey=" + apiKey;    
    string response;
    OptionData optionData;
    if(!performCurlRequest(optionsUrl, response)) {
        return 1;
    }
    try {
        nlohmann::json jsonResponse = nlohmann::json::parse(response);
        if(jsonResponse["status"]=="OK") {
            optionData = jsonResponse.get<OptionData>();
        } else {
            std::cerr << "API response status is not OK" << std::endl;
            return 1;
        } 
    } catch (nlohmann::json::parse_error& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return 1;
    }
    
    std::string nowStr;
    std::string pastStr;

    getDateInfo(nowStr,pastStr);
    std::cout << nowStr << " " << pastStr << std::endl;
    std::string stockUrl = "https://api.polygon.io/v2/aggs/ticker/" + optionData.results.underlying_ticker + "/range/1/day/"+pastStr+ "/" + nowStr+"?adjusted=true&sort=desc&apiKey=" + apiKey;
    string stockResponse;
    if (!performCurlRequest(stockUrl, stockResponse)) {
        return 1;
    }

    try {

        nlohmann::json stockJsonResponse = nlohmann::json::parse(stockResponse);
        if (stockJsonResponse["status"] == "OK" || stockJsonResponse["status"]=="DELAYED") {
            optionData.stockData = stockJsonResponse.get<StockData>();
        } else {
            std::cerr << "API response status is not OK" << std::endl;
            return 1;
        }
    } catch (nlohmann::json::parse_error& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return 1;
    }
    blackScholesParams bsParams;
    
    bsParams.T = dateDifference(pastStr,nowStr);
    bsParams.sigma = optionData.stockData.calculateVolatility();
    if(optionData.results.contract_type=="call") {
        bsParams.isCall = true;
    } else {
        bsParams.isCall = false;
    }

    bsParams.K = optionData.results.strike_price;
    bsParams.S = optionData.stockData.results[0].c;
    bsParams.r = riskFree;
    double callPrice = bsParams.callPrice();
    double putPrice = bsParams.putPrice();
    std::cout << "Call Price: " << callPrice << std::endl;
    std::cout << "Put Price: " << putPrice << std::endl;
    std::cout << "Volatility: " << bsParams.sigma <<std::endl;
    // Calculate and print the Greeks
    std::cout << "Delta: " << bsParams.delta() << std::endl;
    std::cout << "Gamma: " << bsParams.gamma() << std::endl;
    std::cout << "Theta: " << bsParams.theta() << std::endl;
    std::cout << "Vega: " << bsParams.vega() << std::endl;
    std::cout << "Rho: " << bsParams.rho() << std::endl;
    std::string optionLastCloseUrl = "https://api.polygon.io/v2/aggs/ticker/O:"+ optionTicker + "/prev?adjusted=true&apiKey=" + apiKey;
    string optionLastCloseResponse;
    double marketPrice;

    if(!performCurlRequest(optionLastCloseUrl, optionLastCloseResponse)) {
        return 1;
    }
    try {

        nlohmann::json stockJsonResponse = nlohmann::json::parse(optionLastCloseResponse);
        if (stockJsonResponse["status"] == "OK" || stockJsonResponse["status"]=="DELAYED") {
            marketPrice = stockJsonResponse["results"][0]["c"];
        } else {
            std::cerr << "API response status is not OK" << std::endl;
            return 1;
        }
    } catch (nlohmann::json::parse_error& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}