#include      <time.h>
#include      <stdio.h>
#include      <stdlib.h>
#include      <signal.h>
#include      <math.h>
#include      "ts_lib.h"
#include      "edf_hull.h"

#define LEN(v) (sizeof(v)/sizeof(v[0]))

#define FILENAME "experim.csv"

/*
 * Brief list of options:
 *   'i': read task set from stdin
 *   's': random generation of one task set by a selected seed
 */


/*
 * Option 'i': apply the convex hull point reduction to the task set
 * read from stdin (for from text file via stdin redirection). If the
 * option is "iv", then verbose output
 */
void main_input_task(char * argv[]);

/*
 * Option 's': apply the convex hull point reduction to one task set
 * randomly generated by the random seed and the other settings passed
 * via stdin. If the option is "cv", then verbose output
 */
void main_seed(char * argv[]);

void print_usage();

FILE * fdata;

/*
 * Parsing options and invoking the proper function depending of the
 * option
 */
int main(int argc, char * argv[])
{
	char opt;

	if (argc <= 1)
		/* Default is reading from input */
		opt = 'i';
	else
		/* 1st char of 1st parameter */
		opt = argv[1][0];
	switch (opt) {
	case 'i':
		main_input_task(argv+1);
		break;
	case 's':
		main_seed(argv+1);
		break;
	case 'h':
	default:
		print_usage();
	}
}

void print_usage()
{
	printf("Usage (TODO)\n");
}

void main_input_task(char * argv[])
{
	ts_rand_t    rand_setup;
	ts_t         my_task_set;
	edf_points_t my_points;
	int verbose;

	verbose = argv[0] != NULL && argv[0][1] == 'v';
	
	/* Initialization */
	ts_set_zero(&my_task_set);
	edf_set_zero(&my_points);

	ts_read_alloc(&my_task_set);
	if (verbose) {
		/* verbose output */
		ts_print(&my_task_set);
	}
	edf_create_points(&my_task_set, &my_points);
	if (verbose) {
		/* verbose output */
		edf_print_points(&my_points);
	}
	edf_qhull_points(&my_points);
	edf_print_constraints_C(&my_points);
	edf_print_constraints_U(&my_task_set,&my_points);
	edf_free_points(&my_points);
	ts_free(&my_task_set);
}

void main_seed(char * argv[])
{
	ts_t         my_task_set;
	ts_rand_t    rand_setup;
	edf_points_t my_points;
	int verbose, num_tasks;
	char * s=NULL;
	size_t dim=0;
	int i=0;
	struct timespec start, check;
	double time_points, time_qhull;
	
	/* If 's' option, argv[0], cannot be NULL */
	verbose = argv[0][1] == 'v';

	/* Initialization */
	ts_set_zero(&my_task_set);
	edf_set_zero(&my_points);

	/* only method implemented for period generation */
	rand_setup.per_m = per_unif;

	/* Read from stdin the setup */
	getdelim(&s, &dim, ',', stdin);
	sscanf(s, "%ud", &rand_setup.seed);
	getdelim(&s, &dim, ',', stdin);
	sscanf(s, "%d", &num_tasks);
	getdelim(&s, &dim, ',', stdin);
	sscanf(s, "%lf", &rand_setup.per_min);
	getdelim(&s, &dim, ',', stdin);
	sscanf(s, "%lf", &rand_setup.per_max);
	getdelim(&s, &dim, ',', stdin);
	sscanf(s, "%i", &rand_setup.phasing);
	getdelim(&s, &dim, ',', stdin);
	sscanf(s, "%lf", &rand_setup.norm_dl_avg);
	getdelim(&s, &dim, ',', stdin);
	sscanf(s, "%lf", &rand_setup.norm_dl_var);
	getdelim(&s, &dim, ',', stdin);
	sscanf(s, "%lf", &my_task_set.eps);
	
	/* Generate the task set */
	ts_rand(&my_task_set, &rand_setup, num_tasks);
	if (verbose) {
		/* verbose output */
		ts_print(&my_task_set);
	}
	clock_gettime(CLOCK_MONOTONIC, &start);
	edf_create_points(&my_task_set, &my_points);
	clock_gettime(CLOCK_MONOTONIC, &check);
	time_points = (check.tv_nsec-start.tv_nsec)*1e-9
		+(check.tv_sec-start.tv_sec);
	if (verbose) {
		/* verbose output */
		edf_print_points(&my_points);
	}
	clock_gettime(CLOCK_MONOTONIC, &start);
	edf_qhull_points(&my_points);
	clock_gettime(CLOCK_MONOTONIC, &check);
	time_qhull = (check.tv_nsec-start.tv_nsec)*1e-9
		+(check.tv_sec-start.tv_sec);
	printf("All points: %d\n", my_points.num_points);
	printf("Iff points: %d\n", my_points.num_sel);
	printf("Time points: %f, Time qhull: %f\n", time_points, time_qhull);
	edf_free_points(&my_points);
	ts_free(&my_task_set);
}
	



