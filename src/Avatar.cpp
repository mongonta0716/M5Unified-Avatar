// Copyright (c) Shinya Ishikawa. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#include "Avatar.h"

#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

namespace m5avatar {

Avatar::Avatar(Face *face, M5GFX* display, int boundingRectWidth, int boundingRectHeight)
    : face(face),
      _isDrawing(false),
      expression(Expression::Neutral),
      breath(0.0f),
      rightEyeOpenRatio_(1.0f),
      rightGazeV_(0.0f),
      rightGazeH_(0.0f),
      leftEyeOpenRatio_(1.0f),
      leftGazeV_(0.0f),
      leftGazeH_(0.0f),
      isAutoBlink_(true),
      mouthOpenRatio(0.0f),
      rotation(0.0f),
      scale(1.0f),
      palette(),
      speechText(""),
      colorDepth(1),
      batteryIconStatus(BatteryIconStatus::invisible),
      batteryLevel(0),
      speechFont(nullptr)
{
  // display, boundingRectWidth, boundingRectHeightはface生成時に使われているのでここでは特に処理不要
}


unsigned int seed = 0;

#ifndef rand_r
#define init_rand() srand(seed)
#define _rand() rand()
#else
#define init_rand() ;
#define _rand() rand_r(&seed)
#endif

#ifdef SDL_h_
#define TaskResult() return 0
#define TaskDelay(ms) lgfx::delay(ms)
long random(long howbig) { return std::rand() % howbig; }
#else
#define TaskResult() vTaskDelete(NULL)
#define TaskDelay(ms) vTaskDelay(ms / portTICK_PERIOD_MS)
#endif

// TODO(meganetaaan): make read-only
DriveContext::DriveContext(Avatar *avatar) : avatar{avatar} {}

Avatar *DriveContext::getAvatar() { return avatar; }

TaskHandle_t drawTaskHandle;

TaskResult_t drawLoop(void *args) {
  DriveContext *ctx = reinterpret_cast<DriveContext *>(args);
  Avatar *avatar = ctx->getAvatar();
  // update drawings in the display
  while (avatar->isDrawing()) {
    if (avatar->isDrawing()) {
      avatar->draw();
    }
    TaskDelay(10);
  }
  TaskResult();
}

TaskResult_t facialLoop(void *args) {
  int count = 0;
  DriveContext *ctx = reinterpret_cast<DriveContext *>(args);
  Avatar *avatar = ctx->getAvatar();
  uint32_t saccade_interval = 1000;
  uint32_t blink_interval = 1000;
  unsigned long last_saccade_millis = 0;
  unsigned long last_blink_millis = 0;
  bool eye_open = true;
  float vertical = 0.0f;
  float horizontal = 0.0f;
  float breath = 0.0f;
  init_rand();
  // update facial internal state
  while (avatar->isDrawing()) {
    if ((lgfx::millis() - last_saccade_millis) > saccade_interval) {
      vertical = _rand() / (RAND_MAX / 2.0) - 1;
      horizontal = _rand() / (RAND_MAX / 2.0) - 1;
      avatar->setRightGaze(vertical, horizontal);
      avatar->setLeftGaze(vertical, horizontal);
      saccade_interval = 500 + 100 * random(20);
      last_saccade_millis = lgfx::millis();
    }

    if (avatar->getIsAutoBlink()) {
      if ((lgfx::millis() - last_blink_millis) > blink_interval) {
        if (eye_open) {
          avatar->setEyeOpenRatio(1.0f);
          blink_interval = 2500 + 100 * random(20);
        } else {
          avatar->setEyeOpenRatio(0.0f);
          blink_interval = 300 + 10 * random(20);
        }
        eye_open = !eye_open;
        last_blink_millis = lgfx::millis();
      }
    }

    count = (count + 1) % 100;
    breath = sin(count * 2 * PI / 100.0);
    avatar->setBreath(breath);
    TaskDelay(33);  // approx. 30fps
  }
  TaskResult();
}

Avatar::Avatar(int boundingRectWidth, int boundingRectHeight)
    : Avatar(&M5.Display, boundingRectWidth, boundingRectHeight) {}

Avatar::Avatar(M5GFX* display, int boundingRectWidth, int boundingRectHeight)
    : Avatar(new Face(
        // --- サイズスケーリング ---
        [=]() {
          const int BASE_WIDTH = 320, BASE_HEIGHT = 240;
          float scaleX = boundingRectWidth / float(BASE_WIDTH);
          float scaleY = boundingRectHeight / float(BASE_HEIGHT);
          uint16_t mouthMinWidth  = uint16_t(50 * scaleX);
          uint16_t mouthMaxWidth  = uint16_t(90 * scaleX);
          uint16_t mouthMinHeight = uint16_t(4 * scaleY);
          uint16_t mouthMaxHeight = uint16_t(60 * scaleY);
          return new Mouth(mouthMinWidth, mouthMaxWidth, mouthMinHeight, mouthMaxHeight);
        }(),
        // --- 位置スケーリング ---
        [=]() {
          const int BASE_WIDTH = 320, BASE_HEIGHT = 240;
          float scaleX = boundingRectWidth / float(BASE_WIDTH);
          float scaleY = boundingRectHeight / float(BASE_HEIGHT);
          int mouthTop = int(148 * scaleY);
          int mouthLeft = int(163 * scaleX);
          return new BoundingRect(mouthTop, mouthLeft);
        }(),
        [=]() {
          const int BASE_WIDTH = 320, BASE_HEIGHT = 240;
          float scaleX = boundingRectWidth / float(BASE_WIDTH);
          uint16_t eyeRadius = uint16_t(8 * scaleX);
          return new Eye(eyeRadius, false);
        }(),
        [=]() {
          const int BASE_WIDTH = 320, BASE_HEIGHT = 240;
          float scaleX = boundingRectWidth / float(BASE_WIDTH);
          float scaleY = boundingRectHeight / float(BASE_HEIGHT);
          int eyeRTop = int(93 * scaleY);
          int eyeRLeft = int(90 * scaleX);
          return new BoundingRect(eyeRTop, eyeRLeft);
        }(),
        [=]() {
          const int BASE_WIDTH = 320, BASE_HEIGHT = 240;
          float scaleX = boundingRectWidth / float(BASE_WIDTH);
          uint16_t eyeRadius = uint16_t(8 * scaleX);
          return new Eye(eyeRadius, true);
        }(),
        [=]() {
          const int BASE_WIDTH = 320, BASE_HEIGHT = 240;
          float scaleX = boundingRectWidth / float(BASE_WIDTH);
          float scaleY = boundingRectHeight / float(BASE_HEIGHT);
          int eyeLTop = int(96 * scaleY);
          int eyeLLeft = int(230 * scaleX);
          return new BoundingRect(eyeLTop, eyeLLeft);
        }(),
        [=]() {
          const int BASE_WIDTH = 320, BASE_HEIGHT = 240;
          float scaleX = boundingRectWidth / float(BASE_WIDTH);
          float scaleY = boundingRectHeight / float(BASE_HEIGHT);
          uint16_t eyeblowWidth  = uint16_t(32 * scaleX);
          uint16_t eyeblowHeight = uint16_t(4 * scaleY);
          return new Eyeblow(eyeblowWidth, eyeblowHeight, false);
        }(),
        [=]() {
          const int BASE_WIDTH = 320, BASE_HEIGHT = 240;
          float scaleX = boundingRectWidth / float(BASE_WIDTH);
          float scaleY = boundingRectHeight / float(BASE_HEIGHT);
          int eyeblowRTop = int(67 * scaleY);
          int eyeblowRLeft = int(96 * scaleX);
          return new BoundingRect(eyeblowRTop, eyeblowRLeft);
        }(),
        [=]() {
          const int BASE_WIDTH = 320, BASE_HEIGHT = 240;
          float scaleX = boundingRectWidth / float(BASE_WIDTH);
          float scaleY = boundingRectHeight / float(BASE_HEIGHT);
          uint16_t eyeblowWidth  = uint16_t(32 * scaleX);
          uint16_t eyeblowHeight = uint16_t(4 * scaleY);
          return new Eyeblow(eyeblowWidth, eyeblowHeight, true);
        }(),
        [=]() {
          const int BASE_WIDTH = 320, BASE_HEIGHT = 240;
          float scaleX = boundingRectWidth / float(BASE_WIDTH);
          float scaleY = boundingRectHeight / float(BASE_HEIGHT);
          int eyeblowLTop = int(72 * scaleY);
          int eyeblowLLeft = int(230 * scaleX);
          return new BoundingRect(eyeblowLTop, eyeblowLLeft);
        }(),
        new BoundingRect(0, 0, boundingRectWidth, boundingRectHeight),
        new M5Canvas(display),
        new M5Canvas(display),
        display),
      display, boundingRectWidth, boundingRectHeight) {}

Avatar::Avatar() : Avatar(&M5.Display, 320, 240) {}

Avatar::Avatar(M5GFX* display) : Avatar(display, 320, 240) {}

Avatar::Avatar(Face *face) : Avatar(face, &M5.Display) {}

Avatar::Avatar(Face *face, M5GFX* display)
    : face{face},
      _isDrawing{false},
      expression{Expression::Neutral},
      breath{0},
      leftEyeOpenRatio_{1.0f},
      leftGazeH_{1.0f},
      leftGazeV_{1.0f},
      rightEyeOpenRatio_{1.0f},
      rightGazeH_{1.0f},
      rightGazeV_{1.0f},
      isAutoBlink_{true},
      mouthOpenRatio{0},
      rotation{0.0f},
      scale{1},
      palette{ColorPalette()},
      speechText{""},
      colorDepth{1},
      batteryIconStatus{BatteryIconStatus::invisible},
      batteryLevel{0},
      speechFont{nullptr} {
  // 念のため明示的に初期化
  rotation = 0.0f;
}

Avatar::~Avatar() { delete face; }

void Avatar::setFace(Face *newFace) {
  if (face != newFace) {
    delete face; // 古いfaceを解放
    face = newFace;
  }
}

Face *Avatar::getFace() const { return face; }

void Avatar::addTask(TaskFunction_t f, const char *name,
                     const uint32_t stack_size, UBaseType_t priority,
                     TaskHandle_t *const task_handle,
                     const BaseType_t core_id) {
  DriveContext *ctx = new DriveContext(this);
#ifdef SDL_h_
  if (task_handle == NULL) {
    SDL_CreateThreadWithStackSize(f, name, stack_size, ctx);
  } else {
    *task_handle = SDL_CreateThreadWithStackSize(f, name, stack_size, ctx);
  }
#else
  // TODO(meganetaaan): set a task handler
  xTaskCreateUniversal(f,           /* Function to implement the task */
                       name,        /* Name of the task */
                       stack_size,  /* Stack size in words */
                       ctx,         /* Task input parameter */
                       priority,    /* Priority of the task */
                       task_handle, /* Task handle. */
                       core_id);    /* Core No*/
#endif
}

void Avatar::init(int colorDepth) {
  // for compatibility with older version
  start(colorDepth);
}

void Avatar::stop() { _isDrawing = false; }

void Avatar::suspend() {
#ifndef SDL_h_
  vTaskSuspend(drawTaskHandle);
#endif
}

void Avatar::resume() {
#ifndef SDL_h_
  vTaskResume(drawTaskHandle);
#endif
}

void Avatar::start(int colorDepth) {
  // if the task already started, don't create another task;
  if (_isDrawing) return;
  _isDrawing = true;

  this->colorDepth = colorDepth;
  DriveContext *ctx = new DriveContext(this);
#ifdef SDL_h_
  drawTaskHandle =
      SDL_CreateThreadWithStackSize(drawLoop, "drawLoop", 2048, ctx);
  SDL_CreateThreadWithStackSize(facialLoop, "facialLoop", 1024, ctx);
#else
  // TODO(meganetaaan): keep handle of these tasks
  xTaskCreateUniversal(drawLoop,        /* Function to implement the task */
                       "drawLoop",      /* Name of the task */
                       8192,            /* Stack size in words */
                       ctx,             /* Task input parameter */
                       1,               /* Priority of the task */
                       &drawTaskHandle, /* Task handle. */
                       APP_CPU_NUM);

  xTaskCreateUniversal(facialLoop,   /* Function to implement the task */
                       "facialLoop", /* Name of the task */
                       1024,         /* Stack size in words */
                       ctx,          /* Task input parameter */
                       2,            /* Priority of the task */
                       NULL,         /* Task handle. */
                       APP_CPU_NUM);
#endif
}

void Avatar::draw() {
  Gaze rightGaze = Gaze(this->rightGazeV_, this->rightGazeV_);
  Gaze leftGaze = Gaze(this->leftGazeV_, this->leftGazeH_);
  DrawContext *ctx = new DrawContext(
      this->expression, this->breath, &this->palette, rightGaze,
      this->rightEyeOpenRatio_, leftGaze, this->leftEyeOpenRatio_,
      this->mouthOpenRatio, this->speechText, this->rotation, this->scale,
      this->colorDepth, this->batteryIconStatus, this->batteryLevel,
      this->speechFont, this->face ? this->face->getDisplay() : nullptr);
  face->draw(ctx);
  delete ctx;
}

bool Avatar::isDrawing() { return _isDrawing; }

void Avatar::setExpression(Expression expression) {
  suspend();
  this->expression = expression;
  resume();
}

Expression Avatar::getExpression() { return this->expression; }

void Avatar::setBreath(float breath) { this->breath = breath; }

float Avatar::getBreath() { return this->breath; }

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
void Avatar::setRotation(float degree) {
  this->rotation = degree;
  Face* f = this->getFace();
  if (!f) return;
  BoundingRect* rect = f->getBoundingRect();
  if (!rect) return;
  rect->setRotation(this->rotation);
}


float Avatar::getRotation() const { return this->rotation; }

void Avatar::setScale(float scale) { this->scale = scale; }

void Avatar::setPosition(int top, int left) {
  this->getFace()->getBoundingRect()->setPosition(top, left);
}

void Avatar::setColorPalette(ColorPalette cp) { palette = cp; }

ColorPalette Avatar::getColorPalette(void) const { return this->palette; }

void Avatar::setMouthOpenRatio(float ratio) { this->mouthOpenRatio = ratio; }

void Avatar::setEyeOpenRatio(float ratio) {
  setRightEyeOpenRatio(ratio);
  setLeftEyeOpenRatio(ratio);
}

void Avatar::setLeftEyeOpenRatio(float ratio) {
  this->leftEyeOpenRatio_ = ratio;
}

float Avatar::getLeftEyeOpenRatio() { return this->leftEyeOpenRatio_; }

void Avatar::setRightEyeOpenRatio(float ratio) {
  this->rightEyeOpenRatio_ = ratio;
}

float Avatar::getRightEyeOpenRatio() { return this->rightEyeOpenRatio_; }

void Avatar::setIsAutoBlink(bool b) { this->isAutoBlink_ = b; }

bool Avatar::getIsAutoBlink() { return this->isAutoBlink_; }

void Avatar::setRightGaze(float vertical, float horizontal) {
  this->rightGazeV_ = vertical;
  this->rightGazeH_ = horizontal;
}

void Avatar::getRightGaze(float *vertical, float *horizontal) {
  *vertical = this->rightGazeV_;
  *horizontal = this->rightGazeH_;
}

void Avatar::setLeftGaze(float vertical, float horizontal) {
  this->leftGazeV_ = vertical;
  this->leftGazeH_ = horizontal;
}

void Avatar::getLeftGaze(float *vertical, float *horizontal) {
  *vertical = this->leftGazeV_;
  *horizontal = this->leftGazeH_;
}

void Avatar::getGaze(float *vertical, float *horizontal){
  *vertical = 0.5f * this->leftGazeV_ + 0.5f * this->rightGazeV_;
  *horizontal = 0.5f * this->leftGazeH_ + 0.5f * this->rightGazeH_;
}

void Avatar::setSpeechText(const char *speechText) {
  this->speechText = String(speechText);
}

void Avatar::setSpeechFont(const lgfx::IFont *speechFont) {
  this->speechFont = speechFont;
}

void Avatar::setBatteryIcon(bool batteryIcon) {
  if (!batteryIcon) {
    batteryIconStatus = BatteryIconStatus::invisible;
  } else {
    batteryIconStatus = BatteryIconStatus::unknown;
  }
}

void Avatar::setBatteryStatus(bool isCharging, int32_t batteryLevel) {
  if (this->batteryIconStatus != BatteryIconStatus::invisible) {
    if (isCharging) {
      this->batteryIconStatus = BatteryIconStatus::charging;
    } else {
      this->batteryIconStatus = BatteryIconStatus::discharging;
    }
    this->batteryLevel = batteryLevel;
  }
}

}  // namespace m5avatar
