#include <iostream>
#include "include/sqlite_wrapper.h"
#include <string>
#include <sys/sysinfo.h>
#include <vector>
#include <math.h>
#include <algorithm>

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
    std::string db_path = argv[1];
    int batch_size = std::stoi(argv[2]);
    float eps = std::stof(argv[3]);
    int minPts = std::stoi(argv[4]);

    // Read in the database

    SqliteWrapper db_read = SqliteWrapper(db_path);
    std::string db_results_name = "minPts_" + std::to_string(minPts) + "__eps_" + std::to_string(eps) + ".db";
    SqliteWrapper db_results = SqliteWrapper(db_results_name);
    db_results.debug = false;
    std::string table_name = "quality_flag";

    std::vector<std::pair<std::string, std::string>> column_dtype {
            {"EUID", "TEXT PRIMARY KEY"},
            {"QUALITY", "INT"}
    };
    db_results.CreateTable(table_name, column_dtype);

    std::vector<std::pair<std::string, std::string>> columns_query {
            {"EUID", "TEXT"},
            {"LEAD", "INT"},
            {"CURVELENGTH", "REAL"},
            {"HISTENTROPY", "REAL"},
    };

    unsigned long num_rows = db_read.GetNumRows("wvfm_params");

    unsigned long batches_to_do = num_rows / LEADS / batch_size;
    std::cout << batches_to_do << std::endl;
    std::cout << num_rows / LEADS << std::endl;


    for (unsigned long batch = 0; batch < batches_to_do; batch++){
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

            std::cout << quality[0] << std::endl;
            std::cout << quality[1] << std::endl;
            std::cout << quality[2] << std::endl;
            db_results.BatchInsert(table_name, write_arrs, batch_size);
        }


    }

    return 0;

}
