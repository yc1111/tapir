// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 * store/benchmark/benchClient.cc:
 *   Benchmarking client for a distributed transactional store.
 *
 **********************************************************************/

#include "store/common/truetime.h"
#include "store/common/frontend/client.h"
#include "store/strongstore/client.h"
#include "store/weakstore/client.h"
#include "store/tapirstore/client.h"

using namespace std;

//ycsb
double theta = 0;
size_t row_num = 100000;
double zetan = 0;
double zeta_2_theta = 0;
double readperc, writeperc;

double zeta(uint32_t n) {
  double sum = 0;
  for(uint32_t i = 1; i <= n; ++i) {
  sum += pow(1.0/i,theta);
  }
  return sum;
}

uint32_t zipf(uint32_t n) {
  double alpha = 1/(1 - theta);
  double eta = (1 - pow(2.0 / n, 1 - theta)) /
    (1 - zeta_2_theta / zetan);
  double u = (double)(rand() % 10000000) / 10000000;
  double uz = u*zetan;
  if(uz < 1) return 1;
  if(uz < 1 + pow(0.5,theta)) return 2;
  return 1 + (uint32_t)(n * pow(eta*u - eta + 1, alpha));
}

int
main(int argc, char **argv)
{
    const char *configPath = NULL;
    const char *keysPath = NULL;
    int duration = 10;
    int nShards = 1;
    int tLen = 10;
    int wPer = 50; // Out of 100
    int closestReplica = -1; // Closest replica id.
    int skew = 0; // difference between real clock and TrueTime
    int error = 0; // error bars

    Client *client;
    enum {
        MODE_UNKNOWN,
        MODE_TAPIR,
        MODE_WEAK,
        MODE_STRONG
    } mode = MODE_UNKNOWN;
    
    // Mode for strongstore.
    strongstore::Mode strongmode;

    int opt;
    while ((opt = getopt(argc, argv, "c:d:N:l:w:k:f:m:e:s:z:r:")) != -1) {
        switch (opt) {
        case 'c': // Configuration path
        { 
            configPath = optarg;
            break;
        }

        case 'f': // Generated keys path
        { 
            keysPath = optarg;
            break;
        }

        case 'N': // Number of shards.
        { 
            char *strtolPtr;
            nShards = strtoul(optarg, &strtolPtr, 10);
            if ((*optarg == '\0') || (*strtolPtr != '\0') ||
                (nShards <= 0)) {
                fprintf(stderr, "option -n requires a numeric arg\n");
            }
            break;
        }

        case 'd': // Duration in seconds to run.
        { 
            char *strtolPtr;
            duration = strtoul(optarg, &strtolPtr, 10);
            if ((*optarg == '\0') || (*strtolPtr != '\0') ||
                (duration <= 0)) {
                fprintf(stderr, "option -n requires a numeric arg\n");
            }
            break;
        }

        case 'l': // Length of each transaction (deterministic!)
        {
            char *strtolPtr;
            tLen = strtoul(optarg, &strtolPtr, 10);
            if ((*optarg == '\0') || (*strtolPtr != '\0') ||
                (tLen <= 0)) {
                fprintf(stderr, "option -l requires a numeric arg\n");
            }
            break;
        }

        case 'w': // Percentage of writes (out of 100)
        {
            char *strtolPtr;
            wPer = strtoul(optarg, &strtolPtr, 10);
            if ((*optarg == '\0') || (*strtolPtr != '\0') ||
                (wPer < 0 || wPer > 100)) {
                fprintf(stderr, "option -w requires a arg b/w 0-100\n");
            }
            break;
        }

        case 'k': // Number of keys to operate on.
        {
            break;
        }

        case 's': // Simulated clock skew.
        {
            char *strtolPtr;
            skew = strtoul(optarg, &strtolPtr, 10);
            if ((*optarg == '\0') || (*strtolPtr != '\0') || (skew < 0))
            {
                fprintf(stderr,
                        "option -s requires a numeric arg\n");
            }
            break;
        }

        case 'e': // Simulated clock error.
        {
            char *strtolPtr;
            error = strtoul(optarg, &strtolPtr, 10);
            if ((*optarg == '\0') || (*strtolPtr != '\0') || (error < 0))
            {
                fprintf(stderr,
                        "option -e requires a numeric arg\n");
            }
            break;
        }

        case 'z': // Zipf coefficient for key selection.
        {
            char *strtolPtr;
            theta = strtod(optarg, &strtolPtr);
            if ((*optarg == '\0') || (*strtolPtr != '\0'))
            {
                fprintf(stderr,
                        "option -z requires a numeric arg\n");
            }
            break;
        }

        case 'r': // Preferred closest replica.
        {
            char *strtolPtr;
            closestReplica = strtod(optarg, &strtolPtr);
            if ((*optarg == '\0') || (*strtolPtr != '\0'))
            {
                fprintf(stderr,
                        "option -r requires a numeric arg\n");
            }
            break;
        }

        case 'm': // Mode to run in [occ/lock/...]
        {
            if (strcasecmp(optarg, "txn-l") == 0) {
                mode = MODE_TAPIR;
            } else if (strcasecmp(optarg, "txn-s") == 0) {
                mode = MODE_TAPIR;
            } else if (strcasecmp(optarg, "qw") == 0) {
                mode = MODE_WEAK;
            } else if (strcasecmp(optarg, "occ") == 0) {
                mode = MODE_STRONG;
                strongmode = strongstore::MODE_OCC;
            } else if (strcasecmp(optarg, "lock") == 0) {
                mode = MODE_STRONG;
                strongmode = strongstore::MODE_LOCK;
            } else if (strcasecmp(optarg, "span-occ") == 0) {
                mode = MODE_STRONG;
                strongmode = strongstore::MODE_SPAN_OCC;
            } else if (strcasecmp(optarg, "span-lock") == 0) {
                mode = MODE_STRONG;
                strongmode = strongstore::MODE_SPAN_LOCK;
            } else {
                fprintf(stderr, "unknown mode '%s'\n", optarg);
                exit(0);
            }
            break;
        }

        default:
            fprintf(stderr, "Unknown argument %s\n", argv[optind]);
            break;
        }
    }

    if (mode == MODE_TAPIR) {
        client = new tapirstore::Client(configPath, nShards,
                    closestReplica, TrueTime(skew, error));
    } else if (mode == MODE_WEAK) {
        client = new weakstore::Client(configPath, nShards,
                    closestReplica);
    } else if (mode == MODE_STRONG) {
        client = new strongstore::Client(strongmode, configPath,
                    nShards, closestReplica, TrueTime(skew, error));
    } else {
        fprintf(stderr, "option -m is required\n");
        exit(0);
    }

    struct timeval t0, t1, t2, t3, t4;

    int nTransactions = 0;
    int tCount = 0;
    double tLatency = 0.0;
    int getCount = 0;
    double getLatency = 0.0;
    int putCount = 0;
    double putLatency = 0.0;
    int beginCount = 0;
    double beginLatency = 0.0;
    int commitCount = 0;
    double commitLatency = 0.0;

    gettimeofday(&t0, NULL);
    srand(t0.tv_sec + t0.tv_usec);

    row_num = 100000;
    zetan = zeta(row_num);
    zeta_2_theta = zeta(2);

    while (1) {
        gettimeofday(&t4, NULL);
        client->Begin();
        gettimeofday(&t1, NULL);
        
        beginCount++;
        beginLatency += ((t1.tv_sec - t4.tv_sec)*1000000 + (t1.tv_usec - t4.tv_usec));
        
        for (int j = 0; j < tLen; j++) {
            auto key = std::to_string(zipf(row_num));
            string value;

            if (rand() % 100 < wPer) {
                gettimeofday(&t3, NULL);
                client->Put(key, key);
                gettimeofday(&t4, NULL);
                
                putCount++;
                putLatency += ((t4.tv_sec - t3.tv_sec)*1000000 + (t4.tv_usec - t3.tv_usec));
            } else {
                gettimeofday(&t3, NULL);
                client->Get(key, value);
                gettimeofday(&t4, NULL);

                getCount++;
                getLatency += ((t4.tv_sec - t3.tv_sec)*1000000 + (t4.tv_usec - t3.tv_usec));
            }
        }

        gettimeofday(&t3, NULL);
        bool status = client->Commit();
        gettimeofday(&t2, NULL);

        commitCount++;
        commitLatency += ((t2.tv_sec - t3.tv_sec)*1000000 + (t2.tv_usec - t3.tv_usec));

        long latency = (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec);

        fprintf(stderr, "%d %ld.%06ld %ld.%06ld %ld %d\n", nTransactions+1, t1.tv_sec,
                t1.tv_usec, t2.tv_sec, t2.tv_usec, latency, status?1:0);

        if (status) {
            tCount++;
            tLatency += latency;
        }
        nTransactions++;

        gettimeofday(&t1, NULL);
        if ( ((t1.tv_sec-t0.tv_sec)*1000000 + (t1.tv_usec-t0.tv_usec)) > duration*1000000) 
            break;
    }

    fprintf(stderr, "# Commit_Ratio: %lf\n", (double)tCount/nTransactions);
    fprintf(stderr, "# Overall_Latency: %lf\n", tLatency/tCount);
    fprintf(stderr, "# Begin: %d, %lf\n", beginCount, beginLatency/beginCount);
    fprintf(stderr, "# Get: %d, %lf\n", getCount, getLatency/getCount);
    fprintf(stderr, "# Put: %d, %lf\n", putCount, putLatency/putCount);
    fprintf(stderr, "# Commit: %d, %lf\n", commitCount, commitLatency/commitCount);
    
    return 0;
}

