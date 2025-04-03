#pragma leco tool
#include <stdio.h>
#include <string.h>

import jojo;
import jute;

using namespace jute::literals;

static void write_line(auto line) {
  jojo::append("out/model.obj", line);
  jojo::append("out/model.obj", "\n"_hs);
}

static void write_n(jute::view f, char c) {
  float n = jute::to_f(f) * 10;

  char buf[256];
  snprintf(buf, 256, "%0.6f%c", n, c);
  jojo::append("out/model.obj", jute::view { buf, strlen(buf) });
}

int main(int argc, char ** argv) {
  jojo::write("out/model.obj", ""_hs);

  jojo::readlines("model.obj", [&](auto line) {
    if (line.size() < 2) return write_line(line);

    auto [cmd, vr] = line.split(' ');
    if (cmd != "v") return write_line(line);

    auto [x, xr] = vr.split(' ');
    auto [y, yr] = xr.split(' ');
    auto [z, zr] = yr.split(' ');

    jojo::append("out/model.obj", "v "_hs);
    write_n(x, ' ');
    write_n(y, ' ');
    write_n(z, '\n');
  });
}
