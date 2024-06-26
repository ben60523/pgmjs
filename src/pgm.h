/**
 * https://sun.iwu.edu/~shelley/sie/zoo/journal/pgm.h.html
 */
#ifndef PGM_H
#define PGM_H

/*max size of an image*/
#define MAX 800
#define MAP_IDX(sx, i, j) ((sx) * (j) + (i))

/*
#define LOW_VALUE 0
#define HIGH_VALUE 255
*/

/*RGB color struct with integral types*/
typedef struct {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} RGB_INT;

struct PGMstructure {
  int maxVal;
  int width;
  int height;
  RGB_INT data[MAX][MAX];
};

struct OccGridMapStructure {
  double resolution;
  int width;
  int height;
  int *data;
};

typedef struct PGMstructure PGMImage;
typedef struct OccGridMapStructure OccGridMap;

/***prototypes**********************************************************/
/***********************************************************************/

void getPGMfile(const char filename[], PGMImage *img);
void save(PGMImage *img);

#endif