#include <omp.h>
#include <vector>
#include <map>
#include <limits>
#include "rapidcsv.h"

void save_doc(rapidcsv::Document doc);
std::vector<long> encode(std::string column_name, rapidcsv::Document document);


int main(int argc, char * argv[] ) {

    int thread_num = atoi(argv[1]);
    std::string path_to_data = argv[2];

    omp_set_num_threads(thread_num);
    rapidcsv::Document doc(path_to_data, rapidcsv::LabelParams(0, -1));

    double start_time = omp_get_wtime();
    std::vector<std::string> columns = doc.GetColumnNames();
    std::cout << columns.size() << std::endl;

    #pragma parallel for default(none) shared(columns, doc)
    for (std::vector<std::string>::iterator it = columns.begin(); it != columns.end(); ++it) {
        doc.SetColumn(*it, encode(*it, doc));
    }
    double end_time = omp_get_wtime();
    std::cout << "Labels encoding took " << end_time - start_time << " seconds" << std::endl;
    save_doc(doc);
}


std::vector<long> encode(std::string column_name, rapidcsv::Document document) {
    std::vector<std::string> rows = document.GetColumn<std::string>(column_name);
    std::map<std::string, long> labels_encoded;
    std::vector<long> results;

    long i = 0;
    #pragma parallel for shared(labels_encoded, i)
    for (std::vector<std::string>::iterator it = rows.begin(); it != rows.end(); ++it) {
        if (labels_encoded.find(*it)  ==  labels_encoded.end()) {
            labels_encoded.insert({*it, i});
            i++;
        }
    }

    #pragma parallel for shared(results, labels_encoded)
    for (std::vector<std::string>::iterator it = rows.begin(); it != rows.end(); ++it) {
        results.push_back(labels_encoded.find(*it)->second);
    }

    return results;
}


void save_doc(rapidcsv::Document doc) {
    std::ofstream myFile("result_omp.csv");
    doc.Save(myFile);
}

