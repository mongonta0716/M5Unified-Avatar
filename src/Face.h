// Copyright (c) Shinya Ishikawa. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#ifndef FACE_H_
#define FACE_H_

#include "Balloon.h"
#include "BoundingRect.h"
#include "Eye.h"
#include "Eyeblow.h"
#include "Mouth.h"
#include "Effect.h"
#include "BatteryIcon.h"

namespace m5avatar {

class Face {
 private:
  Drawable *mouth_;
  Drawable *eyeR_;
  Drawable *eyeL_;
  Drawable *eyeblowR_;
  Drawable *eyeblowL_;
  BoundingRect *mouthPos_;
  BoundingRect *eyeRPos_;
  BoundingRect *eyeLPos_;
  BoundingRect *eyeblowRPos_;
  BoundingRect *eyeblowLPos_;
  BoundingRect *boundingRect_;
  M5Canvas *sprite_;
  M5Canvas *tmpSprite_;
  Balloon *b_;
  Effect *h_;
  BatteryIcon *battery_;

 public:
  // constructor
  Face(M5GFX* display = &M5.Display);
  Face(Drawable *mouth, Drawable *eyeR, Drawable *eyeL, Drawable *eyeblowR,
       Drawable *eyeblowL, M5GFX* display);
  // TODO(meganetaaan): apply builder pattern
  Face(Drawable *mouth, BoundingRect *mouthPos, Drawable *eyeR,
       BoundingRect *eyeRPos, Drawable *eyeL, BoundingRect *eyeLPos,
       Drawable *eyeblowR, BoundingRect *eyeblowRPos, Drawable *eyeblowL,
       BoundingRect *eyeblowLPos, M5GFX* display);
  Face(Drawable *mouth, BoundingRect *mouthPos, Drawable *eyeR,
       BoundingRect *eyeRPos, Drawable *eyeL, BoundingRect *eyeLPos,
       Drawable *eyeblowR, BoundingRect *eyeblowRPos, Drawable *eyeblowL,
       BoundingRect *eyeblowLPos,
       BoundingRect *boundingRect, M5Canvas *spr, M5Canvas *tmpSpr);
  ~Face();
  Face(const Face &other) = default;
  Face &operator=(const Face &other) = default;

  Drawable *getLeftEye();
  Drawable *getRightEye();

  // void setParts(PartsType p, Drawable parts);
  // Drawable *getParts(PartsType p);

  Drawable *getMouth();
  BoundingRect *getBoundingRect();
  void setBoundingRect(BoundingRect *rect);

  void setLeftEye(Drawable *eye);
  void setRightEye(Drawable *eye);
  void setMouth(Drawable *mouth);
  void setLeftEyeblow();
  void setRightEyeblow();

  void draw(DrawContext *ctx);
};
}  // namespace m5avatar

#endif  // FACE_H_
