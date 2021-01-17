#include <mpi.h>
#include <omp.h>
#include <vector>
#include <map>
#include <limits>
#include "rapidcsv.h"

void save_doc(rapidcsv::Document doc);
std::vector<long> encode(std::string column_name, rapidcsv::Document document);


int main(int argc, char * argv[] ) {


    int processRank, processesNumber;
    MPI_Init(&argc, &argv);
    omp_set_num_threads(2);
    MPI_Comm_size(MPI_COMM_WORLD, &processesNumber);
    MPI_Comm_rank(MPI_COMM_WORLD, &processRank);
    MPI_Status Stat;
    std::string path_to_data = argv[1];


    omp_set_num_threads(thread_num);
    rapidcsv::Document doc(path_to_data, rapidcsv::LabelParams(0, -1));
    long number_of_rows = doc.GetRowCount();

    std::vector<std::string> columns = doc.GetColumnNames();

    int slaveProcesses = processesNumber - 1;
    int columnsNumber = columns.size();

    double startTime = MPI_Wtime();
    if (processRank != 0) {
        std::vector<long> results;
        for (std::vector<std::string>::iterator it = columns.begin() + (processRank - 1);
            it < processesNumber * processRank && it != columns.end(); ++it) {
            std::vector current_results = encode(*it, doc);
            std::copy (current_results.begin(), current_results.end(), std::back_inserter(results));
        }
        long *results_array = (long *) malloc(normalized_values.size() * sizeof(long));
        std::copy(results.begin(), results.end(), results_array);

        MPI_Send(results_array, sizeof(long) * results.size(), MPI_BYTE, 0, 1, MPI_COMM_WORLD);
    } else {
        int size = number_of_rows * columnNumber;
        long *results = new long [size];

        std::vector<std::vector<long>> final_result;
        for (int proc_rank = 1; proc_rank < processesNumber; proc_rank++) {
            MPI_Recv(results, sizeof(long) * size / (processesNumber - 1), MPI_BYTE, proc_rank, 1,
                                                   MPI_COMM_WORLD, &Stat);
        }
        int i = 0;
        for (auto const &name : doc.GetColumnNames()) {
            std::vector<long double> temp_vector(results + (number_of_rows * i), results + (number_of_rows * (i + 1) - 1));
            doc.setColumns(name, temp_vector)
            i++;
        }
    }
    MPI_Finalize();
    double end_time = MPI_Wtime();
    std::cout << "Labels encoding took " << end_time - start_time << " seconds for MPI" << std::endl;
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
