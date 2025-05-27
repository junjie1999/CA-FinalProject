#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>
#include <signal.h>
#include "/home/gem5/include/gem5/m5ops.h"
#include "transcripts_data.h"
#include "query_data.h"
#include "hash-map.h"
#include "gem5_utils.h"

int index_sequences_count;

// Sample function for generating hash-map index and correctness check during evaluation
// DO NOT CHANGE
void quantify_default(HashMap *map, const char **query_sequences_used, int query_sequences_used_count, int k, int *final_results){
    char *kmer = malloc(k+1);
    kmer[k] = '\0';

    // Keep record of transcript-wise query kmer count
    // Use maximum number of transcripts to minimize memory reallocations
    int *matched_transcripts_counts = (int*)malloc(index_sequences_count*sizeof(int));

    for (int i = 0; i < query_sequences_used_count; ++i) {
        const char *seq = query_sequences_used[i];
        int len = strlen(seq);
        int kmer_count = len+1-k;
	
        // Initialize transcript-wise query kmer counts
        for (int j = 0; j < index_sequences_count; ++j)
            matched_transcripts_counts[j] = 0;
        
        for (int j = 0; j < kmer_count; ++j) {
            strncpy(kmer, &seq[j], k);
            IntArray matched_transcripts_idxs;
            init_int_array(&matched_transcripts_idxs);
            get_values(map, kmer, &matched_transcripts_idxs);
            // if(matched_transcripts_idxs.size) {
            //     printf("q: %d\n", i);
            //     printf("kmer: %s   ", kmer);
            // }

            for (int l = 0; l < matched_transcripts_idxs.size; ++l) {
                // if( matched_transcripts_idxs.data[l]>=0 &&  matched_transcripts_idxs.data[l]<=3)
                // printf("%d ", matched_transcripts_idxs.data[l]);
                matched_transcripts_counts[matched_transcripts_idxs.data[l]]++;
            }
            // if(matched_transcripts_idxs.size) printf("\n");
        }

        // Find transcript with maximum matches, and record its match count
        int max_count_for_a_transcript = 0;
        for (int j = 0; j < index_sequences_count; ++j) {
            if (max_count_for_a_transcript < matched_transcripts_counts[j])
                max_count_for_a_transcript = matched_transcripts_counts[j];
        }
        


        // Proceed only if any of the transcripts matched atleast once for atleast one kmer
        if (max_count_for_a_transcript == 0)
            continue;

        // Record all transcripts that matched with the query sequence for maximum number of times
        IntArray transcript_idxs_with_maximum_count;
        init_int_array(&transcript_idxs_with_maximum_count);
        for (int j = 0; j < index_sequences_count; ++j){
            if(matched_transcripts_counts[j] == max_count_for_a_transcript){
                add_to_int_array(&transcript_idxs_with_maximum_count, j);
            }
        }

        // Increase count of final matched transcripts for each query sequence
        for (int j = 0; j < transcript_idxs_with_maximum_count.size; ++j){
            final_results[transcript_idxs_with_maximum_count.data[j]]++;
        }
    }
}

void check_correctness(int *final_results_default, int *final_results, int count){
    for (int i = 0; i < count; ++i) {
        if ( final_results[i] != final_results_default[i]) {
            printf("Failed\n");
	        return;
	    }
    }
    printf("Passed");
}



// Function to be evaluated. Implement this.
// See definition of HashMap and its operations in hash-map.h
// Use your own algorithm to quantify
// DO NOT CHANGE function arguments
void quantify(HashMap *map, const char **query_sequences_used, int query_sequences_used_count, int k, int *final_results){
    int *matched_transcripts_counts = (int*)malloc(index_sequences_count * sizeof(int));

    for (int i = 0; i < query_sequences_used_count; ++i) {
        const char *seq = query_sequences_used[i];
        int len = strlen(seq);
        int kmer_count = len + 1 - k;

        for (int j = 0; j < index_sequences_count; ++j)
            matched_transcripts_counts[j] = 0;

        for (int j = 0; j < kmer_count; ++j) {
            IntArray matched_transcripts_idxs;
            init_int_array(&matched_transcripts_idxs);
            get_values_kmer_raw(map, &seq[j], k, &matched_transcripts_idxs);

            for (int l = 0; l < matched_transcripts_idxs.size; ++l) {
                matched_transcripts_counts[matched_transcripts_idxs.data[l]]++;
            }
        }

        // Find transcript with maximum matches, and record its match count
        int max_count_for_a_transcript = 0;
        for (int j = 0; j < index_sequences_count; ++j) {
            if (max_count_for_a_transcript < matched_transcripts_counts[j])
                max_count_for_a_transcript = matched_transcripts_counts[j];
        }

        // Proceed only if any of the transcripts matched atleast once for atleast one kmer
        if (max_count_for_a_transcript == 0)
            continue;

        for (int j = 0; j < index_sequences_count; ++j){
            final_results[j] += (matched_transcripts_counts[j] == max_count_for_a_transcript);
        }

    }
}

