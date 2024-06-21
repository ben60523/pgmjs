#include <napi.h>

#include <vector>

#include "js_native_api.h"
#include "napi-inl.h"

extern "C" {
#include "pgm.h"
}

int **popData(int **data) {}

Napi::Value readPGM(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  std::string fPath = info[0].As<Napi::String>();
  PGMImage *image = new PGMImage();
  if (fPath.length() > 0) {
    const char *fPathChar = fPath.c_str();
    getPGMfile(fPathChar, image);
  }

  int w = image->width;
  int h = image->height;
  int maxVal = image->maxVal;
  int data[w][h];
  Napi::Int32Array buf = Napi::Int32Array::New(env, w * h);

  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      buf[i * w + j] = image->data[i][j].red;
    }
  }

  // save(image);

  free(image);

  return buf;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "readPGM"),
              Napi::Function::New(env, readPGM));
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init);