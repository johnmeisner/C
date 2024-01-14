/**************************************************************************
 *                                                                        *
 *     File Name:  test_util.c                                            *
 *     Author:                                                            *
 *         DENSO International America, Inc.                              *
 *         North America Research Laboratory, California Office           *
 *         3252 Business Park Drive                                       *
 *         Vista, CA 92081                                                *
 *                                                                        *
 **************************************************************************/



#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "i2v_general.h"
#include "i2v_util.h"


/* Tests out i2v_signal_phase_to_movement_phase_state() */
void test_1()
{
    unsigned int i, j, cnt_good=0, cnt_bad=0;

    // Struct to create arrays of inputs and expected output
    struct test_step {
        i2vSigPhases signal_phase;
        i2vMovementPhaseState green_means;
        i2vMovementPhaseState expected_output_movement_phase_state;
    };


    /* Test1 - MikeTest 1 with GreenMeans=Protected
     *
     * NOTE: MikeTest1 is from Mike's conversation w Terry over FYA:
     *   From: Michael Lockerman <michael.lockerman@na.denso.com> 
     *   Sent: Tuesday, April 27, 2021 1:17 PM
     *   To: Terry Macaulay <terry.macaulay@na.denso.com>
     *   Subject: Re: green means and FYA extra credit
     *
     *   But I'd hesitate to call it 'correct'.
     *   The only case it works for is a steady-yellow that's only 1/10 second long.
     *   Which is not a real-world scenario.
     *   
     *   I'd suggest updating the unit test to TX 2 of each blob.  (Or at least 2 of the steady-yellows.)
     *   As in .. send 2 of each (instead of a single) in this order.
     *   steady-red 
     *   steady-green
     *   steady-yellow
     *   flashing-yellow
     *   steady-yellow
     *   steady-red
     *
     * The signal phases for input into this test, with GreenMeans=Protected, are:
     *    R R G G Y Y FY FY Y Y R R
     */
    struct test_step test1_data[] = {
        /* Signal Phase,             GreenMeans,                     Expected MPS output */
        { SIG_PHASE_RED,             MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_STOP_AND_REMAIN },
        { SIG_PHASE_RED,             MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_STOP_AND_REMAIN },
        { SIG_PHASE_GREEN,           MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_PROTECTED_MOVEMENT_ALLOWED },
        { SIG_PHASE_GREEN,           MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_PROTECTED_MOVEMENT_ALLOWED },
        { SIG_PHASE_YELLOW,          MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_PROTECTED_CLEARENCE },
        { SIG_PHASE_YELLOW,          MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_PROTECTED_CLEARENCE },
        { SIG_PHASE_FLASHING_YELLOW, MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_PERMISSIVE_MOVEMENT_ALLOWED },
        { SIG_PHASE_FLASHING_YELLOW, MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_PERMISSIVE_MOVEMENT_ALLOWED },
        { SIG_PHASE_YELLOW,          MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_PERMISSIVE_CLEARENCE },
        { SIG_PHASE_YELLOW,          MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_PERMISSIVE_CLEARENCE },
        { SIG_PHASE_RED,             MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_STOP_AND_REMAIN },
        { SIG_PHASE_RED,             MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_STOP_AND_REMAIN },
        { 0, 0, 0}, /* Marks data end */
    };
    // Test2 - MikeTest 1 with GreenMeans=Permitted
    // This is the same test sequence as test1, but with GreenMeans=Permissive,
    //     which changes expected results!
    // R R G G Y Y FY FY Y Y R R
    struct test_step test2_data[] = {
        /* Signal Phase,             GreenMeans,                     Expected MPS output */
        { SIG_PHASE_RED,             MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_STOP_AND_REMAIN },
        { SIG_PHASE_RED,             MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_STOP_AND_REMAIN },
        { SIG_PHASE_GREEN,           MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_PERMISSIVE_MOVEMENT_ALLOWED },
        { SIG_PHASE_GREEN,           MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_PERMISSIVE_MOVEMENT_ALLOWED },
        { SIG_PHASE_YELLOW,          MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_PERMISSIVE_CLEARENCE },
        { SIG_PHASE_YELLOW,          MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_PERMISSIVE_CLEARENCE },
        { SIG_PHASE_FLASHING_YELLOW, MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_PERMISSIVE_MOVEMENT_ALLOWED },
        { SIG_PHASE_FLASHING_YELLOW, MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_PERMISSIVE_MOVEMENT_ALLOWED },
        { SIG_PHASE_YELLOW,          MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_PERMISSIVE_CLEARENCE },
        { SIG_PHASE_YELLOW,          MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_PERMISSIVE_CLEARENCE },
        { SIG_PHASE_RED,             MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_STOP_AND_REMAIN },
        { SIG_PHASE_RED,             MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_STOP_AND_REMAIN },
        { 0, 0, 0}, /* Marks data end */
    };

    // Test3 - Tests in email dated Wed 2021-05-19 10:15 AM - GreenMeans=Protected
    // The email was a conversation with Mike to clarify the protected/permissive rules
    // Here, a green light follows a flashing yellow and again, the GreenMeans affects protected/permissive.
    // G G Y Y FY FY G Y Y R R
    struct test_step test3_data[] = {
        /* Signal Phase,             GreenMeans,                     Expected MPS output */
        { SIG_PHASE_GREEN,           MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_PROTECTED_MOVEMENT_ALLOWED },
        { SIG_PHASE_GREEN,           MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_PROTECTED_MOVEMENT_ALLOWED },
        { SIG_PHASE_YELLOW,          MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_PROTECTED_CLEARENCE },
        { SIG_PHASE_YELLOW,          MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_PROTECTED_CLEARENCE },
        { SIG_PHASE_FLASHING_YELLOW, MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_PERMISSIVE_MOVEMENT_ALLOWED },
        { SIG_PHASE_FLASHING_YELLOW, MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_PERMISSIVE_MOVEMENT_ALLOWED },
        { SIG_PHASE_GREEN,           MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_PROTECTED_MOVEMENT_ALLOWED },
        { SIG_PHASE_YELLOW,          MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_PROTECTED_CLEARENCE },
        { SIG_PHASE_YELLOW,          MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_PROTECTED_CLEARENCE },
        { SIG_PHASE_RED,             MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_STOP_AND_REMAIN },
        { SIG_PHASE_RED,             MPS_PROTECTED_MOVEMENT_ALLOWED, MPS_STOP_AND_REMAIN },
        { 0, 0, 0}, /* Marks data end */
    };

    // Test4 - Tests in email dated Wed 2021-05-19 10:15 AM - GreenMeans=Permissive
    // G G Y Y FY FY G Y Y R R
    struct test_step test4_data[] = {
        /* Signal Phase,             GreenMeans,                     Expected MPS output */
        { SIG_PHASE_GREEN,           MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_PERMISSIVE_MOVEMENT_ALLOWED },
        { SIG_PHASE_GREEN,           MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_PERMISSIVE_MOVEMENT_ALLOWED },
        { SIG_PHASE_YELLOW,          MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_PERMISSIVE_CLEARENCE },
        { SIG_PHASE_YELLOW,          MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_PERMISSIVE_CLEARENCE },
        { SIG_PHASE_FLASHING_YELLOW, MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_PERMISSIVE_MOVEMENT_ALLOWED },
        { SIG_PHASE_FLASHING_YELLOW, MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_PERMISSIVE_MOVEMENT_ALLOWED },
        { SIG_PHASE_GREEN,           MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_PERMISSIVE_MOVEMENT_ALLOWED },
        { SIG_PHASE_YELLOW,          MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_PERMISSIVE_CLEARENCE },
        { SIG_PHASE_YELLOW,          MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_PERMISSIVE_CLEARENCE },
        { SIG_PHASE_RED,             MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_STOP_AND_REMAIN },
        { SIG_PHASE_RED,             MPS_PERMISSIVE_MOVEMENT_ALLOWED, MPS_STOP_AND_REMAIN },
        { 0, 0, 0}, /* Marks data end */
    };

    // Assemble all tests

    struct test_step * tests[] = { test1_data, test2_data, test3_data, test4_data, NULL /* Data end */ };

    for (j=0; tests[j] != NULL; j++) {
        printf("Test %d\n", j+1);
        struct test_step * curtest = tests[j];
        i2vMovementPhaseState cur_mps = MPS_UNAVAILABLE;
        for (i=0; curtest[i].green_means != 0; i++) {
            cur_mps = i2v_signal_phase_to_movement_phase_state(
                curtest[i].signal_phase,
                curtest[i].green_means,
                cur_mps);
            if (cur_mps == curtest[i].expected_output_movement_phase_state) {
                ++cnt_good;
            } else {
                ++cnt_bad;
                printf("BAD: Test %d Step %d (i=%d) got %d %s expected %d %s\n",
                    j+1, i+1, i, cur_mps, 
                    i2v_movement_phase_state_to_string(cur_mps),
                    curtest[i].expected_output_movement_phase_state,
                    i2v_movement_phase_state_to_string(curtest[i].expected_output_movement_phase_state));
            }
        }
    }

    printf("Tests done: %d good, %d bad steps\n", cnt_good, cnt_bad);
}


int main()
{
    printf("Hello\n");
    test_1();
    return 0;
}