#if 0
int main_spanDL(int argc, char* argv[])
{
#define NUM_TASKS 6

	ts_t         my_task_set;
	ts_rand_t    rand_setup;
	edf_points_t my_points;
	double max_d, max_o;
	struct timespec start, check;
	double time_points, time_qhull;
	double dl_span[] = {0.5, 0.6, 0.7, 0.8, 0.9, 1, 1.1, 1.2, 1.3, 1.4, 1.5};
	double var_span[] = {0.4, 0.3, 0.2, 0.1, 0};
	int i, j, k, iter=0;
	double cur_dl, min_dl, max_dl;

	if (argc >= 2) {
		fdata = fopen(argv[1],"a");
	} else {
		fdata = fopen(FILENAME,"a");
	}

	my_task_set.num = 0;
	my_task_set.per = NULL;
	my_task_set.dl = NULL;
	my_task_set.phi = NULL;
	rand_setup.per_m = per_unif;
	rand_setup.per_min = 2;
	rand_setup.per_max = 20;
	rand_setup.norm_dl_avg = 0.5;
	rand_setup.norm_dl_var = 0.2;
	rand_setup.phasing = 0;
	my_points.alloc_points = 0;
	my_points.t0 = NULL;
	my_points.t1 = NULL;
	my_points.vec_p = NULL;
	my_points.qh_vec_p = NULL;
	my_points.num_sel = 0;
	my_points.vec_sel = NULL;
	srand(time(NULL));
	while (1) {
		for(i=0; i<LEN(dl_span); i++) {
			rand_setup.norm_dl_avg = dl_span[i];
			for(j=0; j<LEN(var_span); j++) {
				rand_setup.norm_dl_var = var_span[j];
				if (rand_setup.norm_dl_avg-rand_setup.norm_dl_var >= 1-1e-8) {
					/* All tasks with deadline >= period */
					continue;
				}
				if (rand_setup.norm_dl_avg+rand_setup.norm_dl_var <= 1.1) {
					/* Constrained deadline
					   tasks. They seem to be
					   easier: SKIPPING */
					continue;
				}
				while(1) {
					rand_setup.seed = rand();
					ts_rand(&my_task_set, &rand_setup, NUM_TASKS);
					/*  min/max deadline/period */
					max_dl = min_dl =
						my_task_set.dl[0]/ my_task_set.per[0];
					for (k=1; k<my_task_set.num; k++) {
						cur_dl = my_task_set.dl[k]/my_task_set.per[k];
						if (cur_dl < min_dl)
							min_dl = cur_dl;
						if (cur_dl > max_dl)
							max_dl = cur_dl;
					}
					if (max_dl < 0.9 || min_dl >=1)
						/* Hardest seems to be
						 * a mixture of arb
						 * and constrained
						 * DL */
						continue;
					if (fabs(my_task_set.h_per_tol) <= 1e-20)
						/* Keep only if hyperP exact */
						break;
				}
				fprintf(fdata, "%u,%u,%f,%f,%u,%f,%f,%f,",
					rand_setup.seed, rand_setup.num_tasks,
					rand_setup.per_min, rand_setup.per_max, rand_setup.phasing,
					rand_setup.norm_dl_avg, rand_setup.norm_dl_var,
					my_task_set.h_per);
				fflush(fdata);
				clock_gettime(CLOCK_MONOTONIC, &start);
				edf_create_points(&my_task_set, &my_points);
				clock_gettime(CLOCK_MONOTONIC, &check);
				time_points = (check.tv_nsec-start.tv_nsec)*1e-9
					+(check.tv_sec-start.tv_sec);
				edf_qhull_points(&my_points);
				clock_gettime(CLOCK_MONOTONIC, &check);
				time_qhull = (check.tv_nsec-start.tv_nsec)*1e-9
					+(check.tv_sec-start.tv_sec)-time_points;
				fprintf(fdata, "%u,%u,%f,%f\n",
					my_points.num_points, my_points.num_sel,
					time_points, time_qhull);
			}
		}
		printf("%d\t", ++iter);
	}
}
#endif


