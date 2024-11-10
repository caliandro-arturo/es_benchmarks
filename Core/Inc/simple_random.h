/******************************************************************************
*   Copyright 2021 Politecnico di Milano
*
*   Licensed under the Apache License, Version 2.0 (the "License");
*   you may not use this file except in compliance with the License.
*   You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*   Unless required by applicable law or agreed to in writing, software
*   distributed under the License is distributed on an "AS IS" BASIS,
*   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*   See the License for the specific language governing permissions and
*   limitations under the License.
*******************************************************************************/

/*
   32-bits Uniform random number generator U[0,1) lfsr113, based on the
   work of Pierre L'Ecuyer (http://www.iro.umontreal.ca/~lecuyer/myftp/papers/tausme2.ps)
*/

#ifndef SIMPLE_RANDOM_H_
#define SIMPLE_RANDOM_H_
#include <stdint.h>

void random_set_seed(uint32_t seed);

double random_get(void);

void random_get_array(double a[], int len);

void random_get_sarray(double a[], int len);

void random_get_iarray(uint32_t a[], int len);

void random_get_barray(int a[], int len);

#endif
