#include <omp.h>
#include <vector>
#include <map>
#include "rapidcsv.h"

std::vector<long double> getNormalizedVals(const std::vector<long double>& vals, long double first, long double second);
long double get_avg(std::vector<long double>& vals);
long double get_std(std::vector<long double>& vals, long double avg);
std::map<std::string, std::vector<long double>> get_normalized_values(const rapidcsv::Document &doc, const std::map<std::string, std::pair<long double, long double>> &min_maxs);
void save_doc(std::map<std::string, std::vector<long double>> normalized_values, rapidcsv::Document doc);

std::map<std::string, std::pair<long double, long double>>  get_avg_std(const rapidcsv::Document &doc, const std::vector<std::string> &columns);

int main(int argc, char * argv[] ) {

    int thread_num = atoi(argv[1]);
    std::string path_to_data = argv[2];

    omp_set_num_threads(thread_num);
    rapidcsv::Document doc(path_to_data, rapidcsv::LabelParams(0, -1));

    double start_time = omp_get_wtime();
    std::vector<std::string> columns = doc.GetColumnNames();
    std::map<std::string, std::pair<long double, long double>> avg_std = get_avg_std(doc, columns);
    std::map<std::string, std::vector<long double>> normalized_values = get_normalized_values(doc, avg_std);
    double end_time = omp_get_wtime();
    save_doc(normalized_values, doc);
    std::cout << "Normalization standard_scaler took " << end_time - start_time << " seconds" << std::endl;

}

std::map<std::string, std::pair<long double, long double>>  get_avg_std(const rapidcsv::Document &doc, const std::vector<std::string> &columns) {
    std::map<std::string, std::pair<long double, long double>> avg_stds;
    #pragma omp parallel for
    for (auto const& col: columns) {
        std::vector<long double> values = doc.GetColumn<long double>(col);
        long double avg = get_avg(values);
        long double std = get_std(values, avg);
        avg_stds.insert({col, {avg, std}});
    }
    return avg_stds;
}

long double get_avg(std::vector<long double>& vals) {
    long double n = vals.size();
    long double avg = 0;
    #pragma omp parallel for shared(avg)
    for (auto const& val: vals) {
        avg += val;
    }
    return avg/n;
}

long double get_std(std::vector<long double>& vals, long double avg) {
    long double n = vals.size();
    long double var = 0;
    #pragma omp parallel for shared(var)
    for (auto const& val: vals) {
        var += (val - avg) * (val - avg);
    }
    return sqrt(var/n);
}

std::map<std::string, std::vector<long double>> get_normalized_values(const rapidcsv::Document &doc, const std::map<std::string, std::pair<long double, long double>> &standard_scaler) {
    std::map<std::string, std::vector<long double>> normalized_values;
    #pragma omp parallel shared(normalized_values)
    for (auto const& avg_std: standard_scaler) {
        std::vector<long double> normalized_vector = getNormalizedVals(doc.GetColumn<long double>(avg_std.first), avg_std.second.first, avg_std.second.second);
        normalized_values.insert({avg_std.first, normalized_vector});
    }
    return normalized_values;
}

std::vector<long double> getNormalizedVals(const std::vector<long double>& vals, long double col_avg, long double col_std) {
    std::vector<long double> normalized_vals;
    for (auto const& val: vals) {
        normalized_vals.push_back((val - col_avg)/(col_std));
    }
    return normalized_vals;
}

void save_doc(std::map<std::string, std::vector<long double>> normalized_values, rapidcsv::Document doc) {
    std::ofstream myFile("result2.csv");
    for (auto const& norm: normalized_values) {
        doc.SetColumn(norm.first, norm.second);
    }
    doc.Save(myFile);

}