#if 0
int main_span_per(int argc, char* argv[])
/*int main(int argc, char* argv[])*/
{
	ts_t         my_task_set;
	ts_rand_t    rand_setup;
	edf_points_t my_points;
	double max_d, max_o;
	struct timespec start, check;
	double time_points, time_qhull;
	int num_span[] = {4};
	int i, j, k, iter=0;
	double cur_dl, min_dl, max_dl;

	if (argc >= 2) {
		fdata = fopen(argv[1],"a");
	} else {
		fdata = fopen(FILENAME,"a");
	}

	/* Initialization */
	my_task_set.num = 0;
	my_task_set.per = NULL;
	my_task_set.dl = NULL;
	my_task_set.phi = NULL;
	rand_setup.per_m = per_unif;
	rand_setup.num_tasks = 4;  /* to be changed in this experiment */
	rand_setup.per_min = 2;
	rand_setup.per_max = 200;
	rand_setup.norm_dl_avg = 1;
	rand_setup.norm_dl_var = 0.4;
	rand_setup.phasing = 0;
	my_points.alloc_points = 0;
	my_points.t0 = NULL;
	my_points.t1 = NULL;
	my_points.vec_p = NULL;
	my_points.qh_vec_p = NULL;
	my_points.num_sel = 0;
	my_points.vec_sel = NULL;
	srand(time(NULL));
	while (1) {
		for(i=0; i<LEN(num_span); i++) {
			rand_setup.num_tasks = num_span[i];
			while(1) {
				rand_setup.seed = rand();
				ts_rand(&my_task_set, &rand_setup);
				/*  min/max deadline/period */
				max_dl = min_dl =
					my_task_set.dl[0]/ my_task_set.per[0];
				for (k=1; k<my_task_set.num; k++) {
					cur_dl = my_task_set.dl[k]/my_task_set.per[k];
					if (cur_dl < min_dl)
						min_dl = cur_dl;
					if (cur_dl > max_dl)
						max_dl = cur_dl;
				}
				if (max_dl < 0.9 || min_dl >=1)
					/* Hardest seems to be a
					 * mixture of arb and
					 * constrained DL */
					continue;
				if (fabs(my_task_set.h_per_tol) <= 1e-20)
					/* Keep only if hyperP exact */
					break;
			}
			fprintf(fdata, "%u,%u,%f,%f,%u,%f,%f,%f,",
				rand_setup.seed, rand_setup.num_tasks,
				rand_setup.per_min, rand_setup.per_max, rand_setup.phasing,
				rand_setup.norm_dl_avg, rand_setup.norm_dl_var,
				my_task_set.h_per);
			fflush(fdata);
			clock_gettime(CLOCK_MONOTONIC, &start);
			edf_create_points(&my_task_set, &my_points);
			clock_gettime(CLOCK_MONOTONIC, &check);
			time_points = (check.tv_nsec-start.tv_nsec)*1e-9
				+(check.tv_sec-start.tv_sec);
			edf_qhull_points(&my_points);
			clock_gettime(CLOCK_MONOTONIC, &check);
			time_qhull = (check.tv_nsec-start.tv_nsec)*1e-9
				+(check.tv_sec-start.tv_sec)-time_points;
			fprintf(fdata, "%u,%u,%f,%f\n",
				my_points.num_points, my_points.num_sel,
				time_points, time_qhull);
		}
		/*printf("%d\t", ++iter); */
	}
}
#endif


