#include <omp.h>
#include <vector>
#include <map>
#include "rapidcsv.h"

void print_row(std::vector<float>& values);
std::vector<float> get_knn_y(rapidcsv::Document doc, size_t testing_index_beg, std::vector<float> testing_y);
float count_smallest_distance_y(std::vector<float>, rapidcsv::Document doc, size_t testing_index_beg, std::vector<float> testing_y);
float calculate_distance(std::vector<float> test_row, std::vector<float>learning_row);
float calculate_accuracy(std::vector<float> learning_y, std::vector<float> testing_y, size_t testing_index_beg);

int main(int argc, char * argv[] ) {
    int thread_num = atoi(argv[1]);
    std::string path_to_data = argv[2];

    omp_set_num_threads(thread_num);
    rapidcsv::Document doc(path_to_data, rapidcsv::LabelParams(0, -1));
    std::vector<float> testing_y = doc.GetColumn<float>("RainTomorrow");
    doc.RemoveColumn("RainTomorrow");

    double start_time = omp_get_wtime();
    size_t row_amount = doc.GetRowCount();
    size_t testing_index_beg = row_amount * 0.99;

    std::vector<float> learning_y = get_knn_y(doc, testing_index_beg, testing_y);
    float accuracy = calculate_accuracy(learning_y, testing_y, testing_index_beg);
    double end_time = omp_get_wtime();
    std::cout << "Accuracy for 1 neighbour = " << accuracy << std::endl;
    std::cout << "Count time = " << end_time - start_time << std::endl;
}

std::vector<float> get_knn_y(rapidcsv::Document doc, size_t testing_index_beg, std::vector<float> testing_y) {
    std::vector<float> closest_ys;
    #pragma omp parallel for shared(closest_ys)
    for (size_t testing_index = testing_index_beg; testing_index < doc.GetRowCount(); ++testing_index) {
        closest_ys.push_back(count_smallest_distance_y(doc.GetRow<float>(testing_index), doc, testing_index_beg, testing_y));
    }
    return closest_ys;
}

float count_smallest_distance_y(std::vector<float> test_row, rapidcsv::Document doc, size_t testing_index_beg, std::vector<float> testing_y) {
    float nearest_distance = calculate_distance(test_row, doc.GetRow<float>(0));
    int nearest_neighbour = 0;
    #pragma omp parallel for shared(nearest_distance, nearest_neighbour)
    for (size_t iterator = 1; iterator < testing_index_beg; ++iterator) {
        float new_distance = calculate_distance(test_row, doc.GetRow<float>(iterator));
        if (new_distance < nearest_distance) {
            nearest_distance = new_distance;
            nearest_neighbour = iterator;
        }
    }
    return testing_y[nearest_neighbour];
}

float calculate_distance(std::vector<float> test_row, std::vector<float>learning_row) {
    float distance = 0.f;
    #pragma omp parallel for shared(distance)
    for (size_t iterator = 0; iterator < test_row.size(); ++iterator) {
        distance += (test_row[iterator] - learning_row[iterator]) * (test_row[iterator] - learning_row[iterator]);
    }
    return sqrt(distance);
}

float calculate_accuracy(std::vector<float> learning_y, std::vector<float> testing_y, size_t testing_index_beg) {
    float accuracy = 0.f;
    for (size_t iterator = 0; iterator < learning_y.size(); ++iterator) {
        accuracy += learning_y[iterator] == testing_y[(int)(iterator + testing_index_beg)] ? 1 : 0;
    }
    return accuracy / learning_y.size();
}



void print_row(std::vector<float>& values) {
    for(auto const& val: values) {
        std::cout << val << "  ";
    }
    std::cout << std::endl;
}

