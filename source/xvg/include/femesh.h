/***********************************************************************
*
*  Name:          femesh.h
*
*  Author:        Paul Charette
*
*  Last Modified:
*                 - Paul Charette, 5 April 1997
*                   File creation.
*
*  Purpose:       Defines the fe mesh for a circular membrane
*                 having 32 elements (0..31: 8 linear triangles and
*                 24 bilinear rectangles) and 37 nodes (1..37).
*
*                 The format of the array below (one element definition
*                 per line) is the following:
*                  - element type (R_TYPE or T_TYPE)
*                  - clockwise list of nodes (4)
*                  - clockwise list of neighbouring elements (8)
*                    (values of -1 indicate a non-existant node or element)
*
***********************************************************************/
MeshElement MeshElements[32] = {
  {R_TYPE, 17,  8,  7, 18,  25, -1, -1, -1, 1, 4, 3, 2,     0, 0}, /* e0 */
  {R_TYPE, 18,  7,  6, 19,   0, -1, -1, -1, 26, 5, 4, 3,    0, 0}, /* e1 */
  {R_TYPE, 20,  9, 17, 21,  24, -1, 25, 0, 3, 8, 7, 6,      0, 0}, /* e2 */
  {R_TYPE, 21, 17, 18, 22,   2, 25, 0, 1, 4, 9, 8, 7,       0, 0}, /* e3 */
  {R_TYPE, 22, 18, 19, 23,   3, 0, 1, 26, 5, 10, 9, 8,      0, 0}, /* e4 */
  {R_TYPE, 23, 19,  5, 24,   4, 1, 26, -1, 27, 11, 10, 9,   0, 0}, /* e5 */
  {R_TYPE, 11, 10, 20, 25,  -1, -1, 24, 2, 7, 13, 12, -1,   0, 0}, /* e6 */
  {R_TYPE, 25, 20, 21, 26,   6, 24, 2, 3, 8, 14, 13, 12,    0, 0}, /* e7 */
  {R_TYPE, 26, 21, 22, 27,   7, 2, 3, 4, 9, 15, 14, 13,     0, 0}, /* e8 */
  {R_TYPE, 27, 22, 23, 28,   8, 3, 4, 5, 10, 16, 15, 14,    0, 0}, /* e9 */
  {R_TYPE, 28, 23, 24, 29,   9, 4, 5, 27, 11, 17, 16, 15,   0, 0}, /* e10*/
  {R_TYPE, 29, 24,  4,  3,  10, 5, 27, -1, -1, -1, 17, 16,  0, 0}, /* e11*/
  {R_TYPE, 12, 11, 25, 30,  -1, -1, 6, 7, 13, 18, 31, -1,   0, 0}, /* e12*/
  {R_TYPE, 30, 25, 26, 31,  12, 6, 7, 8, 14, 19, 18, 31,    0, 0}, /* e13*/
  {R_TYPE, 31, 26, 27, 32,  13, 7, 8, 9, 15, 20, 19, 18,    0, 0}, /* e14*/
  {R_TYPE, 32, 27, 28, 33,  14, 8, 9, 10, 16, 21, 20, 19,   0, 0}, /* e15*/
  {R_TYPE, 33, 28, 29, 34,  15, 9, 10, 11, 17, 28, 21, 20,  0, 0}, /* e16*/
  {R_TYPE, 34, 29,  3,  2,  16, 10, 11, -1, -1, -1, 28, 21, 0, 0}, /* e17*/
  {R_TYPE, 13, 30, 31, 35,  31, 12, 13, 14, 19, 22, 30, -1, 0, 0}, /* e18*/
  {R_TYPE, 35, 31, 32, 36,  18, 13, 14, 15, 20, 23, 22, 30, 0, 0}, /* e19*/
  {R_TYPE, 36, 32, 33, 37,  19, 14, 15, 16, 21, 29, 23, 22, 0, 0}, /* e20*/
  {R_TYPE, 37, 33, 34,  1,  20, 15, 16, 17, 28, -1, 29, 23, 0, 0}, /* e21*/
  {R_TYPE, 14, 35, 36, 15,  30, 18, 19, 20, 23, -1, -1, -1, 0, 0}, /* e22*/
  {R_TYPE, 15, 36, 37, 16,  22, 19, 20, 21, 29, -1, -1, -1, 0, 0}, /* e23*/
  {T_TYPE, 10,  9, 20, -1,   2,  7,  6, -1, -1, -1, -1, 25, 0, 0}, /* e24*/
  {T_TYPE,  9,  8, 17, -1,   0,  3,  2, 24, -1, -1, -1, -1, 0, 0}, /* e25*/
  {T_TYPE,  6,  5, 19, -1,   5,  4,  1, -1, -1, -1, -1, 27, 0, 0}, /* e26*/
  {T_TYPE,  5,  4, 24, -1,  11, 10,  5, 26, -1, -1, -1, -1, 0, 0}, /* e27*/
  {T_TYPE,  2,  1, 34, -1,  21, 16, 17, -1, -1, -1, -1, 29, 0, 0}, /* e28*/
  {T_TYPE,  1, 16, 37, -1,  23, 20, 21, 28, -1, -1, -1, -1, 0, 0}, /* e29*/
  {T_TYPE, 14, 13, 35, -1,  18, 19, 22, -1, -1, -1, -1, 31, 0, 0}, /* e30*/
  {T_TYPE, 13, 12, 30, -1,  12, 13, 18, 30, -1, -1, -1, -1, 0, 0}  /* e31*/
};
