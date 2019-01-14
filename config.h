#ifndef CONFIG_H
#define CONFIG_H

#define SKIP_OPTIMAL
#define SKIP_NAIVE

//Measurement testing params (default: 3, 10, 1000)
#define MIN_DIM         (3)
#define STEP            (10)
#define MAX_DIM         (1000)

//Axes params (default: 1, MAX_DIM)
#define Y_MAX_VAL       (1)
#define X_MAX_VAL       (MAX_DIM)

#endif // CONFIG_H