/* 將最終結果寫入文字檔  
 * filename : 輸出檔名  
 * results  : 指向結果陣列  
 * count    : 陣列長度  
 */
void write_final_results(const char *filename,
                         const int *results,
                         int count)
{
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < count; ++i) {
        /* 逐行輸出：<index>\t<count>\n */
        if (fprintf(fp, "%d\t%d\n", i, results[i]) < 0) {
            perror("fprintf failed");
            fclose(fp);
            exit(EXIT_FAILURE);
        }
    }

    if (fclose(fp) != 0) {
        perror("fclose failed");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]){
    int k = atoi(argv[1]);
    int mode = 0;
    if (argc > 2) mode = atoi(argv[2]);

    // Select one from transcript_sequences and query_sequences to use as query_sequences_used. The other one should be treated as index_sequences accordingly.
    const char **index_sequences = transcript_sequences;
    index_sequences_count        = transcript_sequences_count;

    const char **query_sequences_used = query_sequences;
    int query_sequences_used_count    = query_sequences_count;

    // Final quantification results to be evaluated
    int *final_results = (int*)malloc(index_sequences_count*sizeof(int));
    for(int i = 0; i < index_sequences_count; ++i)
        final_results[i] = 0;

    // Read the index (hash-map) saved in index.c
    HashMap *map = create_hashmap();
    printf("create done!\n");
    read_hashmap_from_file(map, "index_result/hash-map.txt");
    // read_hashmap_from_file_default(map, "index_result/hash-map.txt");

    // Reset Gem5 simulation statistics
    m5_reset_stats(0,0);

    // Function to implement
    // Call index function to index index_sequences within a single Logic Unit
    // You are free to use Vector Coprocessors (SIMD), which will fetch higher evaluation score if it improves overall runtime
    // Goal of this function is to improve the performance of Indexing by a single Logic Unit
    if (mode == 0) {
        quantify(map, query_sequences_used, query_sequences_used_count, k, final_results);
    } else {
        printf("Running default\n");
        quantify_default(map, query_sequences_used, query_sequences_used_count, k, final_results);
    }
    printf("===============\n");
    // write_final_results("final_results.txt", final_results, index_sequences_count);
    for(int i =0; i < index_sequences_count; ++i)
       printf("Transcript %d has %d matched queries\n", i, final_results[i]);

    // Dump Gem5 simulation final statistics
    m5_dump_stats(0,0);

    // Print Gem5 simulation time for the evaluated indexing function
    print_gem5_simulated_time("m5out/stats.txt");

    // Function for correctness check. Keep commented to avoid long Gem5 simulation time
    // DO NOT CHANGE
    // int *final_results_default = (int*)malloc(index_sequences_count*sizeof(int));
    // for(int i = 0; i < index_sequences_count; ++i)
	//     final_results_default[i] = 0 ;
    // free(map); map = create_hashmap();
    
    // read_hashmap_from_file_default(map, "index_result/hash-map.txt");
    // quantify_default(map, query_sequences_used, query_sequences_used_count, k, final_results_default);
    // // write_final_results("final_results_default.txt", final_results_default, index_sequences_count);
    // // Uncomment to print final quantification results
    // for(int i =0; i < index_sequences_count; ++i)
    //    printf("Transcript %d has %d matched queries\n", i, final_results_default[i]);
    // check_correctness(final_results_default, final_results, index_sequences_count);


    // free_hashmap(map);
    // free(final_results);
    // free(final_results_default);

    return 0;
}
