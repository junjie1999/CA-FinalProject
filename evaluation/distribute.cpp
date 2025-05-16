#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <thread>
#include <functional> //for lambda expression
#include "sequence_util.h"

double calculateCharCountStdDev(const std::vector<std::vector<std::string>>& data, double& mean, int& largest_partition_idx) {
    std::vector<int> charCounts;

    // Flatten and count characters in each string
    for (const auto& row : data) {
        int total_count_per_partition = 0;
        for (const auto& str : row) {
            total_count_per_partition += str.size();
        }
	    charCounts.push_back(total_count_per_partition);
    }

    if (charCounts.empty()) return 0.0;

    // Calculate mean
    double sum = 0.0;
    for (int count : charCounts) {
        sum += count;
    }
    mean = sum / charCounts.size();

    // largest deviation
    double largest_deviation = 0.0;

    // Calculate variance
    double variance = 0.0;
    for (int i = 0; i < charCounts.size(); ++i) {
        int count = charCounts[i];
        variance += (count - mean) * (count - mean);
        if (abs(count-mean) > largest_deviation) {
            largest_deviation = abs(count-mean);
            largest_partition_idx = i;
        }
    }
    variance /= charCounts.size(); // or use (charCounts.size() - 1) for sample std dev

    return std::sqrt(variance);
}

// Sample single-threaded function which distributes transcript_sequences and query_sequences in equal numbers among the Logic Units
// Use this in main() instead of distribute() to understand requirements
// DO NOT CHANGE
void distribute_default(
    std::vector<std::string> transcript_sequences, 
    std::vector<std::string> query_sequences, 
    std::vector<std::vector<std::string>>& transcript_sequence_partitions, 
    std::vector<std::vector<std::string>>& query_sequence_partitions, 
    int k,
    int logic_unit_count
){
    /* Distribute given sequences to partitions. */
    
    int total_sequence_count = transcript_sequences.size();
    int base_sequence_count  = total_sequence_count / logic_unit_count;
    int remainder = total_sequence_count % logic_unit_count;

    // Transcript
    int start_idx = 0;
    for (int i = 0; i < logic_unit_count; ++i) {
        int partitionSize = base_sequence_count + (i < remainder ? 1 : 0);  // Distribute extra items
        transcript_sequence_partitions[i].insert(
            transcript_sequence_partitions[i].end(), //insert position
            transcript_sequences.begin() + start_idx, //interval start
            transcript_sequences.begin() + start_idx + partitionSize //interval end
        );
        start_idx += partitionSize;
    }

    total_sequence_count = query_sequences.size();
    base_sequence_count = total_sequence_count / logic_unit_count;
    remainder = total_sequence_count % logic_unit_count;

    // Query
    start_idx = 0;
    for (int i = 0; i < logic_unit_count; ++i) {
        int partitionSize = base_sequence_count + (i < remainder ? 1 : 0);  // Distribute extra items
        query_sequence_partitions[i].insert(
            query_sequence_partitions[i].end(), 
            query_sequences.begin() + start_idx, 
            query_sequences.begin() + start_idx + partitionSize
        );
        start_idx += partitionSize;
    }
}


