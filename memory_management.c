#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define INF INT_MAX

void simulate_FIFO(int num_frames, int num_requests, int *requests, FILE *out) {
    fprintf(out, "FIFO\n");
    int *frames = malloc(num_frames * sizeof(int));
    int *page_table = malloc((INT_MAX) * sizeof(int)); // using dynamic mapping, but allocate maximum
    int *queue = malloc(num_frames * sizeof(int));
    for(int i = 0; i < num_frames; i++) frames[i] = -1;
    for(int i = 0; i < num_requests; i++) queue[i] = -1;
    int head = 0, tail = 0, count = 0;
    int page_faults = 0;

    for (int i = 0; i < num_requests; i++) {
        int page = requests[i];
        int frame = -1;
        // lookup if in frame
        for (int f = 0; f < num_frames; f++) {
            if (frames[f] == page) { frame = f; break; }
        }
        if (frame != -1) {
            fprintf(out, "Page %d already in Frame %d\n", page, frame);
        } else {
            // page fault
            page_faults++;
            if (count < num_frames) {
                frame = count;
                frames[frame] = page;
                queue[tail] = frame;
                tail = (tail + 1) % num_frames;
                count++;
                fprintf(out, "Page %d loaded into Frame %d\n", page, frame);
            } else {
                int victim = queue[head];
                head = (head + 1) % num_frames;
                int old_page = frames[victim];
                fprintf(out, "Page %d unloaded from Frame %d, Page %d loaded into Frame %d\n", old_page, victim, page, victim);
                frames[victim] = page;
                queue[tail] = victim;
                tail = (tail + 1) % num_frames;
            }
        }
    }
    fprintf(out, "%d page faults\n", page_faults);

    free(frames);
    free(page_table);
    free(queue);
}

void simulate_Optimal(int num_frames, int num_requests, int *requests, FILE *out) {
    fprintf(out, "Optimal\n");
    int *frames = malloc(num_frames * sizeof(int));
    for(int i = 0; i < num_frames; i++) frames[i] = -1;
    int page_faults = 0;

    for(int i = 0; i < num_requests; i++) {
        int page = requests[i];
        int frame = -1;
        for(int f = 0; f < num_frames; f++) if(frames[f] == page) { frame = f; break; }
        if(frame != -1) {
            fprintf(out, "Page %d already in Frame %d\n", page, frame);
        } else {
            page_faults++;
            // find free
            int free_f = -1;
            for(int f = 0; f < num_frames; f++) if(frames[f] == -1) { free_f = f; break; }
            if(free_f != -1) {
                frames[free_f] = page;
                fprintf(out, "Page %d loaded into Frame %d\n", page, free_f);
            } else {
                int victim = -1;
                int farthest = -1;
                for(int f = 0; f < num_frames; f++) {
                    int next_use = INF;
                    for(int k = i + 1; k < num_requests; k++) {
                        if(requests[k] == frames[f]) { next_use = k; break; }
                    }
                    if(next_use > farthest) {
                        farthest = next_use;
                        victim = f;
                    }
                }
                int old_page = frames[victim];
                fprintf(out, "Page %d unloaded from Frame %d, Page %d loaded into Frame %d\n", old_page, victim, page, victim);
                frames[victim] = page;
            }
        }
    }
    fprintf(out, "%d page faults\n", page_faults);
    free(frames);
}

void simulate_LRU(int num_frames, int num_requests, int *requests, FILE *out) {
    fprintf(out, "LRU\n");
    int *frames = malloc(num_frames * sizeof(int));
    int *timestamps = malloc(num_frames * sizeof(int));
    for(int i = 0; i < num_frames; i++) { frames[i] = -1; timestamps[i] = 0; }
    int page_faults = 0;
    int time = 0;

    for(int i = 0; i < num_requests; i++) {
        int page = requests[i];
        int frame = -1;
        time++;
        for(int f = 0; f < num_frames; f++) {
            if(frames[f] == page) { frame = f; timestamps[f] = time; break; }
        }
        if(frame != -1) {
            fprintf(out, "Page %d already in Frame %d\n", page, frame);
        } else {
            page_faults++;
            int free_f = -1;
            for(int f = 0; f < num_frames; f++) if(frames[f] == -1) { free_f = f; break; }
            if(free_f != -1) {
                frames[free_f] = page;
                timestamps[free_f] = time;
                fprintf(out, "Page %d loaded into Frame %d\n", page, free_f);
            } else {
                int victim = 0;
                int oldest = timestamps[0];
                for(int f = 1; f < num_frames; f++) {
                    if(timestamps[f] < oldest) { oldest = timestamps[f]; victim = f; }
                }
                int old_page = frames[victim];
                fprintf(out, "Page %d unloaded from Frame %d, Page %d loaded into Frame %d\n", old_page, victim, page, victim);
                frames[victim] = page;
                timestamps[victim] = time;
            }
        }
    }
    fprintf(out, "%d page faults\n", page_faults);
    free(frames);
    free(timestamps);
}

int main() {
    FILE *in = fopen("input.txt", "r");
    if (!in) {
        perror("Failed to open input.txt");
        return EXIT_FAILURE;
    }
    int num_pages, num_frames, num_requests;
    if (fscanf(in, "%d %d %d", &num_pages, &num_frames, &num_requests) != 3) {
        fprintf(stderr, "Invalid input format\n");
        fclose(in);
        return EXIT_FAILURE;
    }
    int *requests = malloc(num_requests * sizeof(int));
    for (int i = 0; i < num_requests; i++) {
        fscanf(in, "%d", &requests[i]);
    }
    fclose(in);

    FILE *out = fopen("output.txt", "w");
    if (!out) {
        perror("Failed to open output.txt");
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
