// memory_management.c by Jordan Penaloza for CSC139
// Usage: gcc .\memory_management.c -o memsim
// ./memsim
// Make sure input.txt and memsim are in the same directory
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define INF INT_MAX

void simulate_FIFO(int num_frames, int num_requests, int *requests, FILE *out) {
    fprintf(out, "FIFO\n");

    int *frames = malloc(num_frames * sizeof(int));
    int *queue  = malloc(num_frames * sizeof(int));
    if (!frames || !queue) {
        fprintf(stderr, "malloc failed in FIFO\n");
        exit(EXIT_FAILURE);
    }

    // initialize
    for (int i = 0; i < num_frames; i++) {
        frames[i] = -1;
        queue[i]  = -1;
    }

    int head = 0, tail = 0, count = 0, page_faults = 0;

    for (int i = 0; i < num_requests; i++) {
        int page = requests[i], frame = -1;
        // check for hit
        for (int f = 0; f < num_frames; f++) {
            if (frames[f] == page) {
                frame = f;
                break;
            }
        }

        if (frame != -1) {
            fprintf(out, "Page %d already in frame %d\n", page, frame);
        } else {
            // page fault
            page_faults++;
            if (count < num_frames) {
                // free frame available
                frame = count++;
                frames[frame] = page;
                fprintf(out, "Page %d loaded into frame %d\n", page, frame);
                queue[tail++] = frame;
                if (tail == num_frames) tail = 0;
            } else {
                // replace oldest
                int victim = queue[head++];
                if (head == num_frames) head = 0;
                fprintf(out,
                    "Page %d unloaded from frame %d, Page %d loaded into frame %d\n",
                    frames[victim], victim, page, victim
                );
                frames[victim] = page;
                queue[tail++] = victim;
                if (tail == num_frames) tail = 0;
            }
        }
    }

    fprintf(out, "%d page faults\n", page_faults);
    free(frames);
    free(queue);
}

void simulate_Optimal(int num_frames, int num_requests, int *requests, FILE *out) {
    fprintf(out, "Optimal\n");

    int *frames = malloc(num_frames * sizeof(int));
    if (!frames) {
        fprintf(stderr, "malloc failed in Optimal\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < num_frames; i++)
        frames[i] = -1;

    int page_faults = 0;
    for (int i = 0; i < num_requests; i++) {
        int page = requests[i], frame = -1;
        for (int f = 0; f < num_frames; f++) {
            if (frames[f] == page) {
                frame = f;
                break;
            }
        }
        if (frame != -1) {
            fprintf(out, "Page %d already in frame %d\n", page, frame);
        } else {
            page_faults++;
            // find a free frame
            int free_f = -1;
            for (int f = 0; f < num_frames; f++) {
                if (frames[f] == -1) {
                    free_f = f;
                    break;
                }
            }
            if (free_f != -1) {
                frames[free_f] = page;
                fprintf(out, "Page %d loaded into frame %d\n", page, free_f);
            } else {
                // choose victim with farthest next use
                int victim = 0, farthest = -1;
                for (int f = 0; f < num_frames; f++) {
                    int next_use = INF;
                    for (int k = i + 1; k < num_requests; k++) {
                        if (requests[k] == frames[f]) {
                            next_use = k;
                            break;
                        }
                    }
                    if (next_use > farthest) {
                        farthest = next_use;
                        victim = f;
                    }
                }
                fprintf(out,
                    "Page %d unloaded from frame %d, Page %d loaded into frame %d\n",
                    frames[victim], victim, page, victim
                );
                frames[victim] = page;
            }
        }
    }
    fprintf(out, "%d page faults\n", page_faults);
    free(frames);
}

void simulate_LRU(int num_frames, int num_requests, int *requests, FILE *out) {
    fprintf(out, "LRU\n");

    int *frames     = malloc(num_frames * sizeof(int));
    int *timestamps = malloc(num_frames * sizeof(int));
    if (!frames || !timestamps) {
        fprintf(stderr, "malloc failed in LRU\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < num_frames; i++) {
        frames[i]     = -1;
        timestamps[i] = 0;
    }

    int page_faults = 0, time = 0;
    for (int i = 0; i < num_requests; i++) {
        int page = requests[i], frame = -1;
        time++;
        for (int f = 0; f < num_frames; f++) {
            if (frames[f] == page) {
                frame = f;
                timestamps[f] = time;
                break;
            }
        }
        if (frame != -1) {
            fprintf(out, "Page %d already in frame %d\n", page, frame);
        } else {
            page_faults++;
            // find free
            int free_f = -1;
            for (int f = 0; f < num_frames; f++) {
                if (frames[f] == -1) {
                    free_f = f;
                    break;
                }
            }
            if (free_f != -1) {
                frames[free_f] = page;
                timestamps[free_f] = time;
                fprintf(out, "Page %d loaded into frame %d\n", page, free_f);
            } else {
                // find least recently used
                int victim = 0, oldest = timestamps[0];
                for (int f = 1; f < num_frames; f++) {
                    if (timestamps[f] < oldest) {
                        oldest = timestamps[f];
                        victim = f;
                    }
                }
                fprintf(out,
                    "Page %d unloaded from frame %d, Page %d loaded into frame %d\n",
                    frames[victim], victim, page, victim
                );
                frames[victim]     = page;
                timestamps[victim] = time;
            }
        }
    }
    fprintf(out, "%d page faults\n", page_faults);
    free(frames);
    free(timestamps);
}

int main(void) {
    FILE *in = fopen("input.txt", "r");
    if (!in) {
        perror("Failed to open input.txt");
        return EXIT_FAILURE;
    }

    // Skip UTF-8 BOM if present
    unsigned char bom[3];
    if (fread(bom, 1, 3, in) == 3) {
        if (!(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)) {
            fseek(in, 0, SEEK_SET);
        }
    } else {
        fseek(in, 0, SEEK_SET);
    }

    int num_pages, num_frames, num_requests;
    if (fscanf(in, "%d %d %d", &num_pages, &num_frames, &num_requests) != 3) {
        fprintf(stderr, "Invalid input format — couldn't read three ints\n");
        fclose(in);
        return EXIT_FAILURE;
    }

    int *requests = malloc(num_requests * sizeof(int));
    if (!requests) {
        fprintf(stderr, "malloc failed for requests array\n");
        fclose(in);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < num_requests; i++) {
        fscanf(in, "%d", &requests[i]);
    }
    fclose(in);

    FILE *out = fopen("output.txt", "w");
    if (!out) {
        perror("Failed to open output.txt");
        free(requests);
        return EXIT_FAILURE;
    }

    simulate_FIFO(num_frames, num_requests, requests, out);
    fprintf(out, "\n");
    simulate_Optimal(num_frames, num_requests, requests, out);
    fprintf(out, "\n");
    simulate_LRU(num_frames, num_requests, requests, out);

    fclose(out);
    free(requests);
    return 0;
}
