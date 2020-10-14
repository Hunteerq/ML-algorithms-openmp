#include <iostream>
#include "rapidcsv.h"

int main() {

    rapidcsv::Document doc("data/weatherAUS.csv", rapidcsv::LabelParams(0, 0));
    std::vector<std::string> columns = doc.GetColumnNames();
    for (auto column : columns) {
        std::cout << doc.GetColumn<std::string>(column)[0] << std::endl;
        break;

    }

    return 0;
}


