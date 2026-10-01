#ifndef UTILS_H
#define UTILS_H

static inline int IsEqual(double a, double b) {
    const double eps = 1e-7;
    double diff = a - b;
    return (diff < 0 ? -diff : diff) < eps;
}

#endif /* UTILS_H */