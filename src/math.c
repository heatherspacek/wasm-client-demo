#include "math.h"
// fifth-degree approximations, from:
// https://gist.github.com/publik-void/067f7f2fef32dbe5c27d6e215f824c91#sin-rel-error-minimized-degree-5
// https://gist.github.com/publik-void/067f7f2fef32dbe5c27d6e215f824c91#cos-rel-error-minimized-degree-6
double sin(double x1)
{
    float x2 = x1*x1;
    return x1*(0.999891821255810892885564707156941565 + x2*(-0.165960116540878989063185380996540407 + 0.00760290334336935120704015646842617915*x2));
}

double cos(double x1)
{
    double x2 = x1*x1;
    return 0.999970210689953068626323587055728078 + x2*(-0.499782706704688809140466617726333455 + x2*(0.0413661149638482252569383872576459943 - 0.0012412397582398600702129604944720102*x2));
}

double fabs(double x)
{
    if (x < 0) {
        return -x;
    }
    return x;
}