// Function to be evaluated. Implement this.
// You can implement multi-threading for better performance
// DO NOT CHANGE function arguments
void distribute(
    std::vector<std::string> transcript_sequences,
    std::vector<std::string> query_sequences,
    std::vector<std::vector<std::string>>& transcript_sequence_partitions,
    std::vector<std::vector<std::string>>& query_sequence_partitions,
    int k,
    int logic_unit_count
){
    /* Distribute given sequences to partitions. */

    auto distribute_sequences = [](auto& source, auto& partitions, int logic_unit_count) {
        auto it = source.begin();
        int total = source.size();
        int base = total / logic_unit_count;
        int remainder = total % logic_unit_count;

        for (int i = 0; i < logic_unit_count; ++i) {
            int count = base + (i < remainder ? 1 : 0);
            auto end = it + count;
            partitions[i].insert(partitions[i].end(), it, end);
            it = end;
        }
    };

    // Distribute transcript sequences
    distribute_sequences(transcript_sequences, transcript_sequence_partitions, logic_unit_count);

    // Distribute query sequences
    distribute_sequences(query_sequences, query_sequence_partitions, logic_unit_count);
/** /
    int i;
    int partitionSize;

    // Transcript
    auto transcript_begin = transcript_sequences.begin();

    int total_sequence_count = transcript_sequences.size();
    int base_sequence_count  = total_sequence_count / logic_unit_count;
    int remainder = total_sequence_count % logic_unit_count;

    int start_idx = 0;
    for (i = 0; i < logic_unit_count; ++i) {
        partitionSize = base_sequence_count + (i < remainder ? 1 : 0);  // Distribute extra items
        transcript_sequence_partitions[i].insert(
            transcript_sequence_partitions[i].end(), //insert position
            transcript_begin + start_idx, //interval start
            transcript_begin + start_idx + partitionSize //interval end
        );
        start_idx += partitionSize;
    }

    // Query
    auto query_begin = query_sequences.begin();

    total_sequence_count = query_sequences.size();
    base_sequence_count = total_sequence_count / logic_unit_count;
    remainder = total_sequence_count % logic_unit_count;

    start_idx = 0;
    for (i = 0; i < logic_unit_count; ++i) {
        partitionSize = base_sequence_count + (i < remainder ? 1 : 0);  // Distribute extra items
        query_sequence_partitions[i].insert(
            query_sequence_partitions[i].end(), 
            query_begin + start_idx, 
            query_begin + start_idx + partitionSize
        );
        start_idx += partitionSize;
    }
/**/
    /* Distribute given sequences to partitions with multi-threading. */
    /** /


    int n_threads = 2;


    // Transcript
    int t_total_sequence_count = transcript_sequences.size();
    int t_base_sequence_count  = t_total_sequence_count / logic_unit_count;
    int t_remainder = t_total_sequence_count % logic_unit_count;

    // Query
    int q_total_sequence_count = query_sequences.size();
    int q_base_sequence_count = q_total_sequence_count / logic_unit_count;
    int q_remainder = q_total_sequence_count % logic_unit_count;


    std::vector<std::thread> threads;


    int t_start_idx = 0;
    int q_start_idx = 0;

    int lu_base_count = logic_unit_count / n_threads;
    int lu_remainder  = logic_unit_count % n_threads;
    int lu_start = 0;
    for (int tid = 0; tid < n_threads; tid++) {
        int lu_partitionSize = lu_base_count + (tid < lu_remainder ? 1 : 0);
        int lu_end = lu_start+lu_partitionSize;
        std::cout<<lu_start<<"~"<<lu_end<<std::endl;

        int t_chunkSize = 0;
        int q_chunkSize = 0;
        for (int i = lu_start; i < lu_end; ++i) {
            t_chunkSize += t_base_sequence_count + (i < t_remainder ? 1 : 0);
            q_chunkSize += q_base_sequence_count + (i < q_remainder ? 1 : 0);
        }

        threads.emplace_back(
            //lambda function, read only inside [], local variable inside ()
            [&, lu_start, lu_end](int t_start_idx, int q_start_idx) {
                for (int i = lu_start; i < lu_end; ++i) {
                    int t_partitionSize = t_base_sequence_count + (i < t_remainder ? 1 : 0);  // Distribute extra items
                    int q_partitionSize = q_base_sequence_count + (i < q_remainder ? 1 : 0);

                        // Transcript
                    transcript_sequence_partitions[i].insert(
                        transcript_sequence_partitions[i].end(), //insert position
                        transcript_sequences.begin() + t_start_idx, //interval start
                        transcript_sequences.begin() + t_start_idx + t_partitionSize //interval end
                    );
                    // Query
                    query_sequence_partitions[i].insert(
                        query_sequence_partitions[i].end(), 
                        query_sequences.begin() + q_start_idx, 
                        query_sequences.begin() + q_start_idx + q_partitionSize
                    );

                    t_start_idx += t_partitionSize;
                    q_start_idx += q_partitionSize;
                } //for i
            },// lambda
            t_start_idx, //function variables
            q_start_idx
        );

        lu_start += lu_partitionSize;
        t_start_idx += t_chunkSize;
        q_start_idx += q_chunkSize;
    } //for tid

    for (auto &t: threads)
        t.join();
    /**/
}


