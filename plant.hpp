#pragma once
// Your model of the actuator, reconstructed from the decoded CSVs. This is
// the Part B deliverable, alongside your written notes.
//
// Implement step(): given a commanded velocity and a timestep, return the
// measured output angle. The placeholder below is a bare integrator with
// gain 1 -- NOT the real actuator. Replace it with what the data shows
// (dynamics, gain, any nonlinearity, any lag), or the harness proves nothing.

#include <cmath>

struct Plant {
    // add whatever state your model needs (velocity, motor-side angle, ...)
    double angle = 0.0;

    // u_cmd : commanded velocity, deg/s
    // dt    : timestep, seconds
    // return: measured output angle, deg
    double step(double u_cmd, double dt) {
        angle += u_cmd * dt;                   // placeholder dynamics -- replace this
        return std::round(angle / 0.1) * 0.1;  // the sensor reads to 0.1 deg
    }

    /*
    Pseudocode:
    Logic and understanding:

    from step_test.csv
    u_val = 15 (commanded rate while on, deg/s)
    switched on at t_on = 0
    t_off = 3.005
    y_init = 0 (angle at t_on)
    y_at_off = 54.8 (angle at t_off)
    y_final = 56.1 (last row, t = 9.995)

    Important parameters:
    <3.005,0,54.8>
    <0.19,15,0>
    <0.195,15,0.1>
    

    Steady velocity v = (y2 - y1) / (t2 - t1) = (54.7 - 15.7) / (3.0 - 1.0) = 19.5 deg/s

    Gain K = v / u_val = 19.5 / 15 = 1.30   (dimensionless, positive)

    Time constant (tao), method 1, from the curve-off after the command stops:
    curve-off distance = y_final - y_at_off = 56.1 - 54.8 = 1.3 deg
    curve-off distance = v * tao, so tao = curve-off distance / v = 1.3 / 19.5 = 0.067 s

    From the data, it seems like the actuator was at rest until about t = 0.19 s even though
    the command was already 15, then moved at a steady 19.5 deg/s, and glided a little
    further (1.3 deg) after the command stopped.

    General case:
    the lag is on the velocity, and the angle is the integral of the velocity.

    tao * dv/dt + v = K * u
    d(angle)/dt = v

    Change in velocity = (dt/tao) * (K * u_cmd - velocity);
    Change in angle    = velocity * dt;

    */

    void reset() { angle = 0.0; }
};
