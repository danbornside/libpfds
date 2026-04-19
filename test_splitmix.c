/* This file is part of libpfds, Persistent Functional Data Structures in C.
 *
 * libpfds is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * libpfds is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * libpfds. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include "splitmix.h"

int main() {
    SplitMix64_initialSeed();

    long bins[20] = {};
    SplitMix64 myGen;
    SplitMix64_initDefault(&myGen);
    for (int i = 0; i < 10000 ; i++) {
        double x = SplitMix64_nextDouble(&myGen);
        bins[(int)(x * 20)]++;
    }
    for (int i = 0; i < 20 ; i++) {
        printf("[%.03f , %.03f): ", i/20.0, (i+1)/20.0);
        for(int j = 0; j < bins[i] >> 4 ; j++) printf("*");
        printf("\n");
        bins[i] = 0;
    }

    for (int i = 0; i < 10000 ; i++) {
        int x = SplitMix64_nextIntRange(&myGen, 3, 13);
        bins[x]++;
    }
    for (int i = 0; i < 20 ; i++) {
        printf("%3d: ", i);
        for(int j = 0; j < bins[i] >> 4 ; j++) printf("*");
        printf("\n");
        bins[i] = 0;
    }

    for (int i = 0; i < 10000 ; i++) {
        double x = SplitMix64_nextDoubleRange(&myGen, 3.5, 13.5);
        bins[(int)x]++;
    }
    for (int i = 0; i < 20 ; i++) {
        printf("[%4.1f, %4.1f): ", (double) i, (double) (i+1));
        for(int j = 0; j < bins[i] >> 4 ; j++) printf("*");
        printf("\n");
        bins[i] = 0;
    }
    return 0;
}