#if 0
int main_num(int argc, char* argv[])
/*int main(int argc, char* argv[])*/
{
	ts_t         my_task_set;
	ts_rand_t    rand_setup;
	edf_points_t my_points;
	double max_d, max_o;
	struct timespec start, check;
	double time_points, time_qhull;
	int num_span[] = {8, 7, 6, 5, 4, 3, 2};
	int i, j, k, iter=0;
	double cur_dl, min_dl, max_dl;

	if (argc >= 2) {
		fdata = fopen(argv[1],"a");
	} else {
		fdata = fopen(FILENAME,"a");
	}

	/* Initialization */
	my_task_set.num = 0;
	my_task_set.per = NULL;
	my_task_set.dl = NULL;
	my_task_set.phi = NULL;
	rand_setup.per_m = per_unif;
	rand_setup.num_tasks = 3;  /* to be changed in this experiment */
	rand_setup.per_min = 2;
	rand_setup.per_max = 1000;  /* to be changed in this experiment */
	rand_setup.norm_dl_avg = 1;
	rand_setup.norm_dl_var = 0.4;
	rand_setup.phasing = 0;
	my_points.alloc_points = 0;
	my_points.t0 = NULL;
	my_points.t1 = NULL;
	my_points.vec_p = NULL;
	my_points.qh_vec_p = NULL;
	my_points.num_sel = 0;
	my_points.vec_sel = NULL;
	srand(time(NULL));
	while (1) {
		for(i=0; i<LEN(num_span); i++) {
			rand_setup.num_tasks = num_span[i];
			rand_setup.per_max = ceil(pow(1e6,1.0/(double)num_span[i]))+10;
			while(1) {
				rand_setup.seed = rand();
				ts_rand(&my_task_set, &rand_setup);
				/*  min/max deadline/period */
				max_dl = min_dl =
					my_task_set.dl[0]/ my_task_set.per[0];
				for (k=1; k<my_task_set.num; k++) {
					cur_dl = my_task_set.dl[k]/my_task_set.per[k];
					if (cur_dl < min_dl)
						min_dl = cur_dl;
					if (cur_dl > max_dl)
						max_dl = cur_dl;
				}
				if (max_dl < 0.9 || min_dl >=1)
					/* Hardest seems to be a
					 * mixture of arb and
					 * constrained DL */
					continue;
				if (fabs(my_task_set.h_per_tol) <= 1e-20)
					/* Keep only if hyperP exact */
					break;
			}
			fprintf(fdata, "%u,%u,%f,%f,%u,%f,%f,%f,",
				rand_setup.seed, rand_setup.num_tasks,
				rand_setup.per_min, rand_setup.per_max, rand_setup.phasing,
				rand_setup.norm_dl_avg, rand_setup.norm_dl_var,
				my_task_set.h_per);
			fflush(fdata);
			clock_gettime(CLOCK_MONOTONIC, &start);
			edf_create_points(&my_task_set, &my_points);
			clock_gettime(CLOCK_MONOTONIC, &check);
			time_points = (check.tv_nsec-start.tv_nsec)*1e-9
				+(check.tv_sec-start.tv_sec);
			edf_qhull_points(&my_points);
			clock_gettime(CLOCK_MONOTONIC, &check);
			time_qhull = (check.tv_nsec-start.tv_nsec)*1e-9
				+(check.tv_sec-start.tv_sec)-time_points;
			fprintf(fdata, "%u,%u,%f,%f\n",
				my_points.num_points, my_points.num_sel,
				time_points, time_qhull);
		}
		/*printf("%d\t", ++iter); */
	}
}
#endif