int main(int argc, char *argv[]) {
    // Parse the program inputs
    std::string transcript_filename = argv[1];
    std::string query_filename      = argv[2];
    int k                = std::stoi(argv[3]); // kmer length
    int logic_unit_count = std::stoi(argv[4]); // No. of Logic Units
    int thread_count     = std::stoi(argv[5]); // No. of threads, if multi-threading is implemented. Change value in Makefile for test
    std::string mode;
    if (argc > 6) mode = argv[6];

    // Read the reference and query sequences files
    std::vector<std::string> transcript_sequences = readFastaSequences(transcript_filename);
    std::vector<std::string> query_sequences      = readFastaSequences(query_filename);

    // Distribution results to be evaluated
    std::vector<std::vector<std::string>> transcript_sequence_partitions(logic_unit_count);
    std::vector<std::vector<std::string>> query_sequence_partitions(logic_unit_count);

    auto start_time = std::chrono::high_resolution_clock::now();

    // Function to implement
    // Call distribution function to distribute transcript_sequences and query_sequences among logic_unit_count Logic Units
    // You are free to use multi-threading, which will fetch higher evaluation score if it improves overall runtime
    // Goal of this function is to balance the Indexing and the Quantification workloads among all the Logic Units
    if (mode == "default") {
        printf("Running default\n");
        distribute_default(transcript_sequences, query_sequences, std::ref(transcript_sequence_partitions), std::ref(query_sequence_partitions), k, logic_unit_count);
    } else
        distribute(transcript_sequences, query_sequences, std::ref(transcript_sequence_partitions), std::ref(query_sequence_partitions), k, logic_unit_count);

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end_time - start_time;

    // Evaluate Standard Deviation of total number of characters in each partition of transcript_sequence_partitions and query_sequence_partitions.
    // DO NOT CHANGE
    double transcript_sequence_mean;
    double query_sequence_mean;
    int    largest_transcripts_partition_idx = 0;
    int    largest_query_partition_idx       = 0;
    double standard_deviation_transcripts = calculateCharCountStdDev (transcript_sequence_partitions, std::ref(transcript_sequence_mean), std::ref(largest_transcripts_partition_idx));
    double standard_deviation_query       = calculateCharCountStdDev (query_sequence_partitions, std::ref(query_sequence_mean), std::ref(largest_query_partition_idx));
    if (standard_deviation_transcripts + standard_deviation_query < (transcript_sequence_mean + query_sequence_mean)*0.2){
        std::cout << "Passed" << std::endl;
    } else
        std::cout << "Failed" << std::endl;

    std::cout << "Distribution took " << duration.count() << " milliseconds.\n";

    std::cout << standard_deviation_transcripts << "\t" << standard_deviation_query << std::endl;
    std::cout << transcript_sequence_mean << "\t" << query_sequence_mean << std::endl;

    // Export Indexing and Quantification workloads of the Logic Unit with largest workload after distribution
    exportTwoVectorsToHeaders( //sequence_util.h
        transcript_sequence_partitions[largest_transcripts_partition_idx],
        query_sequence_partitions[largest_query_partition_idx],
        "transcripts_data.h",
        "query_data.h",
        "transcript_sequences",
        "query_sequences"
    );

    return 0;
}
