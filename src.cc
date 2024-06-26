#include <napi.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <utility>
#include <vector>

extern "C" {
#include "pgm.h"
}

static PGMImage *image;
static OccGridMap *occ;
// static int *v_tbl;
static std::vector<int> v_tbl;
static std::vector<std::vector<std::pair<int, int>>> parents;

bool findPath(int x, int y, int w, int h, int goal_x, int goal_y, int parent_x,
              int parent_y, bool isRotation) {
  if (v_tbl[y * h + x] != 0) {
    return false;
  }
  if (x >= w || y >= h) {
    // printf("out of bound\r\n");
    return false;
  }

  v_tbl[y * h + x] = 1;
  if (image->data[y][x].red < 205) {
    // printf("(%d, %d) cannot be reached\r\n", x, y);
    v_tbl[y * h + x] = 2;
    return false;
  }

  int robot_size_x = 0.55 / 0.05;
  int robot_size_y = 0.35 / 0.05;

  if (isRotation) {
    robot_size_x = 0.35 / 0.05;
    robot_size_y = 0.55 / 0.05;
  }

  for (int j = (y - robot_size_y); j <= (y + robot_size_y); j++) {
    for (int i = (x - robot_size_x); i <= (x + robot_size_x); i++) {
      if (j < 0 || j >= h || i < 0 || i >= w) {
        // printf("out-of-bound\r\n");
        v_tbl[y * h + x] = 2;
        return false;
      }
      int data = image->data[j][i].red;
      if (data == 0 || data == 205) {
        // printf("the robot cannot pass this node\r\n");
        v_tbl[y * h + x] = 2;
        return false;
      }
    }
  }

  parents[x][y] = {parent_x, parent_y};

  if (goal_x == x && goal_y == y) {
    return true;
  }

  if (findPath(x + 1, y, w, h, goal_x, goal_y, x, y, false)) {
    return true;
  }
  if (findPath(x - 1, y, w, h, goal_x, goal_y, x, y, false)) {
    return true;
  }
  if (findPath(x, y + 1, w, h, goal_x, goal_y, x, y, true)) {
    return true;
  }
  if (findPath(x, y - 1, w, h, goal_x, goal_y, x, y, true)) {
    return true;
  }

  v_tbl[y * h + x] = 2;
  return false;
}

Napi::Value readPGM(const Napi::CallbackInfo &info) {
  printf("version: 0.0.0\r\n");
  Napi::Env env = info.Env();
  std::string fPath = info[0].As<Napi::String>();
  image = new PGMImage();
  if (fPath.length() > 0) {
    const char *fPathChar = fPath.c_str();
    getPGMfile(fPathChar, image);
  }

  int w = image->width;
  int h = image->height;
  int maxVal = image->maxVal;
  Napi::Int32Array buf = Napi::Int32Array::New(env, w * h);

  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      // int y = (h - 1) - i;
      buf[i * w + j] = image->data[i][j].red;
    }
  }
  Napi::Array res = Napi::Array::New(env, 3);
  res[1] = w;
  res[2] = h;
  napi_set_element(env, res, 0, buf);
  napi_set_element(env, res, 1, Napi::Number::New(env, w));
  napi_set_element(env, res, 2, Napi::Number::New(env, h));

  // save(image);
  return res;
}

