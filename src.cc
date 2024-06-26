#include <napi.h>

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <vector>

extern "C" {
#include "pgm.h"
}

typedef struct {
  int x, y;
  int dist;
} Node;

static PGMImage *image;
// static OccGridMap *occ;
// static int *v_tbl;
static std::vector<std::vector<bool>> v_tbl;
static std::vector<std::vector<std::pair<int, int>>> parents;

bool isValid(int x, int y) {
  int w = image->width;
  int h = image->height;
  int data = (image->data[y][x].red + image->data[y][x].green +
              image->data[y][x].blue) /
             3;
  return (x >= 0) && (x < w) && (y >= 0) && (y < h) && (data >= 254) &&
         (!v_tbl[x][y]);
}

int BFS(int startX, int startY, int endX, int endY) {
  // If the start or end cell is a wall, return -1 (no path possible)
  int w = image->width;
  int h = image->height;
  int start_data =
      (image->data[startY][startX].red + image->data[startY][startX].blue +
       image->data[startY][startX].green) /
      3;
  int end_data = (image->data[endY][endX].red + image->data[endY][endX].blue +
                  image->data[endY][endX].green) /
                 3;
  if (start_data == 0 || end_data == 0) {
    return -1;
  }

  // v_tbl array to keep track of v_tbl cells
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      v_tbl[i][j] = false;
    }
  }

  // Queue for BFS
  // Node queue[w * h];
  Node *queue = (Node*)malloc(w * h * sizeof(Node));
  int front = 0, rear = 0;

  // Enqueue the starting cell and mark it as v_tbl
  Node n = Node();
  n.x = startX;
  n.y = startY;
  n.dist = 0;
  queue[rear++] = n;
  v_tbl[startX][startY] = true;

  // Perform BFS
  while (front < rear) {
    Node curr = queue[front++];

    int x = curr.x;
    int y = curr.y;
    int dist = curr.dist;

    // If we reached the destination cell, return the distance
    if (x == endX && y == endY) {
      free(queue);
      return dist;
    }

    // Explore all possible movements from the current cell
    int rowNum[] = {-1, 1, 0, 0};
    int colNum[] = {0, 0, -1, 1};
    for (int k = 0; k < 4; k++) {
      int newRow = x + rowNum[k];
      int newCol = y + colNum[k];

      // Check if the move is valid
      if (isValid(newRow, newCol)) {
        // Mark the cell as v_tbl and enqueue it
        v_tbl[newRow][newCol] = true;
        parents[newRow][newCol] = {x, y};
        Node next_n = Node();
        next_n.x = newRow;
        next_n.y = newCol;
        next_n.dist = dist + 1;
        queue[rear++] = next_n;
      }
    }
  }

  // If the destination is not reachable, return -1
  free(queue);
  return -1;
}

// bool findPath(int x, int y, int w, int h, int goal_x, int goal_y, int
// parent_x,
//               int parent_y, bool isRotation, int &min_dist, int dist) {
//   // if (v_tbl[y * h + x] != 0) {
//   //   return false;
//   // }
//   printf("(%d, %d)\n", x, y);
//   // if (x >= w || y >= h) {
//   //   // printf("out of bound\r\n");
//   //   return false;
//   // }

//   v_tbl[y * h + x] = 1;
//   // if (image->data[y][x].red < 205) {
//   //   // printf("(%d, %d) cannot be reached\r\n", x, y);
//   //   // v_tbl[y * h + x] = 2;
//   //   return false;
//   // }

//   int robot_size_x = 0.55 / 0.05;
//   int robot_size_y = 0.35 / 0.05;

//   if (isRotation) {
//     robot_size_x = 0.35 / 0.05;
//     robot_size_y = 0.55 / 0.05;
//   }

//   // for (int j = (y - robot_size_y); j <= (y + robot_size_y); j++) {
//   //   for (int i = (x - robot_size_x); i <= (x + robot_size_x); i++) {
//   //     if (j < 0 || j >= h || i < 0 || i >= w) {
//   //       // printf("out-of-bound\r\n");
//   //       // v_tbl[y * h + x] = 2;
//   //       return false;
//   //     }
//   //     int data = image->data[j][i].red;
//   //     if (data == 0 || data == 205) {
//   //       // printf("the robot cannot pass this node\r\n");
//   //       // v_tbl[y * h + x] = 2;
//   //       return false;
//   //     }
//   //   }
//   // }

//   if (goal_x == x && goal_y == y) {
//     parents[x][y] = {parent_x, parent_y};
//     min_dist = std::min(dist, min_dist);
//     printf("Distance: %d\r\n", min_dist);
//     return true;
//   }

//   // if (dist > min_dist) {
//   //   // v_tbl[y * h + x] = 2;
//   //   return false;
//   // }
//   parents[x][y] = {parent_x, parent_y};

//   int row[] = {-1, 0, 0, 1};
//   int col[] = {0, -1, 1, 0};

//   for (int k = 0; k < 4; k++) {
//     int newX = x + row[k];
//     int newY = y + col[k];

//     // Check if it's possible to move to the new cell
//     if (newX >= 0 && newY < w && newY >= 0 && newY < h &&
//         image->data[newX][newY].red != 0 && v_tbl[newX + newY * h] == 0) {
//       findPath(newX, newY, w, h, goal_x, goal_y, x, y, false, min_dist,
//                dist + 1);
//     }
//   }
//   v_tbl[y * h + x] = 0;
//   return false;
// }

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

