#include <omp.h>
#include <vector>
#include <map>
#include <limits>
#include "rapidcsv.h"

long double get_max(const std::vector<long double>& values);
long double get_min(const std::vector<long double>& values);

std::vector<long double> getNormalizedVals(const std::vector<long double>& vals, long double first, long double second);
std::map<std::string, std::pair<long double, long double>>  get_min_maxs(const rapidcsv::Document &doc, const std::vector<std::string> &columns);
std::map<std::string, std::vector<long double>> get_normalized_values(const rapidcsv::Document &doc, const std::map<std::string, std::pair<long double, long double>> &min_maxs);

void save_doc(std::map<std::string, std::vector<long double>> normalized_values, rapidcsv::Document doc);

int main(int argc, char * argv[] ) {

    int thread_num = atoi(argv[1]);
    std::string path_to_data = argv[2];

    omp_set_num_threads(thread_num);
    rapidcsv::Document doc(path_to_data, rapidcsv::LabelParams(0, -1));

    double start_time = omp_get_wtime();
    std::vector<std::string> columns = doc.GetColumnNames();
    std::map<std::string, std::pair<long double, long double>> min_maxs = get_min_maxs(doc, columns);
    std::map<std::string, std::vector<long double>> normalized_values = get_normalized_values(doc, min_maxs);
    double end_time = omp_get_wtime();
    save_doc(normalized_values, doc);
    std::cout << "Normalization min-max took " << end_time - start_time << " seconds" << std::endl;
}

void save_doc(std::map<std::string, std::vector<long double>> normalized_values, rapidcsv::Document doc) {
    std::ofstream myFile("result.csv");
    for (auto const& norm: normalized_values) {
        doc.SetColumn(norm.first, norm.second);
    }
    doc.Save(myFile);

}

std::map<std::string, std::vector<long double>> get_normalized_values(const rapidcsv::Document &doc, const std::map<std::string, std::pair<long double, long double>> &min_maxs) {
    std::map<std::string, std::vector<long double>> normalized_values;
    #pragma omp parallel shared(normalized_values)
    for (auto const& min_max: min_maxs) {
        std::vector<long double> normalized_vector = getNormalizedVals(doc.GetColumn<long double>(min_max.first), min_max.second.first, min_max.second.second);
        normalized_values.insert({min_max.first, normalized_vector});
    }
    return normalized_values;
}

std::map<std::string, std::pair<long double, long double>> get_min_maxs(const rapidcsv::Document &doc, const std::vector<std::string> &columns) {
    std::map<std::string, std::pair<long double, long double>> min_maxs;
    #pragma omp parallel for shared(min_maxs)
    for (auto const& column: columns) {
        min_maxs.insert({
                                column,
                                {get_min(doc.GetColumn<long double>(column)), get_max(doc.GetColumn<long double>(column))}
                        });
    }
    return min_maxs;
}

std::vector<long double> getNormalizedVals(const std::vector<long double>& vals, long double col_min, long double col_max) {
    std::vector<long double> normalized_vals;
    for (auto const& val: vals) {
        normalized_vals.push_back((val - col_min)/(col_max-col_min));
    }
    return normalized_vals;
}

long double get_max(const std::vector<long double>& values) {
    long double max = std::numeric_limits<long double>::min();
    for(auto const& value: values) {
        if (max < value) {
            max = value;
        }
    }
    return max;
}

long double get_min(const std::vector<long double>& values) {
    long double min = std::numeric_limits<long double>::max();
    for(auto const& value: values) {
        if (min > value) {
            min = value;
        }
    }
    return min;
}


