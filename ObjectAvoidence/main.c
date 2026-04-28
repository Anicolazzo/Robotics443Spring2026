
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "msp.h"
#include "Clock.h"
#include "Motor.h"
#include "Tachometer.h"
#include "Precision_Moves.h"
#include "ADC14.h"

#define WHEEL_DIAMETER_MM   70.0
#define ROBOT_WIDTH_MM      145.0
#define STEPS_PER_REV       360.0
#define MM_PER_STEP         (M_PI * WHEEL_DIAMETER_MM / STEPS_PER_REV)

#define NAV_RPM             750
#define TURN_PWM            2500
#define PAUSE_MS            80

#define ARRIVE_DIST_MM      20
#define FWD_PAST_MM         300

#define ADC_WALL_THRESHOLD  8900    // center sensor: above this = wall ahead
#define ADC_GAP_THRESHOLD   5000    // right sensor: below this = open gap

#define SIDE_STEP_MM        50
#define SIDE_STEP_MAX_MM    400

#define START_HEADING_DEG   0.0
#define TARGET_X_MM         0.0
#define TARGET_Y_MM         600.0

static double g_x       = 0.0;
static double g_y       = 0.0;
static double g_heading = 0.0;

static void check_sensors(int *wall, int *gap_right){
    uint32_t l1, c1, r1, l2, c2, r2;
    ADC_In17_14_16(&r1, &c1, &l1);
    ADC_In17_14_16(&r2, &c2, &l2);
    *wall      = ((c1 + c2) / 2 > ADC_WALL_THRESHOLD);
    *gap_right = ((r1 + r2) / 2 < ADC_GAP_THRESHOLD);
}

static int32_t mm_to_steps(double mm){
    return (int32_t)((mm / MM_PER_STEP) + 0.5);
}

static int32_t deg_to_steps(double deg){
    if(deg < 0.0) deg = -deg;
    return (int32_t)(((ROBOT_WIDTH_MM / WHEEL_DIAMETER_MM) * deg) + 0.5);
}

static double normalize_angle(double a){
    while(a >  180.0) a -= 360.0;
    while(a < -180.0) a += 360.0;
    return a;
}

static void drive_forward(double dist_mm){
    if(dist_mm <= 0.0) return;

    int32_t L0, R0, L1, R1;
    Tachometer_Get_Steps(&L0, &R0);

    Motor_Forward_RPM(NAV_RPM, NAV_RPM, mm_to_steps(dist_mm), mm_to_steps(dist_mm));

    Tachometer_Get_Steps(&L1, &R1);

    double actual_mm   = ((L1 - L0) + (R1 - R0)) / 2.0 * MM_PER_STEP;
    double heading_rad = g_heading * M_PI / 180.0;
    g_x += actual_mm * cos(heading_rad);
    g_y += actual_mm * sin(heading_rad);

    Clock_Delay1ms(PAUSE_MS);
}

static void pivot_left(double angle_deg){
    if(angle_deg <= 0.0) return;
    Motor_Precision_Left(TURN_PWM, deg_to_steps(angle_deg));
    g_heading = normalize_angle(g_heading + angle_deg);
    Clock_Delay1ms(PAUSE_MS);
}

static void pivot_right(double angle_deg){
    if(angle_deg <= 0.0) return;
    Motor_Precision_Right(TURN_PWM, deg_to_steps(angle_deg));
    g_heading = normalize_angle(g_heading - angle_deg);
    Clock_Delay1ms(PAUSE_MS);
}

static void turn_to_heading(double target_deg){
    double delta = normalize_angle(target_deg - g_heading);
    if(delta > 0.5)
        pivot_left(delta);
    else if(delta < -0.5)
        pivot_right(-delta);
}

static void avoid_obstacle(void){
    int wall, gap_right;
    double stepped = 0.0;

    check_sensors(&wall, &gap_right);

    if(gap_right)
        pivot_right(90.0);
    else
        pivot_left(90.0);
    while(stepped < SIDE_STEP_MAX_MM){
        check_sensors(&wall, &gap_right);
        if(!wall) break;
        drive_forward(SIDE_STEP_MM);
        stepped += SIDE_STEP_MM;
    }

    drive_forward(FWD_PAST_MM);

}
static void navigate_to(double tx, double ty){
    int wall, gap_right;

    while(1){
        double dx   = tx - g_x;
        double dy   = ty - g_y;
        double dist = sqrt(dx*dx + dy*dy);

        if(dist < ARRIVE_DIST_MM) break;

        turn_to_heading(atan2(dy, dx) * 180.0 / M_PI);

        check_sensors(&wall, &gap_right);
        if(wall){
            avoid_obstacle();
            continue;   
        }

        drive_forward(dist);
    }

    Motor_Stop();
}

int main(void){
    Clock_Init48MHz();
    Motor_Init();
    Tachometer_Init();
    ADC0_InitSWTriggerCh17_14_16();

    Clock_Delay1ms(500);

    g_x       = 0.0;
    g_y       = 0.0;
    g_heading = START_HEADING_DEG;

    navigate_to(TARGET_X_MM, TARGET_Y_MM);

    Motor_Stop();
    while(1){}

    return 0;
}
