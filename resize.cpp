#pragma leco tool
#include <stdio.h>
#include <string.h>

import jojo;
import jute;

using namespace jute::literals;

static constexpr jute::view out = "../boosh/bullet.obj";

static void write_line(auto line) {
  jojo::append(out, line);
  jojo::append(out, "\n"_hs);
}

static void write_n(float n, char c) {
  char buf[256];
  snprintf(buf, 256, "%0.6f%c", n, c);
  jojo::append(out, jute::view { buf, strlen(buf) });
}

int main(int argc, char ** argv) {
  jojo::write(out, ""_hs);

  jojo::readlines("model.obj", [&](auto line) {
    if (line.size() < 2) return write_line(line);

    auto [cmd, vr] = line.split(' ');
    if (cmd != "v") return write_line(line);

    auto [x, xr] = vr.split(' ');
    auto [y, yr] = xr.split(' ');
    auto [z, zr] = yr.split(' ');

    jojo::append(out, "v "_hs);
    write_n(jute::to_f(x) * 10 + 0.05, ' ');
    write_n(jute::to_f(y) * 10 - 1.00, ' ');
    write_n(jute::to_f(z) * 10 + 0.05, '\n');
  });
}
