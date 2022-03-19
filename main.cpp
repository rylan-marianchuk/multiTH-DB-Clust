#include <iostream>
#include "include/sqlite_wrapper.h"
#include <string>
#include <sys/sysinfo.h>
#include <vector>

int * getQualityVector(double * dim1, double * dim2, int batch_size){
    int * quality = new int[batch_size];


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

    SqliteWrapper db = SqliteWrapper(db_path.c_str());
    std::string table_name = "minPts=" + std::to_string(minPts) + ";eps=" + std::to_string(eps);
    std::cout << "Creating table: " << table_name << std::endl;

    std::vector<std::pair<std::string, std::string>> column_dtype {
            {"EUID", "TEXT PRIMARY KEY"},
            {"QUALITY", "INT"}
    };
    db.CreateTable(table_name, column_dtype);

    std::vector<std::pair<std::string, std::string>> columns_query {
            {"EUID", "TEXT"},
            {"LEAD", "INT"},
            {"CURVELENGTH", "REAL"},
            {"HISTENTROPY", "REAL"},
    };

    unsigned long num_rows = db.GetNumRows("wvfm_params");

    unsigned long batches_to_do = num_rows / batch_size;
    std::cout << batches_to_do << std::endl;

    for (unsigned long batch = 0; batch < batches_to_do; batch++){
        std::vector<std::variant<int*, double*, std::string*>> result_arrs = db.RandomBatchQuery("wvfm_params", "EUID", columns_query, batch_size);

        int * quality = getQualityVector();

        std::vector<std::variant<int*, double*, std::string*>> write_arrs {
            result_arrs[0],
            quality
        };

        db.BatchInsert(table_name, write_arrs, batch_size);

    }


}
