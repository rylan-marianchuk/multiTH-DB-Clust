#include <iostream>
#include "include/sqlite_wrapper.h"
#include <string>
#include <sys/sysinfo.h>
#include <vector>
#include <math.h>
#include <algorithm>
#include <filesystem>

#define LEADS 8

int * getQualityVector(double * dim1, double * dim2, float eps, int minPts, int batch_size){
    int * quality = new int[batch_size];

    double dim1min = * (std::min_element(dim1, dim1 + batch_size));
    double dim1max = * (std::max_element(dim1, dim1 + batch_size));
    double dim2min = * (std::min_element(dim2, dim2 + batch_size));
    double dim2max = * (std::max_element(dim2, dim2 + batch_size));

    double dim1range = dim1max - dim1min;
    double dim2range = dim2max - dim2min;
    // Normalize both dimensions
    for (int i = 0; i < batch_size; i++){
        dim1[i] -= dim1min;
        dim1[i] /= dim1range;

        dim2[i] -= dim2min;
        dim2[i] /= dim2range;

        quality[i] = 0;
    }

    for (int i = 0; i < batch_size; i++){
        int count = 0;
        for (int j = 0; j < batch_size; j++){
            if (std::sqrt((dim1[i] - dim1[j])*(dim1[i] - dim1[j]) + (dim2[i] - dim2[j])*(dim2[i] - dim2[j])) < eps){
                count++;
            }
        }
        if (count < minPts){
            quality[i] = 1;
        }
    }

    return quality;
}

int main(int argc, char * argv[]){
    /*
     * Invoke with parameters
     *
     */
    std::string read_db_path = argv[1];
    std::string read_db_table = argv[2];
    int batch_size = std::stoi(argv[3]);
    float eps = std::stof(argv[4]);
    int minPts = std::stoi(argv[5]);

    std::string copied_db = std::filesystem::path(read_db_path).stem().string() + "_copy.db";
    std::filesystem::copy(read_db_path, copied_db);

    SqliteWrapper db_read = SqliteWrapper(copied_db);
    std::string db_results_name = "minPts" + std::to_string(minPts) + "_eps" + std::to_string(eps) + ".db";
    SqliteWrapper db_results = SqliteWrapper(db_results_name);
    std::string results_table_name = "quality_flag";

    std::vector<std::pair<std::string, std::string>> column_dtype {
            {"EUID", "TEXT PRIMARY KEY"},
            {"QUALITY", "INT"}
    };
    db_results.CreateTable(results_table_name, column_dtype);

    std::vector<std::pair<std::string, std::string>> columns_query {
            {"EUID", "TEXT"},
            {"LEAD", "INT"},
            {"CURVELENGTH", "REAL"},
            {"HISTENTROPY", "REAL"},
    };

    unsigned long num_rows = db_read.GetNumRows(read_db_table);
    int batches_to_do = num_rows / LEADS / batch_size + 1;
    int last_batch_size = (num_rows / LEADS) % batch_size;

    std::cout << "num_rows: " << num_rows <<std::endl;
    std::cout << "batches_to_do: " << batches_to_do << std::endl;
    std::cout << "last_batch_size: " << last_batch_size << std::endl;

    while(batches_to_do > 0){
        if (batches_to_do == 1){
            batch_size = last_batch_size;
        }

        for (int lead = 0; lead < LEADS; lead++){
            std::vector<std::string> restriction {
                    "LEAD=" + std::to_string(lead)
            };
            std::vector<std::variant<int*, double*, std::string*>> result_arrs = db_read.RandomBatchQuery(
                    "wvfm_params",
                    "EUID",
                    columns_query,
                    restriction,
                    batch_size);

            int * quality = getQualityVector(
                    std::get<1>(result_arrs[2]),
                    std::get<1>(result_arrs[3]),
                    eps,
                    minPts,
                    batch_size);

            std::vector<std::variant<int*, double*, std::string*>> write_arrs {
                    result_arrs[0],
                    quality
            };

            db_results.BatchInsert(results_table_name, write_arrs, batch_size);
            db_read.BatchRemoveByKey(read_db_table, "EUID", result_arrs[0], batch_size);
        }
        batches_to_do--;
    }


    db_read.CloseDB();
    db_results.CloseDB();
    std::filesystem::remove(copied_db);
    return 0;

}
