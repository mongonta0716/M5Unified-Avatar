// Copyright (c) Shinya Ishikawa. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#include "Face.h"

#ifndef _min
#define _min(a, b) std::min(a, b)
#endif

namespace m5avatar {
BoundingRect br;

Face::Face(M5GFX* display)
    : Face(new Mouth(50, 90, 4, 60), new BoundingRect(148, 163),
           new Eye(8, false), new BoundingRect(93, 90), new Eye(8, true),
           new BoundingRect(96, 230), new Eyeblow(32, 0, false),
           new BoundingRect(67, 96), new Eyeblow(32, 0, true),
           new BoundingRect(72, 230), display) {}

Face::Face(Drawable *mouth, Drawable *eyeR, Drawable *eyeL, Drawable *eyeblowR,
           Drawable *eyeblowL, M5GFX* display)
    : Face(mouth, new BoundingRect(148, 163), eyeR, new BoundingRect(93, 90),
           eyeL, new BoundingRect(96, 230), eyeblowR, new BoundingRect(67, 96),
           eyeblowL, new BoundingRect(72, 230), display) {}

Face::Face(Drawable *mouth, BoundingRect *mouthPos, Drawable *eyeR,
           BoundingRect *eyeRPos, Drawable *eyeL, BoundingRect *eyeLPos,
           Drawable *eyeblowR, BoundingRect *eyeblowRPos, Drawable *eyeblowL,
           BoundingRect *eyeblowLPos, M5GFX* display)
    : Face(mouth, mouthPos, eyeR, eyeRPos, eyeL, eyeLPos, eyeblowR,
           eyeblowRPos, eyeblowL, eyeblowLPos,
           new BoundingRect(0, 0, 320, 240),
           new M5Canvas(display), new M5Canvas(display), display) {}

Face::Face(Drawable *mouth, BoundingRect *mouthPos, Drawable *eyeR,
       BoundingRect *eyeRPos, Drawable *eyeL, BoundingRect *eyeLPos,
       Drawable *eyeblowR, BoundingRect *eyeblowRPos, Drawable *eyeblowL,
       BoundingRect *eyeblowLPos,
       BoundingRect *boundingRect, M5Canvas *spr, M5Canvas *tmpSpr, M5GFX* display)
    : mouth(mouth),
      eyeR(eyeR),
      eyeL(eyeL),
      eyeblowR(eyeblowR),
      eyeblowL(eyeblowL),
      mouthPos(mouthPos),
      eyeRPos(eyeRPos),
      eyeLPos(eyeLPos),
      eyeblowRPos(eyeblowRPos),
      eyeblowLPos(eyeblowLPos),
      boundingRect(boundingRect),
      sprite(spr),
      tmpSprite(tmpSpr),
      b(new Balloon()),
      h(new Effect()),
      battery(new BatteryIcon()),
      display(display) {}

Face::~Face() {
  delete mouth;
  delete mouthPos;
  delete eyeR;
  delete eyeRPos;
  delete eyeL;
  delete eyeLPos;
  delete eyeblowR;
  delete eyeblowRPos;
  delete eyeblowL;
  delete eyeblowLPos;
  delete sprite;
  delete boundingRect;
  delete b;
  delete h;
  delete battery;
}

void Face::setMouth(Drawable *mouth) { this->mouth = mouth; }

void Face::setLeftEye(Drawable *eyeL) { this->eyeL = eyeL; }

void Face::setRightEye(Drawable *eyeR) { this->eyeR = eyeR; }

Drawable *Face::getMouth() { return mouth; }

Drawable *Face::getLeftEye() { return eyeL; }

Drawable *Face::getRightEye() { return eyeR; }

BoundingRect *Face::getBoundingRect() { return boundingRect; }

void Face::draw(DrawContext *ctx) {
  // Defensive: check all pointers before use
  if (!sprite || !tmpSprite || !boundingRect || !display) {
    Serial.println("Error: Null pointer in Face::draw (sprite/tmpSprite/boundingRect/display)");
    return;
  }
  int w = boundingRect->getWidth();
  int h = boundingRect->getHeight();
  if (w <= 0 || h <= 0) {
    Serial.printf("Error: Sprite size is zero or negative. w=%d, h=%d\n", w, h);
    return;
  }
  sprite->setPsram(true);
  sprite->createSprite(w, h);
  if (!sprite->getBuffer()) {
    Serial.println("Error: Sprite buffer allocation failed.");
    return;
  }
  // Already checked w and h above
  sprite->setColorDepth(ctx->getColorDepth());
  // NOTE: setting below for 1-bit color depth
  sprite->setBitmapColor(ctx->getColorPalette()->get(COLOR_PRIMARY),
    ctx->getColorPalette()->get(COLOR_BACKGROUND));
  if (ctx->getColorDepth() != 1) {
    sprite->fillSprite(ctx->getColorPalette()->get(COLOR_BACKGROUND));
  } else {
    sprite->fillSprite(0);
  }
  float breath = _min(1.0f, ctx->getBreath());

  // Defensive: check center coordinates for pushRotateZoom
  int cx = w >> 1;
  int cy = h >> 1;
  if (cx <= 0 || cy <= 0) {
    Serial.printf("Error: Center coordinates are invalid: cx=%d, cy=%d\n", cx, cy);
    sprite->deleteSprite();
    return;
  }

  // TODO(meganetaaan): unify drawing process of each parts
  BoundingRect rect = *mouthPos;
  rect.setPosition(rect.getTop() + breath * 3, rect.getLeft());
  // copy context to each draw function
  mouth->draw(sprite, rect, ctx);

  rect = *eyeRPos;
  rect.setPosition(rect.getTop() + breath * 3, rect.getLeft());
  eyeR->draw(sprite, rect, ctx);

  rect = *eyeLPos;
  rect.setPosition(rect.getTop() + breath * 3, rect.getLeft());
  eyeL->draw(sprite, rect, ctx);

  rect = *eyeblowRPos;
  rect.setPosition(rect.getTop() + breath * 3, rect.getLeft());
  eyeblowR->draw(sprite, rect, ctx);

  rect = *eyeblowLPos;
  rect.setPosition(rect.getTop() + breath * 3, rect.getLeft());
  eyeblowL->draw(sprite, rect, ctx);

  // TODO(meganetaaan): make balloons and effects selectable
  b->draw(sprite, br, ctx);
  battery->draw(sprite, br, ctx);
  // drawAccessory(sprite, position, ctx);

  // TODO(meganetaaan): rethink responsibility for transform function
  float scale = ctx->getScale();
  float rotation = boundingRect ? boundingRect->getRotation() : 0.0f;

// ▼▼▼▼ここから▼▼▼▼
  static constexpr uint8_t y_step = 8;

  // Get the display from the sprite
  //M5GFX* display = (M5GFX*)sprite->getParent();

  if (tmpSprite->getBuffer() == nullptr) {
    tmpSprite->setPsram(true);
    // 出力先と同じcolorDepthを指定することで、DMA転送が可能になる。
    // Display自体は16bit or 24bitしか指定できないが、細長なので1bitではなくても大丈夫。
    tmpSprite->setColorDepth(display->getColorDepth());

    // 確保するメモリは高さ8ピクセルの横長の細長い短冊状とする。
    tmpSprite->createSprite(boundingRect->getWidth(), y_step);
  }

  // 背景クリア用の色を設定
  tmpSprite->setBaseColor(ctx->getColorPalette()->get(COLOR_BACKGROUND));
  int y = 0;
  do {
    // 背景色で塗り潰し
    tmpSprite->clear();

    // 傾きとズームを反映してspriteからtmpSpriteに転写
    float avatarRotation = ctx ? ctx->getRotation() : 0.0f;
    sprite->pushRotateZoom(tmpSprite, boundingRect->getWidth()>>1, (boundingRect->getHeight()>>1) - y, avatarRotation, scale, scale);

    // tmpSpriteから画面に転写
    display->startWrite();

    // 事前にstartWriteしておくことで、pushSprite はDMA転送を開始するとすぐに処理を終えて戻ってくる。
    tmpSprite->pushSprite(display, boundingRect->getLeft(), boundingRect->getTop() + y);

    // DMA転送中にdelay処理を設けることにより、DMA転送中に他のタスクへCPU処理時間を譲ることができる。
    lgfx::delay(1);

    // endWriteによってDMA転送の終了を待つ。
    display->endWrite();

  } while ((y += y_step) < boundingRect->getHeight());

// 削除するのが良いかどうか要検討 (次回メモリ確保できない場合は描画できなくなるので、維持しておいても良いかも？)
// tmpSprite->deleteSprite();
// ▲▲▲▲ここまで▲▲▲▲

  sprite->deleteSprite();
}
}  // namespace m5avatar