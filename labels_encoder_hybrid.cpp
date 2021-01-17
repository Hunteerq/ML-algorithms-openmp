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

    int thread_num = atoi(argv[1]);
    omp_set_num_threads(thread_num);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &processesNumber);
    MPI_Comm_rank(MPI_COMM_WORLD, &processRank);
    MPI_Status Stat;
    std::string path_to_data = argv[2];


    rapidcsv::Document doc(path_to_data, rapidcsv::LabelParams(0, -1));
    long number_of_rows = doc.GetRowCount();

    std::vector<std::string> columns = doc.GetColumnNames();

    int slaveProcesses = processesNumber - 1;
    int columnsNumber = columns.size();

    double processTime;
    if (processRank != 0) {
        double startTime = MPI_Wtime();
        std::vector<long> results;
        int columns_for_process = int(columnsNumber / slaveProcesses);
        int first =  columns_for_process * (processRank-1);
        int last = columns_for_process * processRank;

        if (processRank == processesNumber-1) {
            last = columnsNumber;
        }
        int i = 0;
        for (std::vector<std::string>::iterator it = columns.begin(); it != columns.end(); ++it) {
            if (i < last && i >= first) {
                std::vector<long> current_results = encode(*it, doc);
                std::copy (current_results.begin(), current_results.end(), std::back_inserter(results));
            }
            i++;
        }
        long *results_array = (long *) malloc(results.size() * sizeof(long));
        std::copy(results.begin(), results.end(), results_array);
        MPI_Send(results_array, sizeof(long) * results.size(), MPI_BYTE, 0, 1, MPI_COMM_WORLD);
        processTime = MPI_Wtime() - startTime;
    } else {
        int size = number_of_rows * columnsNumber;
        long *results = new long [size];

        std::vector<std::vector<long> > final_result;
        for (int proc_rank = 1; proc_rank < processesNumber; proc_rank++) {
            int columns_for_process = int(columnsNumber / slaveProcesses);
            int first =  columns_for_process * (proc_rank-1);
            int last = columns_for_process * proc_rank;
            if (proc_rank == processesNumber-1) {
                last = columnsNumber;
            }
            MPI_Recv(results, sizeof(long) * number_of_rows * (last - first), MPI_BYTE, proc_rank, 1,
                     MPI_COMM_WORLD, &Stat);
            int i = 0;
            int j = 0;
            for (auto const &name : doc.GetColumnNames()) {
                if (i <last && i >=first) {
                    std::vector<long double> temp_vector(results + (number_of_rows * j), results + (number_of_rows * (j + 1) - 1));
                    doc.SetColumn(name, temp_vector);
                    j++;
                }
                i++;
            }
        }
        save_doc(doc);
    }

    double *processTimes = NULL;
    if (processRank == 0) {
        processTimes = (double *)malloc(sizeof(double) * processesNumber);
    }
    MPI_Gather(&processTime, 1, MPI_DOUBLE, processTimes, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double tempTime = 0;
    if (processRank == 0) {
        for(int i=0; i<processesNumber;++i) {
            tempTime += processTimes[i];
        }
        tempTime /= (processesNumber-1);
        std::cout<< "abels encoding took : " << tempTime << "s \n";
    }
    MPI_Finalize();
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
