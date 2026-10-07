// Host regression tests: g++ -std=c++11 -Wall -Wextra test/sbus_test.cpp -o sbus_test
#include "../lib/main_rx/sbus.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static void pack(const SbusFrame &frame, uint8_t *bytes) {
  for (int i = 0; i < 25; ++i) bytes[i] = 0;
  bytes[0] = 0x0F;
  for (int ch = 0; ch < 16; ++ch) {
    for (int bit = 0; bit < 11; ++bit) {
      if (frame.channels[ch] & (1 << bit)) {
        int offset = ch * 11 + bit;
        bytes[1 + offset / 8] |= 1 << (offset % 8);
      }
    }
  }
  bytes[23] = (frame.lost ? 4 : 0) | (frame.failsafe ? 8 : 0);
}
static bool feed(SbusParser &parser, uint8_t *bytes, SbusFrame &out) {
  bool complete = false;
  for (int i = 0; i < 25; ++i) complete |= parser.push(bytes[i], out);
  return complete;
}
static SbusFrame centered() {
  SbusFrame frame = {};
  for (int i = 0; i < 16; ++i) frame.channels[i] = 1000;
  frame.channels[4] = frame.channels[5] = 200;
  return frame;
}
int main() {
  SbusParser parser;
  SbusFrame frame = {}, decoded;
  uint8_t bytes[25];
  // Includes embedded 0x0F payload bytes, all 11-bit boundaries and CH16.
  for (int n = 0; n < 2048; ++n) {
    for (int ch = 0; ch < 16; ++ch) frame.channels[ch] = (n + ch * 127) & 2047;
    frame.lost = n & 1; frame.failsafe = n & 2;
    pack(frame, bytes);
    assert(feed(parser, bytes, decoded));
    for (int ch = 0; ch < 16; ++ch) assert(frame.channels[ch] == decoded.channels[ch]);
    assert(decoded.lost == frame.lost && decoded.failsafe == frame.failsafe);
  }
  frame = centered(); pack(frame, bytes);
  assert(!parser.push(0xAA, decoded));
  bytes[24] = 0x55;
  assert(!feed(parser, bytes, decoded));
  bytes[24] = 0;
  bool recovered = false;
  for (int attempt = 0; attempt < 3; ++attempt) recovered |= feed(parser, bytes, decoded);
  assert(recovered);
  // A dropped byte must eventually resynchronize on a subsequent frame.
  SbusParser damaged;
  for (int i = 0; i < 25; ++i) if (i != 8) damaged.push(bytes[i], decoded);
  recovered = false;
  for (int attempt = 0; attempt < 3; ++attempt) recovered |= feed(damaged, bytes, decoded);
  assert(recovered);

  RcInput rc;
  assert(rc.read(0).kill && !rc.read(0).connected);
  frame.channels[4] = 1800;
  rc.update(frame, 10);
  assert(rc.read(10).kill); // Startup with CH5 ON never enables motors.
  frame.channels[4] = 200; rc.update(frame, 20);
  frame.channels[4] = 1800; rc.update(frame, 30);
  assert(!rc.read(30).kill && rc.read(30).connected);
  assert(rc.read(30).vertical == 0 && rc.read(30).pitch == 0);
  frame.channels[0] = 1800; frame.channels[1] = 1800;
  frame.channels[2] = 200; frame.channels[3] = 1800; frame.channels[5] = 1800;
  rc.update(frame, 40);
  RcCommands c = rc.read(40);
  assert(fabsf(c.vertical + 0.8f) < 0.001f);
  assert(fabsf(c.roll - 0.34906585f) < 0.0001f);
  assert(fabsf(c.pitch + 0.34906585f) < 0.0001f);
  assert(fabsf(c.yaw - 6.283185307f) < 0.0001f && c.land);
  frame.channels[2] = 1800; rc.update(frame, 50);
  assert(rc.read(50).vertical == 1.0f);
  frame.lost = true; rc.update(frame, 249);
  assert(rc.read(249).connected);
  assert(rc.read(250).kill && !rc.read(250).connected);
  frame.lost = false; rc.update(frame, 260);
  assert(rc.read(260).kill); // Reconnect with CH5 ON requires OFF again.
  frame.channels[4] = 200; rc.update(frame, 270);
  frame.channels[4] = 1800; rc.update(frame, 280);
  assert(!rc.read(280).kill);
  frame.failsafe = true; rc.update(frame, 281);
  assert(rc.read(281).kill && !rc.read(281).connected);
  frame.failsafe = false; frame.channels[2] = 40; rc.update(frame, 282);
  assert(rc.read(282).kill && !rc.read(282).connected);
  frame = centered(); frame.channels[0] = 2047; rc.update(frame, 283);
  assert(!rc.read(283).connected);
  // Timeout arithmetic remains correct across millis() wraparound.
  frame = centered(); rc.update(frame, 0xFFFFFFF0u);
  assert(rc.read(0x000000B7u).connected);
  assert(!rc.read(0x000000B8u).connected);
  puts("PASS: 2048 SBUS frames, resync, mapping, timeout, failsafe, re-enable, clock wrap");
}