Napi::Value loadMap(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  double occ_th = info[0].As<Napi::Number>().DoubleValue();
  double free_th = info[1].As<Napi::Number>().DoubleValue();
  double resolution = info[2].As<Napi::Number>().DoubleValue();

  if (!image) {
    printf("Call readPGM first, and make sure the map file is fed.");
    return Napi::Boolean::New(env, false);
  }

  if (!occ) {
    occ = new OccGridMap();
    occ->resolution = resolution;
    occ->width = image->width;
    occ->height = image->height;
    int *oc_grid = (int *)malloc(image->width * image->height * sizeof(int));
    double sum_pix = 0;
    int count_100 = 0;
    int count_0 = 0;
    int count_1 = 0;
    for (int y = image->height - 1; y >= 0; y--) {
      for (int x = 0; x < image->width; x++) {
        int index = MAP_IDX(image->width, x, image->height - y - 1);
        unsigned char r = image->data[x][y].red;
        unsigned char g = image->data[x][y].green;
        unsigned char b = image->data[x][y].blue;
        double pixel = (r + g + b) / 3.0f;
        double occ = (255.0 - pixel) / 255.0f;
        if (occ > occ_th) {
          pixel = 100;
          count_100++;
        } else if (occ < free_th) {
          pixel = 0;
          count_0++;
        } else {
          // double ratio = (occ - free_th) / (occ_th - free_th);
          // pixel = 1 + 98 * ratio;
          pixel = -1;
          count_1++;
        }
        oc_grid[index] = pixel;
        sum_pix += pixel;
      }
    }
    occ->data = oc_grid;
    double mean = sum_pix / (image->width * image->height);
    printf("mean %.5f\r\n", mean);
    printf("100: %d, 0: %d, -1: %d\r\n", count_100, count_0, count_1);
    free(oc_grid);
    // free(image);
  }

  Napi::ArrayBuffer buf =
      Napi::ArrayBuffer::New(env, occ->data, (image->width * image->height));

  return buf;
}

Napi::Value navigation(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  // robot_pose_x/y/z is map coordinate
  int robot_pose_x = info[0].As<Napi::Number>();
  int robot_pose_y = info[1].As<Napi::Number>();
  int robot_pose_z = info[2].As<Napi::Number>();
  // target_x/y is the distance from the robot pose
  int target_x = info[3].As<Napi::Number>();
  int target_y = info[4].As<Napi::Number>();

  if (!occ) {
    printf("Call loadMap after readPGM to load the configuration of the map.");
    return Napi::Boolean::New(env, false);
  }

  int w = occ->width;
  int h = occ->height;
  if (image->data[target_y][target_x].red < 254) {
    printf("Cannot arrive the goal");
    printf(" data: %d\r\n", image->data[target_y][target_x].red);
    return Napi::Boolean::New(env, false);
  }
  double resolution = occ->resolution;
  // int target_x_pix = target_x / resolution;
  // int target_y_pix = target_y / resolution;
  // v_tbl = (int *)calloc(w * h, sizeof(int));
  v_tbl.clear();
  v_tbl.resize(w * h, 0);
  parents.clear();
  parents.resize(w, std::vector<std::pair<int, int>>(h));
  bool res = findPath(robot_pose_x, robot_pose_y, w, h, target_x, target_y, -1,
                      -1, false);
  // for (int y = 0; y < h; y++) {
  //   for (int x = 0; x < w; x++) {
  //     switch (v_tbl[y * h + x]) {
  //       case 0:
  //         printf("#");
  //         break;
  //       case 1:
  //         printf(" ");
  //         break;
  //       case 2:
  //         printf("X");
  //         break;
  //       default:
  //         break;
  //     }
  //   }
  //   printf("\n");
  // }
  std::vector<std::pair<int, int>> path;
  std::pair<int, int> current = {target_x, target_y};
  std::pair<int, int> start = {robot_pose_x, robot_pose_y};
  while (current != start) {
    path.push_back(current);
    current = parents[current.first][current.second];
  }
  path.push_back(start);
  std::reverse(path.begin(), path.end());
  for (const auto &point : path) {
    printf("(%d, %d) -> ", point.first, point.second);
  }
  return Napi::Boolean::New(env, res);
}

Napi::Value freeObjs(const Napi::CallbackInfo &info) {
  printf("Ready to free all resources\r\n");
  if (image) {
    delete image;
    printf("PMGImage is free\r\n");
  }
  if (occ) {
    delete[] occ;
    printf("OCC_Grid is free\r\n");
  }

  // if (v_tbl) {
  //   free(v_tbl);
  // }

  Napi::Env env = info.Env();
  return Napi::Boolean::New(env, true);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "readPGM"),
              Napi::Function::New(env, readPGM));
  exports.Set(Napi::String::New(env, "loadMap"),
              Napi::Function::New(env, loadMap));
  exports.Set(Napi::String::New(env, "freeObjs"),
              Napi::Function::New(env, freeObjs));
  exports.Set(Napi::String::New(env, "navigation"),
              Napi::Function::New(env, navigation));
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init);