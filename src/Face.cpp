// Copyright (c) Shinya Ishikawa. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#include "Face.h"

#ifndef _min
#define _min(a, b) std::min(a, b)
#endif

//#define DISPLAY_WIDTH 1280
//#define DISPLAY_HEIGH
#define DISPLAY_WIDTH  720 
#define DISPLAY_HEIGHT 1280

namespace m5avatar {
BoundingRect br;

Face::Face(M5GFX* display)
{
    // Calculate positions based on display dimensions
    int w = DISPLAY_WIDTH; //720; //display->width();
    int h = DISPLAY_HEIGHT;  //display->height();
    float scaleX = w / 320.0f;
    float scaleY = h / 240.0f;
    
    // Scale mouth dimensions relative to screen size
    uint16_t mouthMinWidth = static_cast<uint16_t>(50 * scaleX);
    uint16_t mouthMaxWidth = static_cast<uint16_t>(90 * scaleX);
    uint16_t mouthMinHeight = static_cast<uint16_t>(4 * scaleY);
    uint16_t mouthMaxHeight = static_cast<uint16_t>(60 * scaleY);
    
    // Scale eye radius relative to screen size
    uint16_t eyeRadius = static_cast<uint16_t>(8 * scaleX);
    
    // Scale eyeblow dimensions relative to screen size
    uint16_t eyeblowWidth = static_cast<uint16_t>(32 * scaleX);
    uint16_t eyeblowHeight = static_cast<uint16_t>(4 * scaleY);
    
    // Create face with scaled dimensions for all components
    *this = Face(new Mouth(mouthMinWidth, mouthMaxWidth, mouthMinHeight, mouthMaxHeight), 
                 new Eye(eyeRadius, false), new Eye(eyeRadius, true),
                 new Eyeblow(eyeblowWidth, eyeblowHeight, false), 
                 new Eyeblow(eyeblowWidth, eyeblowHeight, true), 
                 display);
}

Face::Face(Drawable *mouth, Drawable *eyeR, Drawable *eyeL, Drawable *eyeblowR,
           Drawable *eyeblowL, M5GFX* display)
{
    // Calculate positions based on display dimensions
    int w = DISPLAY_WIDTH; //display->width();
    int h = DISPLAY_HEIGHT;  //display->height();
    float scaleX = w / 320.0f;
    float scaleY = h / 240.0f;
    
    // Calculate scaled positions
    int mouthTop = static_cast<int>(148 * scaleY);
    int mouthLeft = static_cast<int>(163 * scaleX);
    int eyeRTop = static_cast<int>(93 * scaleY);
    int eyeRLeft = static_cast<int>(90 * scaleX);
    int eyeLTop = static_cast<int>(96 * scaleY);
    int eyeLLeft = static_cast<int>(230 * scaleX);
    int eyeblowRTop = static_cast<int>(67 * scaleY);
    int eyeblowRLeft = static_cast<int>(96 * scaleX);
    int eyeblowLTop = static_cast<int>(72 * scaleY);
    int eyeblowLLeft = static_cast<int>(230 * scaleX);
    
    // Use delegation to the next constructor instead of assignment
    mouth_ = mouth;
    eyeR_ = eyeR;
    eyeL_ = eyeL;
    eyeblowR_ = eyeblowR;
    eyeblowL_ = eyeblowL;
    mouthPos_ = new BoundingRect(mouthTop, mouthLeft);
    eyeRPos_ = new BoundingRect(eyeRTop, eyeRLeft);
    eyeLPos_ = new BoundingRect(eyeLTop, eyeLLeft);
    eyeblowRPos_ = new BoundingRect(eyeblowRTop, eyeblowRLeft);
    eyeblowLPos_ = new BoundingRect(eyeblowLTop, eyeblowLLeft);
    boundingRect_ = new BoundingRect(0, 0, w, h);
    sprite_ = new M5Canvas(display);
    tmpSprite_ = new M5Canvas(display);
    b_ = new Balloon();
    h_ = new Effect();
    battery_ = new BatteryIcon();
}

Face::Face(Drawable *mouth, BoundingRect *mouthPos, Drawable *eyeR,
           BoundingRect *eyeRPos, Drawable *eyeL, BoundingRect *eyeLPos,
           Drawable *eyeblowR, BoundingRect *eyeblowRPos, Drawable *eyeblowL,
           BoundingRect *eyeblowLPos, M5GFX* display)
{
    int w = DISPLAY_WIDTH; //display->width();
    int h = DISPLAY_HEIGHT;  //display->height();
    
    mouth_ = mouth;
    eyeR_ = eyeR;
    eyeL_ = eyeL;
    eyeblowR_ = eyeblowR;
    eyeblowL_ = eyeblowL;
    mouthPos_ = mouthPos;
    eyeRPos_ = eyeRPos;
    eyeLPos_ = eyeLPos;
    eyeblowRPos_ = eyeblowRPos;
    eyeblowLPos_ = eyeblowLPos;
    boundingRect_ = new BoundingRect(0, 0, w, h);
    sprite_ = new M5Canvas(display);
    tmpSprite_ = new M5Canvas(display);
    b_ = new Balloon();
    h_ = new Effect();
    battery_ = new BatteryIcon();
}

Face::Face(Drawable *mouth, BoundingRect *mouthPos, Drawable *eyeR,
       BoundingRect *eyeRPos, Drawable *eyeL, BoundingRect *eyeLPos,
       Drawable *eyeblowR, BoundingRect *eyeblowRPos, Drawable *eyeblowL,
       BoundingRect *eyeblowLPos,
       BoundingRect *boundingRect, M5Canvas *spr, M5Canvas *tmpSpr)
    : mouth_(mouth),
      eyeR_(eyeR),
      eyeL_(eyeL),
      eyeblowR_(eyeblowR),
      eyeblowL_(eyeblowL),
      mouthPos_(mouthPos),
      eyeRPos_(eyeRPos),
      eyeLPos_(eyeLPos),
      eyeblowRPos_(eyeblowRPos),
      eyeblowLPos_(eyeblowLPos),
      boundingRect_(boundingRect),
      sprite_(spr),
      tmpSprite_(tmpSpr),
      b_(new Balloon()),
      h_(new Effect()),
      battery_(new BatteryIcon()) {}

Face::~Face() {
  delete mouth_;
  delete mouthPos_;
  delete eyeR_;
  delete eyeRPos_;
  delete eyeL_;
  delete eyeLPos_;
  delete eyeblowR_;
  delete eyeblowRPos_;
  delete eyeblowL_;
  delete eyeblowLPos_;
  delete sprite_;
  delete tmpSprite_;
  delete boundingRect_;
  delete b_;
  delete h_;
  delete battery_;
}

void Face::setMouth(Drawable *mouth) { this->mouth_ = mouth; }

void Face::setLeftEye(Drawable *eyeL) { this->eyeL_ = eyeL; }

void Face::setRightEye(Drawable *eyeR) { this->eyeR_ = eyeR; }

Drawable *Face::getMouth() { return mouth_; }

Drawable *Face::getLeftEye() { return eyeL_; }

Drawable *Face::getRightEye() { return eyeR_; }

BoundingRect *Face::getBoundingRect() { return boundingRect_; }

void Face::setBoundingRect(BoundingRect *rect) { 
  if (boundingRect_ != rect) {
    // Only replace if it's a different object to avoid self-deletion
    boundingRect_ = rect;
  }
}

void Face::draw(DrawContext *ctx) {
  // Use the larger dimension to create a square canvas to ensure enough space when rotated
  int maxDimension = std::max(boundingRect_->getWidth(), boundingRect_->getHeight());
  sprite_->createSprite(maxDimension, maxDimension);
  sprite_->setColorDepth(ctx->getColorDepth());
  // NOTE: setting below for 1-bit color depth
  sprite_->setBitmapColor(ctx->getColorPalette()->get(COLOR_PRIMARY),
    ctx->getColorPalette()->get(COLOR_BACKGROUND));
  if (ctx->getColorDepth() != 1) {
    sprite_->fillSprite(ctx->getColorPalette()->get(COLOR_BACKGROUND));
  } else {
    sprite_->fillSprite(0);
  }
  float breath = _min(1.0f, ctx->getBreath());

  // TODO(meganetaaan): unify drawing process of each parts
  BoundingRect rect = *mouthPos_;
  rect.setPosition(rect.getTop() + breath * 3, rect.getLeft());
  // copy context to each draw function
  mouth_->draw(sprite_, rect, ctx);

  rect = *eyeRPos_;
  rect.setPosition(rect.getTop() + breath * 3, rect.getLeft());
  eyeR_->draw(sprite_, rect, ctx);

  rect = *eyeLPos_;
  rect.setPosition(rect.getTop() + breath * 3, rect.getLeft());
  eyeL_->draw(sprite_, rect, ctx);

  rect = *eyeblowRPos_;
  rect.setPosition(rect.getTop() + breath * 3, rect.getLeft());
  eyeblowR_->draw(sprite_, rect, ctx);

  rect = *eyeblowLPos_;
  rect.setPosition(rect.getTop() + breath * 3, rect.getLeft());
  eyeblowL_->draw(sprite_, rect, ctx);

  // TODO(meganetaaan): make balloons and effects selectable
  b_->draw(sprite_, br, ctx);
  h_->draw(sprite_, br, ctx);
  battery_->draw(sprite_, br, ctx);
  // drawAccessory(sprite, position, ctx);

  // TODO(meganetaaan): rethink responsibility for transform function
  float scale = ctx->getScale();
  float rotation = ctx->getRotation();

// ▼▼▼▼ここから▼▼▼▼
  static constexpr uint8_t y_step = 8;

  // Get the display from the sprite
  M5GFX* display = (M5GFX*)sprite_->getParent();

  if (tmpSprite_->getBuffer() == nullptr) {
    // 出力先と同じcolorDepthを指定することで、DMA転送が可能になる。
    // Display自体は16bit or 24bitしか指定できないが、細長なので1bitではなくても大丈夫。
    tmpSprite_->setColorDepth(display->getColorDepth());

    // 確保するメモリは高さ8ピクセルの横長の細長い短冊状とする。
    // Use the same maxDimension for width to ensure enough space when rotated
    tmpSprite_->createSprite(maxDimension, y_step);
  }

  // 背景クリア用の色を設定
  tmpSprite_->setBaseColor(ctx->getColorPalette()->get(COLOR_BACKGROUND));
  int y = 0;
  do {
    // 背景色で塗り潰し
    tmpSprite_->clear();

    // 傾きとズームを反映してspriteからtmpSpriteに転写
    // Use maxDimension/2 as the center of rotation to avoid memory access issues
    sprite_->pushRotateZoom(tmpSprite_, maxDimension>>1, (maxDimension>>1) - y, rotation, scale, scale);

    // tmpSpriteから画面に転写
    display->startWrite();

    // 事前にstartWriteしておくことで、pushSprite はDMA転送を開始するとすぐに処理を終えて戻ってくる。
    // Calculate offsets to center the content in the square canvas
    int offsetX = boundingRect_->getLeft() + (maxDimension - boundingRect_->getWidth()) / 2;
    int offsetY = boundingRect_->getTop() + (maxDimension - boundingRect_->getHeight()) / 2 + y;
    tmpSprite_->pushSprite(display, offsetX, offsetY);

    // DMA転送中にdelay処理を設けることにより、DMA転送中に他のタスクへCPU処理時間を譲ることができる。
    lgfx::delay(1);

    // endWriteによってDMA転送の終了を待つ。
    display->endWrite();

  } while ((y += y_step) < boundingRect_->getHeight());

// 削除するのが良いかどうか要検討 (次回メモリ確保できない場合は描画できなくなるので、維持しておいても良いかも？)
// tmpSprite_->deleteSprite();
// ▲▲▲▲ここまで▲▲▲▲

  sprite_->deleteSprite();
}
}  // namespace m5avatar