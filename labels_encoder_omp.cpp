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