// Napi::Value loadMap(const Napi::CallbackInfo &info) {
//   Napi::Env env = info.Env();
//   double occ_th = info[0].As<Napi::Number>().DoubleValue();
//   double free_th = info[1].As<Napi::Number>().DoubleValue();
//   double resolution = info[2].As<Napi::Number>().DoubleValue();

//   if (!image) {
//     printf("Call readPGM first, and make sure the map file is fed.");
//     return Napi::Boolean::New(env, false);
//   }

//   if (!occ) {
//     occ = new OccGridMap();
//     occ->resolution = resolution;
//     occ->width = image->width;
//     occ->height = image->height;
//     int *oc_grid = (int *)malloc(image->width * image->height * sizeof(int));
//     double sum_pix = 0;
//     int count_100 = 0;
//     int count_0 = 0;
//     int count_1 = 0;
//     for (int y = image->height - 1; y >= 0; y--) {
//       for (int x = 0; x < image->width; x++) {
//         int index = MAP_IDX(image->width, x, image->height - y - 1);
//         unsigned char r = image->data[x][y].red;
//         unsigned char g = image->data[x][y].green;
//         unsigned char b = image->data[x][y].blue;
//         double pixel = (r + g + b) / 3.0f;
//         double occ = (255.0 - pixel) / 255.0f;
//         if (occ > occ_th) {
//           pixel = 100;
//           count_100++;
//         } else if (occ < free_th) {
//           pixel = 0;
//           count_0++;
//         } else {
//           // double ratio = (occ - free_th) / (occ_th - free_th);
//           // pixel = 1 + 98 * ratio;
//           pixel = -1;
//           count_1++;
//         }
//         oc_grid[index] = pixel;
//         sum_pix += pixel;
//       }
//     }
//     occ->data = oc_grid;
//     double mean = sum_pix / (image->width * image->height);
//     printf("mean %.5f\r\n", mean);
//     printf("100: %d, 0: %d, -1: %d\r\n", count_100, count_0, count_1);
//     free(oc_grid);
//     // free(image);
//   }

//   Napi::ArrayBuffer buf =
//       Napi::ArrayBuffer::New(env, occ->data, (image->width * image->height));

//   return buf;
// }

Napi::Value navigation(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  // robot_pose_x/y/z is map coordinate
  int robot_pose_x = info[0].As<Napi::Number>();
  int robot_pose_y = info[1].As<Napi::Number>();
  int robot_pose_z = info[2].As<Napi::Number>();
  // target_x/y is the distance from the robot pose
  int target_x = info[3].As<Napi::Number>();
  int target_y = info[4].As<Napi::Number>();

  // if (!occ) {
  //   printf("Call loadMap after readPGM to load the configuration of the
  //   map."); return Napi::Boolean::New(env, false);
  // }

  // int w = occ->width;
  // int h = occ->height;
  int w = image->width;
  int h = image->height;
  if (image->data[target_y][target_x].red < 254) {
    printf("Cannot arrive the goal");
    printf(" data: %d\r\n", image->data[target_y][target_x].red);
    return Napi::Boolean::New(env, false);
  }
  // double resolution = occ->resolution;
  // int target_x_pix = target_x / resolution;
  // int target_y_pix = target_y / resolution;
  // v_tbl = (int *)calloc(w * h, sizeof(int));
  v_tbl.clear();
  v_tbl.resize(w, std::vector<bool>(h));
  parents.clear();
  parents.resize(w, std::vector<std::pair<int, int>>(h));
  // bool res = findPath(robot_pose_x, robot_pose_y, w, h, target_x, target_y,
  // -1,
  //                     -1, false, min_dist, 0);
  bool res = BFS(robot_pose_x, robot_pose_y, target_x, target_y) > 0;
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      if (v_tbl[x][y]) {
        printf(" ");
      } else {
        printf("#");
      }
    }
    printf("\n");
  }
  if (res) {
    std::vector<std::pair<int, int>> path;
    std::pair<int, int> current = {target_x, target_y};
    std::pair<int, int> start = {robot_pose_x, robot_pose_y};
    while (current != start) {
      path.push_back(current);
      current = parents[current.first][current.second];
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    Napi::Array arr = Napi::Array::New(env, path.size());
    int index = 0;
    for (const auto &point : path) {
      // printf("(%d, %d) -> ", point.first, point.second);
      Napi::Object obj = Napi::Object::New(env);
      obj.Set("x", Napi::Number::New(env, point.first));
      obj.Set("y", Napi::Number::New(env, point.second));
      arr[index] = obj;
      index++;
    }

    return arr;
  }
  return Napi::Boolean::New(env, false);
}

Napi::Value freeObjs(const Napi::CallbackInfo &info) {
  printf("Ready to free all resources\r\n");
  if (image) {
    delete image;
    printf("PMGImage is free\r\n");
  }
  // if (occ) {
  //   delete[] occ;
  //   printf("OCC_Grid is free\r\n");
  // }

  // if (v_tbl) {
  //   free(v_tbl);
  // }

  Napi::Env env = info.Env();
  return Napi::Boolean::New(env, true);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "readPGM"),
              Napi::Function::New(env, readPGM));
  // exports.Set(Napi::String::New(env, "loadMap"),
  //             Napi::Function::New(env, loadMap));
  exports.Set(Napi::String::New(env, "freeObjs"),
              Napi::Function::New(env, freeObjs));
  exports.Set(Napi::String::New(env, "navigation"),
              Napi::Function::New(env, navigation));
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init);