#ifndef SBUS_H
#define SBUS_H
#include <stdint.h>

struct SbusFrame {
  uint16_t channels[16];
  bool lost;
  bool failsafe;
};

class SbusParser {
 public:
  bool push(uint8_t byte, SbusFrame &frame) {
    if (size_ == 0 && byte != 0x0F) return false;
    bytes_[size_++] = byte;
    if (size_ != 25) return false;
    if (bytes_[24] != 0x00 || (bytes_[23] & 0xF0) != 0) {
      // Slide to the next header after a dropped/extra byte.
      uint8_t next = 1;
      while (next < 25 && bytes_[next] != 0x0F) ++next;
      size_ = 25 - next;
      for (uint8_t i = 0; i < size_; ++i) bytes_[i] = bytes_[next + i];
      return false;
    }
    for (uint8_t ch = 0; ch < 16; ++ch) {
      const uint8_t bit = ch * 11;
      const uint8_t index = 1 + bit / 8;
      uint32_t packed = bytes_[index] | (uint32_t(bytes_[index + 1]) << 8);
      if (index + 2 <= 22) packed |= uint32_t(bytes_[index + 2]) << 16;
      frame.channels[ch] = (packed >> (bit % 8)) & 0x07FF;
    }
    frame.lost = (bytes_[23] & 0x04) != 0;
    frame.failsafe = (bytes_[23] & 0x08) != 0;
    size_ = 0;
    return true;
  }
 private:
  uint8_t bytes_[25] = {};
  uint8_t size_ = 0;
};

struct RcCommands {
  float vertical = 0.0f;
  float yaw = 0.0f;
  float pitch = 0.0f;
  float roll = 0.0f;
  bool kill = true;
  bool land = false;
  bool connected = false;
};

class RcInput {
 public:
  static constexpr uint32_t TIMEOUT_MS = 200;
  void update(const SbusFrame &frame, uint32_t now) {
    expire(now);
    if (frame.failsafe) { invalidate(); return; }
    if (frame.lost) return; // Lost frames must not extend link life.
    for (uint8_t i = 0; i < 6; ++i) {
      // Also detects the default CH3 900us failsafe sentinel.
      // Disable transmitter mixing and excessive trims before use.
      if (frame.channels[i] < 120 || frame.channels[i] > 1900) {
        invalidate();
        return;
      }
    }
    healthy_ = true;
    lastGood_ = now;
    // Observe CH5 OFF after startup/loss before allowing CH5 ON.
    if (frame.channels[4] < 600) enableAllowed_ = true;
    commands_.connected = true;
    commands_.kill = !enableAllowed_ || frame.channels[4] <= 1400;
    commands_.land = frame.channels[5] > 1400;
    const float throttle = normalized(frame.channels[2]);
    commands_.vertical = throttle < 0 ? throttle * 0.8f : throttle;
    commands_.yaw = normalized(frame.channels[3]) * 6.283185307f;
    commands_.pitch = -normalized(frame.channels[1]) * 0.34906585f;
    commands_.roll = normalized(frame.channels[0]) * 0.34906585f;
  }
  RcCommands read(uint32_t now) {
    expire(now);
    return commands_;
  }
 private:
  static float normalized(uint16_t value) {
    // Manufacturer's SBUS endpoints: 200..1800, center 1000.
    float result = (float(value) - 1000.0f) / 800.0f;
    if (result > 1.0f) result = 1.0f;
    if (result < -1.0f) result = -1.0f;
    if (result > -0.04f && result < 0.04f) return 0.0f;
    return result;
  }
  void expire(uint32_t now) {
    if (healthy_ && uint32_t(now - lastGood_) >= TIMEOUT_MS) invalidate();
  }
  void invalidate() {
    healthy_ = false;
    enableAllowed_ = false;
    commands_ = RcCommands{};
  }
  RcCommands commands_;
  uint32_t lastGood_ = 0;
  bool healthy_ = false;
  bool enableAllowed_ = false;
};
#endif
